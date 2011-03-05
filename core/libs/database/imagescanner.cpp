/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-09-19
 * Description : Scanning of a single image
 *
 * Copyright (C) 2007-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C)      2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "imagescanner.h"

// Qt includes

#include <QImageReader>

// KDE includes

#include <kfilemetainfo.h>
#include <kmimetype.h>
#include <klocale.h>
#include <kdebug.h>

// Local includes

#include "databaseurl.h"
#include "databaseaccess.h"
#include "albumdb.h"
#include "collectionlocation.h"
#include "collectionmanager.h"
#include "imagecomments.h"
#include "imagecopyright.h"
#include "imageextendedproperties.h"
#include "imagehistorygraph.h"
#include "metadatasettings.h"
#include "tagscache.h"

namespace Digikam
{

ImageScanner::ImageScanner(const QFileInfo& info, const ItemScanInfo& scanInfo)
    : m_hasImage(false), m_hasMetadata(false),
      m_fileInfo(info), m_scanInfo(scanInfo), m_scanMode(ModifiedScan), m_hasHistoryToResolve(false)
{
}

ImageScanner::ImageScanner(const QFileInfo& info)
    : m_hasImage(false), m_hasMetadata(false),
      m_fileInfo(info), m_scanMode(ModifiedScan), m_hasHistoryToResolve(false)
{
}

ImageScanner::ImageScanner(qlonglong imageid)
    : m_hasImage(false), m_hasMetadata(false), m_scanMode(ModifiedScan), m_hasHistoryToResolve(false)
{
    ItemShortInfo shortInfo;
    {
        DatabaseAccess access;
        shortInfo  = access.db()->getItemShortInfo(imageid);
        m_scanInfo = access.db()->getItemScanInfo(imageid);
    }

    QString albumRootPath = CollectionManager::instance()->albumRootPath(shortInfo.albumRootID);
    m_fileInfo            = QFileInfo(DatabaseUrl::fromAlbumAndName(shortInfo.itemName,
                                      shortInfo.album, albumRootPath, shortInfo.albumRootID).fileUrl().toLocalFile());
}

qlonglong ImageScanner::id() const
{
    return m_scanInfo.id;
}

void ImageScanner::setCategory(DatabaseItem::Category category)
{
    // we don't have the necessary information in this class, but in CollectionScanner
    m_scanInfo.category = category;
}

void ImageScanner::fileModified()
{
    loadFromDisk();
    updateImage();
    scanFile(ModifiedScan);
}

void ImageScanner::newFile(int albumId)
{
    loadFromDisk();
    addImage(albumId);

    if (!scanFromIdenticalFile())
    {
        scanFile(NewScan);
    }
}

void ImageScanner::newFileFullScan(int albumId)
{
    loadFromDisk();
    addImage(albumId);
    scanFile(NewScan);
}

void ImageScanner::rescan()
{
    loadFromDisk();
    updateImage();
    scanFile(Rescan);
}

void ImageScanner::copiedFrom(int albumId, qlonglong srcId)
{
    loadFromDisk();
    addImage(albumId);

    // first use source, if it exists
    if (!copyFromSource(srcId))

        // check if we can establish identity
        if (!scanFromIdenticalFile())
            // scan newly
        {
            scanFile(NewScan);
        }
}

const ItemScanInfo& ImageScanner::itemScanInfo() const
{
    return m_scanInfo;
}

bool ImageScanner::hasHistoryToResolve() const
{
    return m_hasHistoryToResolve;
}

bool lessThanForIdentity(const ItemScanInfo& a, const ItemScanInfo& b)
{
    if (a.status != b.status)
    {
        // First: sort by status

        // put UndefinedStatus to back
        if (a.status == DatabaseItem::UndefinedStatus)
        {
            return false;
        }

        // enum values are in the order we want it
        return a.status < b.status;
    }
    else
    {
        // Second: sort by modification date, descending
        return a.modificationDate > b.modificationDate;
    }
}

bool ImageScanner::scanFromIdenticalFile()
{
    // Get a list of other images that are identical. Source image shall not be included.
    QList<ItemScanInfo> candidates = DatabaseAccess().db()->getIdenticalFiles(m_scanInfo.uniqueHash,
                                     m_scanInfo.fileSize, m_scanInfo.id);

    if (!candidates.isEmpty())
    {
        // Sort by priority, as implemented by custom lessThan()
        qStableSort(candidates.begin(), candidates.end(), lessThanForIdentity);

        kDebug() << "Recognized" << m_fileInfo.filePath() << "as identical to item" << candidates.first().id;

        // Copy attributes.
        // Todo for the future is to worry about syncing identical files.
        DatabaseAccess().db()->copyImageAttributes(candidates.first().id, m_scanInfo.id);
        return true;
    }

    return false;
}

bool ImageScanner::copyFromSource(qlonglong srcId)
{
    DatabaseAccess access;

    // some basic validity checking
    if (srcId == m_scanInfo.id)
    {
        return false;
    }

    ItemScanInfo info = access.db()->getItemScanInfo(srcId);

    if (!info.id)
    {
        return false;
    }

    kDebug() << "Recognized" << m_fileInfo.filePath() << "as copied from" << srcId;
    access.db()->copyImageAttributes(srcId, m_scanInfo.id);
    return true;
}

void ImageScanner::prepareImage()
{
    m_scanInfo.itemName         = m_fileInfo.fileName();
    m_scanInfo.modificationDate = m_fileInfo.lastModified();
    // there is a limit here for file size <2TB
    m_scanInfo.fileSize         = (int)m_fileInfo.size();
    // category is set by setCategory

    // the QByteArray is an ASCII hex string
    m_scanInfo.uniqueHash       = uniqueHash();
}

void ImageScanner::addImage(int albumId)
{
    prepareImage();

    m_scanInfo.albumID          = albumId;
    m_scanInfo.status           = DatabaseItem::Visible;

    kDebug() << "Adding new item" << m_fileInfo.filePath();
    m_scanInfo.id               = DatabaseAccess().db()->addItem(m_scanInfo.albumID, m_scanInfo.itemName,
                                  m_scanInfo.status, m_scanInfo.category,
                                  m_scanInfo.modificationDate, m_scanInfo.fileSize,
                                  m_scanInfo.uniqueHash);
}

void ImageScanner::updateImage()
{
    prepareImage();

    DatabaseAccess().db()->updateItem(m_scanInfo.id, m_scanInfo.category,
                                      m_scanInfo.modificationDate, m_scanInfo.fileSize, m_scanInfo.uniqueHash);
}

void ImageScanner::scanFile(ScanMode mode)
{
    m_scanMode = mode;

    if (m_scanMode == ModifiedScan)
    {
        if (m_scanInfo.category == DatabaseItem::Image)
        {
            scanImageInformation();
            scanImageHistoryIfModified();
        }
    }
    else
    {
        if (m_scanInfo.category == DatabaseItem::Image)
        {
            scanImageInformation();

            if (m_hasMetadata)
            {
                scanImageMetadata();
                scanImagePosition();
                scanImageComments();
                scanImageCopyright();
                scanIPTCCore();
                scanTags();
                scanImageHistory();
            }
        }
        else if (m_scanInfo.category == DatabaseItem::Video)
        {
            scanVideoFile();
        }
        else if (m_scanInfo.category == DatabaseItem::Audio)
        {
            scanAudioFile();
        }
        else if (m_scanInfo.category == DatabaseItem::Other)
        {
            // unsupported
        }
    }
}

void ImageScanner::scanImageInformation()
{
    DatabaseFields::ImageInformation dbFields = DatabaseFields::ImageInformationAll;
    QVariantList infos;

    if (m_scanMode == NewScan || m_scanMode == Rescan)
    {
        MetadataFields fields;
        fields << MetadataInfo::Rating
               << MetadataInfo::CreationDate
               << MetadataInfo::DigitizationDate
               << MetadataInfo::Orientation;
        QVariantList metadataInfos = m_metadata.getMetadataFields(fields);

        // creation date: fall back to file system property
        if (metadataInfos[1].isNull() || !metadataInfos[1].toDateTime().isValid())
        {
            metadataInfos[1] = creationDateFromFilesystem(m_fileInfo);
        }

        // Some fields should only be overwritten if set in metadata
        if (m_scanMode == Rescan)
        {
            if (metadataInfos[0].isNull() || metadataInfos[0].toInt() == -1)
            {
                dbFields &= ~DatabaseFields::Rating;
                metadataInfos.removeAt(0);
            }
        }

        infos << metadataInfos;
    }

    QSize size = m_img.size();
    infos << size.width()
          << size.height()
          << detectFormat()
          << m_img.originalBitDepth()
          << m_img.originalColorModel();

    if (m_scanMode == NewScan)
    {
        DatabaseAccess().db()->addImageInformation(m_scanInfo.id, infos, dbFields);
    }
    else if (m_scanMode == Rescan)
    {
        DatabaseAccess().db()->changeImageInformation(m_scanInfo.id, infos, dbFields);
    }
    else // ModifiedScan
    {
        // Does _not_ update rating and orientation!
        DatabaseAccess().db()->changeImageInformation(m_scanInfo.id, infos,
                DatabaseFields::Width      |
                DatabaseFields::Height     |
                DatabaseFields::Format     |
                DatabaseFields::ColorDepth |
                DatabaseFields::ColorModel);
    }
}

static bool hasValidField(const QVariantList& list)
{
    for (QVariantList::const_iterator it = list.constBegin();
         it != list.constEnd(); ++it)
    {
        if (!(*it).isNull())
        {
            return true;
        }
    }

    return false;
}

static MetadataFields allImageMetadataFields()
{
    // This list must reflect the order required by AlbumDB::addImageMetadata
    MetadataFields fields;
    fields << MetadataInfo::Make
           << MetadataInfo::Model
           << MetadataInfo::Lens
           << MetadataInfo::Aperture
           << MetadataInfo::FocalLength
           << MetadataInfo::FocalLengthIn35mm
           << MetadataInfo::ExposureTime
           << MetadataInfo::ExposureProgram
           << MetadataInfo::ExposureMode
           << MetadataInfo::Sensitivity
           << MetadataInfo::FlashMode
           << MetadataInfo::WhiteBalance
           << MetadataInfo::WhiteBalanceColorTemperature
           << MetadataInfo::MeteringMode
           << MetadataInfo::SubjectDistance
           << MetadataInfo::SubjectDistanceCategory;
    return fields;
}

void ImageScanner::scanImageMetadata()
{
    QVariantList metadataInfos = m_metadata.getMetadataFields(allImageMetadataFields());

    if (hasValidField(metadataInfos))
    {
        DatabaseAccess().db()->addImageMetadata(m_scanInfo.id, metadataInfos);
    }
}

void ImageScanner::scanImagePosition()
{
    // This list must reflect the order required by AlbumDB::addImagePosition
    MetadataFields fields;
    fields << MetadataInfo::Latitude
           << MetadataInfo::LatitudeNumber
           << MetadataInfo::Longitude
           << MetadataInfo::LongitudeNumber
           << MetadataInfo::Altitude
           << MetadataInfo::PositionOrientation
           << MetadataInfo::PositionTilt
           << MetadataInfo::PositionRoll
           << MetadataInfo::PositionAccuracy
           << MetadataInfo::PositionDescription;

    QVariantList metadataInfos = m_metadata.getMetadataFields(fields);

    if (hasValidField(metadataInfos))
    {
        DatabaseAccess().db()->addImagePosition(m_scanInfo.id, metadataInfos);
    }
}

void ImageScanner::scanImageComments()
{
    MetadataFields fields;
    fields << MetadataInfo::Headline
           << MetadataInfo::Title;

    QVariantList metadataInfos = m_metadata.getMetadataFields(fields);

    // handles all possible fields, multi-language, author, date
    CaptionsMap captions = m_metadata.getImageComments();

    if (captions.isEmpty() && !hasValidField(metadataInfos))
    {
        return;
    }

    DatabaseAccess access;
    ImageComments comments(access, m_scanInfo.id);

    // Description
    if (!captions.isEmpty())
    {
        comments.replaceComments(captions);
    }

    // Headline
    if (!metadataInfos[0].isNull())
    {
        comments.addHeadline(metadataInfos[0].toString());
    }

    // Title
    if (!metadataInfos[1].isNull())
    {
        comments.addTitle(metadataInfos[1].toString());
    }
}

void ImageScanner::scanImageCopyright()
{
    Template t;

    if (!m_metadata.getCopyrightInformation(t))
    {
        return;
    }

    ImageCopyright copyright(m_scanInfo.id);
    // It is not clear if removeAll() should be called if m_scanMode == Rescan
    copyright.removeAll();
    copyright.setFromTemplate(t);
}

void ImageScanner::scanIPTCCore()
{
    MetadataFields fields;
    fields << MetadataInfo::IptcCoreLocationInfo
           << MetadataInfo::IptcCoreIntellectualGenre
           << MetadataInfo::IptcCoreJobID
           << MetadataInfo::IptcCoreScene
           << MetadataInfo::IptcCoreSubjectCode;

    QVariantList metadataInfos = m_metadata.getMetadataFields(fields);

    if (!hasValidField(metadataInfos))
    {
        return;
    }

    ImageExtendedProperties props(m_scanInfo.id);

    if (!metadataInfos[0].isNull())
    {
        IptcCoreLocationInfo loc = metadataInfos[0].value<IptcCoreLocationInfo>();

        if (!loc.isNull())
        {
            props.setLocation(loc);
        }
    }

    if (!metadataInfos[1].isNull())
    {
        props.setIntellectualGenre(metadataInfos[1].toString());
    }

    if (!metadataInfos[2].isNull())
    {
        props.setJobId(metadataInfos[2].toString());
    }

    if (!metadataInfos[3].isNull())
    {
        props.setScene(metadataInfos[3].toStringList());
    }

    if (!metadataInfos[4].isNull())
    {
        props.setSubjectCode(metadataInfos[4].toStringList());
    }
}

void ImageScanner::scanTags()
{
    // Check Keywords tag paths.

    QVariant var         = m_metadata.getMetadataField(MetadataInfo::Keywords);
    QStringList keywords = var.toStringList();

    if (!keywords.isEmpty())
    {
        // get tag ids, create if necessary
        QList<int> tagIds = TagsCache::instance()->getOrCreateTags(keywords);
        DatabaseAccess().db()->addTagsToItems(QList<qlonglong>() << m_scanInfo.id, tagIds);
    }

    // Check Pick Label tag.

    int pickId = m_metadata.getImagePickLabel();
    if (pickId != -1)
    {
        kDebug() << "Pick Label found : " << pickId;

        int tagId = TagsCache::instance()->getTagForPickLabel((PickLabel)pickId);
        if (tagId)
        {
            DatabaseAccess().db()->addTagsToItems(QList<qlonglong>() << m_scanInfo.id, QList<int>() << tagId);
            kDebug() << "Assigned Pick Label Tag  : " << tagId;
        }
        else
        {
            kDebug() << "Cannot find Pick Label Tag for : " << pickId;
        }
    }

    // Check Color Label tag.

    int colorId = m_metadata.getImageColorLabel();
    if (colorId != -1)
    {
        kDebug() << "Color Label found : " << colorId;

        int tagId = TagsCache::instance()->getTagForColorLabel((ColorLabel)colorId);
        if (tagId)
        {
            DatabaseAccess().db()->addTagsToItems(QList<qlonglong>() << m_scanInfo.id, QList<int>() << tagId);
            kDebug() << "Assigned Color Label Tag  : " << tagId;
        }
        else
        {
            kDebug() << "Cannot find Color Label Tag for : " << colorId;
        }
    }
}

void ImageScanner::scanImageHistory()
{
    /** Stage 1 of history scanning */

    QString historyXml = m_metadata.getImageHistory();

    if (!historyXml.isEmpty())
    {
        DatabaseAccess().db()->setImageHistory(m_scanInfo.id, historyXml);
        // Delay history resolution by setting this tag:
        // Resolution depends on the presence of other images, possibly only when the scanning process has finished
        DatabaseAccess().db()->addItemTag(m_scanInfo.id, TagsCache::instance()->
                                          getOrCreateInternalTag(InternalTagName::needResolvingHistory()));
        m_hasHistoryToResolve = true;
    }

    QString uuid = m_metadata.getImageUniqueId();

    if (!uuid.isNull())
    {
        DatabaseAccess().db()->setImageUuid(m_scanInfo.id, uuid);
    }
}

void ImageScanner::scanImageHistoryIfModified()
{
    // If a file has a modified history, it must have a new UUID
    QString previousUuid = DatabaseAccess().db()->getImageUuid(m_scanInfo.id);
    QString currentUuid  = m_metadata.getImageUniqueId();

    if (previousUuid != currentUuid)
    {
        scanImageHistory();
    }
}

bool ImageScanner::resolveImageHistory(qlonglong id, QList<qlonglong>* needTaggingIds)
{
    ImageHistoryEntry history = DatabaseAccess().db()->getImageHistory(id);
    return resolveImageHistory(id, history.history, needTaggingIds);
}

bool ImageScanner::resolveImageHistory(qlonglong imageId, const QString& historyXml,
                                       QList<qlonglong>* needTaggingIds)
{
    /** Stage 2 of history scanning */

    if (historyXml.isNull())
    {
        return true;    // "true" means nothing is left to resolve
    }

    DImageHistory history = DImageHistory::fromXml(historyXml);

    if (history.isNull())
    {
        return true;
    }

    ImageHistoryGraph graph;
    graph.addScannedHistory(history, imageId);

    if (!graph.hasEdges())
    {
        return true;
    }

    QPair<QList<qlonglong>, QList<qlonglong> > cloud = graph.relationCloudParallel();
    DatabaseAccess().db()->addImageRelations(cloud.first, cloud.second, DatabaseRelation::DerivedFrom);

    int needResolvingTag = TagsCache::instance()->getOrCreateInternalTag(InternalTagName::needResolvingHistory());
    int needTaggingTag   = TagsCache::instance()->getOrCreateInternalTag(InternalTagName::needTaggingHistoryGraph());

    // remove the needResolvingHistory tag from all images in graph
    DatabaseAccess().db()->removeTagsFromItems(graph.allImageIds(), QList<int>() << needResolvingTag);

    // mark a single image from the graph (sufficient for find the full relation cloud)
    QList<ImageInfo> roots = graph.rootImages();

    if (!roots.isEmpty())
    {
        DatabaseAccess().db()->addItemTag(roots.first().id(), needTaggingTag);

        if (needTaggingIds)
        {
            *needTaggingIds << roots.first().id();
        }
    }

    return !graph.hasUnresolvedEntries();
}

void ImageScanner::tagImageHistoryGraph(qlonglong id)
{
    /** Stage 3 of history scanning */

    ImageInfo info(id);

    if (info.isNull())
    {
        return;
    }
    //kDebug() << "tagImageHistoryGraph" << id;

    // Load relation cloud, history of info and of all leaves of the tree into the graph, fully resolved
    ImageHistoryGraph graph = ImageHistoryGraph::fromInfo(info, ImageHistoryGraph::LoadAll, ImageHistoryGraph::NoProcessing);
    kDebug() << graph;

    int originalVersionTag     = TagsCache::instance()->getOrCreateInternalTag(InternalTagName::originalVersion());
    int currentVersionTag      = TagsCache::instance()->getOrCreateInternalTag(InternalTagName::currentVersion());
    int intermediateVersionTag = TagsCache::instance()->getOrCreateInternalTag(InternalTagName::intermediateVersion());

    int needTaggingTag         = TagsCache::instance()->getOrCreateInternalTag(InternalTagName::needTaggingHistoryGraph());

    // Remove all relevant tags
    DatabaseAccess().db()->removeTagsFromItems(graph.allImageIds(), QList<int>() << originalVersionTag
            << currentVersionTag << intermediateVersionTag << needTaggingTag);

    if (!graph.hasEdges())
    {
        return;
    }

    // get category info
    QList<qlonglong> originals, intermediates, currents;
    QHash<ImageInfo, HistoryImageId::Types> types = graph.categorize();
    QHash<ImageInfo, HistoryImageId::Types>::iterator it;

    for (it = types.begin(); it != types.end(); ++it)
    {
        kDebug() << "Image" << it.key().id() << "type" << it.value();
        HistoryImageId::Types types = it.value();

        if (types & HistoryImageId::Original)
        {
            originals << it.key().id();
        }
        if (types & HistoryImageId::Intermediate)
        {
            intermediates << it.key().id();
        }
        if (types & HistoryImageId::Current)
        {
            currents << it.key().id();
        }
    }

    if (!originals.isEmpty())
    {
        DatabaseAccess().db()->addTagsToItems(originals, QList<int>() << originalVersionTag);
    }

    if (!intermediates.isEmpty())
    {
        DatabaseAccess().db()->addTagsToItems(intermediates, QList<int>() << intermediateVersionTag);
    }

    if (!currents.isEmpty())
    {
        DatabaseAccess().db()->addTagsToItems(currents, QList<int>() << currentVersionTag);
    }
}

DImageHistory ImageScanner::resolvedImageHistory(const DImageHistory& history, bool mustBeAvailable)
{
    DImageHistory h;
    foreach (const DImageHistory::Entry& e, history.entries())
    {
        // Copy entry, without referredImages
        DImageHistory::Entry entry;
        entry.action = e.action;

        // resolve referredImages
        foreach (const HistoryImageId& id, e.referredImages)
        {
            QList<qlonglong> imageIds = resolveHistoryImageId(id);

            // append each image found in collection to referredImages
            foreach (qlonglong imageId, imageIds)
            {
                ImageInfo info(imageId);

                if (info.isNull())
                {
                    continue;
                }

                if (mustBeAvailable)
                {
                    CollectionLocation location = CollectionManager::instance()->locationForAlbumRootId(info.albumRootId());

                    if (!location.isAvailable())
                    {
                        continue;
                    }
                }

                HistoryImageId newId = info.historyImageId();
                newId.setType(id.m_type);
                entry.referredImages << newId;
            }
        }

        // add to history
        h.entries() << entry;
    }

    return h;
}

bool ImageScanner::sameReferredImage(const HistoryImageId& id1, const HistoryImageId& id2)
{
    if (!id1.isValid() || !id2.isValid())
    {
        return false;
    }

    /*
     * We give the UUID the power of equivalence that none of the other criteria has:
     * For two images a,b with uuids x,y, where x and y not null,
     *  a (same image as) b   <=>   x == y
     */
    if (id1.hasUuid() && id2.hasUuid())
    {
        return id1.m_uuid == id2.m_uuid;
    }

    if (id1.hasUniqueHashIdentifier()
        && id1.m_uniqueHash == id2.m_uniqueHash
        && id1.m_fileSize == id2.m_fileSize)
    {
        return true;
    }

    if (id1.hasFileName() && id1.hasCreationDate()
        && id1.m_fileName == id2.m_fileName
        && id1.m_creationDate == id2.m_creationDate)
    {
        return true;
    }

    if (id1.hasFileOnDisk()
        && id1.m_filePath == id2.m_filePath
        && id1.m_fileName == id2.m_fileName)
    {
        return true;
    }

    return false;
}

QList<qlonglong> ImageScanner::resolveHistoryImageId(const HistoryImageId& historyId)
{
    // first and foremost: UUID
    if (historyId.hasUuid())
    {
        QList<qlonglong> uuidList = DatabaseAccess().db()->getItemsForUuid(historyId.m_uuid);

        if (!uuidList.isEmpty())
        {
            return uuidList;
        }
    }

    // Second: uniqueHash + fileSize. Sufficient to assume that a file is identical, but subject to frequent change.
    if (historyId.hasUniqueHashIdentifier() && DatabaseAccess().db()->isUniqueHashV2())
    {
        QList<ItemScanInfo> infos = DatabaseAccess().db()->getIdenticalFiles(historyId.m_uniqueHash, historyId.m_fileSize);

        if (!infos.isEmpty())
        {
            QList<qlonglong> ids;
            foreach (const ItemScanInfo& info, infos)
            {
                if (info.status != DatabaseItem::Removed)
                {
                    ids << info.id;
                }
            }
            return ids;
        }
    }

    // As a third combination, we try file name and creation date. Susceptible to renaming,
    // but not to metadata changes.
    if (historyId.hasFileName() && historyId.hasCreationDate())
    {
        QList<qlonglong> ids = DatabaseAccess().db()->findByNameAndCreationDate(historyId.m_fileName, historyId.m_creationDate);

        if (!ids.isEmpty())
        {
            return ids;
        }
    }

    // Another possibility: If the original UUID is given, we can find all relations for the image with this UUID,
    // and make an assumption from this group of images. Currently not implemented.

    // resolve old-style by full file path
    if (historyId.hasFileOnDisk())
    {
        QFileInfo file(historyId.filePath());

        if (file.exists())
        {
            CollectionLocation location = CollectionManager::instance()->locationForPath(historyId.path());

            if (!location.isNull())
            {
                QString album = CollectionManager::instance()->album(file.path());
                QString name  = file.fileName();
                ItemShortInfo info = DatabaseAccess().db()->getItemShortInfo(location.id(), album, name);

                if (info.id)
                {
                    return QList<qlonglong>() << info.id;
                }
            }
        }
    }

    return QList<qlonglong>();
}

class lessThanByProximityToSubject
{
public:

    lessThanByProximityToSubject(const ImageInfo& subject) : subject(subject) {}

    bool operator()(const ImageInfo& a, const ImageInfo& b)
    {
        if (a.isNull() || b.isNull())
        {
            // both null: false
            // only a null: a greater than b (null infos at end of list)
            //  (a && b) || (a && !b) = a
            // only b null: a less than b
            if (a.isNull())
            {
                return false;
            }

            return true;
        }

        if (a == b)
        {
            return false;
        }

        // same collection
        if (a.albumId() != b.albumId())
        {
            // same album
            if (a.albumId() == subject.albumId())
            {
                return true;
            }

            if (b.albumId() == subject.albumId())
            {
                return false;
            }

            if (a.albumRootId() != b.albumRootId())
            {
                // different collection
                if (a.albumRootId() == subject.albumRootId())
                {
                    return true;
                }

                if (b.albumRootId() == subject.albumRootId())
                {
                    return false;
                }
            }
        }

        if (a.modDateTime() != b.modDateTime())
        {
            return a.modDateTime() < b.modDateTime();
        }

        if (a.name() != b.name())
        {
            return qAbs(a.name().compare(subject.name())) < qAbs(b.name().compare(subject.name()));
        }

        // last resort
        return a.id() < b.id();
    }

public:

    ImageInfo subject;
};

void ImageScanner::sortByProximity(QList<ImageInfo>& list, const ImageInfo& subject)
{
    if (!list.isEmpty() && !subject.isNull())
    {
        qStableSort(list.begin(), list.end(), lessThanByProximityToSubject(subject));
    }
}

void ImageScanner::scanVideoFile()
{
    /**
    * @todo
    */

    QVariantList metadataInfos;

    if (m_hasMetadata)
    {
        MetadataFields fields;
        fields << MetadataInfo::Rating
               << MetadataInfo::CreationDate;
        metadataInfos = m_metadata.getMetadataFields(fields);

        // if invalid, start with -1 rating
        if (metadataInfos[0].isNull())
        {
            metadataInfos[0] = -1;
        }

        // creation date: fall back to file system property
        if (metadataInfos[1].isNull() || !metadataInfos[1].toDateTime().isValid())
        {
            metadataInfos[1] = creationDateFromFilesystem(m_fileInfo);
        }
    }
    else
    {
        metadataInfos << -1
                      << creationDateFromFilesystem(m_fileInfo);
    }

    QSize size;

    QVariantList infos;
    infos << metadataInfos
          << detectVideoFormat();

    DatabaseAccess().db()->addImageInformation(m_scanInfo.id, infos,
            DatabaseFields::Rating       |
            DatabaseFields::CreationDate |
            DatabaseFields::Format);

    // KFileMetaInfo does not give us any useful information for relevant video files
    /*
    const KFileMetaInfo::WhatFlags flags = KFileMetaInfo::Fastest |
            KFileMetaInfo::TechnicalInfo |
            KFileMetaInfo::ContentInfo;
    KFileMetaInfo metaInfo(m_fileInfo.filePath(), QString(), flags);
    if (metaInfo.isValid())
    {
        QStringList keys = metaInfo.keys();
        foreach (const QString& key, keys)
        {
            KFileMetaInfoItem item = metaInfo.item(key);
            kDebug() << item.name() << item.value();
        }
    }
    */
}

void ImageScanner::scanAudioFile()
{
    /**
    * @todo
    */

    QVariantList infos;
    infos << -1
          << creationDateFromFilesystem(m_fileInfo)
          << detectAudioFormat();

    DatabaseAccess().db()->addImageInformation(m_scanInfo.id, infos,
            DatabaseFields::Rating       |
            DatabaseFields::CreationDate |
            DatabaseFields::Format);
}

void ImageScanner::copyProperties(qlonglong source, qlonglong dest)
{
    kDebug() << "Copying properties from" << source << "to" << dest;
    DatabaseAccess access;

    DatabaseFields::ImageInformation imageInfoFields =
        DatabaseFields::Rating |
        DatabaseFields::CreationDate |
        DatabaseFields::DigitizationDate;
    QVariantList imageInfos = access.db()->getImageInformation(source, imageInfoFields);

    if (!imageInfos.isEmpty())
    {
        access.db()->changeImageInformation(dest, imageInfos, imageInfoFields);
    }

    QVariantList positionData = access.db()->getImagePosition(source, DatabaseFields::ImagePositionsAll);

    if (!positionData.isEmpty())
    {
        access.db()->addImagePosition(dest, positionData, DatabaseFields::ImagePositionsAll);
    }

    ImageComments commentsSource(access, source);
    ImageComments commentsDest(access, dest);
    commentsDest.replaceFrom(commentsSource);
    commentsDest.apply(access);

    ImageCopyright copyrightSource(source);
    ImageCopyright copyrightDest(dest);
    copyrightDest.replaceFrom(source);

    access.db()->copyImageTags(source, dest);
    access.db()->copyImageProperties(source, dest);
}

void ImageScanner::loadFromDisk()
{
    MetadataSettings* const mSettings = MetadataSettings::instance();

    if (mSettings)
    {
        MetadataSettingsContainer set = mSettings->settings();

        m_metadata.setUseXMPSidecar4Reading(set.useXMPSidecar4Reading);
    }

    m_hasMetadata = m_metadata.load(m_fileInfo.filePath());

    if (m_scanInfo.category == DatabaseItem::Image)
    {
        m_hasImage = m_img.loadImageInfo(m_fileInfo.filePath(), false, false, false, false);
    }
    else
    {
        m_hasImage = false;
    }

    // faster than loading twice from disk
    if (m_hasMetadata)
    {
        m_img.setMetadata(m_metadata.data());
    }
}

QString ImageScanner::uniqueHash()
{
    // the QByteArray is an ASCII hex string
    if (m_scanInfo.category == DatabaseItem::Image)
    {
        if (DatabaseAccess().db()->isUniqueHashV2())
            return QString(m_img.getUniqueHashV2());
        else
            return QString(m_img.getUniqueHash());
    }
    else
    {
        if (DatabaseAccess().db()->isUniqueHashV2())
            return QString(DImg::getUniqueHashV2(m_fileInfo.filePath()));
        else
            return QString(DImg::getUniqueHash(m_fileInfo.filePath()));
    }
}

QString ImageScanner::detectFormat()
{
    DImg::FORMAT dimgFormat = m_img.detectedFormat();

    switch (dimgFormat)
    {
        case DImg::JPEG:
            return "JPG";
        case DImg::PNG:
            return "PNG";
        case DImg::TIFF:
            return "TIFF";
        case DImg::PPM:
            return "PPM";
        case DImg::JP2K:
            return "JP2k";
        case DImg::PGF:
            return "PGF";
        case DImg::RAW:
        {
            QString format = "RAW-";
            format += m_fileInfo.suffix().toUpper();
            return format;
        }
        case DImg::NONE:
        case DImg::QIMAGE:
        {
            QByteArray format = QImageReader::imageFormat(m_fileInfo.filePath());

            if (!format.isEmpty())
            {
                return QString(format).toUpper();
            }

            KMimeType::Ptr mimetype = KMimeType::findByPath(m_fileInfo.filePath());

            if (mimetype)
            {
                QString name = mimetype->name();

                if (name.startsWith(QLatin1String("image/")))
                {
                    QString imageTypeName = name.mid(6).toUpper();

                    // cut off the "X-" from some mimetypes
                    if (imageTypeName.startsWith(QLatin1String("X-")))
                    {
                        imageTypeName = imageTypeName.mid(2);
                    }

                    return imageTypeName;
                }
            }

            kWarning() << "Detecting file format failed: KMimeType for" << m_fileInfo.filePath()
                       << "is null";

        }
    }

    return QString();
}

QString ImageScanner::detectVideoFormat()
{
    QString suffix = m_fileInfo.suffix().toUpper();

    if (suffix == "MPEG" || suffix == "MPG" || suffix == "MPO" || suffix == "MPE")
    {
        return "MPEG";
    }

    if (suffix =="ASF" || suffix == "WMV")
    {
        return "WMV";
    }

    return suffix;
}

QString ImageScanner::detectAudioFormat()
{
    QString suffix = m_fileInfo.suffix().toUpper();
    return suffix;
}

QDateTime ImageScanner::creationDateFromFilesystem(const QFileInfo& info)
{
    // creation date is not what it seems on Unix
    QDateTime ctime = info.created();
    QDateTime mtime = info.lastModified();

    if (ctime.isNull())
    {
        return mtime;
    }

    if (mtime.isNull())
    {
        return ctime;
    }

    return qMin(ctime, mtime);
}

QString ImageScanner::formatToString(const QString& format)
{
    if (format == "JPG")
    {
        return "JPEG";
    }
    else if (format == "PNG")
    {
        return format;
    }
    else if (format == "TIFF")
    {
        return format;
    }
    else if (format == "PPM")
    {
        return format;
    }
    else if (format == "JP2K")
    {
        return "JPEG 2000";
    }
    else if (format.startsWith(QLatin1String("RAW-")))
    {
        return i18nc("RAW image file (), the parentheses contain the file suffix, like MRW",
                     "RAW image file (%1)",
                     format.mid(4));
    }
    // video
    else if (format == "MPEG")
    {
        return format;
    }
    else if (format == "AVI")
    {
        return format;
    }
    else if (format == "MOV")
    {
        return "Quicktime";
    }
    else if (format == "WMF")
    {
        return "Windows MetaFile";
    }
    else if (format == "WMV")
    {
        return "Windows Media Video";
    }
    else if (format == "MP4")
    {
        return "MPEG-4";
    }
    else if (format == "3GP")
    {
        return "3GPP";
    }
    // audio
    else if (format == "OGG")
    {
        return "Ogg";
    }
    else if (format == "MP3")
    {
        return format;
    }
    else if (format == "WMA")
    {
        return "Windows Media Audio";
    }
    else if (format == "WAV")
    {
        return "WAVE";
    }
    else
    {
        return format;
    }
}

QString ImageScanner::colorModelToString(DImg::COLORMODEL colorModel)
{
    switch (colorModel)
    {
        case DImg::RGB:
            return i18nc("Color Model: RGB", "RGB");
        case DImg::GRAYSCALE:
            return i18nc("Color Model: Grayscale", "Grayscale");
        case DImg::MONOCHROME:
            return i18nc("Color Model: Monochrome", "Monochrome");
        case DImg::INDEXED:
            return i18nc("Color Model: Indexed", "Indexed");
        case DImg::YCBCR:
            return i18nc("Color Model: YCbCr", "YCbCr");
        case DImg::CMYK:
            return i18nc("Color Model: CMYK", "CMYK");
        case DImg::CIELAB:
            return i18nc("Color Model: CIE L*a*b*", "CIE L*a*b*");
        case DImg::COLORMODELRAW:
            return i18nc("Color Model: Uncalibrated (RAW)", "Uncalibrated (RAW)");
        case DImg::COLORMODELUNKNOWN:
        default:
            return i18nc("Color Model: Unknown", "Unknown");
    }
}

QString ImageScanner::iptcCorePropertyName(MetadataInfo::Field field)
{
    // These strings are specified in DBSCHEMA.ods
    switch (field)
    {
            // copyright table
        case MetadataInfo::IptcCoreCopyrightNotice:
            return "copyrightNotice";
        case MetadataInfo::IptcCoreCreator:
            return "creator";
        case MetadataInfo::IptcCoreProvider:
            return "provider";
        case MetadataInfo::IptcCoreRightsUsageTerms:
            return "rightsUsageTerms";
        case MetadataInfo::IptcCoreSource:
            return "source";
        case MetadataInfo::IptcCoreCreatorJobTitle:
            return "creatorJobTitle";
        case MetadataInfo::IptcCoreInstructions:
            return "instructions";

            // ImageProperties table
        case MetadataInfo::IptcCoreCountryCode:
            return "countryCode";
        case MetadataInfo::IptcCoreCountry:
            return "country";
        case MetadataInfo::IptcCoreCity:
            return "city";
        case MetadataInfo::IptcCoreLocation:
            return "location";
        case MetadataInfo::IptcCoreProvinceState:
            return "provinceState";
        case MetadataInfo::IptcCoreIntellectualGenre:
            return "intellectualGenre";
        case MetadataInfo::IptcCoreJobID:
            return "jobId";
        case MetadataInfo::IptcCoreScene:
            return "scene";
        case MetadataInfo::IptcCoreSubjectCode:
            return "subjectCode";
        case MetadataInfo::IptcCoreContactInfoCity:
            return "creatorContactInfo.city";
        case MetadataInfo::IptcCoreContactInfoCountry:
            return "creatorContactInfo.country";
        case MetadataInfo::IptcCoreContactInfoAddress:
            return "creatorContactInfo.address";
        case MetadataInfo::IptcCoreContactInfoPostalCode:
            return "creatorContactInfo.postalCode";
        case MetadataInfo::IptcCoreContactInfoProvinceState:
            return "creatorContactInfo.provinceState";
        case MetadataInfo::IptcCoreContactInfoEmail:
            return "creatorContactInfo.email";
        case MetadataInfo::IptcCoreContactInfoPhone:
            return "creatorContactInfo.phone";
        case MetadataInfo::IptcCoreContactInfoWebUrl:
            return "creatorContactInfo.webUrl";
        default:
            return QString();
    }
}

void ImageScanner::fillCommonContainer(qlonglong imageid, ImageCommonContainer* container)
{
    QVariantList imagesFields;
    QVariantList imageInformationFields;

    {
        DatabaseAccess access;
        imagesFields = access.db()->getImagesFields(imageid,
                       DatabaseFields::Name             |
                       DatabaseFields::ModificationDate |
                       DatabaseFields::FileSize);

        imageInformationFields = access.db()->getImageInformation(imageid,
                                 DatabaseFields::Rating           |
                                 DatabaseFields::CreationDate     |
                                 DatabaseFields::DigitizationDate |
                                 DatabaseFields::Orientation      |
                                 DatabaseFields::Width            |
                                 DatabaseFields::Height           |
                                 DatabaseFields::Format           |
                                 DatabaseFields::ColorDepth       |
                                 DatabaseFields::ColorModel);
    }

    if (!imagesFields.isEmpty())
    {
        container->fileName             = imagesFields[0].toString();
        container->fileModificationDate = imagesFields[1].toDateTime();
        container->fileSize             = imagesFields[2].toInt();
    }

    if (!imageInformationFields.isEmpty())
    {
        container->rating           = imageInformationFields[0].toInt();
        container->creationDate     = imageInformationFields[1].toDateTime();
        container->digitizationDate = imageInformationFields[2].toDateTime();
        container->orientation      = DMetadata::valueToString(imageInformationFields[3], MetadataInfo::Orientation);
        container->width            = imageInformationFields[4].toInt();
        container->height           = imageInformationFields[5].toInt();
        container->format           = formatToString(imageInformationFields[6].toString());
        container->colorDepth       = imageInformationFields[7].toInt();
        container->colorModel       = colorModelToString((DImg::COLORMODEL)imageInformationFields[8].toInt());
    }
}

void ImageScanner::fillMetadataContainer(qlonglong imageid, ImageMetadataContainer* container)
{
    // read from database
    QVariantList fields = DatabaseAccess().db()->getImageMetadata(imageid);

    // check we have at least one valid field
    container->allFieldsNull = !hasValidField(fields);

    if (container->allFieldsNull)
    {
        return;
    }

    // DMetadata does all translation work
    QStringList strings = DMetadata::valuesToString(fields, allImageMetadataFields());

    // associate with hard-coded variables
    container->make                         = strings[0];
    container->model                        = strings[1];
    container->lens                         = strings[2];
    container->aperture                     = strings[3];
    container->focalLength                  = strings[4];
    container->focalLength35                = strings[5];
    container->exposureTime                 = strings[6];
    container->exposureProgram              = strings[7];
    container->exposureMode                 = strings[8];
    container->sensitivity                  = strings[9];
    container->flashMode                    = strings[10];
    container->whiteBalance                 = strings[11];
    container->whiteBalanceColorTemperature = strings[12];
    container->meteringMode                 = strings[13];
    container->subjectDistance              = strings[14];
    container->subjectDistanceCategory      = strings[15];
}

} // namespace Digikam

/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-03-05
 * Description : Qt item model for database entries
 *
 * Copyright (C) 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C)      2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C)      2010 by Andi Clemens <andi dot clemens at gmx dot net>
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

#include "imagefiltermodel.moc"
#include "imagefiltermodelpriv.moc"
#include "imagefiltermodelthreads.moc"

// KDE includes

#include <kdebug.h>
#include <kstringhandler.h>

// Local includes

#include "databaseaccess.h"
#include "databasechangesets.h"
#include "databasewatch.h"
#include "imageinfolist.h"
#include "imagemodel.h"

namespace Digikam
{

ImageSortFilterModel::ImageSortFilterModel(QObject* parent)
    : KCategorizedSortFilterProxyModel(parent), m_chainedModel(0)
{
}

void ImageSortFilterModel::setSourceImageModel(ImageModel* source)
{
    if (m_chainedModel)
    {
        m_chainedModel->setSourceImageModel(source);
    }
    else
    {
        setDirectSourceImageModel(source);
    }
}

void ImageSortFilterModel::setSourceFilterModel(ImageSortFilterModel* source)
{
    if (source)
    {
        ImageModel* model = sourceImageModel();

        if (model)
        {
            source->setSourceImageModel(model);
        }
    }

    m_chainedModel = source;
    setSourceModel(source);
}

void ImageSortFilterModel::setDirectSourceImageModel(ImageModel* model)
{
    setSourceModel(model);
}

void ImageSortFilterModel::setSourceModel(QAbstractItemModel* model)
{
    // made it protected, only setSourceImageModel is public
    KCategorizedSortFilterProxyModel::setSourceModel(model);
}

ImageModel* ImageSortFilterModel::sourceImageModel() const
{
    if (m_chainedModel)
    {
        return m_chainedModel->sourceImageModel();
    }

    return static_cast<ImageModel*>(sourceModel());
}

ImageSortFilterModel* ImageSortFilterModel::sourceFilterModel() const
{
    return m_chainedModel;
}

ImageFilterModel* ImageSortFilterModel::imageFilterModel() const
{
    // reimplemented in ImageFilterModel
    if (m_chainedModel)
    {
        return m_chainedModel->imageFilterModel();
    }

    return 0;
}

QModelIndex ImageSortFilterModel::mapToSourceImageModel(const QModelIndex& index) const
{
    if (m_chainedModel)
    {
        return m_chainedModel->mapToSourceImageModel(mapToSource(index));
    }

    return mapToSource(index);
}

QModelIndex ImageSortFilterModel::mapFromSourceImageModel(const QModelIndex& albummodel_index) const
{
    if (m_chainedModel)
    {
        return mapFromSource(m_chainedModel->mapFromSourceImageModel(albummodel_index));
    }

    return mapFromSource(albummodel_index);
}

// -------------- Convenience mappers -------------------------------------------------------------------

QList<QModelIndex> ImageSortFilterModel::mapListToSource(const QList<QModelIndex>& indexes) const
{
    QList<QModelIndex> sourceIndexes;
    foreach (const QModelIndex& index, indexes)
    {
        sourceIndexes << mapToSourceImageModel(index);
    }
    return sourceIndexes;
}

QList<QModelIndex> ImageSortFilterModel::mapListFromSource(const QList<QModelIndex>& sourceIndexes) const
{
    QList<QModelIndex> indexes;
    foreach (const QModelIndex& index, sourceIndexes)
    {
        indexes << mapFromSourceImageModel(index);
    }
    return indexes;
}

ImageInfo ImageSortFilterModel::imageInfo(const QModelIndex& index) const
{
    return sourceImageModel()->imageInfo(mapToSourceImageModel(index));
}

qlonglong ImageSortFilterModel::imageId(const QModelIndex& index) const
{
    return sourceImageModel()->imageId(mapToSourceImageModel(index));
}

QList<ImageInfo> ImageSortFilterModel::imageInfos(const QList<QModelIndex>& indexes) const
{
    QList<ImageInfo> infos;
    ImageModel* model = sourceImageModel();
    foreach (const QModelIndex& index, indexes)
    {
        infos << model->imageInfo(mapToSourceImageModel(index));
    }
    return infos;
}

QList<qlonglong> ImageSortFilterModel::imageIds(const QList<QModelIndex>& indexes) const
{
    QList<qlonglong> ids;
    ImageModel* model = sourceImageModel();
    foreach (const QModelIndex& index, indexes)
    {
        ids << model->imageId(mapToSourceImageModel(index));
    }
    return ids;
}

QModelIndex ImageSortFilterModel::indexForPath(const QString& filePath) const
{
    return mapFromSourceImageModel(sourceImageModel()->indexForPath(filePath));
}

QModelIndex ImageSortFilterModel::indexForImageInfo(const ImageInfo& info) const
{
    return mapFromSourceImageModel(sourceImageModel()->indexForImageInfo(info));
}

QModelIndex ImageSortFilterModel::indexForImageId(qlonglong id) const
{
    return mapFromSourceImageModel(sourceImageModel()->indexForImageId(id));
}

QList<ImageInfo> ImageSortFilterModel::imageInfosSorted() const
{
    QList<ImageInfo> infos;
    const int size = rowCount();
    ImageModel* model = sourceImageModel();

    for (int i=0; i<size; ++i)
    {
        infos << model->imageInfo(mapToSourceImageModel(index(i, 0)));
    }

    return infos;
}

// -------------- ImageFilterModelPrivate --------------

ImageFilterModelPrivate::ImageFilterModelPrivate()
{
    imageModel            = 0;
    version               = 0;
    lastDiscardVersion    = 0;
    sentOut               = 0;
    sentOutForReAdd       = 0;
    updateFilterTimer     = 0;
    needPrepare           = false;
    needPrepareComments   = false;
    needPrepareTags       = false;
    needPrepareGroups     = false;
    preparer              = 0;
    filterer              = 0;
    hasOneMatch           = false;
    hasOneMatchForText    = false;

    setupWorkers();
}

ImageFilterModelPrivate::~ImageFilterModelPrivate()
{
    // facilitate thread stopping
    ++version;
    preparer->deactivate();
    filterer->deactivate();
    delete preparer;
    delete filterer;
}

ImageFilterModelWorker::ImageFilterModelWorker(ImageFilterModelPrivate* d)
    : d(d)
{
}

const int PrepareChunkSize = 101;
const int FilterChunkSize  = 2001;

ImageFilterModel::ImageFilterModel(QObject* parent)
    : ImageSortFilterModel(parent),
      d_ptr(new ImageFilterModelPrivate)
{
    d_ptr->init(this);
}

ImageFilterModel::ImageFilterModel(ImageFilterModelPrivate& dd, QObject* parent)
    : ImageSortFilterModel(parent),
      d_ptr(&dd)
{
    d_ptr->init(this);
}

void ImageFilterModelPrivate::init(ImageFilterModel* _q)
{
    q = _q;

    updateFilterTimer  = new QTimer(this);
    updateFilterTimer->setSingleShot(true);
    updateFilterTimer->setInterval(250);

    connect(updateFilterTimer, SIGNAL(timeout()),
            q, SLOT(slotUpdateFilter()));

    // inter-thread redirection
    qRegisterMetaType<ImageFilterModelTodoPackage>("ImageFilterModelTodoPackage");
}

ImageFilterModel::~ImageFilterModel()
{
    Q_D(ImageFilterModel);
    delete d;
}

void ImageFilterModel::setDirectSourceImageModel(ImageModel* sourceModel)
{
    Q_D(ImageFilterModel);

    if (d->imageModel)
    {
        d->imageModel->unsetPreprocessor(d);
        disconnect(d->imageModel, SIGNAL(modelReset()),
                   this, SLOT(slotModelReset()));
        slotModelReset();
    }

    d->imageModel = sourceModel;

    if (d->imageModel)
    {
        d->imageModel->setPreprocessor(d);

        connect(d->imageModel, SIGNAL(preprocess(const QList<ImageInfo>&, const QList<QVariant>&)),
                d, SLOT(preprocessInfos(const QList<ImageInfo>&, const QList<QVariant>&)));

        connect(d->imageModel, SIGNAL(processAdded(const QList<ImageInfo>&, const QList<QVariant>&)),
                d, SLOT(processAddedInfos(const QList<ImageInfo>&, const QList<QVariant>&)));

        connect(d, SIGNAL(reAddImageInfos(const QList<ImageInfo>&, const QList<QVariant>&)),
                d->imageModel, SLOT(reAddImageInfos(const QList<ImageInfo>&, const QList<QVariant>&)));

        connect(d, SIGNAL(reAddingFinished()),
                d->imageModel, SLOT(reAddingFinished()));

        connect(d->imageModel, SIGNAL(modelReset()),
                this, SLOT(slotModelReset()));

        connect(d->imageModel, SIGNAL(imageChange(const ImageChangeset&, const QItemSelection&)),
                this, SLOT(slotImageChange(const ImageChangeset&)));

        connect(d->imageModel, SIGNAL(imageTagChange(const ImageTagChangeset&, const QItemSelection&)),
                this, SLOT(slotImageTagChange(const ImageTagChangeset&)));
    }

    setSourceModel(d->imageModel);
}

QVariant ImageFilterModel::data(const QModelIndex& index, int role) const
{
    Q_D(const ImageFilterModel);

    if (!index.isValid())
    {
        return QVariant();
    }

    switch (role)
    {
            // Attention: This breaks should there ever be another filter model between this and the ImageModel

        case KCategorizedSortFilterProxyModel::CategoryDisplayRole:
            return categoryIdentifier(d->imageModel->imageInfoRef(mapToSource(index)));
        case CategorizationModeRole:
            return d->sorter.categorizationMode;
        case SortOrderRole:
            return d->sorter.sortRole;
            //case CategoryCountRole:
            //  return categoryCount(d->imageModel->imageInfoRef(mapToSource(index)));
        case CategoryAlbumIdRole:
            return d->imageModel->imageInfoRef(mapToSource(index)).albumId();
        case CategoryFormatRole:
            return d->imageModel->imageInfoRef(mapToSource(index)).format();
        case GroupIsOpenRole:
            return d->groupFilter.isAllOpen() ||
                   d->groupFilter.isOpen(d->imageModel->imageInfoRef(mapToSource(index)).id());
        case ImageFilterModelPointerRole:
            return QVariant::fromValue(const_cast<ImageFilterModel*>(this));
    }

    return KCategorizedSortFilterProxyModel::data(index, role);
}

ImageFilterModel* ImageFilterModel::imageFilterModel() const
{
    return const_cast<ImageFilterModel*>(this);
}

DatabaseFields::Set ImageFilterModel::suggestedWatchFlags() const
{
    DatabaseFields::Set watchFlags;
    watchFlags |= DatabaseFields::Name   | DatabaseFields::FileSize     | DatabaseFields::ModificationDate;
    watchFlags |= DatabaseFields::Rating | DatabaseFields::CreationDate | DatabaseFields::Orientation |
                  DatabaseFields::Width  | DatabaseFields::Height;
    watchFlags |= DatabaseFields::Comment;
    watchFlags |= DatabaseFields::ImageRelations;
    return watchFlags;
}

// -------------- Filter settings --------------

void ImageFilterModel::setDayFilter(const QList<QDateTime>& days)
{
    Q_D(ImageFilterModel);
    d->filter.setDayFilter(days);
    setImageFilterSettings(d->filter);
}

void ImageFilterModel::setTagFilter(const QList<int>& includedTags, const QList<int>& excludedTags,
                                    ImageFilterSettings::MatchingCondition matchingCond,
                                    bool showUnTagged, const QList<int>& clTagIds, const QList<int>& plTagIds)
{
    Q_D(ImageFilterModel);
    d->filter.setTagFilter(includedTags, excludedTags, matchingCond, showUnTagged, clTagIds, plTagIds);
    setImageFilterSettings(d->filter);
}

void ImageFilterModel::setRatingFilter(int rating, ImageFilterSettings::RatingCondition ratingCond)
{
    Q_D(ImageFilterModel);
    d->filter.setRatingFilter(rating, ratingCond);
    setImageFilterSettings(d->filter);
}

void ImageFilterModel::setUrlWhitelist(const KUrl::List urlList, const QString& id)
{
    Q_D(ImageFilterModel);
    d->filter.setUrlWhitelist(urlList, id);
    setImageFilterSettings(d->filter);
}

void ImageFilterModel::setIdWhitelist(const QList<qlonglong>& idList, const QString& id)
{
    Q_D(ImageFilterModel);
    d->filter.setIdWhitelist(idList,id);
    setImageFilterSettings(d->filter);
}

void ImageFilterModel::setMimeTypeFilter(int mimeTypeFilter)
{
    Q_D(ImageFilterModel);
    d->filter.setMimeTypeFilter(mimeTypeFilter);
    setImageFilterSettings(d->filter);
}

void ImageFilterModel::setTextFilter(const SearchTextFilterSettings& settings)
{
    Q_D(ImageFilterModel);
    d->filter.setTextFilter(settings);
    setImageFilterSettings(d->filter);
}

void ImageFilterModel::setImageFilterSettings(const ImageFilterSettings& settings)
{
    Q_D(ImageFilterModel);

    {
        QMutexLocker lock(&d->mutex);
        d->version++;
        d->filter              = settings;
        d->filterCopy          = settings;
        d->versionFilterCopy   = d->versionFilter;
        d->groupFilterCopy     = d->groupFilter;

        d->needPrepareComments = settings.isFilteringByText();
        d->needPrepareTags     = settings.isFilteringByTags();
        d->needPrepareGroups   = true;
        d->needPrepare         = d->needPrepareComments || d->needPrepareTags || d->needPrepareGroups;

        d->hasOneMatch         = false;
        d->hasOneMatchForText  = false;
    }

    d->filterResults.clear();

    //d->categoryCountHashInt.clear();
    //d->categoryCountHashString.clear();
    if (d->imageModel)
    {
        d->infosToProcess(d->imageModel->imageInfos());
    }

    emit filterSettingsChanged(settings);
}

void ImageFilterModel::setVersionManagerSettings(const VersionManagerSettings& settings)
{
    Q_D(ImageFilterModel);
    d->versionFilter.setVersionManagerSettings(settings);
    setVersionImageFilterSettings(d->versionFilter);
}

void ImageFilterModel::setExceptionList(const QList<qlonglong>& idList, const QString& id)
{
    Q_D(ImageFilterModel);
    d->versionFilter.setExceptionList(idList, id);
    setVersionImageFilterSettings(d->versionFilter);
}

void ImageFilterModel::setVersionImageFilterSettings(const VersionImageFilterSettings& settings)
{
    Q_D(ImageFilterModel);
    d->versionFilter = settings;
    slotUpdateFilter();
}

bool ImageFilterModel::isGroupOpen(qlonglong group) const
{
    Q_D(const ImageFilterModel);
    return d->groupFilter.isOpen(group);
}

bool ImageFilterModel::isAllGroupsOpen() const
{
    Q_D(const ImageFilterModel);
    return d->groupFilter.isAllOpen();
}

void ImageFilterModel::setGroupOpen(qlonglong group, bool open)
{
    Q_D(ImageFilterModel);
    d->groupFilter.setOpen(group, open);
    setGroupImageFilterSettings(d->groupFilter);
}

void ImageFilterModel::toggleGroupOpen(qlonglong group)
{
    setGroupOpen(group, !isGroupOpen(group));
}

void ImageFilterModel::setAllGroupsOpen(bool open)
{
    Q_D(ImageFilterModel);
    d->groupFilter.setAllOpen(open);
    setGroupImageFilterSettings(d->groupFilter);
}

void ImageFilterModel::setGroupImageFilterSettings(const GroupImageFilterSettings& settings)
{
    Q_D(ImageFilterModel);
    d->groupFilter = settings;
    slotUpdateFilter();
}

void ImageFilterModel::slotUpdateFilter()
{
    Q_D(ImageFilterModel);
    setImageFilterSettings(d->filter);
}

ImageFilterSettings ImageFilterModel::imageFilterSettings() const
{
    Q_D(const ImageFilterModel);
    return d->filter;
}

ImageSortSettings ImageFilterModel::imageSortSettings() const
{
    Q_D(const ImageFilterModel);
    return d->sorter;
}

VersionImageFilterSettings ImageFilterModel::versionImageFilterSettings() const
{
    Q_D(const ImageFilterModel);
    return d->versionFilter;
}

GroupImageFilterSettings ImageFilterModel::groupImageFilterSettings() const
{
    Q_D(const ImageFilterModel);
    return d->groupFilter;
}

void ImageFilterModel::slotModelReset()
{
    Q_D(ImageFilterModel);
    {
        QMutexLocker lock(&d->mutex);
        // discard all packages on the way that are marked as send out for re-add
        d->lastDiscardVersion = d->version;
        d->sentOutForReAdd    = 0;
        // discard all packages on the way
        d->version++;
        d->sentOut            = 0;

        d->hasOneMatch        = false;
        d->hasOneMatchForText = false;
    }
    d->filterResults.clear();
}

bool ImageFilterModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    Q_D(const ImageFilterModel);

    if (source_parent.isValid())
    {
        return false;
    }

    qlonglong id = d->imageModel->imageId(source_row);
    QHash<qlonglong, bool>::const_iterator it = d->filterResults.constFind(id);

    if (it != d->filterResults.constEnd())
    {
        return it.value();
    }

    // usually done in thread and cache, unless source model changed
    ImageInfo info = d->imageModel->imageInfo(source_row);
    bool match = d->filter.matches(info);
    match = match ? d->versionFilter.matches(info) : false;
    return match ? d->groupFilter.matches(info) : false;
}

void ImageFilterModel::setSendImageInfoSignals(bool sendSignals)
{
    if (sendSignals)
    {
        connect(this, SIGNAL(rowsInserted(const QModelIndex&, int, int)),
                this, SLOT(slotRowsInserted(const QModelIndex&, int, int)));
        connect(this, SIGNAL(rowsAboutToBeRemoved(const QModelIndex&, int, int)),
                this, SLOT(slotRowsAboutToBeRemoved(const QModelIndex&, int, int)));
    }
    else
    {
        disconnect(this, SIGNAL(rowsInserted(const QModelIndex&, int, int)),
                   this, SLOT(slotRowsInserted(const QModelIndex&, int, int)));
        disconnect(this, SIGNAL(rowsAboutToBeRemoved(const QModelIndex&, int, int)),
                   this, SLOT(slotRowsAboutToBeRemoved(const QModelIndex&, int, int)));
    }
}

void ImageFilterModel::slotRowsInserted(const QModelIndex& /*parent*/, int start, int end)
{
    QList<ImageInfo> infos;

    for (int i=start; i>end; i++)
    {
        infos << imageInfo(index(i, 0));
    }

    emit imageInfosAdded(infos);
}

void ImageFilterModel::slotRowsAboutToBeRemoved(const QModelIndex& /*parent*/, int start, int end)
{
    QList<ImageInfo> infos;

    for (int i=start; i>end; i++)
    {
        infos << imageInfo(index(i, 0));
    }

    emit imageInfosAboutToBeRemoved(infos);
}

// -------------- Threaded preparation & filtering --------------

void ImageFilterModel::addPrepareHook(ImageFilterModelPrepareHook* hook)
{
    Q_D(ImageFilterModel);
    QMutexLocker lock(&d->mutex);
    d->prepareHooks << hook;
}

void ImageFilterModel::removePrepareHook(ImageFilterModelPrepareHook* hook)
{
    Q_D(ImageFilterModel);
    QMutexLocker lock(&d->mutex);
    d->prepareHooks.removeAll(hook);
}

void ImageFilterModelPrivate::preprocessInfos(const QList<ImageInfo>& infos, const QList<QVariant>& extraValues)
{
    infosToProcess(infos, extraValues, true);
}

void ImageFilterModelPrivate::processAddedInfos(const QList<ImageInfo>& infos, const QList<QVariant>& extraValues)
{
    // These have already been added, we just process them afterwards
    infosToProcess(infos, extraValues, false);
}

void ImageFilterModelPrivate::setupWorkers()
{
    preparer = new ImageFilterModelPreparer(this);
    filterer = new ImageFilterModelFilterer(this);

    // A package in constructed in infosToProcess.
    // Normal flow is infosToProcess -> preparer::process -> filterer::process -> packageFinished.
    // If no preparation is needed, the first step is skipped.
    // If filter version changes, both will discard old package and send them to packageDiscarded.

    connect(this, SIGNAL(packageToPrepare(const ImageFilterModelTodoPackage&)),
            preparer, SLOT(process(ImageFilterModelTodoPackage)));

    connect(this, SIGNAL(packageToFilter(const ImageFilterModelTodoPackage&)),
            filterer, SLOT(process(ImageFilterModelTodoPackage)));

    connect(preparer, SIGNAL(processed(const ImageFilterModelTodoPackage&)),
            filterer, SLOT(process(ImageFilterModelTodoPackage)));

    connect(filterer, SIGNAL(processed(const ImageFilterModelTodoPackage&)),
            this, SLOT(packageFinished(const ImageFilterModelTodoPackage&)));

    connect(preparer, SIGNAL(discarded(const ImageFilterModelTodoPackage&)),
            this, SLOT(packageDiscarded(const ImageFilterModelTodoPackage&)));

    connect(filterer, SIGNAL(discarded(const ImageFilterModelTodoPackage&)),
            this, SLOT(packageDiscarded(const ImageFilterModelTodoPackage&)));
}

void ImageFilterModelPrivate::infosToProcess(const QList<ImageInfo>& infos)
{
    infosToProcess(infos, QList<QVariant>(), false);
}

void ImageFilterModelPrivate::infosToProcess(const QList<ImageInfo>& infos, const QList<QVariant>& extraValues, bool forReAdd)
{
    if (infos.isEmpty())
    {
        return;
    }

    filterer->schedule();

    if (needPrepare)
    {
        preparer->schedule();
    }

    // prepare and filter in chunks
    const int size                      = infos.size();
    const int maxChunkSize              = needPrepare ? PrepareChunkSize : FilterChunkSize;
    QList<ImageInfo>::const_iterator it = infos.constBegin();
    QList<QVariant>::const_iterator xit = extraValues.constBegin();
    int index                           = 0;
    QVector<QVariant> extraValueVector;

    while (it != infos.constEnd())
    {
        QVector<ImageInfo> infoVector(qMin(maxChunkSize, size - index));
        QList<ImageInfo>::const_iterator end = it + infoVector.size();
        qCopy(it, end, infoVector.begin());

        if (xit != extraValues.constEnd())
        {
            extraValueVector                     = QVector<QVariant>(infoVector.size());
            QList<QVariant>::const_iterator xend = xit + extraValueVector.size();
            qCopy(xit, xend, extraValueVector.begin());
            xit = xend;
        }

        it    = end;
        index += infoVector.size();

        ++sentOut;

        if (forReAdd)
        {
            ++sentOutForReAdd;
        }

        if (needPrepare)
        {
            emit packageToPrepare(ImageFilterModelTodoPackage(infoVector, version, forReAdd, extraValueVector));
        }
        else
        {
            emit packageToFilter(ImageFilterModelTodoPackage(infoVector, version, forReAdd, extraValueVector));
        }
    }
}

void ImageFilterModelPrivate::packageFinished(const ImageFilterModelTodoPackage& package)
{
    // check if it got discarded on the journey
    if (package.version != version)
    {
        packageDiscarded(package);
        return;
    }

    // incorporate result
    QHash<qlonglong, bool>::const_iterator it = package.filterResults.constBegin();

    for (; it != package.filterResults.constEnd(); ++it)
    {
        filterResults.insert(it.key(), it.value());
    }

    // re-add if necessary
    if (package.isForReAdd)
    {
        emit reAddImageInfos(package.infos.toList(), package.extraValues.toList());

        if (sentOutForReAdd == 1) // last package
        {
            emit reAddingFinished();
        }
    }

    // decrement counters
    --sentOut;

    if (package.isForReAdd)
    {
        --sentOutForReAdd;
    }

    // If all packages have returned, filtered and readded, and no more are expected,
    // and there is need to tell the filter result to the view, do that
    if (sentOut == 0 && sentOutForReAdd == 0 && !imageModel->isRefreshing())
    {
        q->invalidate(); // use invalidate, not invalidateFilter only. Sorting may have changed as well.
        emit q->filterMatches(hasOneMatch);
        emit q->filterMatchesForText(hasOneMatchForText);
        filterer->deactivate();
        preparer->deactivate();
    }
}

void ImageFilterModelPrivate::packageDiscarded(const ImageFilterModelTodoPackage& package)
{
    // Either, the model was reset, or the filter changed
    // In the former case throw all away, in the latter case, recycle
    if (package.version > lastDiscardVersion)
    {
        // Recycle packages: Send again with current version
        // Do not increment sentOut or sentOutForReAdd here: it was not decremented!

        if (needPrepare)
        {
            emit packageToPrepare(ImageFilterModelTodoPackage(package.infos, version, package.isForReAdd));
        }
        else
        {
            emit packageToFilter(ImageFilterModelTodoPackage(package.infos, version, package.isForReAdd));
        }
    }
}

void ImageFilterModelPreparer::process(ImageFilterModelTodoPackage package)
{
    if (!checkVersion(package))
    {
        emit discarded(package);
        return;
    }

    // get thread-local copy
    bool needPrepareTags, needPrepareComments, needPrepareGroups;
    QList<ImageFilterModelPrepareHook*> prepareHooks;
    {
        QMutexLocker lock(&d->mutex);
        needPrepareTags     = d->needPrepareTags;
        needPrepareComments = d->needPrepareComments;
        needPrepareGroups   = d->needPrepareGroups;
        prepareHooks        = d->prepareHooks;
    }

    //TODO: Make efficient!!
    if (needPrepareComments)
    {
        foreach(const ImageInfo& info, package.infos)
        {
            info.comment();
        }
    }

    if (!checkVersion(package))
    {
        emit discarded(package);
        return;
    }

    //TODO: Make efficient!!
    if (needPrepareTags)
    {
        foreach(const ImageInfo& info, package.infos)
        {
            info.tagIds();
        }
    }

    //TODO: Make efficient!!
    if (needPrepareGroups)
    {
        foreach(const ImageInfo& info, package.infos)
        {
            info.isGrouped();
        }
    }

    foreach (ImageFilterModelPrepareHook* hook, prepareHooks)
    {
        hook->prepare(package.infos);
    }

    emit processed(package);
}

void ImageFilterModelFilterer::process(ImageFilterModelTodoPackage package)
{
    if (!checkVersion(package))
    {
        emit discarded(package);
        return;
    }

    // get thread-local copy
    ImageFilterSettings localFilter;
    VersionImageFilterSettings localVersionFilter;
    GroupImageFilterSettings localGroupFilter;
    bool hasOneMatch;
    bool hasOneMatchForText;
    {
        QMutexLocker lock(&d->mutex);
        localFilter        = d->filterCopy;
        localVersionFilter = d->versionFilterCopy;
        localGroupFilter   = d->groupFilterCopy;
        hasOneMatch        = d->hasOneMatch;
        hasOneMatchForText = d->hasOneMatchForText;
    }

    // Actual filtering. The variants to spare checking hasOneMatch over and over again.
    if (hasOneMatch && hasOneMatchForText)
    {
        foreach (const ImageInfo& info, package.infos)
        {
            package.filterResults[info.id()] = localFilter.matches(info)
                                                && localVersionFilter.matches(info)
                                                && localGroupFilter.matches(info);
        }
    }
    else if (hasOneMatch)
    {
        bool matchForText;
        foreach (const ImageInfo& info, package.infos)
        {
            package.filterResults[info.id()] = localFilter.matches(info, &matchForText)
                                                && localVersionFilter.matches(info)
                                                && localGroupFilter.matches(info);

            if (matchForText)
            {
                hasOneMatchForText = true;
            }
        }
    }
    else
    {
        bool result, matchForText;
        foreach (const ImageInfo& info, package.infos)
        {
            result                           = localFilter.matches(info, &matchForText)
                                                && localVersionFilter.matches(info)
                                                && localGroupFilter.matches(info);
            package.filterResults[info.id()] = result;

            if (result)
            {
                hasOneMatch = true;
            }

            if (matchForText)
            {
                hasOneMatchForText = true;
            }
        }
    }

    if (checkVersion(package))
    {
        QMutexLocker lock(&d->mutex);
        d->hasOneMatch = hasOneMatch;
        d->hasOneMatchForText = hasOneMatchForText;
    }

    emit processed(package);
}

// -------------- Sorting and Categorization -------------------------------------------------------

void ImageFilterModel::setImageSortSettings(const ImageSortSettings& sorter)
{
    Q_D(ImageFilterModel);
    d->sorter = sorter;
    setCategorizedModel(d->sorter.categorizationMode != ImageSortSettings::NoCategories);
    invalidate();
}

void ImageFilterModel::setCategorizationMode(ImageSortSettings::CategorizationMode mode)
{
    Q_D(ImageFilterModel);
    d->sorter.setCategorizationMode(mode);
    setImageSortSettings(d->sorter);
}

void ImageFilterModel::setSortRole(ImageSortSettings::SortRole role)
{
    Q_D(ImageFilterModel);
    d->sorter.setSortRole(role);
    setImageSortSettings(d->sorter);
}

void ImageFilterModel::setSortOrder(ImageSortSettings::SortOrder order)
{
    Q_D(ImageFilterModel);
    d->sorter.setSortOrder(order);
    setImageSortSettings(d->sorter);
}

int ImageFilterModel::compareCategories(const QModelIndex& left, const QModelIndex& right) const
{
    // source indexes
    Q_D(const ImageFilterModel);

    if (!left.isValid() || !right.isValid())
    {
        return -1;
    }

    return compareInfosCategories(d->imageModel->imageInfoRef(left), d->imageModel->imageInfoRef(right));
}

bool ImageFilterModel::subSortLessThan(const QModelIndex& left, const QModelIndex& right) const
{
    // source indexes
    Q_D(const ImageFilterModel);

    if (!left.isValid() || !right.isValid())
    {
        return true;
    }

    if (left == right)
    {
        return false;
    }

    const ImageInfo& leftInfo  = d->imageModel->imageInfoRef(left);

    const ImageInfo& rightInfo = d->imageModel->imageInfoRef(right);

    if (leftInfo == rightInfo)
    {
        return d->sorter.lessThan(left.data(ImageModel::ExtraDataRole), right.data(ImageModel::ExtraDataRole));
    }

    if ((leftInfo.isGrouped() || rightInfo.isGrouped())
            && leftInfo.groupImage() != rightInfo.groupImage())
    {
        // only one of the two is grouped, or both are grouped, but on different images.
        return infosLessThan(leftInfo.isGrouped() ? leftInfo.groupImage() : leftInfo,
                             rightInfo.isGrouped() ? rightInfo.groupImage() : rightInfo);
    }

    return infosLessThan(leftInfo, rightInfo);
}

int ImageFilterModel::compareInfosCategories(const ImageInfo& left, const ImageInfo& right) const
{
    // Note: reimplemented in ImageImageSortFilterModel
    Q_D(const ImageFilterModel);
    return d->sorter.compareCategories(left, right);
}

// Feel free to optimize. QString::number is 3x slower.
static inline QString fastNumberToString(int id)
{
    const int size = sizeof(int) * 2;
    char c[size+1];
    c[size]    = '\0';
    char* p    = c;
    int number = id;

    for (int i=0; i<size; i++)
    {
        *p = 'a' + (number & 0xF);
        number >>= 4;
        p++;
    }

    return QString::fromLatin1(c);
}

QString ImageFilterModel::categoryIdentifier(const ImageInfo& info) const
{
    Q_D(const ImageFilterModel);

    switch (d->sorter.categorizationMode)
    {
        case ImageSortSettings::NoCategories:
            return QString();
        case ImageSortSettings::OneCategory:
            return QString();
        case ImageSortSettings::CategoryByAlbum:
            return fastNumberToString(info.albumId());
        case ImageSortSettings::CategoryByFormat:
            return info.format();
        default:
            return QString();
    }
}

bool ImageFilterModel::infosLessThan(const ImageInfo& left, const ImageInfo& right) const
{
    Q_D(const ImageFilterModel);
    return d->sorter.lessThan(left, right);
}

// -------------- Watching changes -----------------------------------------------------------------

void ImageFilterModel::slotImageTagChange(const ImageTagChangeset& changeset)
{
    Q_D(ImageFilterModel);

    if (!d->imageModel || d->imageModel->isEmpty())
    {
        return;
    }

    // already scheduled to re-filter?
    if (d->updateFilterTimer->isActive())
    {
        return;
    }

    // do we filter at all?
    if (!d->versionFilter.isFilteringByTags()
        && !d->filter.isFilteringByTags()
        && !d->filter.isFilteringByText())
    {
        return;
    }

    // is one of our images affected?
    foreach (const qlonglong& id, changeset.ids())
    {
        // if one matching image id is found, trigger a refresh
        if (d->imageModel->hasImage(id))
        {
            d->updateFilterTimer->start();
            return;
        }
    }
}

void ImageFilterModel::slotImageChange(const ImageChangeset& changeset)
{
    Q_D(ImageFilterModel);

    if (!d->imageModel || d->imageModel->isEmpty())
    {
        return;
    }

    // already scheduled to re-filter?
    if (d->updateFilterTimer->isActive())
    {
        return;
    }

    // is one of the values affected that we filter or sort by?
    DatabaseFields::Set set = changeset.changes();
    bool sortAffected       = (set & d->sorter.watchFlags());
    bool filterAffected     = (set & d->filter.watchFlags()) || (set & d->groupFilter.watchFlags());

    if (!sortAffected && !filterAffected)
    {
        return;
    }

    // is one of our images affected?
    bool imageAffected = false;
    foreach (const qlonglong& id, changeset.ids())
    {
        // if one matching image id is found, trigger a refresh
        if (d->imageModel->hasImage(id))
        {
            imageAffected = true;
            break;
        }
    }

    if (!imageAffected)
    {
        return;
    }

    if (filterAffected)
    {
        d->updateFilterTimer->start();
    }
    else
    {
        invalidate();    // just resort, reuse filter results
    }
}

// -------------------------------------------------------------------------------------------------------

NoDuplicatesImageFilterModel::NoDuplicatesImageFilterModel(QObject* parent)
    : ImageSortFilterModel(parent)
{
}

bool NoDuplicatesImageFilterModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    QModelIndex index = sourceModel()->index(source_row, 0, source_parent);
    return index.data(ImageModel::ExtraDataDuplicateCount).toInt() == 0;
}

} // namespace Digikam

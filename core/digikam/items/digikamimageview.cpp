/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-04-24
 * Description : Qt item view for images
 *
 * Copyright (C) 2009-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2009-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
 *
 * This program is free software you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "digikamimageview_p.h"
#include "digikamimageview.moc"

// Qt includes

#include <QClipboard>
#include <QFileInfo>
#include <QPointer>

// KDE includes

#include <kaction.h>
#include <kactionmenu.h>
#include <kactioncollection.h>
#include <kapplication.h>
#include <kmenu.h>
#include <kmessagebox.h>
#include <kmimetype.h>
#include <krun.h>
#include <kservice.h>
#include <kservicetypetrader.h>
#include <kstandardaction.h>
#include <kurl.h>
#include <kwindowsystem.h>
#include <kdebug.h>

// Local includes

#include "albummanager.h"
#include "albumdb.h"
#include "advancedrenamedialog.h"
#include "advancedrenameprocessdialog.h"
#include "albumsettings.h"
#include "assignnameoverlay.h"
#include "contextmenuhelper.h"
#include "databaseaccess.h"
#include "ddragobjects.h"
#include "digikamapp.h"
#include "digikamimagedelegate.h"
#include "digikamimagefacedelegate.h"
#include "dio.h"
#include "dpopupmenu.h"
#include "facerejectionoverlay.h"
#include "groupindicatoroverlay.h"
#include "imagealbumfiltermodel.h"
#include "imagealbummodel.h"
#include "imagedragdrop.h"
#include "imageratingoverlay.h"
#include "tagslineeditoverlay.h"
#include "imageviewutilities.h"
#include "imagewindow.h"
#include "metadatamanager.h"
#include "thumbnailloadthread.h"
#include "tagregion.h"
#include "addtagslineedit.h"

namespace Digikam
{

DigikamImageView::DigikamImageView(QWidget* parent)
    : ImageCategorizedView(parent), d(new DigikamImageViewPriv(this))
{
    installDefaultModels();

    d->faceiface      = new FaceIface;
    d->normalDelegate = new DigikamImageDelegate(this);
    d->faceDelegate   = new DigikamImageFaceDelegate(this);
    setItemDelegate(d->normalDelegate);
    setSpacing(10);

    AlbumSettings* settings = AlbumSettings::instance();

    imageFilterModel()->setCategorizationMode(ImageSortSettings::CategoryByAlbum);

    imageAlbumModel()->setThumbnailLoadThread(ThumbnailLoadThread::defaultIconViewThread());
    setThumbnailSize((ThumbnailSize::Size)settings->getDefaultIconSize());
    imageAlbumModel()->setPreloadThumbnails(true);

    imageModel()->setDragDropHandler(new ImageDragDropHandler(imageModel()));
    setDragEnabled(true);
    setAcceptDrops(true);
    setDropIndicatorShown(false);

    setToolTipEnabled(settings->showToolTipsIsValid());
    imageFilterModel()->setSortRole((ImageSortSettings::SortRole)settings->getImageSortOrder());
    imageFilterModel()->setSortOrder((ImageSortSettings::SortOrder)settings->getImageSorting());
    imageFilterModel()->setCategorizationMode((ImageSortSettings::CategorizationMode)settings->getImageGroupMode());

    // selection overlay
    addSelectionOverlay(d->normalDelegate);
    addSelectionOverlay(d->faceDelegate);

    // rotation overlays
    d->rotateLeftOverlay = new ImageRotateLeftOverlay(this);
    d->rotateRightOverlay = new ImageRotateRightOverlay(this);
    d->updateOverlays();

    // rating overlay
    ImageRatingOverlay* ratingOverlay = new ImageRatingOverlay(this);
    addOverlay(ratingOverlay);

    // face overlays
    addRejectionOverlay(d->faceDelegate);
    addAssignNameOverlay(d->faceDelegate);

    GroupIndicatorOverlay* groupOverlay = new GroupIndicatorOverlay(this);
    addOverlay(groupOverlay);

    connect(ratingOverlay, SIGNAL(ratingEdited(const QModelIndex&, int)),
            this, SLOT(assignRating(const QModelIndex&, int)));

    connect(groupOverlay, SIGNAL(toggleGroupOpen(const QModelIndex&)),
            this, SLOT(groupIndicatorClicked(const QModelIndex&)));

    connect(groupOverlay, SIGNAL(showButtonContextMenu(const QModelIndex&, QContextMenuEvent*)),
            this, SLOT(showGroupContextMenu(const QModelIndex&, QContextMenuEvent*)));

    d->utilities = new ImageViewUtilities(this);

    connect(d->utilities, SIGNAL(editorCurrentUrlChanged(const KUrl&)),
            this, SLOT(setCurrentUrl(const KUrl&)));

    connect(imageModel()->dragDropHandler(), SIGNAL(assignTags(const QList<ImageInfo>&, const QList<int>&)),
            MetadataManager::instance(), SLOT(assignTags(const QList<ImageInfo>&, const QList<int>&)));

    connect(imageModel()->dragDropHandler(), SIGNAL(addToGroup(const ImageInfo&, const QList<ImageInfo>&)),
            MetadataManager::instance(), SLOT(addToGroup(const ImageInfo&, const QList<ImageInfo>&)));

    connect(imageModel()->dragDropHandler(), SIGNAL(dioResult(KJob*)),
            d->utilities, SLOT(slotDIOResult(KJob*)));

    connect(settings, SIGNAL(setupChanged()),
            this, SLOT(slotSetupChanged()));

    slotSetupChanged();
}

DigikamImageView::~DigikamImageView()
{
    delete d->faceiface;
    delete d;
}

ImageViewUtilities* DigikamImageView::utilities() const
{
    return d->utilities;
}

void DigikamImageView::setThumbnailSize(const ThumbnailSize& size)
{
    imageThumbnailModel()->setPreloadThumbnailSize(size);
    ImageCategorizedView::setThumbnailSize(size);
}

void DigikamImageView::slotSetupChanged()
{
    setToolTipEnabled(AlbumSettings::instance()->showToolTipsIsValid());
    setFont(AlbumSettings::instance()->getIconViewFont());

    d->updateOverlays();

    ImageCategorizedView::slotSetupChanged();
}

void DigikamImageView::setFaceMode(bool on)
{
    if (on)
    {
        // See ImageLister, which creates a search the implements listing tag in the ioslave
        imageAlbumModel()->setSpecialTagListing("faces");
        setItemDelegate(d->faceDelegate);
    }
    else
    {
        imageAlbumModel()->setSpecialTagListing(QString());
        setItemDelegate(d->normalDelegate);
    }
}

void DigikamImageView::addRejectionOverlay(ImageDelegate* delegate)
{
    FaceRejectionOverlay* rejectionOverlay = new FaceRejectionOverlay(this);
    connect(rejectionOverlay, SIGNAL(rejectFace(const QModelIndex&)),
            this, SLOT(slotUntagFace(const QModelIndex&)));

    addOverlay(rejectionOverlay, delegate);
}


void DigikamImageView::addTagEditOverlay(ImageDelegate* delegate)
{
    TagsLineEditOverlay* tagOverlay = new TagsLineEditOverlay(this);

    connect(tagOverlay, SIGNAL(tagEdited(QModelIndex, QString)),
            this, SLOT(assignTag(QModelIndex, QString)));

    addOverlay(tagOverlay, delegate);
}

void DigikamImageView::addAssignNameOverlay(ImageDelegate* delegate)
{
    AssignNameOverlay* nameOverlay = new AssignNameOverlay(this);
    addOverlay(nameOverlay, delegate);
}

void DigikamImageView::slotUntagFace(const QModelIndex& index)
{
    ImageInfo info    = ImageModel::retrieveImageInfo(index);
    DatabaseFace face = d->faceDelegate->face(index);
    kDebug()<<"Untagging face in image " << info.filePath() << "and rect " << face.region().toRect();
    d->faceiface->removeFace(face);
}

void DigikamImageView::activated(const ImageInfo& info)
{
    if (info.isNull())
    {
        return;
    }

    if (AlbumSettings::instance()->getItemLeftClickAction() == AlbumSettings::ShowPreview)
    {
        emit previewRequested(info);
    }
    else
    {
        openInEditor(info);
    }
}

void DigikamImageView::showContextMenuOnInfo(QContextMenuEvent* event, const ImageInfo& info)
{
    QList<ImageInfo> selectedInfos = selectedImageInfosCurrentFirst();
    QList<qlonglong> selectedImageIDs;
    foreach (const ImageInfo& info, selectedInfos)
    {
        selectedImageIDs << info.id();
    }

    // Temporary actions --------------------------------------

    QAction* viewAction = new QAction(SmallIcon("viewimage"), i18nc("View the selected image", "View"), this);
    viewAction->setEnabled(selectedImageIDs.count() == 1);

    // --------------------------------------------------------

    DPopupMenu popmenu(this);
    ContextMenuHelper cmhelper(&popmenu);
    cmhelper.setImageFilterModel(imageFilterModel());

    cmhelper.addAction("move_selection_to_album");
    cmhelper.addAction(viewAction);
    cmhelper.addAction("image_edit");
    cmhelper.addServicesMenu(selectedUrls());
    cmhelper.addGotoMenu(selectedImageIDs);
    cmhelper.addKipiActions(selectedImageIDs);
    cmhelper.addSeparator();
    // --------------------------------------------------------
    cmhelper.addAction("image_find_similar");
    cmhelper.addStandardActionLightTable();
    cmhelper.addQueueManagerMenu();
    cmhelper.addSeparator();
    // --------------------------------------------------------
    cmhelper.addAction("image_rename");
    cmhelper.addAction("cut_album_selection");
    cmhelper.addAction("copy_album_selection");
    cmhelper.addAction("paste_album_selection");
    cmhelper.addStandardActionItemDelete(this, SLOT(deleteSelected()), selectedImageIDs.count());
    cmhelper.addSeparator();
    // --------------------------------------------------------
    cmhelper.addStandardActionThumbnail(selectedImageIDs, currentAlbum());
    // --------------------------------------------------------
    cmhelper.addAssignTagsMenu(selectedImageIDs);
    cmhelper.addRemoveTagsMenu(selectedImageIDs);
    cmhelper.addSeparator();
    // --------------------------------------------------------
    cmhelper.addLabelsAction();
    cmhelper.addGroupMenu(selectedImageIDs);

    // special action handling --------------------------------

    connect(&cmhelper, SIGNAL(signalAssignTag(int)),
            this, SLOT(assignTagToSelected(int)));

    connect(&cmhelper, SIGNAL(signalPopupTagsView()),
            this, SIGNAL(signalPopupTagsView()));

    connect(&cmhelper, SIGNAL(signalRemoveTag(int)),
            this, SLOT(removeTagFromSelected(int)));

    connect(&cmhelper, SIGNAL(signalGotoTag(int)),
            this, SIGNAL(gotoTagAndImageRequested(int)));

    connect(&cmhelper, SIGNAL(signalGotoAlbum(const ImageInfo&)),
            this, SIGNAL(gotoAlbumAndImageRequested(const ImageInfo&)));

    connect(&cmhelper, SIGNAL(signalGotoDate(const ImageInfo&)),
            this, SIGNAL(gotoDateAndImageRequested(const ImageInfo&)));

    connect(&cmhelper, SIGNAL(signalAssignPickLabel(int)),
            this, SLOT(assignPickLabelToSelected(int)));

    connect(&cmhelper, SIGNAL(signalAssignColorLabel(int)),
            this, SLOT(assignColorLabelToSelected(int)));

    connect(&cmhelper, SIGNAL(signalAssignRating(int)),
            this, SLOT(assignRatingToSelected(int)));

    connect(&cmhelper, SIGNAL(signalSetThumbnail(const ImageInfo&)),
            this, SLOT(setAsAlbumThumbnail(const ImageInfo&)));

    connect(&cmhelper, SIGNAL(signalAddToExistingQueue(int)),
            this, SLOT(insertSelectedToExistingQueue(int)));

    connect(&cmhelper, SIGNAL(signalCreateGroup()),
            this, SLOT(createGroupFromSelection()));

    connect(&cmhelper, SIGNAL(signalUngroup()),
            this, SLOT(ungroupSelected()));

    connect(&cmhelper, SIGNAL(signalRemoveFromGroup()),
            this, SLOT(removeSelectedFromGroup()));

    // --------------------------------------------------------

    QAction* choice = cmhelper.exec(event->globalPos());

    if (choice && (choice == viewAction))
    {
        emit previewRequested(info);
    }
}

void DigikamImageView::showGroupContextMenu(const QModelIndex& index, QContextMenuEvent* event)
{
    QList<ImageInfo> selectedInfos = selectedImageInfosCurrentFirst();
    QList<qlonglong> selectedImageIDs;
    foreach (const ImageInfo& info, selectedInfos)
    {
        selectedImageIDs << info.id();
    }

    DPopupMenu popmenu(this);
    ContextMenuHelper cmhelper(&popmenu);
    cmhelper.setImageFilterModel(imageFilterModel());

    cmhelper.addGroupActions(selectedImageIDs);

    // special action handling --------------------------------

    connect(&cmhelper, SIGNAL(signalCreateGroup()),
            this, SLOT(createGroupFromSelection()));

    connect(&cmhelper, SIGNAL(signalUngroup()),
            this, SLOT(ungroupSelected()));

    connect(&cmhelper, SIGNAL(signalRemoveFromGroup()),
            this, SLOT(removeSelectedFromGroup()));


    cmhelper.exec(event->globalPos());
}

void DigikamImageView::showContextMenu(QContextMenuEvent* event)
{
    Album* album = currentAlbum();

    if (!album ||
        album->isRoot() ||
        (album->type() != Album::PHYSICAL && album->type() != Album::TAG) )
    {
        return;
    }

    KMenu popmenu(this);
    KAction* paste        = KStandardAction::paste(this, SLOT(paste()), 0);
    const QMimeData* data = kapp->clipboard()->mimeData(QClipboard::Clipboard);

    /**
    * @todo
    */
    if (!data || !KUrl::List::canDecode(data))
    {
        paste->setEnabled(false);
    }

    popmenu.addAction(paste);
    popmenu.exec(event->globalPos());
    delete paste;
}

void DigikamImageView::openInEditor(const ImageInfo& info)
{
    d->utilities->openInEditor(info, imageInfos(), currentAlbum());
}

void DigikamImageView::openCurrentInEditor()
{
    ImageInfo info = currentInfo();

    if (!info.isNull())
    {
        d->utilities->openInEditor(info, imageInfos(), currentAlbum());
    }
}

void DigikamImageView::openEditor()
{
    ImageInfoList imageInfoList = imageInfos();
    ImageInfo     singleInfo    = currentInfo();
    if (singleInfo.isNull() && !imageInfoList.isEmpty())
    {
        singleInfo = imageInfoList.first();
    }

    d->utilities->openInEditor(singleInfo, imageInfoList, currentAlbum());
}

void DigikamImageView::setOnLightTable()
{
    d->utilities->insertToLightTableAuto(imageInfos(), selectedImageInfos(), currentInfo());
}

void DigikamImageView::addSelectedToLightTable()
{
    // Run Light Table with all selected image files in the current Album.
    // If addTo is false, the light table will be emptied before adding
    // the images.
    ImageInfoList imageInfoList = selectedImageInfos();

    if (!imageInfoList.isEmpty())
    {
        d->utilities->insertToLightTable(imageInfoList, imageInfoList.first(), true);
    }
}

void DigikamImageView::setSelectedOnLightTable()
{
    // Run Light Table with all selected image files in the current Album.
    // If addTo is false, the light table will be emptied before adding
    // the images.
    ImageInfoList imageInfoList = selectedImageInfos();

    if (!imageInfoList.isEmpty())
    {
        d->utilities->insertToLightTable(imageInfoList, imageInfoList.first(), false);
    }
}

void DigikamImageView::insertToQueue()
{
    ImageInfoList imageInfoList = selectedImageInfos();
    ImageInfo     singleInfo    = currentInfo();
    if (singleInfo.isNull() && !imageInfoList.isEmpty())
    {
        singleInfo = imageInfoList.first();
    }
    if (singleInfo.isNull() && model()->rowCount())
    {
        singleInfo = imageInfos().first();
    }
    d->utilities->insertToQueueManagerAuto(imageInfoList, singleInfo);
}

void DigikamImageView::insertSelectedToCurrentQueue()
{
    ImageInfoList imageInfoList = selectedImageInfos();

    if (!imageInfoList.isEmpty())
    {
        d->utilities->insertToQueueManager(imageInfoList, imageInfoList.first(), false);
    }
}

void DigikamImageView::insertSelectedToNewQueue()
{
    ImageInfoList imageInfoList = selectedImageInfos();

    if (!imageInfoList.isEmpty())
    {
        d->utilities->insertToQueueManager(imageInfoList, imageInfoList.first(), true);
    }
}

void DigikamImageView::insertSelectedToExistingQueue(int queueid)
{
    ImageInfoList imageInfoList = selectedImageInfos();

    if (!imageInfoList.isEmpty())
    {
        d->utilities->insertSilentToQueueManager(imageInfoList, imageInfoList.first(), queueid);
    }
}

void DigikamImageView::deleteSelected(bool permanently)
{
    ImageInfoList imageInfoList = selectedImageInfos();

    if (d->utilities->deleteImages(imageInfoList, permanently))
    {
        awayFromSelection();
    }
}

void DigikamImageView::deleteSelectedDirectly(bool permanently)
{
    ImageInfoList imageInfoList = selectedImageInfos();
    d->utilities->deleteImagesDirectly(imageInfoList, permanently);
    awayFromSelection();
}

void DigikamImageView::toggleTagToSelected(int tagID)
{
    ImageInfoList tagToRemove, tagToAssign;

    foreach(ImageInfo info, selectedImageInfos())
    {
        if (info.tagIds().contains(tagID))
            tagToRemove.append(info);
        else
            tagToAssign.append(info);
    }

    MetadataManager::instance()->assignTags(tagToAssign, QList<int>() << tagID);
    MetadataManager::instance()->removeTags(tagToRemove, QList<int>() << tagID);
}

void DigikamImageView::assignTagToSelected(int tagID)
{
    MetadataManager::instance()->assignTags(selectedImageInfos(), QList<int>() << tagID);
}

void DigikamImageView::removeTagFromSelected(int tagID)
{
    MetadataManager::instance()->removeTags(selectedImageInfos(), QList<int>() << tagID);
}

void DigikamImageView::assignPickLabelToSelected(int pickId)
{
    MetadataManager::instance()->assignPickLabel(selectedImageInfos(), pickId);
}

void DigikamImageView::assignPickLabel(const QModelIndex& index, int pickId)
{
    MetadataManager::instance()->assignPickLabel(QList<ImageInfo>() << imageFilterModel()->imageInfo(index), pickId);
}

void DigikamImageView::assignColorLabelToSelected(int colorId)
{
    MetadataManager::instance()->assignColorLabel(selectedImageInfos(), colorId);
}

void DigikamImageView::assignColorLabel(const QModelIndex& index, int colorId)
{
    MetadataManager::instance()->assignColorLabel(QList<ImageInfo>() << imageFilterModel()->imageInfo(index), colorId);
}

void DigikamImageView::assignRatingToSelected(int rating)
{
    MetadataManager::instance()->assignRating(selectedImageInfos(), rating);
}

void DigikamImageView::assignRating(const QModelIndex& index, int rating)
{
    MetadataManager::instance()->assignRating(QList<ImageInfo>() << imageFilterModel()->imageInfo(index), rating);
}

void DigikamImageView::assignTag(const QModelIndex& index, const QString& name)
{
    ImageInfo info = ImageModel::retrieveImageInfo(index);
    DatabaseFace face = d->faceDelegate->face(index);
    int tagId = d->faceiface->tagForPerson(name);
    kDebug()<<"Untagging face in image " << info.filePath() << "name" << name << "tag" << tagId << "region" << face.region().toRect();
    d->faceiface->confirmName(face, tagId);
}

void DigikamImageView::setAsAlbumThumbnail(const ImageInfo& setAsThumbnail)
{
    d->utilities->setAsAlbumThumbnail(currentAlbum(), setAsThumbnail);
}

void DigikamImageView::createNewAlbumForSelected()
{
    d->utilities->createNewAlbumForInfos(selectedImageInfos(), currentAlbum());
}

void DigikamImageView::groupIndicatorClicked(const QModelIndex& index)
{
    ImageInfo info = imageFilterModel()->imageInfo(index);
    if (info.isNull())
    {
        return;
    }
    setCurrentIndex(index);
    imageFilterModel()->toggleGroupOpen(info.id());
}

void DigikamImageView::createGroupFromSelection()
{
    QList<ImageInfo> selectedInfos = selectedImageInfosCurrentFirst();
    ImageInfo groupLeader = selectedInfos.takeFirst();
    MetadataManager::instance()->addToGroup(groupLeader, selectedInfos);
}

void DigikamImageView::ungroupSelected()
{
    MetadataManager::instance()->ungroup(selectedImageInfos());
}

void DigikamImageView::removeSelectedFromGroup()
{
    MetadataManager::instance()->removeFromGroup(selectedImageInfos());
}

void DigikamImageView::setExifOrientationOfSelected(int orientation)
{
    MetadataManager::instance()->setExifOrientation(selectedImageInfos(), orientation);
}

void DigikamImageView::rename()
{
    KUrl::List urls = selectedUrls();
    NewNamesList newNamesList;

    QPointer<AdvancedRenameDialog> dlg = new AdvancedRenameDialog(this);
    dlg->slotAddImages(urls);

    if (dlg->exec() == KDialog::Accepted)
    {
        newNamesList = dlg->newNames();
    }

    delete dlg;

    if (!newNamesList.isEmpty())
    {
        QPointer<AdvancedRenameProcessDialog> dlg = new AdvancedRenameProcessDialog(newNamesList);
        dlg->exec();
        delete dlg;
    }
}

void DigikamImageView::slotRotateLeft()
{
    KActionMenu* action = dynamic_cast<KActionMenu*>(ContextMenuHelper::kipiRotateAction());

    if (action)
    {
        QList<QAction*> list = action->menu()->actions();
        foreach(QAction* ac, list)
        {
            if (ac->objectName() == QString("rotate_ccw"))
            {
                ac->trigger();
            }
        }
    }
}

void DigikamImageView::slotRotateRight()
{
    KActionMenu* action = dynamic_cast<KActionMenu*>(ContextMenuHelper::kipiRotateAction());

    if (action)
    {
        QList<QAction*> list = action->menu()->actions();
        foreach(QAction* ac, list)
        {
            if (ac->objectName() == QString("rotate_cw"))
            {
                ac->trigger();
            }
        }
    }
}

} // namespace Digikam

/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-04-16
 * Description : Qt Model for Albums - drag and drop handling
 *
 * Copyright (C) 2002-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2002-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2011 by Marcel Wiesweg <marcel.wiesweg@gmx.de>
 * Copyright (C) 2009 by Andi Clemens <andi dot clemens at gmx dot net>
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

#include "imagedragdrop.moc"

// Qt includes

#include <QDropEvent>

// KDE includes

#include <kdebug.h>
#include <kiconloader.h>
#include <kio/job.h>
#include <klocale.h>
#include <kmenu.h>

// Local includes

#include "albummanager.h"
#include "cameraui.h"
#include "ddragobjects.h"
#include "dio.h"
#include "imagecategorizedview.h"
#include "imageinfo.h"
#include "imageinfolist.h"

namespace Digikam
{

ImageDragDropHandler::ImageDragDropHandler(ImageModel* model)
    : AbstractItemDragDropHandler(model),
      m_readOnly(false)
{
}

ImageAlbumModel* ImageDragDropHandler::albumModel() const
{
    return qobject_cast<ImageAlbumModel*>(model());
}

void ImageDragDropHandler::setReadOnlyDrop(bool readOnly)
{
    m_readOnly = readOnly;
}

enum DropAction
{
    NoAction,
    CopyAction,
    MoveAction,
    GroupAction,
    AssignTagAction
};

static QAction* addGroupAction(KMenu* menu)
{
    return menu->addAction( SmallIcon("arrow-down-double"),
                              i18nc("@action:inmenu Group images with this image", "Group here"));
}

static QAction* addCancelAction(KMenu* menu)
{
    return menu->addAction( SmallIcon("dialog-cancel"), i18n("C&ancel") );
}

static DropAction copyOrMove(const QDropEvent* e, QWidget* view, bool allowMove = true, bool askForGrouping = false)
{
    if (e->keyboardModifiers() & Qt::ControlModifier)
    {
        return CopyAction;
    }
    else if (e->keyboardModifiers() & Qt::ShiftModifier)
    {
        return MoveAction;
    }

    if (!allowMove && !askForGrouping)
    {
        switch (e->proposedAction())
        {
            case Qt::CopyAction:
                return CopyAction;
            case Qt::MoveAction:
                return MoveAction;
            default:
                return NoAction;
        }
    }

    KMenu popMenu(view);

    QAction* moveAction = 0;
    if (allowMove)
    {
        moveAction = popMenu.addAction( SmallIcon("go-jump"), i18n("&Move Here"));
    }

    QAction* copyAction = popMenu.addAction( SmallIcon("edit-copy"), i18n("&Copy Here"));
    popMenu.addSeparator();

    QAction* groupAction = 0;
    if (askForGrouping)
    {
        groupAction = addGroupAction(&popMenu);
        popMenu.addSeparator();
    }
    addCancelAction(&popMenu);

    popMenu.setMouseTracking(true);
    QAction* choice = popMenu.exec(QCursor::pos());

    if (moveAction && choice == moveAction)
    {
        return MoveAction;
    }
    else if (choice == copyAction)
    {
        return CopyAction;
    }
    else if (groupAction && choice == groupAction)
    {
        return GroupAction;
    }

    return NoAction;
}

static DropAction tagAction(const QDropEvent*, QWidget* view, bool askForGrouping)
{
    KMenu popMenu(view);
    QAction* tagAction = popMenu.addAction(SmallIcon("tag"), i18n("Assign Tag to Dropped Items"));
    QAction* groupAction = 0;
    if (askForGrouping)
    {
        popMenu.addSeparator();
        groupAction = addGroupAction(&popMenu);
    }

    popMenu.addSeparator();
    addCancelAction(&popMenu);

    popMenu.setMouseTracking(true);
    QAction* choice = popMenu.exec(QCursor::pos());

    if (groupAction && choice == groupAction)
    {
        return GroupAction;
    }
    else if (tagAction && choice == tagAction)
    {
        return AssignTagAction;
    }

    return NoAction;
}

static DropAction groupAction(const QDropEvent*, QWidget* view)
{
    KMenu popMenu(view);
    QAction* groupAction = addGroupAction(&popMenu);
    popMenu.addSeparator();
    addCancelAction(&popMenu);

    QAction* choice = popMenu.exec(QCursor::pos());
    if (groupAction && choice == groupAction)
    {
        return GroupAction;
    }

    return NoAction;
}

bool ImageDragDropHandler::dropEvent(QAbstractItemView* abstractview, const QDropEvent* e, const QModelIndex& droppedOn)
{
    ImageCategorizedView* view = static_cast<ImageCategorizedView*>(abstractview);
    Album* album               = view->albumAt(e->pos());

    // unless we are readonly anyway, we always want an album
    if (!m_readOnly && (!album || album->isRoot()) )
    {
        return false;
    }

    PAlbum* palbum = 0;
    TAlbum* talbum = 0;

    if (album)
    {
        Album* currentAlbum = albumModel() ? albumModel()->currentAlbum() : 0;

        if (album->type() == Album::PHYSICAL)
        {
            palbum = static_cast<PAlbum*>(album);
        }
        else if (currentAlbum && currentAlbum->type() == Album::PHYSICAL)
        {
            palbum = static_cast<PAlbum*>(currentAlbum);
        }


        if (album->type() == Album::TAG)
        {
            talbum = static_cast<TAlbum*>(album);
        }
        else if (currentAlbum && currentAlbum->type() == Album::TAG)
        {
            talbum = static_cast<TAlbum*>(currentAlbum);
        }
    }

    if (DItemDrag::canDecode(e->mimeData()))
    {
        // Drag & drop inside of digiKam
        KUrl::List urls;
        KUrl::List kioURLs;
        QList<int> albumIDs;
        QList<qlonglong> imageIDs;

        if (!DItemDrag::decode(e->mimeData(), urls, kioURLs, albumIDs, imageIDs))
        {
            return false;
        }

        if (urls.isEmpty() || kioURLs.isEmpty() || albumIDs.isEmpty() || imageIDs.isEmpty())
        {
            return false;
        }

        DropAction action = NoAction;

        ImageInfo droppedOnInfo;
        if (droppedOn.isValid())
        {
            droppedOnInfo = model()->imageInfo(droppedOn);
        }

        if (m_readOnly)
        {
            emit imageInfosDropped(ImageInfoList(imageIDs));
            return true;
        }
        else if (palbum)
        {
            // Check if items dropped come from outside current album.
            KUrl::List       extUrls, intUrls;
            QList<qlonglong> extImageIDs, intImageIDs;

            for (QList<qlonglong>::const_iterator it = imageIDs.constBegin(); it != imageIDs.constEnd(); ++it)
            {
                ImageInfo info(*it);

                if (info.albumId() != album->id())
                {
                    extUrls << info.databaseUrl();
                    extImageIDs << *it;
                }
                else
                {
                    intUrls << info.databaseUrl();
                    intImageIDs << *it;
                }
            }

            if (intUrls.isEmpty() && extUrls.isEmpty())
            {
                return false;
            }

            bool onlyExternal = (intUrls.isEmpty() && !extUrls.isEmpty());
            bool onlyInternal = (!intUrls.isEmpty() && extUrls.isEmpty());
            bool mixed        = (!intUrls.isEmpty() && !extUrls.isEmpty());

            // Check for drop of image on itself
            if (intImageIDs.size() == 1 && intImageIDs.first() == droppedOnInfo.id())
            {
                return false;
            }

            if (droppedOn.isValid() && onlyInternal)
            {
                action = groupAction(e, view);
            }
            else
            {
                // Determine action. Show Menu only if there are any album-external items.
                // Ask for grouping if dropped-on is valid (gives LinkAction)
                action = copyOrMove(e, view, mixed || onlyExternal, !droppedOnInfo.isNull());
            }

            if (onlyExternal)
            {
                // Only external items: copy or move as requested

                if (action == MoveAction)
                {
                    KIO::Job* job = DIO::move(extUrls, extImageIDs, palbum);
                    connect(job, SIGNAL(result(KJob*)),
                            this, SIGNAL(dioResult(KJob*)));
                    return true;
                }
                else if (action == CopyAction)
                {
                    KIO::Job* job = DIO::copy(extUrls, extImageIDs, palbum);
                    connect(job, SIGNAL(result(KJob*)),
                            this, SIGNAL(dioResult(KJob*)));
                    return true;
                }
            }
            else if (onlyInternal)
            {
                // Only items from the current album:
                // Move is a no-op. Do not show menu to ask for copy or move.
                // If the user indicates a copy operation (holding Ctrl), copy.

                if (action == CopyAction)
                {
                    KIO::Job* job = DIO::copy(intUrls, intImageIDs, palbum);
                    connect(job, SIGNAL(result(KJob*)),
                            this, SIGNAL(dioResult(KJob*)));
                    return true;
                }
                else if (action == MoveAction)
                {
                    return false;
                }
            }
            else if (mixed)
            {
                // Mixed items.
                // For move operations, ignore items from current album.
                // If user requests copy, copy.

                if (action == MoveAction)
                {
                    KIO::Job* job = DIO::move(extUrls, extImageIDs, palbum);
                    connect(job, SIGNAL(result(KJob*)),
                            this, SIGNAL(dioResult(KJob*)));
                    return true;
                }
                else if (action == CopyAction)
                {
                    KIO::Job* job = DIO::copy(extUrls+intUrls, extImageIDs+intImageIDs, palbum);
                    connect(job, SIGNAL(result(KJob*)),
                            this, SIGNAL(dioResult(KJob*)));
                     return true;
                }
            }
        }
        else if (talbum)
        {
            action = tagAction(e, view, droppedOn.isValid());

            if (action == AssignTagAction)
            {
                emit assignTags(ImageInfoList(imageIDs), QList<int>() << talbum->id());
                return true;
            }
        }
        else
        {
            if (droppedOn.isValid())
            {
                // Ask if the user wants to group
                action = groupAction(e, view);
            }
        }

        if (action == GroupAction)
        {
            if (droppedOnInfo.isNull())
            {
                return false;
            }
            emit addToGroup(droppedOnInfo, ImageInfoList(imageIDs));
            return true;
        }

        return false;
    }
    else if (KUrl::List::canDecode(e->mimeData()))
    {
        if (!palbum && !m_readOnly)
        {
            return false;
        }

        // Drag & drop outside of digiKam

        KUrl::List srcURLs = KUrl::List::fromMimeData(e->mimeData());

        if (m_readOnly)
        {
            emit urlsDropped(srcURLs);
            return true;
        }

        DropAction action = copyOrMove(e, view);

        if (action == MoveAction)
        {
            KIO::Job* job = DIO::move(srcURLs, palbum);
            connect(job, SIGNAL(result(KJob*)),
                    this, SIGNAL(dioResult(KJob*)));
        }
        else if (action == CopyAction)
        {
            KIO::Job* job = DIO::copy(srcURLs, palbum);
            connect(job, SIGNAL(result(KJob*)),
                    this, SIGNAL(dioResult(KJob*)));
        }

        return true;
    }
    else if (DTagDrag::canDecode(e->mimeData()))
    {
        int tagID;

        if (!DTagDrag::decode(e->mimeData(), tagID))
        {
            return false;
        }

        TAlbum* talbum = AlbumManager::instance()->findTAlbum(tagID);

        if (!talbum)
        {
            return false;
        }

        KMenu popMenu(view);

        QList<ImageInfo> selectedInfos  = view->selectedImageInfosCurrentFirst();
        QAction* assignToSelectedAction = 0;
        QAction* assignToThisAction = 0;

        if (selectedInfos.count() > 1)
            assignToSelectedAction = popMenu.addAction(SmallIcon("tag"), 
                                                       i18n("Assign '%1' to &Selected Items", 
                                                            talbum->tagPath().mid(1)));

        if (droppedOn.isValid())
            assignToThisAction = popMenu.addAction(SmallIcon("tag"), 
                                                   i18n("Assign '%1' to &This Item", 
                                                        talbum->tagPath().mid(1)));

        QAction* assignToAllAction = popMenu.addAction(SmallIcon("tag"), 
                                                       i18n("Assign '%1' to &All Items", 
                                                            talbum->tagPath().mid(1)));

        popMenu.addSeparator();
        popMenu.addAction(SmallIcon("dialog-cancel"), i18n("&Cancel"));

        popMenu.setMouseTracking(true);
        QAction* choice = popMenu.exec(view->mapToGlobal(e->pos()));

        if (choice)
        {
            if (choice == assignToSelectedAction)    // Selected Items
            {
                emit assignTags(selectedInfos, QList<int>() << tagID);
            }
            else if (choice == assignToAllAction)    // All Items
            {
                emit assignTags(model()->imageInfos(), QList<int>() << tagID);
            }
            else if (choice == assignToThisAction)  // Dropped Item only.
            {
                emit assignTags(QList<ImageInfo>() << model()->imageInfo(droppedOn), QList<int>() << tagID);
            }
        }

        return true;
    }
    else if (DTagListDrag::canDecode(e->mimeData()))
    {
        QList<int> tagIDs;
        DTagListDrag::decode(e->mimeData(), tagIDs);

        KMenu popMenu(view);

        QList<ImageInfo> selectedInfos = view->selectedImageInfosCurrentFirst();
        QAction* assignToSelectedAction = 0;

        if (selectedInfos.count() > 1)
        {
            assignToSelectedAction = popMenu.addAction(SmallIcon("tag"), i18n("Assign Tags to &Selected Items"));
        }

        QAction* assignToThisAction = 0;

        if (droppedOn.isValid())
        {
            assignToThisAction = popMenu.addAction(SmallIcon("tag"), i18n("Assign Tags to &This Item"));
        }

        QAction* assignToAllAction = popMenu.addAction(SmallIcon("tag"), i18n("Assign Tags to &All Items"));

        popMenu.addSeparator();
        popMenu.addAction(SmallIcon("dialog-cancel"), i18n("&Cancel"));

        popMenu.setMouseTracking(true);
        QAction* choice = popMenu.exec(view->mapToGlobal(e->pos()));

        if (choice)
        {
            if (choice == assignToSelectedAction)    // Selected Items
            {
                emit assignTags(selectedInfos, tagIDs);
            }
            else if (choice == assignToAllAction)    // All Items
            {
                emit assignTags(model()->imageInfos(), tagIDs);
            }
            else if (choice == assignToThisAction)    // Dropped item only.
            {
                emit assignTags(QList<ImageInfo>() << model()->imageInfo(droppedOn), tagIDs);
            }
        }

        return true;
    }
    else if (DCameraItemListDrag::canDecode(e->mimeData()))
    {
        if (!palbum)
        {
            return false;
        }

        CameraUI* ui = dynamic_cast<CameraUI*>(e->source());

        if (!ui)
        {
            return false;
        }

        KMenu popMenu(view);
        popMenu.addTitle(SmallIcon("digikam"), i18n("My Albums"));
        QAction* downAction    = popMenu.addAction(SmallIcon("file-export"),
                                 i18n("Download From Camera"));
        QAction* downDelAction = popMenu.addAction(SmallIcon("file-export"),
                                 i18n("Download && Delete From Camera"));
        popMenu.addSeparator();
        popMenu.addAction(SmallIcon("dialog-cancel"), i18n("C&ancel"));
        popMenu.setMouseTracking(true);
        QAction* choice = popMenu.exec(view->mapToGlobal(e->pos()));

        if (choice)
        {
            if (choice == downAction)
            {
                ui->slotDownload(true, false, palbum);
            }
            else if (choice == downDelAction)
            {
                ui->slotDownload(true, true, palbum);
            }
        }

        return true;
    }

    return false;
}

Qt::DropAction ImageDragDropHandler::accepts(const QDropEvent* e, const QModelIndex& /*dropIndex*/)
{
    if (albumModel() && !albumModel()->currentAlbum())
    {
        return Qt::IgnoreAction;
    }

    if (DItemDrag::canDecode(e->mimeData()) || KUrl::List::canDecode(e->mimeData()))
    {
        if (e->keyboardModifiers() & Qt::ControlModifier)
        {
            return Qt::CopyAction;
        }
        else if (e->keyboardModifiers() & Qt::ShiftModifier)
        {
            return Qt::MoveAction;
        }

        return Qt::MoveAction;
    }

    if (DTagDrag::canDecode(e->mimeData()) ||
        DTagListDrag::canDecode(e->mimeData()) ||
        DCameraItemListDrag::canDecode(e->mimeData()) ||
        DCameraDragObject::canDecode(e->mimeData()))
    {
        return Qt::MoveAction;
    }

    return Qt::IgnoreAction;
}

QStringList ImageDragDropHandler::mimeTypes() const
{
    QStringList mimeTypes;
    mimeTypes << DItemDrag::mimeTypes()
              << DTagDrag::mimeTypes()
              << DTagListDrag::mimeTypes()
              << DCameraItemListDrag::mimeTypes()
              << DCameraDragObject::mimeTypes()
              << KUrl::List::mimeDataTypes();
    return mimeTypes;
}

QMimeData* ImageDragDropHandler::createMimeData(const QList<QModelIndex>& indexes)
{
    QList<ImageInfo> infos = model()->imageInfos(indexes);

    KUrl::List urls;
    KUrl::List kioURLs;
    QList<int> albumIDs;
    QList<qlonglong> imageIDs;

    foreach (const ImageInfo& info, infos)
    {
        urls.append(info.fileUrl());
        kioURLs.append(info.databaseUrl());
        albumIDs.append(info.albumId());
        imageIDs.append(info.id());
    }

    if (urls.isEmpty())
    {
        return 0;
    }

    return new DItemDrag(urls, kioURLs, albumIDs, imageIDs);
}

} // namespace Digikam

/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-06
 * Description : Albums folder view.
 *
 * Copyright (C) 2005-2006 by Joern Ahrens <joern dot ahrens at kdemail dot net>
 * Copyright (C) 2006-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009-2011 by Andi Clemens <andi dot clemens at gmx dot net>
 * Copyright (C) 2009-2011 by Johannes Wienke <languitar at semipol dot de>
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

#include "albumselectiontreeview.moc"

// Qt includes

#include <QAction>
#include <QEvent>
#include <QSortFilterProxyModel>
#include <QToolTip>

// KDE includes

#include <kdebug.h>
#include <kiconloader.h>
#include <kmenu.h>

// Local includes

#include "albummanager.h"
#include "contextmenuhelper.h"
#include "itemviewtooltip.h"
#include "tooltipfiller.h"
#include "batchthumbsgenerator.h"

namespace Digikam
{

class AlbumViewToolTip: public ItemViewToolTip
{
public:

    AlbumViewToolTip(AlbumSelectionTreeView* view) :
        ItemViewToolTip(view)
    {
    }

    AlbumSelectionTreeView* view() const
    {
        return static_cast<AlbumSelectionTreeView*>(ItemViewToolTip::view());
    }

protected:

    virtual QString tipContents()
    {
        PAlbum* album = view()->albumForIndex(currentIndex());
        return ToolTipFiller::albumTipContents(album, view()->albumModel()->albumCount(album));
    }
};

class AlbumSelectionTreeView::AlbumSelectionTreeViewPriv
{

public:

    AlbumSelectionTreeViewPriv() :
        enableToolTips(false),
        albumModificationHelper(0),
        toolTip(0),
        renameAction(0),
        resetIconAction(0),
        findDuplAction(0),
        rebuildThumbsAction(0)
    {
    }

    bool                     enableToolTips;

    AlbumModificationHelper* albumModificationHelper;
    AlbumViewToolTip*        toolTip;

    QAction*                 renameAction;
    QAction*                 resetIconAction;
    QAction*                 findDuplAction;
    QAction*                 rebuildThumbsAction;
};

AlbumSelectionTreeView::AlbumSelectionTreeView(QWidget* parent, AlbumModel* model,
                                               AlbumModificationHelper* albumModificationHelper)
    : AlbumTreeView(parent), d(new AlbumSelectionTreeViewPriv)
{
    setAlbumModel(model);
    d->albumModificationHelper = albumModificationHelper;
    d->toolTip                 = new AlbumViewToolTip(this);
    d->renameAction            = new QAction(SmallIcon("edit-rename"), i18n("Rename..."), this);
    d->resetIconAction         = new QAction(SmallIcon("view-refresh"), i18n("Reset Album Icon"), this);
    d->findDuplAction          = new QAction(SmallIcon("tools-wizard"), i18n("Find Duplicates..."), this);
    d->rebuildThumbsAction     = new QAction(SmallIcon("view-process-all"), i18n("Rebuild Thumbnails..."), this);

    setSortingEnabled(true);
    setSelectAlbumOnClick(true);
    setEnableContextMenu(true);
}

AlbumSelectionTreeView::~AlbumSelectionTreeView()
{
    delete d;
}

void AlbumSelectionTreeView::setEnableToolTips(bool enable)
{
    d->enableToolTips = enable;
}

QString AlbumSelectionTreeView::contextMenuTitle() const
{
    return i18n("My Albums");
}

void AlbumSelectionTreeView::addCustomContextMenuActions(ContextMenuHelper& cmh, Album* a)
{
    PAlbum* album = dynamic_cast<PAlbum*> (a);
    if (!album)
    {
        return;
    }

    d->renameAction->setEnabled(!album->isAlbumRoot());

    // --------------------------------------------------------

    cmh.addAction("album_new");
    cmh.addAction(d->renameAction);
    cmh.addAction(d->resetIconAction);
    cmh.addAction("album_openinfilemanager");
    cmh.addAction("album_openinterminal");
    cmh.addSeparator();
    // --------------------------------------------------------
    cmh.addAction(d->findDuplAction);
    cmh.addAction(d->rebuildThumbsAction);
    cmh.addImportMenu();
    cmh.addExportMenu();
    cmh.addBatchMenu();
    cmh.addAlbumActions();
    cmh.addSeparator();
    // --------------------------------------------------------
    cmh.addAction("album_delete");
    cmh.addSeparator();
    // --------------------------------------------------------
    cmh.addAction("album_propsEdit");
}

void AlbumSelectionTreeView::handleCustomContextMenuAction(QAction* action, AlbumPointer<Album> a)
{
    Album* alb    = a;
    PAlbum* album = dynamic_cast<PAlbum*> (alb);

    if (!action || !album)
    {
        return;
    }

    if (action == d->resetIconAction)
    {
        QString err;
        AlbumManager::instance()->updatePAlbumIcon(album, 0, err);
    }
    else if (action == d->renameAction)
    {
        d->albumModificationHelper->slotAlbumRename(album);
    }
    else if (action == d->findDuplAction)
    {
        kDebug() << "emitting signal for finding duplicates";
        emit signalFindDuplicatesInAlbum(album);
    }
    else if (action == d->rebuildThumbsAction)
    {
        BatchThumbsGenerator* thumbsGenerator = new BatchThumbsGenerator(this, album->id());
        thumbsGenerator->show();
    }
}

bool AlbumSelectionTreeView::viewportEvent(QEvent* event)
{
    // let the base class handle the event if it is not a tool tip request
    if (event->type() != QEvent::ToolTip)
    {
        return AlbumTreeView::viewportEvent(event);
    }

    // only show tool tips if requested
    if (!d->enableToolTips)
    {
        return false;
    }

    // check that we got a correct event
    QHelpEvent* helpEvent = dynamic_cast<QHelpEvent*> (event);

    if (!helpEvent)
    {
        kError() << "Unable to determine the correct type of the event. "
                 << "This should not happen.";
        return false;
    }

    // find the item this tool tip belongs to
    QModelIndex index = indexAt(helpEvent->pos());

    if (!index.isValid())
    {
        return true;
    }

    PAlbum* album = albumForIndex(index);

    if (!album || album->isRoot() || album->isAlbumRoot())
    {
        // there was no album so we really don't want to show a tooltip.
        return true;
    }

    QRect itemRect = visualRect(index);

    if (!itemRect.contains(helpEvent->pos()))
    {
        return true;
    }

    QStyleOptionViewItem option = viewOptions();
    option.rect                 = itemRect;
    // visualRect can be larger than viewport, intersect with viewport rect
    option.rect                 &= viewport()->rect();
    option.state                |= (index == currentIndex() ? QStyle::State_HasFocus : QStyle::State_None);
    d->toolTip->show(helpEvent, option, index);

    return true;
}

} // namespace Digikam

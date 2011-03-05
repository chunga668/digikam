/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-05-21
 * Description : Searches folder view
 *
 * Copyright (C) 2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2008-2009 by Marcel Wiesweg <marcel.wiesweg@gmx.de>
 * Copyright (C) 2006-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009 by Andi Clemens <andi dot clemens at gmx dot net>
 * Copyright (C) 2009 by Johannes Wienke <languitar at semipol dot de>
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

#include "searchfolderview.moc"

// Qt includes

#include <qaction.h>

// KDE includes

#include <kiconloader.h>

// Local includes

#include "contextmenuhelper.h"

namespace Digikam
{

class NormalSearchTreeViewPriv
{
public:

    NormalSearchTreeViewPriv() :
        newAction(0),
        editAction(0)
    {
    }

    QAction* newAction;
    QAction* editAction;

};

NormalSearchTreeView::NormalSearchTreeView(QWidget* parent,
        SearchModel* searchModel,
        SearchModificationHelper* searchModificationHelper) :
    EditableSearchTreeView(parent, searchModel, searchModificationHelper),
    d(new NormalSearchTreeViewPriv)
{

    d->newAction = new QAction(SmallIcon("document-new"),
                               i18nc("Create new search", "New..."), this);
    d->editAction = new QAction(SmallIcon("edit-find"),
                                i18nc("Edit selected search", "Edit..."), this);

}

NormalSearchTreeView::~NormalSearchTreeView()
{
    delete d;
}

void NormalSearchTreeView::addCustomContextMenuActions(ContextMenuHelper& cmh,
        Album* album)
{

    cmh.addAction(d->newAction);
    cmh.addSeparator();

    EditableSearchTreeView::addCustomContextMenuActions(cmh, album);

    SAlbum* salbum = dynamic_cast<SAlbum*> (album);

    d->editAction->setEnabled(salbum);
    cmh.addAction(d->editAction);

}

void NormalSearchTreeView::handleCustomContextMenuAction(QAction* action, AlbumPointer<Album> album)
{

    Album* a = album;
    SAlbum* salbum = dynamic_cast<SAlbum*> (a);

    if (action == d->newAction && salbum)
    {
        emit newSearch();
    }
    else if (action == d->editAction && salbum)
    {
        emit editSearch(salbum);
    }
    else
    {
        EditableSearchTreeView::handleCustomContextMenuAction(action, album);
    }

}

}
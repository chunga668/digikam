/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-06-17
 * Description : Find Duplicates tree-view search album item.
 *
 * Copyright (C) 2008-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "findduplicatesalbumitem.h"

// Qt includes

#include <QPainter>

// KDE includes

#include <klocale.h>
#include <kstringhandler.h>

// Local includes

#include "album.h"
#include "searchxml.h"

namespace Digikam
{

class FindDuplicatesAlbumItem::FindDuplicatesAlbumItemPriv
{

public:

    FindDuplicatesAlbumItemPriv() :
        album(0)
    {
    }

    SAlbum*   album;
    ImageInfo refImgInfo;
};

FindDuplicatesAlbumItem::FindDuplicatesAlbumItem(QTreeWidget* parent, SAlbum* album)
    : QTreeWidgetItem(parent), d(new FindDuplicatesAlbumItemPriv)
{
    d->album = album;

    if (d->album)
    {
        d->refImgInfo = ImageInfo(d->album->title().toLongLong());
        setText(0, d->refImgInfo.name());

        SearchXmlReader reader(d->album->query());
        reader.readToFirstField();
        QList<int> list;
        list << reader.valueToIntList();
        setText(1, QString::number(list.count()));
    }
}

FindDuplicatesAlbumItem::~FindDuplicatesAlbumItem()
{
    delete d;
}

void FindDuplicatesAlbumItem::setThumb(const QPixmap& pix)
{
    int iconSize = treeWidget()->iconSize().width();
    QPixmap pixmap(iconSize+2, iconSize+2);
    pixmap.fill(Qt::transparent);
    QPainter p(&pixmap);
    p.drawPixmap((pixmap.width()/2)  - (pix.width()/2),
                 (pixmap.height()/2) - (pix.height()/2), pix);

    QIcon icon = QIcon(pixmap);
    //  We make sure the preview icon stays the same regardless of the role
    icon.addPixmap(pixmap, QIcon::Selected, QIcon::On);
    icon.addPixmap(pixmap, QIcon::Selected, QIcon::Off);
    icon.addPixmap(pixmap, QIcon::Active,   QIcon::On);
    icon.addPixmap(pixmap, QIcon::Active,   QIcon::Off);
    icon.addPixmap(pixmap, QIcon::Normal,   QIcon::On);
    icon.addPixmap(pixmap, QIcon::Normal,   QIcon::Off);
    setIcon(0, icon);
}

SAlbum* FindDuplicatesAlbumItem::album() const
{
    return d->album;
}

KUrl FindDuplicatesAlbumItem::refUrl() const
{
    return d->refImgInfo.fileUrl();
}

bool FindDuplicatesAlbumItem::operator<(const QTreeWidgetItem& other) const
{
    int column = treeWidget()->sortColumn();
    int result = KStringHandler::naturalCompare(text(column), other.text(column));

    if (result < 0)
    {
        return true;
    }

    return false;
}

}  // namespace Digikam

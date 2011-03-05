/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-11-17
 * Description : Albums history manager.
 *
 * Copyright (C) 2004 by Joern Ahrens <joern.ahrens@kdemail.net>
 * Copyright (C) 2006-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef ALBUMHISTORY_H
#define ALBUMHISTORY_H

/** @file albumhistory.h */

// Qt includes

#include <QList>
#include <QMap>
#include <QObject>
#include <QStringList>

//KDE includes

#include <KUrl>

namespace Digikam
{

class Album;
class HistoryItem;
class HistoryPosition;
class ImageInfo;
class ImageInfoList;

/**
 * Manages the history of the last visited albums.
 *
 * The user is able to navigate through the albums, he has
 * opened during a session.
 */
class AlbumHistory : public QObject
{
    Q_OBJECT

public:

    AlbumHistory();
    ~AlbumHistory();

    void            addAlbum(Album* album, QWidget* widget = 0);
    void            deleteAlbum(Album* album);
    void            clearHistory();
    void            back(Album** album, QWidget** widget, unsigned int steps=1);
    void            forward(Album** album, QWidget** widget, unsigned int steps=1);
    void            getCurrentAlbum(Album** album, QWidget** widget) const;

    void            getBackwardHistory(QStringList& list) const;
    void            getForwardHistory(QStringList& list) const;

    bool            isForwardEmpty() const;
    bool            isBackwardEmpty() const;

Q_SIGNALS:

    void            signalSetCurrent(qlonglong imageId);
    void            signalSetSelectedInfos(const QList<ImageInfo>&);

public Q_SLOTS:

    void            slotAlbumCurrentChanged();
    void            slotAlbumDeleted(Album* album);
    void            slotAlbumSelected();
    void            slotClearSelectPAlbum(const ImageInfo& imageInfo);
    void            slotClearSelectTAlbum(int id);
    void            slotCurrentChange(const ImageInfo& info);
    void            slotImageSelected(const ImageInfoList& selectedImage);

private:

    HistoryItem*    getCurrentAlbum() const;
    void            forward(unsigned int steps=1);

private:

    class AlbumHistoryPriv;
    AlbumHistoryPriv* const d;
};

}  // namespace Digikam

#endif /* ALBUMHISTORY_H */

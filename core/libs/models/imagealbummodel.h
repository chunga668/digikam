/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-03-08
 * Description : Qt item model for database entries, listing done with ioslave
 *
 * Copyright (C) 2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef IMAGEALBUMMODEL_H
#define IMAGEALBUMMODEL_H

// Local includes

#include "imagethumbnailmodel.h"
#include "album.h"

namespace KIO
{
class Job;
}
class KJob;

namespace Digikam
{

class ImageChangeset;
class CollectionImageChangeset;
class SearchChangeset;
class Album;
class ImageAlbumModelPriv;

class ImageAlbumModel : public ImageThumbnailModel
{
    Q_OBJECT

public:

    ImageAlbumModel(QObject* parent = 0);
    ~ImageAlbumModel();

    Album* currentAlbum() const;

    bool hasScheduledRefresh() const;
    bool isRecursingAlbums() const;
    bool isRecursingTags() const;

public Q_SLOTS:

    /**
     * Call this method to populate the model with data from the given album.
     * If called with 0, the model will be empty.
     * Opening the same album again is a no-op.
     */
    void openAlbum(Album* album);
    /** Reloads the current album */
    void refresh();

    void setRecurseAlbums(bool recursiveListing);
    void setRecurseTags(bool recursiveListing);

    void setSpecialTagListing(const QString& specialListing);

Q_SIGNALS:

    void listedAlbumChanged(Album* album);

protected Q_SLOTS:

    void scheduleRefresh();
    void scheduleIncrementalRefresh();

    void slotResult(KJob* job);
    void slotData(KIO::Job* job, const QByteArray& data);

    void slotNextRefresh();
    void slotNextIncrementalRefresh();

    virtual void slotImageChange(const ImageChangeset& changeset);
    virtual void slotImageTagChange(const ImageTagChangeset& changeset);
    void slotCollectionImageChange(const CollectionImageChangeset& changeset);
    void slotSearchChange(const SearchChangeset& changeset);

    void slotAlbumAdded(Album* album);
    void slotAlbumDeleted(Album* album);
    void slotAlbumRenamed(Album* album);
    void slotAlbumsCleared();

    void incrementalRefresh();

protected:

    void startListJob(Album* album);

private:

    ImageAlbumModelPriv* const d;
};

} // namespace Digikam

#endif // IMAGEALBUMMODEL_H

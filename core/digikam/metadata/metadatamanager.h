/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-05-05
 * Description : Metadata operations on images
 *
 * Copyright (C) 2009-2011 by Marcel Wiesweg <marcel.wiesweg@gmx.de>
 * Copyright (C) 2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef METADATAMANAGER_H
#define METADATAMANAGER_H

// Local includes

#include "imageinfo.h"

namespace Digikam
{

class MetadataHub;
class MetadataHubOnTheRoad;

class MetadataManager : public QObject
{
    Q_OBJECT

public:

    enum GroupAction
    {
        AddToGroup,
        RemoveFromGroup,
        SplitGroup
    };

    static MetadataManager* instance();

    bool requestShutDown();
    void shutDown();

public Q_SLOTS:

    void assignTag(const ImageInfo& info, int tagID);
    void assignTag(const QList<ImageInfo>& infos, int tagID);
    void assignTags(const ImageInfo& info, const QList<int>& tagIDs);
    void assignTags(const QList<ImageInfo>& infos, const QList<int>& tagIDs);
    void assignTags(const QList<qlonglong>& imageIds, const QList<int>& tagIDs);

    void removeTag(const ImageInfo& info, int tagID);
    void removeTag(const QList<ImageInfo>& infos, int tagID);
    void removeTags(const ImageInfo& info, const QList<int>& tagIDs);
    void removeTags(const QList<ImageInfo>& infos, const QList<int>& tagIDs);

    void assignPickLabel(const ImageInfo& infos, int pickId);
    void assignPickLabel(const QList<ImageInfo>& infos, int pickId);

    void assignColorLabel(const ImageInfo& infos, int colorId);
    void assignColorLabel(const QList<ImageInfo>& infos, int colorId);

    void assignRating(const ImageInfo& infos, int rating);
    void assignRating(const QList<ImageInfo>& infos, int rating);

    void addToGroup(const ImageInfo& pick, const QList<ImageInfo>& infos);
    void removeFromGroup(const ImageInfo& info);
    void removeFromGroup(const QList<ImageInfo>& infos);
    void ungroup(const ImageInfo& info);
    void ungroup(const QList<ImageInfo>& infos);

    void setExifOrientation(const QList<ImageInfo>& infos, int orientation);
    void applyMetadata(const QList<ImageInfo>& infos, const MetadataHub& hub);
    void applyMetadata(const QList<ImageInfo>& infos, const MetadataHubOnTheRoad& hub);

Q_SIGNALS:

    void progressMessageChanged(const QString& descriptionOfAction);
    void progressValueChanged(float percent);
    void progressFinished();

    void orientationChangeFailed(const QStringList& failedFileNames);

public:

    // Declared public due to use by MetadataManagerWorker, MetadataManagerDatabaseWorker, and MetadataManagerFileWorker
    class MetadataManagerPriv;

private:

    friend class MetadataManagerCreator;
    MetadataManager();
    ~MetadataManager();

    MetadataManagerPriv* const d;
};

} // namespace Digikam

#endif /* METADATAMANAGER_H */

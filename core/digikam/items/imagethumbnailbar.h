/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-02-06
 * Description : Thumbnail bar for images
 *
 * Copyright (C) 2009-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef IMAGETHUMBNAILBAR_H
#define IMAGETHUMBNAILBAR_H

// Local includes

#include "imagecategorizedview.h"

class QMimeData;

namespace Digikam
{

class ImageViewUtilities;
class ImageThumbnailBarPriv;

class ImageThumbnailBar : public ImageCategorizedView
{
    Q_OBJECT

public:

    ImageThumbnailBar(QWidget* parent = 0);
    ~ImageThumbnailBar();

    /// Sets the policy always for the one scroll bar which is relevant, depending on orientation
    void setScrollBarPolicy(Qt::ScrollBarPolicy policy);
    void setFlow(QListView::Flow newFlow);

    void installRatingOverlay();

    /**
     * This installs a duplicate filter model, if the ImageModel may contain duplicates.
     * Otherwise, just use setModels().
     */
    void setModelsFiltered(ImageModel* model, ImageSortFilterModel* filterModel);

public Q_SLOTS:

    void assignRating(const QModelIndex& index, int rating);
    void slotDockLocationChanged(Qt::DockWidgetArea area);

protected:

    virtual void slotSetupChanged();
    virtual bool event(QEvent*);

private:

    ImageThumbnailBarPriv* const d;
};

} // namespace Digikam

#endif /* IMAGETHUMBNAILBAR_H */

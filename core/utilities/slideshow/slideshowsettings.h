/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-02-13
 * Description : slide show settings container.
 *
 * Copyright (C) 2007-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef SLIDESHOWSETTINGSCONTAINER_H
#define SLIDESHOWSETTINGSCONTAINER_H

// Qt includes

#include <QMap>
#include <QColor>
#include <QString>

// KDE includes

#include <kurl.h>

// Local includes

#include "photoinfocontainer.h"
#include "digikam_export.h"

namespace Digikam
{

/** This class contain the information of one picture to slide */
class DIGIKAM_EXPORT SlidePictureInfo
{

public:

    SlidePictureInfo()
    {
        rating     = 0;
        colorLabel = 0;
        pickLabel  = 0;
    };

    ~SlidePictureInfo() {};

public:

    int                rating;
    int                colorLabel;
    int                pickLabel;

    /** Picture Comment */
    QString            comment;

    /** Exif photo info of picture */
    PhotoInfoContainer photoInfo;
};

// --------------------------------------------------------------------------------

/** This class contain all settings to perform a slide show of a group of pictures */
class DIGIKAM_EXPORT SlideShowSettings
{

public:

    SlideShowSettings()
    {
        exifRotate           = true;
        printName            = true;
        printDate            = false;
        printComment         = false;
        printLabels          = false;
        printApertureFocal   = false;
        printMakeModel       = false;
        printExpoSensitivity = false;
        loop                 = false;
        delay                = 5;
    };

    ~SlideShowSettings() {};

public:

    // Global Slide Show Settings

    /** Auto-rotate image accordingly with Exif Rotation tag */
    bool                         exifRotate;

    /** Print picture file name during slide */
    bool                         printName;

    /** Print picture creation date during slide */
    bool                         printDate;

    /** Print camera Aperture and Focal during slide */
    bool                         printApertureFocal;

    /** Print camera Make and Model during slide */
    bool                         printMakeModel;

    /** Print camera Exposure and Sensitivity during slide */
    bool                         printExpoSensitivity;

    /** Print picture comment during slide */
    bool                         printComment;

    /** Print color label, pick label, and rating during slide */
    bool                         printLabels;

    /** Print color label during slide */
    bool                         printColorLabel;

    /** Slide pictures in loop */
    bool                         loop;

    /** Delay in seconds */
    int                          delay;

    /** List of pictures URL to slide */
    KUrl::List                   fileList;

    /** Map of pictures information to slide */
    QMap<KUrl, SlidePictureInfo> pictInfoMap;
};

}  // namespace Digikam

#endif  // SLIDESHOWSETTINGSCONTAINER_H

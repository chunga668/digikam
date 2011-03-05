/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-08-20
 * Description : Metadata Settings Container.
 *
 * Copyright (C) 2010-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "metadatasettingscontainer.h"

// KDE includes

#include <kconfiggroup.h>

// LibKExiv2 includes

#include <libkexiv2/kexiv2.h>

// Local includes

#include "metadatasettings.h"

using namespace KExiv2Iface;

namespace Digikam
{

MetadataSettingsContainer::MetadataSettingsContainer()
{
    exifRotate            = true;
    exifSetOrientation    = true;
    saveComments          = false;
    saveDateTime          = false;
    savePickLabel        = false;
    saveColorLabel        = false;
    saveRating            = false;
    saveTemplate          = false;
    saveTags              = false;
    writeRawFiles         = false;
    useXMPSidecar4Reading = false;
    metadataWritingMode   = KExiv2::WRITETOIMAGEONLY;
    updateFileTimeStamp   = false;
}

void MetadataSettingsContainer::readFromConfig(KConfigGroup& group)
{
    exifRotate            = group.readEntry("EXIF Rotate",                 true);
    exifSetOrientation    = group.readEntry("EXIF Set Orientation",        true);

    saveTags              = group.readEntry("Save Tags",                   false);
    saveTemplate          = group.readEntry("Save Template",               false);

    saveComments          = group.readEntry("Save EXIF Comments",          false);
    saveDateTime          = group.readEntry("Save Date Time",              false);
    savePickLabel         = group.readEntry("Save Pick Label",             false);
    saveColorLabel        = group.readEntry("Save Color Label",            false);
    saveRating            = group.readEntry("Save Rating",                 false);

    writeRawFiles         = group.readEntry("Write RAW Files",             false);
    useXMPSidecar4Reading = group.readEntry("Use XMP Sidecar For Reading", false);
    metadataWritingMode   = group.readEntry("Metadata Writing Mode",       (int)KExiv2::WRITETOIMAGEONLY);
    updateFileTimeStamp   = group.readEntry("Update File Timestamp",       false);
}

void MetadataSettingsContainer::writeToConfig(KConfigGroup& group) const
{
    group.writeEntry("EXIF Rotate",                 exifRotate);
    group.writeEntry("EXIF Set Orientation",        exifSetOrientation);

    group.writeEntry("Save Tags",                   saveTags);
    group.writeEntry("Save Template",               saveTemplate);

    group.writeEntry("Save EXIF Comments",          saveComments);
    group.writeEntry("Save Date Time",              saveDateTime);
    group.writeEntry("Save Pick Label",             savePickLabel);
    group.writeEntry("Save Color Label",            saveColorLabel);
    group.writeEntry("Save Rating",                 saveRating);

    group.writeEntry("Write RAW Files",             writeRawFiles);
    group.writeEntry("Use XMP Sidecar For Reading", useXMPSidecar4Reading);
    group.writeEntry("Metadata Writing Mode",       metadataWritingMode);
    group.writeEntry("Update File Timestamp",       updateFileTimeStamp);
}

}  // namespace Digikam

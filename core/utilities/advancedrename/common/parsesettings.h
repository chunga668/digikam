/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-09-12
 * Description : parse settings class
 *
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

#ifndef PARSESETTINGS_H
#define PARSESETTINGS_H

// Qt includes

#include <QDateTime>
#include <QFileInfo>
#include <QString>

// Local includes

#include "imageinfo.h"
#include "parseresults.h"
#include "advancedrenamemanager.h"

namespace Digikam
{

class ParseSettings
{
public:

    // default ctors
    ParseSettings()
    {
        init();
    };
    ParseSettings(const QString& _parseString) :
        parseString(_parseString)
    {
        init();
    };

    // ImageInfo ctors
    ParseSettings(const ImageInfo& info)
    {
        init(info);
    };
    ParseSettings(const QString& _parseString, const ImageInfo& info) :
        parseString(_parseString)
    {
        init(info);
    };

    // --------------------------------------------------------

    bool isValid()
    {
        QFileInfo fi(fileUrl.toLocalFile());
        return fi.isReadable();
    };

public:

    KUrl                     fileUrl;
    QString                  parseString;
    QString                  str2Modify;
    QDateTime                creationTime;
    ParseResults             results;
    ParseResults             invalidModifiers;
    ParseResults::ResultsKey currentResultsKey;

    int                      startIndex;
    bool                     useOriginalFileExtension;
    bool                     applyModifiers;
    AdvancedRenameManager*   manager;

private:

    void init()
    {
        startIndex               = 1;
        useOriginalFileExtension = true;
        applyModifiers           = true;
        str2Modify.clear();
        manager                  = 0;
    };

    void init(const ImageInfo& info)
    {
        init();
        fileUrl = info.fileUrl();
    }
};

} // namespace Digikam

#endif /* PARSESETTINGS_H */

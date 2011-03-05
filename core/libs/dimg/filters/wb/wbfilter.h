/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-16-01
 * Description : white balance color correction.
 *
 * Copyright (C) 2007-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2008 by Guillaume Castagnino <casta at xwing dot info>
 * Copyright (C) 2010 by Martin Klapetek <martin dot klapetek at gmail dot com>
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

#ifndef WBFILTER_H
#define WBFILTER_H

// Qt includes.

#include <QColor>

// Local includes

#include "digikam_export.h"
#include "dimgthreadedfilter.h"
#include "globals.h"
#include "wbcontainer.h"

using namespace Digikam;

namespace Digikam
{

class DImg;
class WBFilterPriv;

class DIGIKAM_EXPORT WBFilter : public DImgThreadedFilter
{

public:
    explicit WBFilter(QObject* parent = 0);
    explicit WBFilter(DImg* orgImage, QObject* parent=0, const WBContainer& settings=WBContainer());
    explicit WBFilter(const WBContainer& settings, DImgThreadedFilter* master, const DImg& orgImage, const DImg& destImage,
                      int progressBegin=0, int progressEnd=100);
    virtual ~WBFilter();

    static void autoExposureAdjustement(const DImg* img, double& black, double& expo);
    static void autoWBAdjustementFromColor(const QColor& tc, double& temperature, double& green);

    static QString          FilterIdentifier()
    {
        return "digikam:WhiteBalanceFilter";
    }
    static QString          DisplayableName()
    {
        return I18N_NOOP("White Balance Tool");
    }
    static QList<int>       SupportedVersions()
    {
        return QList<int>() << 1;
    }
    static int              CurrentVersion()
    {
        return 1;
    }
    virtual QString         filterIdentifier() const
    {
        return FilterIdentifier();
    }
    virtual FilterAction    filterAction();
    void                    readParameters(const FilterAction& action);

protected:

    void filterImage();

protected:

    WBContainer m_settings;

private:

    void setRGBmult();
    void setLUTv();
    void adjustWhiteBalance(uchar* data, int width, int height, bool sixteenBit);
    inline unsigned short pixelColor(int colorMult, int index, int value);

    static void setRGBmult(double& temperature, double& green, float& mr, float& mg, float& mb);

private:

    WBFilterPriv* const d;
};

}  // namespace Digikam

#endif /* WBFILTER_H */

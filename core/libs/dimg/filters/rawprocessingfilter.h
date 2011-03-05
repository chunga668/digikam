/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-12-14
 * Description : Filter to manage and help with raw loading
 *
 * Copyright (C) 2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef RAWPROCESSINGFILTERS_H
#define RAWPROCESSINGFILTERS_H

// Qt includes

// Local includes

#include "digikam_export.h"
#include "dimgthreadedfilter.h"
#include "drawdecoding.h"
#include "iccprofile.h"

namespace Digikam
{

class DImgLoaderObserver;
class FilterAction;

class DIGIKAM_EXPORT RawProcessingFilter : public DImgThreadedFilter
{
public:

    /**
     * This is a special filter.
     * It implements RAW post processing.
     * Additionally, it provides some facilities for use from the DImg Raw loader.
     *
     * The original image shall come from libkdcraw without further modification.
     */

    /**
     * Default constructor. You need to call setSettings() and setOriginalImage()
     * before starting the filter.
     */
    RawProcessingFilter(QObject* parent = 0);

    /**
     * Traditional constructor
     */
    RawProcessingFilter(DImg* orgImage, QObject* parent, const DRawDecoding& settings,
                        const QString& name = QString());

    /**
     * For use with a master filter. Computation is started immediately.
     */
    RawProcessingFilter(const DRawDecoding& settings,
                        DImgThreadedFilter* master, const DImg& orgImage, const DImg& destImage,
                        int progressBegin=0, int progressEnd=100, const QString& name=QString());

    /**
     * Set the raw decoding settings. The post processing is carried out here,
     * the libraw settings are needed to construct the FilterAction.
     */
    void setSettings(const DRawDecoding& settings);
    DRawDecoding settings() const;

    /**
     * As additional and first post-processing step, convert the image's
     * color space to the specified profile.
     */
    void setOutputProfile(const IccProfile& profile);

    /**
     * Normally, filters post progress and are cancelled by DynamicThread facilities.
     * Here, as an alternative, a DImgLoaderObserver is set. It's continueQuery is called
     * and progress is posted in the given interval.
     */
    void setObserver(DImgLoaderObserver *observer, int progressBegin, int progressEnd);

    static QString          FilterIdentifier()
    {
        return "digikam:RawConverter";
    }
    static QString          DisplayableName()
    {
        return I18N_NOOP("Raw Conversion");
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

    virtual void filterImage();

    void postProgress(int); // not virtual
    bool continueQuery() const; // not virtual

    DRawDecoding        m_settings;
    IccProfile          m_customOutputProfile;
    DImgLoaderObserver* m_observer;

};

} // namespace Digikam

#endif // RAWPROCESSINGFILTERS_H

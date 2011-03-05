/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-17-07
 * Description : An invert image threaded image filter.
 *
 * Copyright (C) 2005-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "invertfilter.h"

// KDE includes

#include <kdebug.h>

namespace Digikam
{

InvertFilter::InvertFilter(QObject* parent)
    : DImgThreadedFilter(parent)
{
    initFilter();
}

InvertFilter::InvertFilter(DImg* orgImage, QObject* parent)
    : DImgThreadedFilter(orgImage, parent, "InvertFilter")
{
    initFilter();
}

InvertFilter::InvertFilter(DImgThreadedFilter* parentFilter,
                           const DImg& orgImage, const DImg& destImage,
                           int progressBegin, int progressEnd)
    : DImgThreadedFilter(parentFilter, orgImage, destImage, progressBegin, progressEnd,
                         parentFilter->filterName() + ": InvertFilter")
{
    filterImage();
}

InvertFilter::~InvertFilter()
{
    cancelFilter();
}

/** Performs image colors inversion. This tool is used for negate image
    resulting of a positive film scanned.
 */
void InvertFilter::filterImage()
{
    m_destImage.putImageData(m_orgImage.bits());

    if (!m_destImage.sixteenBit())        // 8 bits image.
    {
        uchar* ptr = m_destImage.bits();

        for (uint i = 0 ; i < m_destImage.numPixels() ; ++i)
        {
            ptr[0] = 255 - ptr[0];
            ptr[1] = 255 - ptr[1];
            ptr[2] = 255 - ptr[2];
            ptr[3] = 255 - ptr[3];
            ptr += 4;
        }
    }
    else               // 16 bits image.
    {
        unsigned short* ptr = (unsigned short*)m_destImage.bits();

        for (uint i = 0 ; i < m_destImage.numPixels() ; ++i)
        {
            ptr[0] = 65535 - ptr[0];
            ptr[1] = 65535 - ptr[1];
            ptr[2] = 65535 - ptr[2];
            ptr[3] = 65535 - ptr[3];
            ptr += 4;
        }
    }
}

FilterAction InvertFilter::filterAction()
{
    FilterAction action(FilterIdentifier(), CurrentVersion());
    action.setDisplayableName(DisplayableName());

    return action;
}

void InvertFilter::readParameters(const Digikam::FilterAction& /*action*/)
{
}

}  // namespace Digikam
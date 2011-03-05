/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-02-14
 * Description : image data interface for image plugins
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2004-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "imageiface.h"

// Qt includes

#include <QWidget>
#include <QSize>
#include <QPixmap>
#include <QBitmap>
#include <QPainter>

// KDE includes

#include <kdebug.h>

// Local includes

#include "exposurecontainer.h"
#include "iccmanager.h"
#include "iccsettingscontainer.h"
#include "icctransform.h"
#include "dimginterface.h"
#include "dmetadata.h"

namespace Digikam
{

class ImageIface::ImageIfacePriv
{
public:

    ImageIfacePriv() :
        usePreviewSelection(false),
        originalWidth(0),
        originalHeight(0),
        originalBytesDepth(0),
        constrainWidth(0),
        constrainHeight(0),
        previewWidth(0),
        previewHeight(0)
    {
    }

    QPixmap checkPixmap();

public:

    bool    usePreviewSelection;

    int     originalWidth;
    int     originalHeight;
    int     originalBytesDepth;

    int     constrainWidth;
    int     constrainHeight;

    int     previewWidth;
    int     previewHeight;

    QPixmap qcheck;

    DImg    previewImage;
    DImg    targetPreviewImage;
};

QPixmap ImageIface::ImageIfacePriv::checkPixmap()
{
    if (qcheck.isNull())
    {
        qcheck = QPixmap(8, 8);

        QPainter p;
        p.begin(&qcheck);
        p.fillRect(0, 0, 4, 4, QColor(144, 144, 144));
        p.fillRect(4, 4, 4, 4, QColor(144, 144, 144));
        p.fillRect(0, 4, 4, 4, QColor(100, 100, 100));
        p.fillRect(4, 0, 4, 4, QColor(100, 100, 100));
        p.end();
    }

    return qcheck;
}

ImageIface::ImageIface(int w, int h)
    : d(new ImageIfacePriv)
{
    d->constrainWidth     = w;
    d->constrainHeight    = h;
    d->originalWidth      = DImgInterface::defaultInterface()->origWidth();
    d->originalHeight     = DImgInterface::defaultInterface()->origHeight();
    d->originalBytesDepth = DImgInterface::defaultInterface()->bytesDepth();
}

ImageIface::~ImageIface()
{
    delete d;
}

void ImageIface::setPreviewType(bool useSelect)
{
    d->usePreviewSelection = useSelect;
}

bool ImageIface::previewType()
{
    return d->usePreviewSelection;
}

DColor ImageIface::getColorInfoFromOriginalImage(const QPoint& point)
{
    if ( !DImgInterface::defaultInterface()->getImage() || point.x() > originalWidth() || point.y() > originalHeight() )
    {
        kWarning() << "Coordinate out of range or no image data available!";
        return DColor();
    }

    return DImgInterface::defaultInterface()->getImg()->getPixelColor(point.x(), point.y());
}

DColor ImageIface::getColorInfoFromPreviewImage(const QPoint& point)
{
    if ( d->previewImage.isNull() || point.x() > previewWidth() || point.y() > previewHeight() )
    {
        kWarning() << "Coordinate out of range or no image data available!";
        return DColor();
    }

    return d->previewImage.getPixelColor(point.x(), point.y());
}

DColor ImageIface::getColorInfoFromTargetPreviewImage(const QPoint& point)
{
    if ( d->targetPreviewImage.isNull() || point.x() > previewWidth() || point.y() > previewHeight() )
    {
        kWarning() << "Coordinate out of range or no image data available!";
        return DColor();
    }

    return d->targetPreviewImage.getPixelColor(point.x(), point.y());
}

uchar* ImageIface::setPreviewImageSize(int w, int h) const
{
    d->previewImage.reset();
    d->targetPreviewImage.reset();

    d->constrainWidth  = w;
    d->constrainHeight = h;

    return (getPreviewImage());
}

uchar* ImageIface::getPreviewImage() const
{
    if (d->previewImage.isNull())
    {
        DImg* im = 0;

        if (!d->usePreviewSelection)
        {
            im = DImgInterface::defaultInterface()->getImg();

            if (!im || im->isNull())
            {
                return 0;
            }
        }
        else
        {
            int    x, y, w, h;
            bool   s    = DImgInterface::defaultInterface()->sixteenBit();
            bool   a    = DImgInterface::defaultInterface()->hasAlpha();
            uchar* data = DImgInterface::defaultInterface()->getImageSelection();
            DImgInterface::defaultInterface()->getSelectedArea(x, y, w, h);
            im          = new DImg(w, h, s, a, data, true);
            delete [] data;

            if (!im)
            {
                return 0;
            }

            if (im->isNull())
            {
                delete im;
                return 0;
            }

            im->setIccProfile(DImgInterface::defaultInterface()->getEmbeddedICC());
        }

        QSize sz(im->width(), im->height());
        sz.scale(d->constrainWidth, d->constrainHeight, Qt::KeepAspectRatio);

        d->previewImage  = im->smoothScale(sz.width(), sz.height());
        d->previewWidth  = d->previewImage.width();
        d->previewHeight = d->previewImage.height();

        // only create another copy if needed, in putPreviewImage
        d->targetPreviewImage = d->previewImage;

        if (d->usePreviewSelection)
        {
            delete im;
        }
    }

    DImg previewData = d->previewImage.copyImageData();
    return previewData.stripImageData();
}

DImg ImageIface::getPreviewImg()
{
    DImg preview(previewWidth(), previewHeight(), previewSixteenBit(), previewHasAlpha(), getPreviewImage());
    return preview;
}

uchar* ImageIface::getOriginalImage() const
{
    DImg* im = DImgInterface::defaultInterface()->getImg();

    if (!im || im->isNull())
    {
        return 0;
    }

    DImg origData = im->copyImageData();
    return origData.stripImageData();
}

DImg* ImageIface::getOriginalImg() const
{
    return DImgInterface::defaultInterface()->getImg();
}

uchar* ImageIface::getImageSelection() const
{
    return DImgInterface::defaultInterface()->getImageSelection();
}

void ImageIface::putPreviewImage(uchar* data)
{
    if (!data)
    {
        return;
    }

    d->targetPreviewImage.detach();
    d->targetPreviewImage.putImageData(data);
}

void ImageIface::putPreviewIccProfile(const IccProfile& profile)
{
    d->targetPreviewImage.detach();
    d->targetPreviewImage.setIccProfile(profile);
}

void ImageIface::putOriginalImage(const QString& caller, const FilterAction& action, uchar* data, int w, int h)
{
    if (!data)
    {
        return;
    }

    DImgInterface::defaultInterface()->putImage(caller, action, data, w, h);
}

void ImageIface::putOriginalIccProfile(const IccProfile& profile)
{
    DImgInterface::defaultInterface()->putIccProfile( profile );
}

void ImageIface::putImageSelection(const QString& caller, const FilterAction& action, uchar* data)
{
    if (!data)
    {
        return;
    }

    DImgInterface::defaultInterface()->putImageSelection(caller, action, data);
}

int ImageIface::previewWidth()
{
    return d->previewWidth;
}

int ImageIface::previewHeight()
{
    return d->previewHeight;
}

bool ImageIface::previewSixteenBit()
{
    return originalSixteenBit();
}

bool ImageIface::previewHasAlpha()
{
    return originalHasAlpha();
}

int ImageIface::originalWidth()
{
    return DImgInterface::defaultInterface()->origWidth();
}

int ImageIface::originalHeight()
{
    return DImgInterface::defaultInterface()->origHeight();
}

bool ImageIface::originalSixteenBit()
{
    return DImgInterface::defaultInterface()->sixteenBit();
}

bool ImageIface::originalHasAlpha()
{
    return DImgInterface::defaultInterface()->hasAlpha();
}

int ImageIface::selectedWidth()
{
    int x, y, w, h;
    DImgInterface::defaultInterface()->getSelectedArea(x, y, w, h);
    return w;
}

int ImageIface::selectedHeight()
{
    int x, y, w, h;
    DImgInterface::defaultInterface()->getSelectedArea(x, y, w, h);
    return h;
}

int ImageIface::selectedXOrg()
{
    int x, y, w, h;
    DImgInterface::defaultInterface()->getSelectedArea(x, y, w, h);
    return x;
}

int ImageIface::selectedYOrg()
{
    int x, y, w, h;
    DImgInterface::defaultInterface()->getSelectedArea(x, y, w, h);
    return y;
}

void ImageIface::convertOriginalColorDepth(int depth)
{
    DImgInterface::defaultInterface()->convertDepth(depth);
}

QPixmap ImageIface::convertToPixmap(DImg& img)
{
    return DImgInterface::defaultInterface()->convertToPixmap(img);
}

IccProfile ImageIface::getOriginalIccProfile()
{
    return DImgInterface::defaultInterface()->getEmbeddedICC();
}

KExiv2Data ImageIface::getOriginalMetadata()
{
    return DImgInterface::defaultInterface()->getImg()->getMetadata();
}

void ImageIface::setOriginalMetadata(const KExiv2Data& meta)
{
    DImgInterface::defaultInterface()->getImg()->setMetadata(meta);
}

PhotoInfoContainer ImageIface::getPhotographInformation() const
{
    DMetadata meta(DImgInterface::defaultInterface()->getImg()->getMetadata());
    return meta.getPhotographInformation();
}

void ImageIface::paint(QPaintDevice* device, int x, int y, int w, int h, QPainter* painter)
{
    QPainter localPainter;
    QPainter* p=0;

    if (painter)
    {
        p = painter;
    }
    else
    {
        p = &localPainter;
        p->begin(device);
    }

    int width  = w > 0 ? qMin(d->previewWidth, w)  : d->previewWidth;
    int height = h > 0 ? qMin(d->previewHeight, h) : d->previewHeight;

    if ( !d->targetPreviewImage.isNull() )
    {
        if (d->targetPreviewImage.hasAlpha())
        {
            p->drawTiledPixmap(x, y, width, height, d->checkPixmap());
        }

        QPixmap pixImage;
        ICCSettingsContainer iccSettings = DImgInterface::defaultInterface()->getICCSettings();

        if (iccSettings.enableCM && iccSettings.useManagedView)
        {
            IccManager manager(d->targetPreviewImage);
            IccTransform monitorICCtrans = manager.displayTransform();
            pixImage = d->targetPreviewImage.convertToPixmap(monitorICCtrans);
        }
        else
        {
            pixImage = d->targetPreviewImage.convertToPixmap();
        }

        p->drawPixmap(x, y, pixImage, 0, 0, width, height);

        // Show the Over/Under exposure pixels indicators

        ExposureSettingsContainer* expoSettings = DImgInterface::defaultInterface()->getExposureSettings();

        if (expoSettings->underExposureIndicator || expoSettings->overExposureIndicator)
        {
            ExposureSettingsContainer* expoSettings = DImgInterface::defaultInterface()->getExposureSettings();
            QImage pureColorMask                    = d->targetPreviewImage.pureColorMask(expoSettings);
            QPixmap pixMask                         = QPixmap::fromImage(pureColorMask);
            p->drawPixmap(x, y, pixMask, 0, 0, width, height);
        }
    }

    if (!painter)
    {
        p->end();
    }
}

}   // namespace Digikam

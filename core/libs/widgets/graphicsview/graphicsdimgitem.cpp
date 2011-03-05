/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-04-30
 * Description : Graphics View item for DImg
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

#include "graphicsdimgitem.moc"
#include "dimgitemspriv.h"

// Qt includes

#include <QPainter>
#include <QPoint>
#include <QRect>
#include <QStyleOptionGraphicsItem>

// KDE includes

#include <kdebug.h>

// Local includes

#include "dimg.h"
#include "imagezoomsettings.h"

namespace Digikam
{

CachedPixmaps::CachedPixmaps(int maxCount)
    : maxCount(maxCount)
{
}

CachedPixmaps::~CachedPixmaps()
{
    clear();
}

void CachedPixmaps::setMaxCount(int count)
{
    maxCount = count;
}

void CachedPixmaps::clear()
{
    foreach (const CachedPixmapKey& key, keys)
    {
        QPixmapCache::remove(key.key);
    }
    keys.clear();
}

bool CachedPixmaps::find(const QRect& region, QPixmap* pix, QRect* source) const
{
    foreach (const CachedPixmapKey& key, keys)
    {
        if (key.region.contains(region) && QPixmapCache::find(key.key, pix))
        {
            if (key.region == region)
            {
                *source = QRect();
            }
            else
            {
                QPoint startPoint = region.topLeft() - key.region.topLeft();
                *source = QRect(startPoint, region.size());
            }

            return true;
        }
    }
    return false;
}

void CachedPixmaps::insert(const QRect& region, const QPixmap& pixmap)
{
    if (keys.size() >= maxCount)
    {
        CachedPixmapKey key = keys.dequeue();
        QPixmapCache::remove(key.key);
    }

    CachedPixmapKey key;
    key.region = region;
    key.key = QPixmapCache::insert(pixmap);
    keys.enqueue(key);
}

// ---

GraphicsDImgItem::GraphicsDImgItem(QGraphicsItem* parent)
    : QGraphicsObject(parent),
      d_ptr(new GraphicsDImgItemPrivate)
{
    d_ptr->init(this);
}

GraphicsDImgItem::GraphicsDImgItem(GraphicsDImgItemPrivate& dd, QGraphicsItem* parent)
    : QGraphicsObject(parent),
      d_ptr(&dd)
{
    d_ptr->init(this);
}

void GraphicsDImgItem::GraphicsDImgItemPrivate::init(GraphicsDImgItem* q)
{
    // ItemCoordinateCache is very slow, DeviceCoordinateCache makes severe render artifacts
    q->setCacheMode(QGraphicsItem::NoCache);
    // This flag is crucial for our performance! Limits redrawing area.
    q->setFlag(QGraphicsItem::ItemUsesExtendedStyleOption);
    q->setAcceptedMouseButtons(Qt::NoButton);
}

GraphicsDImgItem::~GraphicsDImgItem()
{
    Q_D(GraphicsDImgItem);
    delete d;
}

void GraphicsDImgItem::setImage(const DImg& img)
{
    Q_D(GraphicsDImgItem);
    d->image = img;
    d->zoomSettings.setImageSize(img.size(), img.originalSize());
    d->cachedPixmaps.clear();
    sizeHasChanged();
    emit imageChanged();
}

DImg GraphicsDImgItem::image() const
{
    Q_D(const GraphicsDImgItem);
    return d->image;
}

void GraphicsDImgItem::sizeHasChanged()
{
    Q_D(GraphicsDImgItem);
    QGraphicsItem::prepareGeometryChange();
    d->cachedPixmaps.clear();
    emit imageSizeChanged(d->zoomSettings.zoomedSize());
}

const ImageZoomSettings* GraphicsDImgItem::zoomSettings() const
{
    Q_D(const GraphicsDImgItem);
    return &d->zoomSettings;
}

ImageZoomSettings* GraphicsDImgItem::zoomSettings()
{
    Q_D(GraphicsDImgItem);
    return &d->zoomSettings;
}

QRectF GraphicsDImgItem::boundingRect() const
{
    Q_D(const GraphicsDImgItem);
    // always return full integer sizes, we can only scale to integer
    return QRectF(QPointF(0,0), d->zoomSettings.zoomedSize()).toAlignedRect();
}

void GraphicsDImgItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget*)
{
    Q_D(GraphicsDImgItem);
    QRect drawRect = option->exposedRect.intersected(boundingRect()).toAlignedRect();

    QPixmap pix;
    QRect pixSourceRect;

    if (d->cachedPixmaps.find(drawRect, &pix, &pixSourceRect))
    {
        if (pixSourceRect.isNull())
        {
            painter->drawPixmap(drawRect.topLeft(), pix);
        }
        else
        {
            painter->drawPixmap(drawRect.topLeft(), pix, pixSourceRect);
        }
    }
    else
    {
        QSize completeSize = boundingRect().size().toSize();

        // scale "as if" scaling to whole image, but clip output to our exposed region
        DImg scaledImage = d->image.smoothScaleClipped(completeSize.width(), completeSize.height(),
                           drawRect.x(), drawRect.y(), drawRect.width(), drawRect.height());

        pix = scaledImage.convertToPixmap();
        d->cachedPixmaps.insert(drawRect, pix);

        painter->drawPixmap(drawRect.topLeft(), pix);
    }

    /*
        QPixmap pix(visibleWidth(), visibleHeight());
        pix.fill(ThemeEngine::instance()->baseColor());
        QPainter p(&pix);
        QFileInfo info(d->path);
        p.setPen(QPen(ThemeEngine::instance()->textRegColor()));
        p.drawText(0, 0, pix.width(), pix.height(),
                   Qt::AlignCenter|Qt::TextWordWrap,
                   i18n("Cannot display preview for\n\"%1\"",
                   info.fileName()));
        p.end();
        // three copies - but the image is small
        setImage(DImg(pix.toImage()));
    */
}

} // namespace Digikam

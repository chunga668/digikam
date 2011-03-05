/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-04-19
 * Description : Qt item view for images - the delegate
 *
 * Copyright (C) 2002-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2009      by Andi Clemens <andi dot clemens at gmx dot net>
 * Copyright (C) 2006-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2002-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "itemviewimagedelegate.moc"
#include "itemviewimagedelegatepriv.h"

// C++ includes

#include <cmath>

// Qt includes

#include <QCache>
#include <QPainter>
#include <QIcon>

// KDE includes

#include <kglobal.h>
#include <kio/global.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kdebug.h>

// Local includes

#include "imagedelegateoverlay.h"
#include "themeengine.h"
#include "colorlabelwidget.h"
#include "globals.h"

namespace Digikam
{

ItemViewImageDelegatePrivate::ItemViewImageDelegatePrivate()
{
    spacing        = 0;
    thumbSize      = 0;

    // painting constants
    radius         = 3;
    margin         = 5;

    makeStarPolygon();

    ratingPixmaps   = QVector<QPixmap>(10);
}

void ItemViewImageDelegatePrivate::init(ItemViewImageDelegate* _q)
{
    q = _q;

    q->connect(ThemeEngine::instance(), SIGNAL(signalThemeChanged()),
               q, SLOT(slotThemeChanged()));
}

void ItemViewImageDelegatePrivate::clearRects()
{
    gridSize       = QSize(0, 0);
    rect           = QRect(0, 0, 0, 0);
    ratingRect     = QRect(0, 0, 0, 0);
}

void ItemViewImageDelegatePrivate::makeStarPolygon()
{
    // Pre-computed star polygon for a 15x15 pixmap.
    starPolygon << QPoint(0,  6);
    starPolygon << QPoint(5,  5);
    starPolygon << QPoint(7,  0);
    starPolygon << QPoint(9,  5);
    starPolygon << QPoint(14, 6);
    starPolygon << QPoint(10, 9);
    starPolygon << QPoint(11, 14);
    starPolygon << QPoint(7,  11);
    starPolygon << QPoint(3,  14);
    starPolygon << QPoint(4,  9);

    starPolygonSize = QSize(15, 15);
}

ItemViewImageDelegate::ItemViewImageDelegate(QObject* parent)
    : DItemDelegate(parent), d_ptr(new ItemViewImageDelegatePrivate)
{
    d_ptr->init(this);
}

ItemViewImageDelegate::ItemViewImageDelegate(ItemViewImageDelegatePrivate& dd, QObject* parent)
    : DItemDelegate(parent), d_ptr(&dd)
{
    d_ptr->init(this);
}

ItemViewImageDelegate::~ItemViewImageDelegate()
{
    Q_D(ItemViewImageDelegate);
    removeAllOverlays();
    delete d;
}

ThumbnailSize ItemViewImageDelegate::thumbnailSize() const
{
    Q_D(const ItemViewImageDelegate);
    return d->thumbSize;
}

void ItemViewImageDelegate::setThumbnailSize(const ThumbnailSize& thumbSize)
{
    Q_D(ItemViewImageDelegate);

    if ( d->thumbSize != thumbSize)
    {
        d->thumbSize = thumbSize;
        invalidatePaintingCache();
    }
}

void ItemViewImageDelegate::setSpacing(int spacing)
{
    Q_D(ItemViewImageDelegate);

    if (d->spacing == spacing)
    {
        return;
    }

    d->spacing = spacing;
    invalidatePaintingCache();
}

int ItemViewImageDelegate::spacing() const
{
    Q_D(const ItemViewImageDelegate);
    return d->spacing;
}

QRect ItemViewImageDelegate::rect() const
{
    Q_D(const ItemViewImageDelegate);
    return d->rect;
}

QRect ItemViewImageDelegate::pixmapRect() const
{
    return QRect();
}

QRect ItemViewImageDelegate::imageInformationRect() const
{
    return QRect();
}

QRect ItemViewImageDelegate::ratingRect() const
{
    Q_D(const ItemViewImageDelegate);
    return d->ratingRect;
}

void ItemViewImageDelegate::setRatingEdited(const QModelIndex& index)
{
    Q_D(ItemViewImageDelegate);
    d->editingRating = index;
}

QSize ItemViewImageDelegate::sizeHint(const QStyleOptionViewItem& /*option*/, const QModelIndex& /*index*/) const
{
    Q_D(const ItemViewImageDelegate);
    return d->rect.size();
}

QSize ItemViewImageDelegate::gridSize() const
{
    Q_D(const ItemViewImageDelegate);
    return d->gridSize;
}

bool ItemViewImageDelegate::acceptsToolTip(const QPoint&, const QRect& visualRect, const QModelIndex&, QRect* retRect) const
{
    if (retRect)
    {
        *retRect = visualRect;
    }

    return true;
}

bool ItemViewImageDelegate::acceptsActivation(const QPoint& , const QRect& visualRect, const QModelIndex&, QRect* retRect) const
{
    if (retRect)
    {
        *retRect = visualRect;
    }

    return true;
}

QAbstractItemDelegate* ItemViewImageDelegate::asDelegate()
{
    return this;
}

void ItemViewImageDelegate::overlayDestroyed(QObject* o)
{
    ImageDelegateOverlayContainer::overlayDestroyed(o);
}

void ItemViewImageDelegate::mouseMoved(QMouseEvent* e, const QRect& visualRect, const QModelIndex& index)
{
    // 3-way indirection DItemDelegate -> ItemViewImageDelegate -> ImageDelegateOverlayContainer
    ImageDelegateOverlayContainer::mouseMoved(e, visualRect, index);
}

void ItemViewImageDelegate::setDefaultViewOptions(const QStyleOptionViewItem& option)
{
    Q_D(ItemViewImageDelegate);
    d->font = option.font;
    invalidatePaintingCache();
}

void ItemViewImageDelegate::slotThemeChanged()
{
    invalidatePaintingCache();
}

void ItemViewImageDelegate::slotSetupChanged()
{
    invalidatePaintingCache();
}

void ItemViewImageDelegate::invalidatePaintingCache()
{
    Q_D(ItemViewImageDelegate);
    QSize oldGridSize = d->gridSize;
    updateSizeRectsAndPixmaps();

    if (oldGridSize != d->gridSize)
    {
        emit gridSizeChanged(d->gridSize);
        // emit sizeHintChanged(QModelIndex());
    }

    emit visualChange();
}

QRect ItemViewImageDelegate::drawThumbnail(QPainter* p, const QRect& thumbRect, const QPixmap& background, const QPixmap& thumbnail) const
{
    p->drawPixmap(0, 0, background);

    if (thumbnail.isNull())
    {
        return QRect();
    }

    QRect r = thumbRect;
    /*p->drawPixmap(r.x() + (r.width()-thumbnail.width())/2,
                    r.y() + (r.height()-thumbnail.height())/2,
                    thumbnail);*/

    QRect actualPixmapRect(r.x() + (r.width()-thumbnail.width())/2,
                           r.y() + (r.height()-thumbnail.height())/2,
                           thumbnail.width(), thumbnail.height());

    /*p->save();
    QRegion pixmapClipRegion = QRegion(d->rect) - QRegion(actualPixmapRect);
    p->setClipRegion(pixmapClipRegion);*/
    //p->drawPixmap(0, 0, background);

    QPixmap borderPix = thumbnailBorderPixmap(actualPixmapRect.size());
    p->drawPixmap(actualPixmapRect.x()-3, actualPixmapRect.y()-3, borderPix);

    p->drawPixmap(r.x() + (r.width()-thumbnail.width())/2,
                  r.y() + (r.height()-thumbnail.height())/2,
                  thumbnail);
    //p->restore();
    return actualPixmapRect;
}

void ItemViewImageDelegate::drawRating(QPainter* p, const QModelIndex& index, const QRect& ratingRect, int rating, bool isSelected) const
{
    Q_D(const ItemViewImageDelegate);

    if (d->editingRating != index)
    {
        p->drawPixmap(ratingRect, ratingPixmap(rating, isSelected));
    }

    /*else
        p->drawPixmap(r, ratingPixmap(-1, isSelected));*/
}

void ItemViewImageDelegate::drawName(QPainter* p,const QRect& nameRect, const QString& name) const
{
    Q_D(const ItemViewImageDelegate);
    p->setFont(d->fontReg);
    p->drawText(nameRect, Qt::AlignCenter, squeezedTextCached(p, nameRect.width(), name));
}

void ItemViewImageDelegate::drawComments(QPainter* p, const QRect& commentsRect, const QString& comments) const
{
    Q_D(const ItemViewImageDelegate);
    p->setFont(d->fontCom);
    p->drawText(commentsRect, Qt::AlignCenter, squeezedTextCached(p, commentsRect.width(), comments));
}

void ItemViewImageDelegate::drawCreationDate(QPainter* p, const QRect& dateRect, const QDateTime& date) const
{
    Q_D(const ItemViewImageDelegate);
    p->setFont(d->fontXtra);
    QString str = dateToString(date);
    str = i18nc("date of image creation", "created: %1", str);
    p->drawText(dateRect, Qt::AlignCenter, squeezedTextCached(p, dateRect.width(), str));
}

void ItemViewImageDelegate::drawModificationDate(QPainter* p, const QRect& dateRect, const QDateTime& date) const
{
    Q_D(const ItemViewImageDelegate);
    p->setFont(d->fontXtra);
    QString str = dateToString(date);
    str = i18nc("date of last image modification", "modified: %1",str);
    p->drawText(dateRect, Qt::AlignCenter, squeezedTextCached(p, dateRect.width(), str));
}

void ItemViewImageDelegate::drawImageSize(QPainter* p, const QRect& dimsRect, const QSize& dims) const
{
    Q_D(const ItemViewImageDelegate);

    if (dims.isValid())
    {
        p->setFont(d->fontXtra);
        QString mpixels, resolution;
        mpixels.setNum(dims.width()*dims.height()/1000000.0, 'f', 2);

        if (dims.isValid())
            resolution = i18nc("%1 width, %2 height, %3 mpixels", "%1x%2 (%3Mpx)",
                               dims.width(), dims.height(), mpixels);
        else
        {
            resolution = i18nc("unknown image resolution", "Unknown");
        }

        p->drawText(dimsRect, Qt::AlignCenter, squeezedTextCached(p, dimsRect.width(), resolution));
    }
}

void ItemViewImageDelegate::drawFileSize(QPainter* p, const QRect& r, int bytes) const
{
    Q_D(const ItemViewImageDelegate);
    p->setFont(d->fontXtra);
    p->drawText(r, Qt::AlignCenter, squeezedTextCached(p, r.width(), KIO::convertSize(bytes)));
}

void ItemViewImageDelegate::drawTags(QPainter* p, const QRect& r, const QString& tagsString, bool isSelected) const
{
    Q_D(const ItemViewImageDelegate);
    p->setFont(d->fontCom);
    p->setPen(isSelected ? ThemeEngine::instance()->textSpecialSelColor()
              : ThemeEngine::instance()->textSpecialRegColor());

    p->drawText(r, Qt::AlignCenter, squeezedTextCached(p, r.width(), tagsString));
}

void ItemViewImageDelegate::drawFocusRect(QPainter* p, const QStyleOptionViewItem& option, bool isSelected) const
{
    Q_D(const ItemViewImageDelegate);

    if (option.state & QStyle::State_HasFocus) //?? is current item
    {
        p->setPen(QPen(isSelected ? ThemeEngine::instance()->textSelColor()
                       : ThemeEngine::instance()->textRegColor(),
                       1, Qt::DotLine));
        p->drawRect(1, 1, d->rect.width()-3, d->rect.height()-3);
    }
}

void ItemViewImageDelegate::drawPickLabelIcon(QPainter* p, const QRect& r, int pickId) const
{
    // Draw Pick Label icon
    if (pickId != NoPickLabel)
    {
        QIcon icon;

        if (pickId == RejectedLabel)
        {
            icon = KIconLoader::global()->loadIcon("flag-red", KIconLoader::NoGroup, r.width());
        }
        else if (pickId == PendingLabel)
        {
            icon = KIconLoader::global()->loadIcon("flag-yellow", KIconLoader::NoGroup, r.width());
        }
        else if (pickId == AcceptedLabel)
        {
            icon = KIconLoader::global()->loadIcon("flag-green", KIconLoader::NoGroup, r.width());
        }
        icon.paint(p, r);
    }
}

void ItemViewImageDelegate::drawGroupIndicator(QPainter* p, const QRect& r,
                                               int numberOfGroupedImages, bool open) const
{
    if (numberOfGroupedImages)
    {
        QIcon icon;
        if (open)
        {
            icon = KIconLoader::global()->loadIcon("document-import", KIconLoader::NoGroup, r.width());
        }
        else
        {
            icon = KIconLoader::global()->loadIcon("document-multiple", KIconLoader::NoGroup, r.width());
        }
        qreal op = p->opacity();
        p->setOpacity(0.5);
        icon.paint(p, r);
        p->setOpacity(op);

        QString text = QString::number(numberOfGroupedImages);
        /*
        QRect br = p->boundingRect(r, Qt::AlignLeft|Qt::AlignTop, text).adjusted(0,0,1,1);
        int rectSize = qMax(br.width(), br.height());
        QRect textRect = QRect(0, 0, rectSize, rectSize);
        textRect.moveLeft((r.width() - textRect.width()) / 2);
        textRect.moveTop((r.height() - textRect.height()) * 4 / 5);
        p->fillRect(textRect.translated(r.topLeft(), QColor(0, 0, 0, 128));
        */
        p->drawText(r, Qt::AlignCenter, text);
    }
}

void ItemViewImageDelegate::drawColorLabelRect(QPainter* p, const QStyleOptionViewItem& option,
                                               bool isSelected, int colorId) const
{
    Q_D(const ItemViewImageDelegate);
    Q_UNUSED(option);
    Q_UNUSED(isSelected);

    if (colorId > NoColorLabel)
    {
        // This draw a simple rectangle around item.

        p->setPen(QPen(ColorLabelWidget::labelColor((ColorLabel)colorId), 5, Qt::SolidLine));
        p->drawRect(3, 3, d->rect.width()-7, d->rect.height()-7);
    }
}

void ItemViewImageDelegate::drawMouseOverRect(QPainter* p, const QStyleOptionViewItem& option) const
{
    Q_D(const ItemViewImageDelegate);

    if (option.state & QStyle::State_MouseOver)
    {
        p->setPen(QPen(option.palette.color(QPalette::Highlight), 3, Qt::SolidLine));
        p->drawRect(1, 1, d->rect.width()-3, d->rect.height()-3);
    }
}

void ItemViewImageDelegate::prepareFonts()
{
    Q_D(ItemViewImageDelegate);

    d->fontReg  = d->font;
    d->fontCom  = d->font;
    d->fontXtra = d->font;
    d->fontCom.setItalic(true);

    int fnSz = d->fontReg.pointSize();

    if (fnSz > 0)
    {
        d->fontCom.setPointSize(fnSz-1);
        d->fontXtra.setPointSize(fnSz-2);
    }
    else
    {
        fnSz = d->fontReg.pixelSize();
        d->fontCom.setPixelSize(fnSz-1);
        d->fontXtra.setPixelSize(fnSz-2);
    }
}

void ItemViewImageDelegate::prepareMetrics(int maxWidth)
{
    Q_D(ItemViewImageDelegate);

    QFontMetrics fm(d->fontReg);
    d->oneRowRegRect = fm.boundingRect(0, 0, maxWidth, 0xFFFFFFFF,
                                       Qt::AlignTop | Qt::AlignHCenter,
                                       "XXXXXXXXX");
    fm = QFontMetrics(d->fontCom);
    d->oneRowComRect = fm.boundingRect(0, 0, maxWidth, 0xFFFFFFFF,
                                       Qt::AlignTop | Qt::AlignHCenter,
                                       "XXXXXXXXX");
    fm = QFontMetrics(d->fontXtra);
    d->oneRowXtraRect = fm.boundingRect(0, 0, maxWidth, 0xFFFFFFFF,
                                        Qt::AlignTop | Qt::AlignHCenter,
                                        "XXXXXXXXX");
}

void ItemViewImageDelegate::prepareBackground()
{
    Q_D(ItemViewImageDelegate);

    if (!d->rect.isValid())
    {
        d->regPixmap = QPixmap();
        d->selPixmap = QPixmap();
    }
    else
    {
        d->regPixmap = ThemeEngine::instance()->thumbRegPixmap(d->rect.width(), d->rect.height());
        d->selPixmap = ThemeEngine::instance()->thumbSelPixmap(d->rect.width(), d->rect.height());
    }
}

void ItemViewImageDelegate::prepareRatingPixmaps(bool composeOverBackground)
{
    /// Please call this method after prepareBackground() and when d->ratingPixmap is set

    Q_D(ItemViewImageDelegate);

    if (!d->ratingRect.isValid())
    {
        return;
    }

    // We use antialiasing and want to pre-render the pixmaps.
    // So we need the background at the time of painting,
    // and the background may be a gradient, and will be different for selected items.
    // This makes 5*2 (small) pixmaps.
    for (int sel=0; sel<2; ++sel)
    {
        QPixmap basePix;

        if (composeOverBackground)
        {
            // do this once for regular, once for selected backgrounds
            if (sel)
            {
                basePix = d->selPixmap.copy(d->ratingRect);
            }
            else
            {
                basePix = d->regPixmap.copy(d->ratingRect);
            }
        }
        else
        {
            basePix = QPixmap(d->ratingRect.size());
            basePix.fill(Qt::transparent);
        }

        for (int rating=1; rating<=5; ++rating)
        {
            // we store first the 5 regular, then the 5 selected pixmaps, for simplicity
            int index = (sel * 5 + rating) - 1;

            // copy background
            d->ratingPixmaps[index] = basePix;
            // open a painter
            QPainter painter(&d->ratingPixmaps[index]);

            // use antialiasing
            painter.setRenderHint(QPainter::Antialiasing, true);
            painter.setBrush(ThemeEngine::instance()->textSpecialRegColor());
            QPen pen(ThemeEngine::instance()->textRegColor());
            // set a pen which joins the lines at a filled angle
            pen.setJoinStyle(Qt::MiterJoin);
            painter.setPen(pen);

            // move painter while drawing polygons
            painter.translate( lround((d->ratingRect.width() - d->margin - rating*(d->starPolygonSize.width()+1))/2.0) + 2, 1 );

            for (int s=0; s<rating; ++s)
            {
                painter.drawPolygon(d->starPolygon, Qt::WindingFill);
                painter.translate(d->starPolygonSize.width() + 1, 0);
            }
        }
    }
}

QPixmap ItemViewImageDelegate::ratingPixmap(int rating, bool selected) const
{
    Q_D(const ItemViewImageDelegate);

    if (rating < 1 || rating > 5)
    {
        /*
        QPixmap pix;
        if (selected)
            pix = d->selPixmap.copy(d->ratingRect);
        else
            pix = d->regPixmap.copy(d->ratingRect);

        return pix;
        */
        return QPixmap();
    }

    --rating;

    if (selected)
    {
        return d->ratingPixmaps[5 + rating];
    }
    else
    {
        return d->ratingPixmaps[rating];
    }
}

} // namespace Digikam

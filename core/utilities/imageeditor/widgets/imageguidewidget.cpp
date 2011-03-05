/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-11-16
 * Description : a widget to display an image with guides
 *
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

#include "imageguidewidget.moc"

// Qt includes

#include <QRegion>
#include <QPainter>
#include <QPen>
#include <QTimer>
#include <QRect>
#include <QBrush>
#include <QFont>
#include <QFontMetrics>

// KDE includes

#include <kstandarddirs.h>
#include <kcursor.h>
#include <kdebug.h>

// Local includes

#include "dimg.h"
#include "imageiface.h"
#include "previewtoolbar.h"
#include "exposurecontainer.h"
#include "iccsettingscontainer.h"

namespace Digikam
{

static const QString beforeLabel = i18nc("Preview image (before filter has been applied)", "Before");
static const QString afterLabel  = i18nc("Preview image (after filter has been applied)", "After");

class ImageGuideWidget::ImageGuideWidgetPriv
{
public:

    ImageGuideWidgetPriv() :
        sixteenBit(false),
        focus(false),
        spotVisible(false),
        onMouseMovePreviewToggled(true),
        drawLineBetweenPoints(false),
        drawingMask(false),
        enableDrawMask(false),
        eraseMask(false),
        width(0),
        height(0),
        timerID(0),
        guideMode(0),
        guideSize(0),
        flicker(0),
        renderingPreviewMode(PreviewToolBar::NoPreviewMode),
        penWidth(10),
        pixmap(0),
        maskPixmap(0),
        previewPixmap(0),
        iface(0)
    {
    }

    bool        sixteenBit;
    bool        focus;
    bool        spotVisible;
    bool        onMouseMovePreviewToggled;
    bool        drawLineBetweenPoints;
    bool        drawingMask;
    bool        enableDrawMask;
    bool        eraseMask;

    int         width;
    int         height;
    int         timerID;
    int         guideMode;
    int         guideSize;
    int         flicker;
    int         renderingPreviewMode;
    int         penWidth;

    // Current spot position in preview coordinates.
    QPoint      spot;
    QPolygon    selectedPoints;

    QRect       rect;

    QColor      guideColor;
    QColor      paintColor;

    QPixmap*    pixmap;
    QPixmap*    maskPixmap;
    QPixmap*    previewPixmap;

    QCursor     maskCursor;

    QPoint      lastPoint;

    ImageIface* iface;

    DImg        preview;
};

ImageGuideWidget::ImageGuideWidget(QWidget* parent,
                                   bool spotVisible, int guideMode,
                                   const QColor& guideColor, int guideSize,
                                   bool blink, bool useImageSelection)
    : QWidget(parent), d(new ImageGuideWidgetPriv)
{
    int w = 480, h = 320;
    d->spotVisible = spotVisible;
    d->guideMode   = guideMode;
    d->guideColor  = guideColor;
    d->guideSize   = guideSize;

    setMinimumSize(w, h);
    setMouseTracking(true);
    setAttribute(Qt::WA_DeleteOnClose);

    d->iface        = new ImageIface(w, h);
    d->iface->setPreviewType(useImageSelection);
    uchar* data     = d->iface->getPreviewImage();
    d->width        = d->iface->previewWidth();
    d->height       = d->iface->previewHeight();
    bool sixteenBit = d->iface->previewSixteenBit();
    bool hasAlpha   = d->iface->previewHasAlpha();
    d->preview      = DImg(d->width, d->height, sixteenBit, hasAlpha, data);
    d->preview.setIccProfile( d->iface->getOriginalImg()->getIccProfile() );
    delete [] data;

    d->pixmap        = new QPixmap(w, h);
    d->rect          = QRect(w/2-d->width/2, h/2-d->height/2, d->width, d->height);
    d->maskPixmap    = new QPixmap(d->rect.width(), d->rect.height());
    d->previewPixmap = new QPixmap(d->rect.width(), d->rect.height());
    d->maskPixmap->fill(QColor(0,0,0,0));
    d->previewPixmap->fill(QColor(0,0,0,0));


    d->paintColor.setRgb(255, 255, 255, 255);

    d->lastPoint = QPoint(d->rect.x(),d->rect.y());

    resetSpotPosition();
    setSpotVisible(d->spotVisible, blink);
}

ImageGuideWidget::~ImageGuideWidget()
{
    delete d->iface;

    if (d->timerID)
    {
        killTimer(d->timerID);
    }

    if (d->pixmap)
    {
        delete d->pixmap;
    }

    if (d->maskPixmap)
    {
        delete d->maskPixmap;
    }

    if (d->previewPixmap)
    {
        delete d->previewPixmap;
    }

    delete d;
}

ImageIface* ImageGuideWidget::imageIface() const
{
    return d->iface;
}

void ImageGuideWidget::ICCSettingsChanged()
{
    updatePreview();
}

void ImageGuideWidget::exposureSettingsChanged()
{
    updatePreview();
}

void ImageGuideWidget::resetSpotPosition()
{
    d->spot.setX( d->width  / 2 );
    d->spot.setY( d->height / 2 );
    updatePreview();
}

void ImageGuideWidget::slotPreviewModeChanged(int mode)
{
    d->renderingPreviewMode = mode;
    updatePreview();
}

int ImageGuideWidget::previewMode()
{
    return (d->renderingPreviewMode);
}

QPoint ImageGuideWidget::getSpotPosition()
{
    return (QPoint( (int)((float)d->spot.x() * (float)d->iface->originalWidth()  / (float)d->width),
                    (int)((float)d->spot.y() * (float)d->iface->originalHeight() / (float)d->height)));
}

DColor ImageGuideWidget::getSpotColor(int getColorFrom)
{
    if (getColorFrom == OriginalImage)                          // Get point color from full original image
    {
        return (d->iface->getColorInfoFromOriginalImage(getSpotPosition()));
    }
    else if (getColorFrom == PreviewImage)                      // Get point color from full preview image
    {
        return (d->iface->getColorInfoFromPreviewImage(d->spot));
    }

    // In other cases, get point color from preview target image
    return (d->iface->getColorInfoFromTargetPreviewImage(d->spot));
}

void ImageGuideWidget::setSpotVisible(bool spotVisible, bool blink)
{
    d->spotVisible = spotVisible;

    if (blink)
    {
        if (d->spotVisible)
        {
            d->timerID = startTimer(800);
        }
        else
        {
            killTimer(d->timerID);
            d->timerID = 0;
        }
    }

    updatePreview();
}

void ImageGuideWidget::slotChangeGuideColor(const QColor& color)
{
    d->guideColor = color;
    updatePreview();
}

void ImageGuideWidget::slotChangeGuideSize(int size)
{
    d->guideSize = size;
    updatePreview();
}

void ImageGuideWidget::updatePreview()
{
    updatePixmap();
    update();
}

void ImageGuideWidget::updatePixmap()
{
    QPainter p(d->pixmap);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setBackgroundMode(Qt::TransparentMode);

    QString text;
    p.setPen(QPen(Qt::red, 1));

    d->pixmap->fill(palette().color(QPalette::Background));

    if (d->renderingPreviewMode == PreviewToolBar::PreviewOriginalImage ||
        (d->renderingPreviewMode == PreviewToolBar::PreviewToggleOnMouseOver && !d->onMouseMovePreviewToggled))
    {
        p.drawPixmap(d->rect, *d->previewPixmap);
        drawText(&p, QPoint(d->rect.x() + 20, d->rect.y() + 20), beforeLabel);
    }
    else if (d->renderingPreviewMode == PreviewToolBar::PreviewTargetImage ||
             d->renderingPreviewMode == PreviewToolBar::NoPreviewMode      ||
             (d->renderingPreviewMode == PreviewToolBar::PreviewToggleOnMouseOver && d->onMouseMovePreviewToggled))
    {
        d->iface->paint(d->pixmap, d->rect.x(), d->rect.y(), d->rect.width(), d->rect.height(), &p);

        if (d->renderingPreviewMode == PreviewToolBar::PreviewTargetImage ||
            d->renderingPreviewMode == PreviewToolBar::PreviewToggleOnMouseOver)
        {
            drawText(&p, QPoint(d->rect.x() + 20, d->rect.y() + 20), afterLabel);
        }
    }
    else if (d->renderingPreviewMode == PreviewToolBar::PreviewBothImagesVert ||
             d->renderingPreviewMode == PreviewToolBar::PreviewBothImagesVertCont)
    {
        if (d->renderingPreviewMode == PreviewToolBar::PreviewBothImagesVert)
        {
            // Drawing original image.
            p.drawPixmap(d->rect, *d->previewPixmap);

            // Drawing target image under the original.
            d->iface->paint(d->pixmap, d->rect.x()+d->rect.width()/2, d->rect.y(), d->rect.width()/2, d->rect.height(), &p);
        }
        else
        {
            // Drawing target image.
            d->iface->paint(d->pixmap, d->rect.x(), d->rect.y(), d->rect.width(), d->rect.height(), &p);

            // Drawing original image under the target.
            p.drawPixmap(d->rect.x(), d->rect.y(), *d->previewPixmap, 0, 0, d->rect.width()/2, d->rect.height());
        }

        // Drawing information and others stuff.
        p.fillRect(d->rect.right(), 0, width(), height(), palette().color(QPalette::Background));

        p.setPen(QPen(Qt::white, 2, Qt::SolidLine));
        p.drawLine(d->rect.x()+d->rect.width()/2-1, d->rect.y(),
                   d->rect.x()+d->rect.width()/2-1, d->rect.y()+d->rect.height());
        p.setPen(QPen(Qt::red, 2, Qt::DotLine));
        p.drawLine(d->rect.x()+d->rect.width()/2-1, d->rect.y(),
                   d->rect.x()+d->rect.width()/2-1, d->rect.y()+d->rect.height());

        drawText(&p, QPoint(d->rect.x() + 20, d->rect.y() + 20), beforeLabel);
        drawText(&p, QPoint(d->rect.x() + d->rect.width()/2 + 20, d->rect.y() + 20), afterLabel);
    }
    else if (d->renderingPreviewMode == PreviewToolBar::PreviewBothImagesHorz ||
             d->renderingPreviewMode == PreviewToolBar::PreviewBothImagesHorzCont)
    {
        if (d->renderingPreviewMode == PreviewToolBar::PreviewBothImagesHorz)
        {
            // Drawing original image.
            p.drawPixmap(d->rect, *d->previewPixmap);

            // Drawing target image under the original.
            d->iface->paint(d->pixmap, d->rect.x(), d->rect.y()+d->rect.height()/2, d->rect.width(), d->rect.height()/2, &p);
        }
        else
        {
            // Drawing target image.
            d->iface->paint(d->pixmap, d->rect.x(), d->rect.y(), d->rect.width(), d->rect.height(), &p);

            // Drawing original image under the target.
            p.drawPixmap(d->rect.x(), d->rect.y(), *d->previewPixmap,
                         0, 0, d->rect.width(), d->rect.height()/2);
        }

        p.fillRect(0, d->rect.bottom(), width(), height(), palette().color(QPalette::Background));

        p.setPen(QPen(Qt::white, 2, Qt::SolidLine));
        p.drawLine(d->rect.x(),
                   d->rect.y()+d->rect.height()/2-1,
                   d->rect.x()+d->rect.width(),
                   d->rect.y()+d->rect.height()/2-1);
        p.setPen(QPen(Qt::red, 2, Qt::DotLine));
        p.drawLine(d->rect.x(),
                   d->rect.y()+d->rect.height()/2-1,
                   d->rect.x()+d->rect.width(),
                   d->rect.y()+d->rect.height()/2-1);

        drawText(&p, QPoint(d->rect.x() + 20, d->rect.y() + 20), beforeLabel);
        drawText(&p, QPoint(d->rect.x() + 20, d->rect.y() + d->rect.height()/2 + 20), afterLabel);
    }

    if (d->spotVisible)
    {
        // Adapt spot from image coordinate to widget coordinate.
        int xspot = d->spot.x() + d->rect.x();
        int yspot = d->spot.y() + d->rect.y();

        switch (d->guideMode)
        {
            case HVGuideMode:
            {
                p.setPen(QPen(Qt::white, d->guideSize, Qt::SolidLine));
                p.drawLine(xspot, d->rect.top() + d->flicker, xspot, d->rect.bottom() - d->flicker);
                p.drawLine(d->rect.left() + d->flicker, yspot, d->rect.right() - d->flicker, yspot);
                p.setPen(QPen(d->guideColor, d->guideSize, Qt::DotLine));
                p.drawLine(xspot, d->rect.top() + d->flicker, xspot, d->rect.bottom() - d->flicker);
                p.drawLine(d->rect.left() + d->flicker, yspot, d->rect.right() - d->flicker, yspot);
                break;
            }

            case PickColorMode:
            {
                p.setPen(QPen(d->guideColor, 1, Qt::SolidLine));
                p.drawLine(xspot-10, yspot-10, xspot+10, yspot+10);
                p.drawLine(xspot+10, yspot-10, xspot-10, yspot+10);
                p.setPen(QPen(d->guideColor, 3, Qt::SolidLine));
                p.drawEllipse( xspot-5, yspot-5, 11, 11 );

                if (d->flicker%2 != 0)
                {
                    p.setPen(QPen(Qt::white, 1, Qt::SolidLine));
                    p.drawEllipse( xspot-5, yspot-5, 11, 11 );
                }

                break;
            }
        }
    }

    // draw additional points added by the image plugin
    if (d->selectedPoints.count() > 0)
    {
        QPainter::RenderHints hints = p.renderHints();

        QColor semiTransGuideColor  = QColor(d->guideColor.red(), d->guideColor.green(), d->guideColor.blue(), 75);

        QPoint point;
        int x = 0;
        int y = 0;

        for (int i = 0; i < d->selectedPoints.count(); ++i)
        {
            point = d->selectedPoints.point(i);
            point = translatePointPosition(point);
            x     = point.x();
            y     = point.y();

            p.save();
            p.setRenderHint(QPainter::Antialiasing, true);
            p.setPen(QPen(d->guideColor, 2, Qt::SolidLine));
            p.setBrush(QBrush(semiTransGuideColor));
            p.drawEllipse(point, 6, 6);

            p.restore();
            p.setPen(QPen(d->guideColor, 1, Qt::SolidLine));
            p.setBrush(Qt::NoBrush);
            p.setRenderHint(QPainter::Antialiasing, false);
            p.drawPoint(point);
            p.drawText(QPoint(x+10, y-5), QString::number(i+1));

            // draw a line between the points
            if (d->drawLineBetweenPoints &&
                (i+1) < d->selectedPoints.count() && !d->selectedPoints.point(i+1).isNull())
            {
                p.save();
                p.setPen(QPen(d->guideColor, d->guideSize, Qt::SolidLine));
                QPoint point2 = d->selectedPoints.point(i+1);
                point2        = translatePointPosition(point2);
                p.setRenderHint(QPainter::Antialiasing, true);
                p.drawLine(point, point2);
                p.restore();
            }
        }

        p.setRenderHints(hints);
    }

    p.end();
}

void ImageGuideWidget::drawText(QPainter* p, const QPoint& corner, const QString& text)
{
    p->save();
    QFontMetrics fontMt = p->fontMetrics();
    QRect      fontRect = fontMt.boundingRect(text);
    QRect textRect;
    textRect.setTopLeft(corner);
    textRect.setSize( QSize(fontRect.width()+5, fontRect.height()+2) );

    // Draw background
    p->setPen(Qt::black);
    QColor semiTransBg = palette().color(QPalette::Window);
    semiTransBg.setAlpha(190);
    p->setBrush(semiTransBg);
    //p->translate(0.5, 0.5);
    p->drawRoundRect(textRect, 10, 10);

    // Draw shadow and text
    p->setPen(palette().color(QPalette::Window).dark(115));
    p->drawText(textRect.translated(3, 1), text);
    p->setPen(palette().color(QPalette::WindowText));
    p->drawText(textRect.translated(2, 0), text);

    p->restore();
}

void ImageGuideWidget::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.drawPixmap(0, 0, *d->pixmap);

    if (d->enableDrawMask && d->onMouseMovePreviewToggled == false )
    {
        p.setOpacity(0.7);
        p.drawPixmap(d->rect.x(), d->rect.y(), *d->maskPixmap);

        if (d->renderingPreviewMode == PreviewToolBar::PreviewOriginalImage ||
            (d->renderingPreviewMode == PreviewToolBar::PreviewToggleOnMouseOver && !d->onMouseMovePreviewToggled))
        {
            drawText(&p, QPoint(d->rect.x() + 20, d->rect.y() + 20), beforeLabel);
        }
        else if (d->renderingPreviewMode == PreviewToolBar::PreviewTargetImage ||
                 d->renderingPreviewMode == PreviewToolBar::NoPreviewMode      ||
                 (d->renderingPreviewMode == PreviewToolBar::PreviewToggleOnMouseOver && d->onMouseMovePreviewToggled))
        {
            drawText(&p, QPoint(d->rect.x() + 20, d->rect.y() + 20), afterLabel);
        }
    }

    p.end();
}

void ImageGuideWidget::timerEvent(QTimerEvent* e)
{
    if (e->timerId() == d->timerID)
    {
        if (d->flicker == 5)
        {
            d->flicker=0;
        }
        else
        {
            d->flicker++;
        }

        updatePreview();
    }
    else
    {
        QWidget::timerEvent(e);
    }
}

void ImageGuideWidget::resizeEvent(QResizeEvent* e)
{
    blockSignals(true);
    delete d->pixmap;
    delete d->previewPixmap;

    int w     = e->size().width();
    int h     = e->size().height();
    int old_w = d->width;
    int old_h = d->height;

    uchar* data     = d->iface->setPreviewImageSize(w, h);
    d->width        = d->iface->previewWidth();
    d->height       = d->iface->previewHeight();
    bool sixteenBit = d->iface->previewSixteenBit();
    bool hasAlpha   = d->iface->previewHasAlpha();
    d->preview      = DImg(d->width, d->height, sixteenBit, hasAlpha, data);
    d->preview.setIccProfile( d->iface->getOriginalImg()->getIccProfile() );
    delete [] data;

    d->pixmap         = new QPixmap(w, h);
    d->previewPixmap  = new QPixmap(w, h);
    d->rect           = QRect(w/2-d->width/2, h/2-d->height/2, d->width, d->height);
    *d->maskPixmap    = d->maskPixmap->scaled(d->width,d->height,Qt::IgnoreAspectRatio);
    *d->previewPixmap = d->iface->convertToPixmap(d->preview);

    d->spot.setX((int)((float)d->spot.x() * ( (float)d->width  / (float)old_w)));
    d->spot.setY((int)((float)d->spot.y() * ( (float)d->height / (float)old_h)));
    updatePixmap();
    blockSignals(false);
    emit signalResized();
}

void ImageGuideWidget::mousePressEvent(QMouseEvent* e)
{
    if (e->button() == Qt::LeftButton)
    {
        if ( !d->focus && d->rect.contains( e->x(), e->y() ) && d->spotVisible )
        {
            d->focus = true;
            d->spot.setX(e->x()-d->rect.x());
            d->spot.setY(e->y()-d->rect.y());
        }
        else if (d->enableDrawMask)
        {
            d->lastPoint = QPoint(e->x()-d->rect.x(), e->y()-d->rect.y());
            d->drawingMask = true;
        }

        updatePreview();
    }
}

void ImageGuideWidget::mouseReleaseEvent(QMouseEvent* e)
{
    if (d->rect.contains(e->x(), e->y()))
    {
        if ( d->focus && d->spotVisible)
        {
            d->focus = false;
            updatePreview();
            d->spot.setX(e->x()-d->rect.x());
            d->spot.setY(e->y()-d->rect.y());

            DColor color;
            QPoint point = getSpotPosition();

            if (d->renderingPreviewMode == PreviewToolBar::PreviewOriginalImage)
            {
                color = getSpotColor(OriginalImage);
                emit spotPositionChangedFromOriginal( color, d->spot );
            }
            else if (d->renderingPreviewMode == PreviewToolBar::PreviewTargetImage ||
                     d->renderingPreviewMode == PreviewToolBar::NoPreviewMode)
            {
                color = getSpotColor(TargetPreviewImage);
                emit spotPositionChangedFromTarget( color, d->spot );
            }
            else if (d->renderingPreviewMode == PreviewToolBar::PreviewBothImagesVert)
            {
                if (d->spot.x() > d->rect.width()/2)
                {
                    color = getSpotColor(TargetPreviewImage);
                    emit spotPositionChangedFromTarget(color, QPoint(d->spot.x() - d->rect.width()/2,
                                                       d->spot.y()));
                }
                else
                {
                    color = getSpotColor(OriginalImage);
                    emit spotPositionChangedFromOriginal( color, d->spot );
                }
            }
            else if (d->renderingPreviewMode == PreviewToolBar::PreviewBothImagesVertCont)
            {
                if (d->spot.x() > d->rect.width()/2)
                {
                    color = getSpotColor(TargetPreviewImage);
                    emit spotPositionChangedFromTarget( color, d->spot);
                }
                else
                {
                    color = getSpotColor(OriginalImage);
                    emit spotPositionChangedFromOriginal( color, d->spot );
                }
            }
            else if (d->renderingPreviewMode == PreviewToolBar::PreviewBothImagesHorz)
            {
                if (d->spot.y() > d->rect.height()/2)
                {
                    color = getSpotColor(TargetPreviewImage);
                    emit spotPositionChangedFromTarget(color, QPoint(d->spot.x(),
                                                       d->spot.y() - d->rect.height()/2 ));
                }
                else
                {
                    color = getSpotColor(OriginalImage);
                    emit spotPositionChangedFromOriginal( color, d->spot );
                }
            }
            else if (d->renderingPreviewMode == PreviewToolBar::PreviewBothImagesHorzCont)
            {
                if (d->spot.y() > d->rect.height()/2)
                {
                    color = getSpotColor(TargetPreviewImage);
                    emit spotPositionChangedFromTarget( color, d->spot);
                }
                else
                {
                    color = getSpotColor(OriginalImage);
                    emit spotPositionChangedFromOriginal( color, d->spot );
                }
            }
        }
        else if (e->button() == Qt::LeftButton && d->drawingMask)
        {
            d->drawingMask = false;
            updatePreview();
        }
    }
}

void ImageGuideWidget::mouseMoveEvent(QMouseEvent* e)
{
    if (d->rect.contains(e->x(), e->y()))
    {
        if ( d->focus && d->spotVisible )
        {
            setCursor(Qt::CrossCursor);
            d->spot.setX(e->x()-d->rect.x());
            d->spot.setY(e->y()-d->rect.y());
        }
        else if (d->enableDrawMask)
        {
            setCursor(d->maskCursor);

            if ((e->buttons() & Qt::LeftButton) && d->drawingMask)
            {
                QPoint currentPos = QPoint(e->x()-d->rect.x(), e->y()-d->rect.y());
                drawLineTo(currentPos);
                updatePreview();
            }
        }
    }
    else
    {
        unsetCursor();
    }
}

void ImageGuideWidget::enterEvent(QEvent*)
{
    if ( !d->focus && d->renderingPreviewMode == PreviewToolBar::PreviewToggleOnMouseOver )
    {
        d->onMouseMovePreviewToggled = false;
        updatePixmap();
        repaint();
    }
}

void ImageGuideWidget::leaveEvent(QEvent*)
{
    if ( !d->focus && d->renderingPreviewMode == PreviewToolBar::PreviewToggleOnMouseOver )
    {
        d->onMouseMovePreviewToggled = true;
        updatePixmap();
        update();
    }
}

void ImageGuideWidget::setPoints(const QPolygon& p, bool drawLine)
{
    d->selectedPoints        = p;
    d->drawLineBetweenPoints = drawLine;
    updatePreview();
}

void ImageGuideWidget::resetPoints()
{
    d->selectedPoints.clear();
}

void ImageGuideWidget::drawLineTo(const QPoint& endPoint)
{
    drawLineTo(d->penWidth, d->eraseMask, d->paintColor, d->lastPoint, endPoint);
}

void ImageGuideWidget::drawLineTo(int width, bool erase, const QColor& color, const QPoint& start, const QPoint& end)
{

    QPainter painter(d->maskPixmap);

    if (erase)
    {
        // drawLine() seems to ignore composition modes, use a tmp pixmap and combine it later on
        QPixmap tmpMask(d->maskPixmap->width(), d->maskPixmap->height());
        tmpMask.fill(Qt::transparent);
        QPainter tmpPainter(&tmpMask);

        painter.setRenderHint(QPainter::Antialiasing, false);
        painter.setCompositionMode(QPainter::CompositionMode_DestinationOut);

        QPen eraser;
        eraser.setColor(Qt::yellow);
        eraser.setStyle(Qt::SolidLine);
        eraser.setWidth(width);
        eraser.setCapStyle(Qt::RoundCap);
        eraser.setJoinStyle(Qt::RoundJoin);

        tmpPainter.setPen(eraser);
        tmpPainter.setBrush(QBrush());
        tmpPainter.drawLine(start, end);
        tmpPainter.end();

        painter.drawPixmap(0, 0, tmpMask);
    }
    else
    {
        painter.setPen(QPen(color, width, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter.drawLine(start, end);
    }

    int rad = (width / 2) + 2;

    update(QRect(start, end).normalized().adjusted(-rad, -rad, +rad, +rad));
    d->lastPoint = end;

    painter.end();
}

void ImageGuideWidget::setPaintColor(const QColor& color)
{
    d->paintColor = color;
}

void ImageGuideWidget::setMaskEnabled(bool enabled)
{
    d->enableDrawMask = enabled;
    updateMaskCursor();
    updatePreview();
}

void ImageGuideWidget::setEraseMode(bool erase)
{
    d->eraseMask = erase;
}

QImage ImageGuideWidget::getMask() const
{
    QImage mask = d->maskPixmap->toImage();
    return mask;
}

QPoint ImageGuideWidget::translatePointPosition(QPoint& point)
{
    int x = (int)(point.x() * (float)(d->width)  / (float) d->iface->originalWidth());
    int y = (int)(point.y() * (float)(d->height) / (float) d->iface->originalHeight());
    x += d->rect.x() + 1;
    y += d->rect.y() + 1;
    return (QPoint(x,y));
}

void ImageGuideWidget::setMaskPenSize(int size)
{
    d->penWidth = size;
    updateMaskCursor();
}

void ImageGuideWidget::updateMaskCursor()
{
    int size = d->penWidth;

    if (size > 64)
    {
        size = 64;
    }

    if (size < 3)
    {
        size = 3;
    }

    QPixmap pix(size, size);
    pix.fill(Qt::transparent);

    QPainter p(&pix);
    p.setRenderHint(QPainter::Antialiasing, true);

    p.drawEllipse(1, 1, size-2, size-2);

    d->maskCursor = QCursor(pix);
}

}  // namespace Digikam

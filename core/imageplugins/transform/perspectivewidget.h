/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date  : 2005-01-18
 * Description : a widget class to edit perspective.
 *
 * Copyright (C) 2005-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef PERSPECTIVEWIDGET_H
#define PERSPECTIVEWIDGET_H

// Qt includes

#include <QWidget>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPoint>
#include <QRect>
#include <QColor>

// Local includes

#include "dimg.h"
#include "matrix.h"

namespace Digikam
{
class ImageIface;
}

using namespace Digikam;

namespace DigikamTransformImagePlugin
{

class PerspectiveWidget : public QWidget
{
    Q_OBJECT

public:

    PerspectiveWidget(int width, int height, QWidget* parent=0);
    ~PerspectiveWidget();

    QRect  getTargetSize();
    QPoint getTopLeftCorner();
    QPoint getTopRightCorner();
    QPoint getBottomLeftCorner();
    QPoint getBottomRightCorner();
    void   reset();

    float getAngleTopLeft();
    float getAngleTopRight();
    float getAngleBottomLeft();
    float getAngleBottomRight();

    void  applyPerspectiveAdjustment();

    ImageIface* imageIface() const;

public Q_SLOTS:

    void slotToggleAntiAliasing(bool a);
    void slotToggleDrawWhileMoving(bool draw);
    void slotToggleDrawGrid(bool grid);

    void slotChangeGuideColor(const QColor& color);
    void slotChangeGuideSize(int size);
    void slotInverseTransformationChanged(bool isEnabled);

Q_SIGNALS:

    void signalPerspectiveChanged(const QRect& newSize, float topLeftAngle, float topRightAngle,
                                  float bottomLeftAngle, float bottomRightAngle, bool valid);

protected:

    void paintEvent(QPaintEvent*);
    void resizeEvent(QResizeEvent*);
    void mousePressEvent(QMouseEvent*);
    void mouseReleaseEvent(QMouseEvent*);
    void mouseMoveEvent(QMouseEvent*);

private:  // Widget methods.

    void   updatePixmap();

    void   transformAffine(DImg* orgImage, DImg* destImage,
                           const Matrix& matrix, DColor background);

    QPoint buildPerspective(const QPoint& orignTopLeft, const QPoint& orignBottomRight,
                            const QPoint& transTopLeft, const QPoint& transTopRight,
                            const QPoint& transBottomLeft, const QPoint& transBottomRight,
                            DImg* orgImage=0, DImg* destImage=0,
                            DColor background=DColor());

private:

    class PerspectiveWidgetPriv;
    PerspectiveWidgetPriv* const d;
};

}  // namespace DigikamTransformImagePlugin

#endif /* PERSPECTIVEWIDGET_H */

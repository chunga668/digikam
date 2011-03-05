/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-12-08
 * Description : a widget to display date and time statistics of pictures
 *
 * Copyright (C) 2007-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef TIMELINEWIDGET_H
#define TIMELINEWIDGET_H

// Qt includes

#include <QList>
#include <QString>
#include <QWidget>
#include <QDateTime>
#include <QPaintEvent>
#include <QWheelEvent>
#include <QResizeEvent>
#include <QMouseEvent>

// Local inclues

#include "searchmodificationhelper.h"

namespace Digikam
{

class TimeLineWidget : public QWidget
{
    Q_OBJECT

public:

    enum TimeUnit
    {
        Day = 0,
        Week,
        Month,
        Year
    };

    enum SelectionMode
    {
        Unselected=0,      // No selection.
        FuzzySelection,    // Partially selected.
        Selected           // Fully selected.
    };

    enum ScaleMode
    {
        LinScale=0,        // Linear scale.
        LogScale           // Logarithmic scale.
    };

public:

    TimeLineWidget(QWidget* parent=0);
    ~TimeLineWidget();

    void      setTimeUnit(TimeUnit timeUnit);
    TimeUnit  timeUnit() const;

    void      setScaleMode(ScaleMode scaleMode);
    ScaleMode scaleMode() const;

    void      setCursorDateTime(const QDateTime& dateTime);
    QDateTime cursorDateTime() const;
    int       cursorInfo(QString& infoDate);

    /** Return a list of Date-Range based on selection performed on days-map */
    DateRangeList selectedDateRange(int& totalCount);
    void          setSelectedDateRange(const DateRangeList& list);

    int  totalIndex();
    int  indexForRefDateTime();
    int  indexForCursorDateTime();
    void setCurrentIndex(int index);

Q_SIGNALS:

    void signalCursorPositionChanged();
    void signalSelectionChanged();
    void signalRefDateTimeChanged();
    void signalDateMapChanged();

public Q_SLOTS:

    void slotDatesMap(const QMap<QDateTime, int>&);
    void slotPrevious();
    void slotNext();
    void slotBackward();
    void slotForward();
    void slotResetSelection();

private Q_SLOTS:

    void slotThemeChanged();
    void slotFlickerTimer();

private:

    void showEvent(QShowEvent*);
    void hideEvent(QHideEvent*);

    QDateTime     prevDateTime(const QDateTime& dt);
    QDateTime     nextDateTime(const QDateTime& dt);

    int           maxCount();
    int           indexForDateTime(const QDateTime& date);
    int           statForDateTime(const QDateTime& dt, SelectionMode* selected);
    void          setRefDateTime(const QDateTime& dateTime);

    void          paintEvent(QPaintEvent*);
    void          wheelEvent(QWheelEvent*);

    void          mousePressEvent(QMouseEvent*);
    void          mouseMoveEvent(QMouseEvent*);
    void          mouseReleaseEvent(QMouseEvent*);

    QDateTime     dateTimeForPoint(const QPoint& pt, bool* isOnSelectionArea);
    QDateTime     firstDayOfWeek(int year, int weekNumber);

    void          resetSelection();
    void          setDateTimeSelected(const QDateTime& dt, SelectionMode selected);
    void          setDaysRangeSelection(const QDateTime& dts, const QDateTime& dte, SelectionMode selected);
    SelectionMode checkSelectionForDaysRange(const QDateTime& dts, const QDateTime& dte);
    void          updateWeekSelection(const QDateTime& dts, const QDateTime& dte);
    void          updateMonthSelection(const QDateTime& dts, const QDateTime& dte);
    void          updateYearSelection(const QDateTime& dts, const QDateTime& dte);
    void          updateAllSelection();

    // helper methods for painting
    int           calculateTop(int& val);
    void          paintItem(QPainter& p, const QRect& barRect,
                            const QDateTime& ref, const int& separatorPosition,
                            const QColor& dateColor, const QColor& subDateColor);

private:

    class TimeLineWidgetPriv;
    TimeLineWidgetPriv* const d;
};

}  // namespace Digikam

#endif // TIMELINEWIDGET_H

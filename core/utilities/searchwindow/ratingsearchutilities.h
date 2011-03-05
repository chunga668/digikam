/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-03-14
 * Description : User interface for searches
 *
 * Copyright (C) 2008 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef RATINGSEARCHUTILITIES_H
#define RATINGSEARCHUTILITIES_H

// Qt includes

#include <QAbstractListModel>
#include <QItemDelegate>
#include <QLabel>
#include <QComboBox>

// Local includes

#include "ratingwidget.h"
#include "comboboxutilities.h"

class QTreeView;
class QVBoxLayout;

namespace Digikam
{

class RatingComboBoxModel;
class RatingComboBoxWidget;
class RatingComboBox : public ModelIndexBasedComboBox
{
    Q_OBJECT

public:

    /** An advanced widget for entering a rating,
     *  including support for Null and NoRating values
     */

    enum RatingValue
    {
        /** The rating value. All values except Null correspond to
         *  the integers used by the database. */
        Null     = -2,
        NoRating = -1,
        Rating0  = 0,
        Rating1  = 1,
        Rating2  = 2,
        Rating3  = 3,
        Rating4  = 4,
        Rating5  = 5
    };

    RatingComboBox(QWidget* parent = 0);

    void setRatingValue(RatingValue value);
    RatingValue ratingValue() const;

Q_SIGNALS:

    void ratingValueChanged(int value);

protected Q_SLOTS:

    void currentValueChanged(const QModelIndex& current, const QModelIndex& previous);
    void ratingWidgetChanged(int);

protected:

    RatingComboBoxModel*     m_model;
    RatingComboBoxWidget*    m_ratingWidget;

private:

    bool                     m_syncing;
};

// -- Internal classes ----------------------------------------------------------------

class RatingStarDrawer
{
public:

    RatingStarDrawer();

    QRect drawStarPolygons(QPainter* p, int numberOfStars) const;

protected:

    QPolygon                         starPolygon;
    QSize                            starPolygonSize;
};

// -------------------------------------------------------------------------

class RatingComboBoxWidget : public RatingWidget, public RatingStarDrawer
{
    Q_OBJECT

public:

    /// Internal
    // Sub-classing the classic RatingWidget,
    // this provides support for the Null and NoRating states.

    RatingComboBoxWidget(QWidget* parent = 0);

    RatingComboBox::RatingValue ratingValue() const;
    void setRatingValue(RatingComboBox::RatingValue value);

Q_SIGNALS:

    void ratingValueChanged(int value);

protected Q_SLOTS:

    void slotRatingChanged(int);

protected:

    virtual void paintEvent(QPaintEvent*);

    RatingComboBox::RatingValue m_value;
};

// -------------------------------------------------------------------------

class RatingComboBoxModel : public QAbstractListModel
{
public:

    enum CustomRoles
    {
        RatingRole = Qt::UserRole
    };

    RatingComboBoxModel(QObject* parent = 0);

    virtual int rowCount(const QModelIndex& parent) const;
    virtual QVariant data(const QModelIndex& index, int role) const;
    virtual QModelIndex index(int row, int column = 0, const QModelIndex& parent = QModelIndex()) const;

    QModelIndex indexForRatingValue(RatingComboBox::RatingValue value) const;

protected:

    QVariant ratingValueToDisplay(RatingComboBox::RatingValue value) const;

    QList<RatingComboBox::RatingValue> m_entries;
};

// -------------------------------------------------------------------------

class RatingComboBoxDelegate : public QItemDelegate, public RatingStarDrawer
{
public:

    RatingComboBoxDelegate(QObject* parent = 0);

    virtual QSize sizeHint ( const QStyleOptionViewItem& option, const QModelIndex& index ) const;
    virtual void paint ( QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const;

protected:

    void drawRating(QPainter* painter, const QRect& rect, int rating, bool selectable) const;
};

} // namespace Digikam

#endif // RATINGSEARCHUTILITIES_H

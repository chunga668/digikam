/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-01-30
 * Description : a button bar to navigate between album items
 *               using status bar.
 *
 * Copyright (C) 2007-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef STATUS_NAVIGATE_BAR_H
#define STATUS_NAVIGATE_BAR_H

// Qt includes

#include <QtGui/QWidget>
#include <QtCore/QString>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class StatusNavigateBarPriv;

class DIGIKAM_EXPORT StatusNavigateBar : public QWidget
{
    Q_OBJECT

public:

    enum CurrentItemPosition
    {
        ItemCurrent=0,
        ItemFirst,
        ItemLast,
        NoNavigation
    };

public:

    StatusNavigateBar(QWidget* parent=0);
    ~StatusNavigateBar();

    void setNavigateBarState(bool hasPrev, bool hasNext);
    void setButtonsState(int itemType);
    int  getButtonsState();

Q_SIGNALS:

    void signalFirstItem();
    void signalPrevItem();
    void signalNextItem();
    void signalLastItem();

private :

    StatusNavigateBarPriv* const d;
};

}  // namespace Digikam

#endif /* STATUS_NAVIGATE_BAR_H */

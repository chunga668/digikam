/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-12-10
 * Description : tool tip widget for iconview, thumbbar, and folderview items
 *
 * Copyright (C) 2008-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DITEMTOOLTIP_H
#define DITEMTOOLTIP_H

// Qt includes

#include <QtGui/QFont>
#include <QtGui/QLabel>
#include <QtCore/QString>
#include <QtCore/QEvent>
#include <QtGui/QResizeEvent>
#include <QtGui/QTextDocument>

// KDE includes

#include <klocale.h>
#include <kglobalsettings.h>

// Local includes

#include "themeengine.h"
#include "digikam_export.h"

namespace Digikam
{

class DItemToolTipPriv;

class DIGIKAM_EXPORT DToolTipStyleSheet
{

public:

    DToolTipStyleSheet(const QFont& font = KGlobalSettings::generalFont());
    QString breakString(const QString& input);
    QString elidedText(const QString& input, Qt::TextElideMode mode);

    const int maxStringLength;

    QString   unavailable;
    QString   tipHeader;
    QString   tipFooter;
    QString   headBeg;
    QString   headEnd;
    QString   cellBeg;
    QString   cellMid;
    QString   cellEnd;
    QString   cellSpecBeg;
    QString   cellSpecMid;
    QString   cellSpecEnd;
};

class DIGIKAM_EXPORT DItemToolTip : public QLabel
{

public:

    DItemToolTip(QWidget* parent = 0);
    virtual ~DItemToolTip();

protected:

    bool    event(QEvent*);
    void    resizeEvent(QResizeEvent*);
    void    paintEvent(QPaintEvent*);
    void    updateToolTip();
    bool    toolTipIsEmpty() const;
    void    reposition();
    void    renderArrows();

    virtual QRect   repositionRect()=0;
    virtual QString tipContents()=0;

private:

    DItemToolTipPriv* const d;
};

}  // namespace Digikam

#endif /* DITEMTOOLTIP_H */

/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-05-22
 * Description : header list view item
 *
 * Copyright (C) 2010 by Andi Clemens <andi dot clemens at gmx dot net>
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

#include "dbheaderlistitem.moc"

// Qt includes

#include <QPalette>
#include <QFont>
#include <QPainter>

// KDE includes

#include <klocale.h>

// Local includes

#include "themeengine.h"

namespace Digikam
{

DbHeaderListItem::DbHeaderListItem(QTreeWidget* parent, const QString& key)
    : QObject(parent), QTreeWidgetItem(parent)
{
    // Reset all item flags: item is not selectable.
    setFlags(Qt::ItemIsEnabled);

    setDisabled(false);
    setExpanded(true);

    setFirstColumnSpanned(true);
    setTextAlignment(0, Qt::AlignCenter);
    QFont fn0(font(0));
    fn0.setBold(true);
    fn0.setItalic(false);
    setFont(0, fn0);
    QFont fn1(font(1));
    fn1.setBold(true);
    fn1.setItalic(false);
    setFont(1, fn1);
    setText(0, key);
    slotThemeChanged();

    connect(ThemeEngine::instance(), SIGNAL(signalThemeChanged()),
            this, SLOT(slotThemeChanged()));
}

DbHeaderListItem::~DbHeaderListItem()
{
}

void DbHeaderListItem::slotThemeChanged()
{
    setBackground(0, QBrush(ThemeEngine::instance()->thumbSelColor()));
    setBackground(1, QBrush(ThemeEngine::instance()->thumbSelColor()));
    setForeground(0, QBrush(ThemeEngine::instance()->textSelColor()));
    setForeground(1, QBrush(ThemeEngine::instance()->textSelColor()));
}

}  // namespace Digikam

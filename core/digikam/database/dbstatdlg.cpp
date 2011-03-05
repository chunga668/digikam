/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-05-28
 * Description : database statistics dialog
 *
 * Copyright (C) 2009-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "dbstatdlg.h"

// Qt includes

#include <QStringList>
#include <QString>
#include <QFont>
#include <QTreeWidget>
#include <QHeaderView>

// KDE includes

#include <klocale.h>
#include <kiconloader.h>
#include <kapplication.h>
#include <kstandarddirs.h>
#include <kaboutdata.h>

// Local includes

#include "daboutdata.h"
#include "albumdb.h"
#include "albumsettings.h"
#include "databaseaccess.h"
#include "config-digikam.h"

namespace Digikam
{

DBStatDlg::DBStatDlg(QWidget* parent)
    : InfoDlg(parent)
{
    kapp->setOverrideCursor(Qt::WaitCursor);

    setCaption(i18n("Database Statistics"));
    listView()->setHeaderLabels(QStringList() << i18n("Format") << i18n("Count"));

    // get image format statistics
    int total                   = 0;
    QMap<QString, int>     stat = DatabaseAccess().db()->getImageFormatStatistics();
    QMap<QString, QString> map;

    for (QMap<QString, int>::const_iterator it = stat.constBegin(); it != stat.constEnd(); ++it)
    {
        total += it.value();
        map.insert(it.key(), QString::number(it.value()));
    }

    setInfoMap(map);

    // To see total count of items at end of list.
    QTreeWidgetItem* ti = new QTreeWidgetItem(listView(), QStringList()
            << i18n("Total Items") << QString::number(total));
    QFont ft = ti->font(0);
    ft.setBold(true);
    ti->setFont(0, ft);
    ft = ti->font(1);
    ft.setBold(true);
    ti->setFont(1, ft);

    // Add space.
    new QTreeWidgetItem(listView(), QStringList());

    // get album statistics
    int albums                 = DatabaseAccess().db()->scanAlbums().count();
    new QTreeWidgetItem(listView(), QStringList() << i18n("Albums") << QString::number(albums));

    // get tags statistics
    int tags                  = DatabaseAccess().db()->scanTags().count();
    new QTreeWidgetItem(listView(), QStringList() << i18n("Tags") << QString::number(tags));

    // Add space.
    new QTreeWidgetItem(listView(), QStringList());

    // Database Backend information

    QString dbBe = AlbumSettings::instance()->getDatabaseType();
    new QTreeWidgetItem(listView(), QStringList() << i18n("Database backend")
                        << dbBe);

    if (dbBe != QString("QSQLITE"))
    {
        QString internal = AlbumSettings::instance()->getInternalDatabaseServer() ? i18n("Yes") : i18n("No");
        new QTreeWidgetItem(listView(), QStringList() << i18n("Database internal server") << internal);
    }

    kapp->restoreOverrideCursor();
}

DBStatDlg::~DBStatDlg()
{
}

}  // namespace Digikam

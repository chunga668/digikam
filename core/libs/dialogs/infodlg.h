/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-07-11
 * Description : general info list dialog
 *
 * Copyright (C) 2008-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009 by Andi Clemens <andi dot clemens at gmx dot net>
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

#ifndef INFODLG_H
#define INFODLG_H

// Qt includes

#include <QtCore/QMap>

// KDE includes

#include <kdialog.h>

// Local includes

#include "digikam_export.h"

class QTreeWidget;

namespace Digikam
{

class InfoDlgPriv;

class DIGIKAM_EXPORT InfoDlg : public KDialog
{
    Q_OBJECT

public:

    InfoDlg(QWidget* parent);
    virtual ~InfoDlg();

    virtual void setInfoMap(const QMap<QString, QString>& list);
    QTreeWidget* listView() const;

private Q_SLOTS:

    virtual void slotCopy2ClipBoard();

private:

    InfoDlgPriv* const d;
};

}  // namespace Digikam

#endif  // INFODLG_H

/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-02-10
 * Description : Camera type selection dialog
 *
 * Copyright (C) 2003-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2006-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef CAMERASELECTION_H
#define CAMERASELECTION_H

// Qt includes

#include <QString>
#include <QStringList>

// KDE includes

#include <kdialog.h>

// Local includes

#include "searchtextbar.h"

class QTreeWidgetItem;

namespace Digikam
{

class CameraSelectionPriv;

class CameraSelection : public KDialog
{
    Q_OBJECT

public:

    CameraSelection( QWidget* parent = 0 );
    ~CameraSelection();

    void setCamera(const QString& title, const QString& model,
                   const QString& port,  const QString& path);

    QString currentTitle();
    QString currentModel();
    QString currentPortPath();
    QString currentCameraPath();

Q_SIGNALS:

    void signalOkClicked(const QString& title, const QString& model,
                         const QString& port,  const QString& path);

private:

    void getCameraList();
    void getSerialPortList();

private Q_SLOTS:

    void slotPTPCameraLinkUsed();
    void slotUMSCameraLinkUsed();
    void slotSelectionChanged(QTreeWidgetItem*, int);
    void slotPortChanged();
    void slotOkClicked();
    void slotSearchTextChanged(const SearchTextSettings&);

private:

    CameraSelectionPriv* const d;
};

}  // namespace Digikam

#endif // CAMERASELECTION_H

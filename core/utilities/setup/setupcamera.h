/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-02-10
 * Description : camera setup tab.
 *
 * Copyright (C) 2003-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2003-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef SETUPCAMERA_H
#define SETUPCAMERA_H

// Qt includes

#include <QScrollArea>

// Local includes

#include "dbusydlg.h"

namespace Digikam
{

class SetupCameraPriv;

class SetupCamera : public QScrollArea
{
    Q_OBJECT

public:

    SetupCamera( QWidget* parent = 0 );
    ~SetupCamera();

    void applySettings();

private Q_SLOTS:

    void slotProcessGphotoUrl(const QString& url);

    void slotSelectionChanged();

    void slotAddCamera();
    void slotRemoveCamera();
    void slotEditCamera();
    void slotAutoDetectCamera();

    void slotAddedCamera(const QString& title, const QString& model,
                         const QString& port,  const QString& path);
    void slotEditedCamera(const QString& title, const QString& model,
                          const QString& port,  const QString& path);

private:

    SetupCameraPriv* const d;
};

// -------------------------------------------------------------------------

class CameraAutoDetectThread : public DBusyThread
{
    Q_OBJECT

public:

    CameraAutoDetectThread(QObject* parent);
    virtual ~CameraAutoDetectThread();

    int     result();
    QString model() const;
    QString port() const;

private:

    void run();

private:

    int     m_result;

    QString m_model;
    QString m_port;
};

}  // namespace Digikam

#endif // SETUPCAMERA_H

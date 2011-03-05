/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-02-03
 * Description : Cameras list container
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

#ifndef CAMERALIST_H
#define CAMERALIST_H

// Qt includes

#include <QList>
#include <QObject>

class QString;
class QDateTime;

class KAction;

namespace Digikam
{

class CameraType;
class CameraListPrivate;

class CameraList : public QObject
{
    Q_OBJECT

public:

    CameraList(QObject* parent, const QString& file);
    ~CameraList();

    bool load();
    bool save();
    void clear();

    void insert(CameraType* ctype);
    void remove(CameraType* ctype);

    CameraType*         find(const QString& title);
    CameraType*         autoDetect(bool& retry);
    QList<CameraType*>* cameraList();

    bool changeCameraStartIndex(const QString& cameraTitle, int startIndex);

    static bool findConnectedCamera(int vendorId, int productId, QString& model, QString& port);

    static CameraList* defaultList();

Q_SIGNALS:

    void signalCameraAdded(CameraType*);
    void signalCameraRemoved(KAction*);

private:

    void insertPrivate(CameraType* ctype);
    void removePrivate(CameraType* ctype);

private:

    static CameraList* m_defaultList;
    CameraListPrivate* const d;
};

}  // namespace Digikam

#endif /* CAMERALIST_H */

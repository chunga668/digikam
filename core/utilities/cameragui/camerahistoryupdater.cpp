/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-10-16
 * Description : history updater thread for cameraui
 *
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

#include "camerahistoryupdater.moc"

// Qt includes

#include <QList>
#include <QMutex>
#include <QVariant>
#include <QWaitCondition>
#include <QWidget>

// KDE includes

#include <kdebug.h>

// Local includes

#include "downloadhistory.h"

namespace Digikam
{

class CameraHistoryUpdaterPriv
{
    typedef QList<CHUpdateItem> CHUpdateItemsList;

public:

    CameraHistoryUpdaterPriv() :
        close(false),
        canceled(false),
        running(false)
    {}

    bool              close;
    bool              canceled;
    bool              running;

    QMutex            mutex;
    QWaitCondition    condVar;
    CHUpdateItemsList updateItems;
};

// --------------------------------------------------------

CameraHistoryUpdater::CameraHistoryUpdater(QWidget* parent)
    : QThread(parent), d(new CameraHistoryUpdaterPriv)
{
    qRegisterMetaType<CHUpdateItemMap>("CHUpdateItemMap");
}

CameraHistoryUpdater::~CameraHistoryUpdater()
{
    // clear updateItems, stop processing
    slotCancel();

    // stop thread
    {
        QMutexLocker lock(&d->mutex);
        d->running = false;
        d->condVar.wakeAll();
    }
    wait();

    delete d;
}

void CameraHistoryUpdater::slotCancel()
{
    d->canceled = true;
    QMutexLocker lock(&d->mutex);
    d->updateItems.clear();
}

void CameraHistoryUpdater::run()
{
    while (d->running)
    {
        {
            CHUpdateItem item;

            QMutexLocker lock(&d->mutex);

            if (!d->updateItems.isEmpty())
            {
                item = d->updateItems.takeFirst();
                sendBusy(true);
                proccessMap(item.first, item.second);
            }
            else
            {
                sendBusy(false);
                d->condVar.wait(&d->mutex);
                continue;
            }
        }
    }

    sendBusy(false);
}

void CameraHistoryUpdater::sendBusy(bool val)
{
    emit signalBusy(val);
}

void CameraHistoryUpdater::addItems(const QByteArray& id, CHUpdateItemMap& map)
{
    if (map.empty())
    {
        return;
    }

    QMutexLocker lock(&d->mutex);
    d->running = true;
    d->canceled = false;
    d->updateItems << CHUpdateItem(id, map);

    if (!isRunning())
    {
        start(LowPriority);
    }

    d->condVar.wakeAll();
}

void CameraHistoryUpdater::proccessMap(const QByteArray& id, CHUpdateItemMap& map)
{
    CHUpdateItemMap& _map        = map;
    CHUpdateItemMap::iterator it = _map.begin();

    do
    {
        // We query database to check if (*it).have been already downloaded from camera.
        switch (DownloadHistory::status(id, (*it).name, (*it).size, (*it).mtime))
        {
            case DownloadHistory::NotDownloaded:
                (*it).downloaded = GPItemInfo::NewPicture;
                break;
            case DownloadHistory::Downloaded:
                (*it).downloaded = GPItemInfo::DownloadedYes;
                break;
            default: // DownloadHistory::StatusUnknown
                (*it).downloaded = GPItemInfo::DownloadUnknown;
                break;
        }

        ++it;

    }
    while (it != map.end() && !d->canceled);

    emit signalHistoryMap(_map);
}

}  // namespace Digikam

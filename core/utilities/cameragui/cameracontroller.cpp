/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-09-17
 * Description : digital camera controller
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2006-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "cameracontroller.moc"

// C ANSI includes

extern "C"
{
#include <unistd.h>
}

// C++ includes

#include <typeinfo>
#include <cstdio>

// Qt includes

#include <QMutex>
#include <QWaitCondition>
#include <QVariant>
#include <QImage>
#include <QFile>
#include <QRegExp>
#include <QFileInfo>
#include <QPointer>

// KDE includes

#include <kiconloader.h>
#include <kio/renamedialog.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kurl.h>
#include <kdebug.h>

// Local includes

#include "templatemanager.h"
#include "thumbnailsize.h"
#include "imagewindow.h"
#include "gpcamera.h"
#include "umscamera.h"
#include "dmetadata.h"
#include "jpegutils.h"

namespace Digikam
{

class CameraCommand
{
public:

    enum Action
    {
        gp_none = 0,
        gp_connect,
        gp_cancel,
        gp_cameraInformation,
        gp_listfolders,
        gp_listfiles,
        gp_download,
        gp_upload,
        gp_delete,
        gp_lock,
        gp_thumbnails,
        gp_exif,
        gp_open,
        gp_freeSpace,
        gp_preview,
        gp_capture
    };

    Action                 action;
    QMap<QString,QVariant> map;
};

class CameraController::CameraControllerPriv
{
public:

    CameraControllerPriv() :
        close(false),
        overwriteAll(false),
        skipAll(false),
        canceled(false),
        running(false),
        downloadTotal(0),
        parent(0),
        timer(0),
        camera(0)
    {
    }

    bool                  close;
    bool                  overwriteAll;
    bool                  skipAll;
    bool                  canceled;
    bool                  running;

    int                   downloadTotal;

    QWidget*              parent;

    QTimer*               timer;

    DKCamera*             camera;

    QMutex                mutex;
    QWaitCondition        condVar;

    QList<CameraCommand*> commands;
};

CameraController::CameraController(QWidget* parent,
                                   const QString& title, const QString& model,
                                   const QString& port, const QString& path)
    : QThread(parent), d(new CameraControllerPriv)
{
    d->parent = parent;

    // URL parsing (c) Stephan Kulow
    if (path.startsWith(QLatin1String("camera:/")))
    {
        KUrl url(path);
        kDebug() << "path " << path << " " << url <<  " " << url.host();
        QString xport = url.host();

        if (xport.startsWith(QLatin1String("usb:")))
        {
            kDebug() << "xport " << xport;
            QRegExp x = QRegExp("(usb:[0-9,]*)");

            if (x.indexIn(xport) != -1)
            {
                QString usbport = x.cap(1);
                kDebug() << "USB " << xport << " " << usbport;
                // if ((xport == usbport) || ((count == 1) && (xport == "usb:"))) {
                //   model = xmodel;
                d->camera = new GPCamera(title, url.user(), "usb:", "/");
                // }
            }
        }
    }

    if (!d->camera)
    {
        if (model.toLower() == "directory browse")
        {
            d->camera = new UMSCamera(title, model, port, path);
        }
        else
        {
            d->camera = new GPCamera(title, model, port, path);
        }
    }

    // setup inter-thread signals

    qRegisterMetaType<GPItemInfo>("GPItemInfo");
    qRegisterMetaType<GPItemInfoList>("GPItemInfoList");

    connect(this, SIGNAL(signalInternalCheckRename(const QString&, const QString&, const QString&, const QString&)),
            this, SLOT(slotCheckRename(const QString&, const QString&, const QString&, const QString&)),
            Qt::BlockingQueuedConnection);

    connect(this, SIGNAL(signalInternalDownloadFailed(const QString&, const QString&)),
            this, SLOT(slotDownloadFailed(const QString&, const QString&)),
            Qt::BlockingQueuedConnection);

    connect(this, SIGNAL(signalInternalUploadFailed(const QString&, const QString&, const QString&)),
            this, SLOT(slotUploadFailed(const QString&, const QString&, const QString&)),
            Qt::BlockingQueuedConnection);

    connect(this, SIGNAL(signalInternalDeleteFailed(const QString&, const QString&)),
            this, SLOT(slotDeleteFailed(const QString&, const QString&)),
            Qt::BlockingQueuedConnection);

    connect(this, SIGNAL(signalInternalLockFailed(const QString&, const QString&)),
            this, SLOT(slotLockFailed(const QString&, const QString&)),
            Qt::BlockingQueuedConnection);

    connect(this, SIGNAL(signalInternalOpen(const QString&, const QString&, const QString&)),
            this, SLOT(slotOpen(const QString&, const QString&, const QString&)));

    d->running = true;
    start();
}

CameraController::~CameraController()
{
    // clear commands, stop camera
    slotCancel();

    // stop thread
    {
        QMutexLocker lock(&d->mutex);
        d->running = false;
        d->condVar.wakeAll();
    }
    wait();

    delete d->camera;
    delete d;
}

bool CameraController::cameraThumbnailSupport()
{
    if (!d->camera)
    {
        return false;
    }

    return d->camera->thumbnailSupport();
}

bool CameraController::cameraDeleteSupport()
{
    if (!d->camera)
    {
        return false;
    }

    return d->camera->deleteSupport();
}

bool CameraController::cameraUploadSupport()
{
    if (!d->camera)
    {
        return false;
    }

    return d->camera->uploadSupport();
}

bool CameraController::cameraMkDirSupport()
{
    if (!d->camera)
    {
        return false;
    }

    return d->camera->mkDirSupport();
}

bool CameraController::cameraDelDirSupport()
{
    if (!d->camera)
    {
        return false;
    }

    return d->camera->delDirSupport();
}

bool CameraController::cameraCaptureImageSupport()
{
    if (!d->camera)
    {
        return false;
    }

    return d->camera->captureImageSupport();
}

QString CameraController::cameraPath()
{
    if (!d->camera)
    {
        return QString();
    }

    return d->camera->path();
}

QString CameraController::cameraTitle()
{
    if (!d->camera)
    {
        return QString();
    }

    return d->camera->title();
}

DKCamera::CameraDriverType CameraController::cameraDriverType()
{
    if (!d->camera)
    {
        return DKCamera::UMSDriver;
    }

    return d->camera->cameraDriverType();
}

QByteArray CameraController::cameraMD5ID()
{
    if (!d->camera)
    {
        return QByteArray();
    }

    return d->camera->cameraMD5ID();
}

QPixmap CameraController::mimeTypeThumbnail(const QString& itemName)
{
    if (!d->camera)
    {
        return QPixmap();
    }

    QFileInfo fi(itemName);
    QString mime = d->camera->mimeType(fi.suffix().toLower());

    if (mime.startsWith(QLatin1String("image/x-raw")))
    {
        return DesktopIcon("kdcraw");
    }
    else if (mime.startsWith(QLatin1String("image/")))
    {
        return DesktopIcon("image-x-generic");
    }
    else if (mime.startsWith(QLatin1String("video/")))
    {
        return DesktopIcon("video-x-generic");
    }
    else if (mime.startsWith(QLatin1String("audio/")))
    {
        return DesktopIcon("audio-x-generic");
    }

    return DesktopIcon("unknown");
}

void CameraController::slotCancel()
{
    d->canceled = true;
    d->camera->cancel();
    QMutexLocker lock(&d->mutex);
    d->commands.clear();
}

void CameraController::run()
{
    while (d->running)
    {
        CameraCommand* command;

        {
            QMutexLocker lock(&d->mutex);

            if (!d->commands.isEmpty())
            {
                command = d->commands.takeFirst();
            }
            else
            {
                sendBusy(false);
                d->condVar.wait(&d->mutex);
                continue;
            }
        }

        sendBusy(true);
        executeCommand(command);
        delete command;
    }

    sendBusy(false);
}

void CameraController::executeCommand(CameraCommand* cmd)
{
    switch (cmd->action)
    {
        case(CameraCommand::gp_connect):
        {
            sendLogMsg(i18n("Connecting to camera..."));

            bool result = d->camera->doConnect();

            emit signalConnected(result);

            if (result)
            {
                sendLogMsg(i18n("Connection established."));
            }
            else
            {
                sendLogMsg(i18n("Connection failed."));
            }

            break;
        }
        case(CameraCommand::gp_cameraInformation):
        {
            sendLogMsg(i18n("Getting camera information..."));

            QString summary, manual, about;

            d->camera->cameraSummary(summary);
            d->camera->cameraManual(manual);
            d->camera->cameraAbout(about);

            emit signalCameraInformation(summary, manual, about);
            break;
        }
        case(CameraCommand::gp_freeSpace):
        {
            sendLogMsg(i18n("Getting available free space on camera..."));
            unsigned long kBSize  = 0;
            unsigned long kBAvail = 0;
            d->camera->getFreeSpace(kBSize, kBAvail);
            emit signalFreeSpace(kBSize, kBAvail);
            break;
        }
        case(CameraCommand::gp_preview):
        {
            sendLogMsg(i18n("Getting preview..."));
            QImage preview;
            d->camera->getPreview(preview);
            emit signalPreview(preview);
            break;
        }
        case(CameraCommand::gp_capture):
        {
            sendLogMsg(i18n("Capture image..."));
            GPItemInfo itemInfo;
            d->camera->capture(itemInfo);
            emit signalUploaded(itemInfo);
            break;
        }
        case(CameraCommand::gp_listfolders):
        {
            sendLogMsg(i18n("Listing folders..."));

            QStringList folderList;
            folderList.append(d->camera->path());
            d->camera->getAllFolders(d->camera->path(), folderList);

            emit signalFolderList(folderList);
            sendLogMsg(i18n("The folders have been listed."));

            break;
        }
        case(CameraCommand::gp_listfiles):
        {
            QString folder = cmd->map["folder"].toString();

            sendLogMsg(i18n("Listing files in %1...", folder));

            GPItemInfoList itemsList;

            if (!d->camera->getItemsInfoList(folder, itemsList, true))
            {
                sendLogMsg(i18n("Failed to list files in %1.", folder), DHistoryView::ErrorEntry);
            }

            if (!itemsList.isEmpty())
            {
                emit signalFileList(itemsList);
            }

            sendLogMsg(i18n("The files in %1 have been listed.", folder));

            break;
        }
        case(CameraCommand::gp_thumbnails):
        {
            QList<QVariant> list = cmd->map["list"].toList();

            for (QList<QVariant>::const_iterator it = list.constBegin(); it != list.constEnd(); ++it)
            {
                if (d->canceled)
                {
                    break;
                }

                QString folder = (*it).toStringList()[0];
                QString file   = (*it).toStringList()[1];

                sendLogMsg(i18n("Getting thumbnails for %1...", file), DHistoryView::StartingEntry, folder, file);
                QImage thumbnail;

                if (d->camera->getThumbnail(folder, file, thumbnail))
                {
                    thumbnail = thumbnail.scaled(ThumbnailSize::Huge, ThumbnailSize::Huge, Qt::KeepAspectRatio);
                    emit signalThumbnail(folder, file, thumbnail);
                }
                else
                {
                    emit signalThumbnailFailed(folder, file);
                }
            }

            break;
        }
        case(CameraCommand::gp_exif):
        {
            QString folder = cmd->map["folder"].toString();
            QString file   = cmd->map["file"].toString();

            if ( typeid(*(d->camera)) == typeid(UMSCamera) )
            {
                emit signalExifFromFile(folder, file);
            }
            else
            {
                sendLogMsg(i18n("Getting EXIF information for %1...", file), DHistoryView::StartingEntry, folder, file);

                char* edata = 0;
                int   esize = 0;
                d->camera->getExif(folder, file, &edata, esize);

                if (edata || esize)
                {
                    QByteArray  ba;
                    QDataStream ds(&ba, QIODevice::WriteOnly);
                    ds.writeRawData(edata, esize);
                    delete [] edata;

                    emit signalExifData(ba);
                }
            }

            break;
        }
        case(CameraCommand::gp_download):
        {
            QString   folder         = cmd->map["folder"].toString();
            QString   file           = cmd->map["file"].toString();
            QString   dest           = cmd->map["dest"].toString();
            bool      autoRotate     = cmd->map["autoRotate"].toBool();
            bool      fixDateTime    = cmd->map["fixDateTime"].toBool();
            QDateTime newDateTime    = cmd->map["newDateTime"].toDateTime();
            QString   templateTitle  = cmd->map["template"].toString();
            bool      convertJpeg    = cmd->map["convertJpeg"].toBool();
            QString   losslessFormat = cmd->map["losslessFormat"].toString();
            sendLogMsg(i18n("Downloading file %1...", file), DHistoryView::StartingEntry, folder, file);

            // download to a temp file

            emit signalDownloaded(folder, file, GPItemInfo::DownloadStarted);

            KUrl tempURL(dest);
            tempURL = tempURL.upUrl();
            tempURL.addPath(QString(".digikam-camera-tmp1-%1").arg(getpid()).append(file));
            kDebug() << "Downloading: " << file << " using (" << tempURL << ")";
            QString temp = tempURL.toLocalFile();

            bool result  = d->camera->downloadItem(folder, file, tempURL.toLocalFile());

            if (!result)
            {
                unlink(QFile::encodeName(tempURL.toLocalFile()));
                emit signalDownloaded(folder, file, GPItemInfo::DownloadFailed);
                sendLogMsg(i18n("Failed to download %1...", file), DHistoryView::ErrorEntry, folder, file);
                break;
            }
            else if (isJpegImage(tempURL.toLocalFile()))
            {
                // Possible modification operations. Only apply it to JPEG for the moment.

                if (autoRotate)
                {
                    kDebug() << "Exif autorotate: " << file << " using (" << tempURL << ")";
                    sendLogMsg(i18n("EXIF rotating file %1...", file), DHistoryView::StartingEntry, folder, file);
                    exifTransform(tempURL.toLocalFile(), file);
                }

                if (!templateTitle.isNull() || fixDateTime)
                {
                    kDebug() << "Set metadata from: " << file << " using (" << tempURL << ")";
                    DMetadata metadata(tempURL.toLocalFile());

                    if (fixDateTime)
                    {
                        sendLogMsg(i18n("Fix Internal date to file %1...", file), DHistoryView::StartingEntry, folder, file);
                        metadata.setImageDateTime(newDateTime, true);
                    }

                    TemplateManager* tm = TemplateManager::defaultManager();

                    if (tm && !templateTitle.isEmpty())
                    {
                        kDebug() << "Metadata template title : " << templateTitle;

                        if (templateTitle == Template::removeTemplateTitle())
                        {
                            metadata.removeMetadataTemplate();
                        }
                        else if (templateTitle.isEmpty())
                        {
                            // Nothing to do.
                        }
                        else
                        {
                            sendLogMsg(i18n("Apply Metadata template to file %1...", file), DHistoryView::StartingEntry, folder, file);
                            metadata.removeMetadataTemplate();
                            metadata.setMetadataTemplate(tm->findByTitle(templateTitle));
                        }
                    }

                    metadata.applyChanges();
                }

                // Convert JPEG file to lossless format if necessary,
                // and move converted image to destination.

                if (convertJpeg)
                {
                    kDebug() << "Convert to LossLess: " << file << " using (" << tempURL << ")";
                    sendLogMsg(i18n("Converting %1 to lossless file format...", file), DHistoryView::StartingEntry, folder, file);

                    KUrl tempURL2(dest);
                    tempURL2 = tempURL2.upUrl();
                    tempURL2.addPath(QString(".digikam-camera-tmp2-%1").arg(getpid()).append(file));
                    temp     = tempURL2.toLocalFile();

                    if (!jpegConvert(tempURL.toLocalFile(), tempURL2.toLocalFile(), file, losslessFormat))
                    {
                        // convert failed. delete the temp file
                        unlink(QFile::encodeName(tempURL.toLocalFile()));
                        unlink(QFile::encodeName(tempURL2.toLocalFile()));
                        result = false;
                    }
                    else
                    {
                        // Else remove only the first temp file.
                        unlink(QFile::encodeName(tempURL.toLocalFile()));
                    }
                }
            }

            // Now we need to move from temp file to destination file.
            // This possibly involves UI operation, do it from main thread
            emit signalInternalCheckRename(folder, file, dest, temp);
            break;
        }
        case(CameraCommand::gp_open):
        {
            QString folder = cmd->map["folder"].toString();
            QString file   = cmd->map["file"].toString();
            QString dest   = cmd->map["dest"].toString();

            sendLogMsg(i18n("Retrieving file %1 from camera...", file), DHistoryView::StartingEntry, folder, file);

            bool result = d->camera->downloadItem(folder, file, dest);

            if (result)
            {
                emit signalInternalOpen(folder, file, dest);
            }
            else
            {
                sendLogMsg(i18n("Failed to retrieve file %1 from camera.", file), DHistoryView::ErrorEntry, folder, file);
            }

            break;
        }
        case(CameraCommand::gp_upload):
        {
            QString folder = cmd->map["destFolder"].toString();

            // We will using the same source file name to create the dest file
            // name in camera.
            QString file   = cmd->map["destFile"].toString();

            // The source file path to download in camera.
            QString src    = cmd->map["srcFilePath"].toString();

            sendLogMsg(i18n("Uploading file %1 to camera...", file), DHistoryView::StartingEntry, folder, file);

            GPItemInfo itemsInfo;

            bool result = d->camera->uploadItem(folder, file, src, itemsInfo);

            if (result)
            {
                emit signalUploaded(itemsInfo);
            }
            else
            {
                emit signalInternalUploadFailed(folder, file, src);
            }

            break;
        }
        case(CameraCommand::gp_delete):
        {
            QString folder = cmd->map["folder"].toString();
            QString file   = cmd->map["file"].toString();

            sendLogMsg(i18n("Deleting file %1...", file), DHistoryView::StartingEntry, folder, file);

            bool result = d->camera->deleteItem(folder, file);

            if (result)
            {
                emit signalDeleted(folder, file, true);
            }
            else
            {
                emit signalInternalDeleteFailed(folder, file);
            }

            break;
        }
        case(CameraCommand::gp_lock):
        {
            QString folder = cmd->map["folder"].toString();
            QString file   = cmd->map["file"].toString();
            bool    lock   = cmd->map["lock"].toBool();

            sendLogMsg(i18n("Toggle lock file %1...", file), DHistoryView::StartingEntry, folder, file);

            bool result = d->camera->setLockItem(folder, file, lock);

            if (result)
            {
                emit signalLocked(folder, file, true);
            }
            else
            {
                emit signalInternalLockFailed(folder, file);
            }

            break;
        }
        default:
        {
            kWarning() << " unknown action specified";
        }
    }
}

void CameraController::sendBusy(bool val)
{
    emit signalBusy(val);
}

void CameraController::sendLogMsg(const QString& msg, DHistoryView::EntryType type,
                                  const QString& folder, const QString& file)
{
    if (!d->canceled)
    {
        emit signalLogMsg(msg, type, folder, file);
    }
}

void CameraController::slotCheckRename(const QString& folder, const QString& file,
                                       const QString& destination, const QString& temp)
{
    // this is the direct continuation of executeCommand, case CameraCommand::gp_download

    bool skip      = false;
    bool cancel    = false;
    bool overwrite = d->overwriteAll;
    QString dest   = destination;

    // Check if dest file already exist, unless we overwrite anyway

    QFileInfo info(dest);

    if (!d->overwriteAll)
    {

        while (info.exists())
        {
            if (d->skipAll)
            {
                skip = true;
                break;
            }

            QPointer<KIO::RenameDialog> dlg = new KIO::RenameDialog(d->parent, i18n("Rename File"),
                    folder + QString("/") + file, dest,
                    KIO::RenameDialog_Mode(KIO::M_MULTI |
                                           KIO::M_OVERWRITE                    |
                                           KIO::M_SKIP));

            int result = dlg->exec();
            dest       = dlg->newDestUrl().toLocalFile();
            info       = QFileInfo(dest);

            delete dlg;

            switch (result)
            {
                case KIO::R_CANCEL:
                {
                    cancel = true;
                    break;
                }
                case KIO::R_SKIP:
                {
                    skip = true;
                    break;
                }
                case KIO::R_AUTO_SKIP:
                {
                    d->skipAll = true;
                    skip       = true;
                    break;
                }
                case KIO::R_OVERWRITE:
                {
                    overwrite = true;
                    break;
                }
                case KIO::R_OVERWRITE_ALL:
                {
                    d->overwriteAll = true;
                    overwrite       = true;
                    break;
                }
                default:
                    break;
            }

            if (cancel || skip || overwrite)
            {
                break;
            }
        }
    }

    if (cancel)
    {
        unlink(QFile::encodeName(temp));
        slotCancel();
        emit signalSkipped(folder, file);
        return;
    }
    else if (skip)
    {
        unlink(QFile::encodeName(temp));
        sendLogMsg(i18n("Skipped file %1", file), DHistoryView::WarningEntry, folder, file);
        emit signalSkipped(folder, file);
        return;
    }

    // move the file to the destination file
    if (rename(QFile::encodeName(temp), QFile::encodeName(dest)) != 0)
    {
        // rename failed. delete the temp file
        unlink(QFile::encodeName(temp));
        emit signalDownloaded(folder, file, GPItemInfo::DownloadFailed);
        sendLogMsg(i18n("Failed to download %1...", file), DHistoryView::ErrorEntry,  folder, file);
    }
    else
    {
        emit signalDownloaded(folder, file, GPItemInfo::DownloadedYes);
        emit signalDownloadComplete(folder, file, info.path(), info.fileName());
        sendLogMsg(i18n("Download successfully %1...", file), DHistoryView::StartingEntry, folder, file);
    }
}

void CameraController::slotDownloadFailed(const QString& folder, const QString& file)
{
    QString msg = i18n("Failed to download file \"%1\".", file);
    sendLogMsg(i18n("Failed to download %1...", file), DHistoryView::ErrorEntry, folder, file);

    if (!d->canceled)
    {
        if (queueIsEmpty())
        {
            KMessageBox::error(d->parent, msg);
        }
        else
        {
            msg += i18n(" Do you want to continue?");
            int result = KMessageBox::warningContinueCancel(d->parent, msg);

            if (result != KMessageBox::Continue)
            {
                slotCancel();
            }
        }
    }
}

void CameraController::slotUploadFailed(const QString& folder, const QString& file, const QString& src)
{
    Q_UNUSED(folder);
    Q_UNUSED(src);

    QString msg = i18n("Failed to upload file \"%1\".", file);
    sendLogMsg(i18n("Failed to upload %1...", file), DHistoryView::ErrorEntry);

    if (!d->canceled)
    {
        if (queueIsEmpty())
        {
            KMessageBox::error(d->parent, msg);
        }
        else
        {
            msg += i18n(" Do you want to continue?");
            int result = KMessageBox::warningContinueCancel(d->parent, msg);

            if (result != KMessageBox::Continue)
            {
                slotCancel();
            }
        }
    }
}

void CameraController::slotDeleteFailed(const QString& folder, const QString& file)
{
    emit signalDeleted(folder, file, false);
    sendLogMsg(i18n("Failed to delete %1...", file), DHistoryView::ErrorEntry, folder, file);

    QString msg = i18n("Failed to delete file \"%1\".",file);

    if (!d->canceled)
    {
        if (queueIsEmpty())
        {
            KMessageBox::error(d->parent, msg);
        }
        else
        {
            msg += i18n(" Do you want to continue?");
            int result = KMessageBox::warningContinueCancel(d->parent, msg);

            if (result != KMessageBox::Continue)
            {
                slotCancel();
            }
        }
    }
}

void CameraController::slotLockFailed(const QString& folder, const QString& file)
{
    emit signalLocked(folder, file, false);
    sendLogMsg(i18n("Failed to lock %1...", file), DHistoryView::ErrorEntry, folder, file);

    QString msg = i18n("Failed to toggle lock file \"%1\".",file);

    if (!d->canceled)
    {
        if (queueIsEmpty())
        {
            KMessageBox::error(d->parent, msg);
        }
        else
        {
            msg += i18n(" Do you want to continue?");
            int result = KMessageBox::warningContinueCancel(d->parent, msg);

            if (result != KMessageBox::Continue)
            {
                slotCancel();
            }
        }
    }
}

void CameraController::slotOpen(const QString& folder, const QString& file, const QString& dest)
{
    Q_UNUSED(folder);
    Q_UNUSED(file);
    Q_UNUSED(dest);

    //TODO: Provide a preview window

    /*
    KUrl url = KUrl::fromPath(dest);
    KUrl::List urlList;
    urlList << url;

    ImageWindow* im = ImageWindow::imageWindow();
    im->loadURL(urlList, url, i18n("Camera \"%1\"",d->camera->model()), false);

    if (im->isHidden())
    {
        im->show();
    }
    else
    {
        im->raise();
    }

    im->setFocus();
    */
}

void CameraController::addCommand(CameraCommand* cmd)
{
    QMutexLocker lock(&d->mutex);
    d->commands << cmd;
    d->condVar.wakeAll();
}

bool CameraController::queueIsEmpty()
{
    QMutexLocker lock(&d->mutex);
    return d->commands.isEmpty();
}

void CameraController::slotConnect()
{
    d->canceled        = false;
    CameraCommand* cmd = new CameraCommand;
    cmd->action        = CameraCommand::gp_connect;
    addCommand(cmd);
}

void CameraController::listFolders()
{
    d->canceled        = false;
    CameraCommand* cmd = new CameraCommand;
    cmd->action        = CameraCommand::gp_listfolders;
    addCommand(cmd);
}

void CameraController::listFiles(const QString& folder)
{
    d->canceled        = false;
    CameraCommand* cmd = new CameraCommand;
    cmd->action        = CameraCommand::gp_listfiles;
    cmd->map.insert("folder", QVariant(folder));
    addCommand(cmd);
}

void CameraController::getThumbnail(const QString& folder, const QString& file)
{
    QList<QVariant> list;
    list.append(QStringList() << folder << file);
    getThumbnails(list);
}

void CameraController::getThumbnails(const QList<QVariant>& list)
{
    d->canceled        = false;
    CameraCommand* cmd = new CameraCommand;
    cmd->action        = CameraCommand::gp_thumbnails;
    cmd->map.insert("list", QVariant(list));
    addCommand(cmd);
}

void CameraController::getExif(const QString& folder, const QString& file)
{
    d->canceled        = false;
    CameraCommand* cmd = new CameraCommand;
    cmd->action        = CameraCommand::gp_exif;
    cmd->map.insert("folder", QVariant(folder));
    cmd->map.insert("file",   QVariant(file));
    addCommand(cmd);
}

void CameraController::getCameraInformation()
{
    d->canceled        = false;
    CameraCommand* cmd = new CameraCommand;
    cmd->action        = CameraCommand::gp_cameraInformation;
    addCommand(cmd);
}

void CameraController::getFreeSpace()
{
    d->canceled        = false;
    CameraCommand* cmd = new CameraCommand;
    cmd->action        = CameraCommand::gp_freeSpace;
    addCommand(cmd);
}

void CameraController::getPreview()
{
    d->canceled        = false;
    CameraCommand* cmd = new CameraCommand;
    cmd->action        = CameraCommand::gp_preview;
    addCommand(cmd);
}

void CameraController::capture()
{
    d->canceled        = false;
    CameraCommand* cmd = new CameraCommand;
    cmd->action        = CameraCommand::gp_capture;
    addCommand(cmd);
}

void CameraController::upload(const QFileInfo& srcFileInfo, const QString& destFile, const QString& destFolder)
{
    d->canceled        = false;
    CameraCommand* cmd = new CameraCommand;
    cmd->action        = CameraCommand::gp_upload;
    cmd->map.insert("srcFilePath", QVariant(srcFileInfo.filePath()));
    cmd->map.insert("destFile",    QVariant(destFile));
    cmd->map.insert("destFolder",  QVariant(destFolder));
    addCommand(cmd);
    kDebug() << "Uploading '" << srcFileInfo.filePath() << "' into camera : '" << destFolder
             << "' (" << destFile << ")";
}

void CameraController::downloadPrep()
{
    d->overwriteAll  = false;
    d->skipAll       = false;
    d->downloadTotal = 0;
}

void CameraController::download(const DownloadSettingsContainer& downloadSettings)
{
    d->canceled        = false;
    CameraCommand* cmd = new CameraCommand;
    cmd->action        = CameraCommand::gp_download;
    cmd->map.insert("folder",            QVariant(downloadSettings.folder));
    cmd->map.insert("file",              QVariant(downloadSettings.file));
    cmd->map.insert("dest",              QVariant(downloadSettings.dest));
    cmd->map.insert("autoRotate",        QVariant(downloadSettings.autoRotate));
    cmd->map.insert("fixDateTime",       QVariant(downloadSettings.fixDateTime));
    cmd->map.insert("newDateTime",       QVariant(downloadSettings.newDateTime));
    cmd->map.insert("template",          QVariant(downloadSettings.templateTitle));
    cmd->map.insert("convertJpeg",       QVariant(downloadSettings.convertJpeg));
    cmd->map.insert("losslessFormat",    QVariant(downloadSettings.losslessFormat));
    addCommand(cmd);
}

void CameraController::deleteFile(const QString& folder, const QString& file)
{
    d->canceled        = false;
    CameraCommand* cmd = new CameraCommand;
    cmd->action        = CameraCommand::gp_delete;
    cmd->map.insert("folder", QVariant(folder));
    cmd->map.insert("file",   QVariant(file));
    addCommand(cmd);
}

void CameraController::lockFile(const QString& folder, const QString& file, bool locked)
{
    d->canceled        = false;
    CameraCommand* cmd = new CameraCommand;
    cmd->action        = CameraCommand::gp_lock;
    cmd->map.insert("folder", QVariant(folder));
    cmd->map.insert("file",   QVariant(file));
    cmd->map.insert("lock",   QVariant(locked));
    addCommand(cmd);
}

void CameraController::openFile(const QString& folder, const QString& file)
{
    d->canceled        = false;
    CameraCommand* cmd = new CameraCommand;
    cmd->action        = CameraCommand::gp_open;
    cmd->map.insert("folder", QVariant(folder));
    cmd->map.insert("file",   QVariant(file));
    cmd->map.insert("dest",   QVariant(KStandardDirs::locateLocal("tmp", file)));
    addCommand(cmd);
}

}  // namespace Digikam

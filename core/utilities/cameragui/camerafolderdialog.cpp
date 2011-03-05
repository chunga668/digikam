/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-07-24
 * Description : a dialog to select a camera folders.
 *
 * Copyright (C) 2006-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "camerafolderdialog.moc"

// Qt includes

#include <QLabel>
#include <QFrame>
#include <QGridLayout>

// KDE includes

#include <kapplication.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kdebug.h>

// Local includes

#include "cameraiconview.h"
#include "camerafolderitem.h"
#include "camerafolderview.h"

namespace Digikam
{

class CameraFolderDialog::CameraFolderDialogPriv
{
public:

    CameraFolderDialogPriv() :
        folderView(0)
    {
    }

    QString           rootPath;

    CameraFolderView* folderView;
};

CameraFolderDialog::CameraFolderDialog(QWidget* parent, CameraIconView* cameraView,
                                       const QStringList& cameraFolderList,
                                       const QString& cameraName, const QString& rootPath)
    : KDialog(parent), d(new CameraFolderDialogPriv)
{
    setHelp("camerainterface.anchor", "digikam");
    setCaption(i18n("%1 - Select Camera Folder",cameraName));
    setButtons(Help|Ok|Cancel);
    setDefaultButton(Ok);
    enableButtonOk(false);
    setModal(true);

    d->rootPath = rootPath;

    QFrame* page = new QFrame(this);
    setMainWidget(page);

    QGridLayout* grid = new QGridLayout(page);
    d->folderView      = new CameraFolderView(page);
    QLabel* logo      = new QLabel(page);
    QLabel* message   = new QLabel(page);

    logo->setPixmap(QPixmap(KStandardDirs::locate("data", "digikam/data/logo-digikam.png"))
                    .scaled(128, 128, Qt::KeepAspectRatio, Qt::SmoothTransformation));

    message->setText(i18n("<p>Please select the camera folder "
                          "where you want to upload the images.</p>"));
    message->setWordWrap(true);

    grid->addWidget(logo,           0, 0, 1, 1);
    grid->addWidget(message,        1, 0, 1, 1);
    grid->addWidget(d->folderView,   0, 1, 3, 1);
    grid->setRowStretch(2, 10);
    grid->setMargin(KDialog::spacingHint());
    grid->setSpacing(KDialog::spacingHint());

    d->folderView->addVirtualFolder(cameraName);
    d->folderView->addRootFolder("/", cameraView->countItemsByFolder(rootPath));

    for (QStringList::const_iterator it = cameraFolderList.constBegin();
         it != cameraFolderList.constEnd(); ++it)
    {
        QString folder(*it);

        if (folder.startsWith(rootPath) && rootPath != QString("/"))
        {
            folder.remove(0, rootPath.length());
        }

        if (folder != QString("/") && !folder.isEmpty())
        {
            QString root = folder.section( '/', 0, -2 );

            if (root.isEmpty())
            {
                root = QString("/");
            }

            QString sub = folder.section( '/', -1 );
            d->folderView->addFolder(root, sub, cameraView->countItemsByFolder(*it));
            kDebug() << "Camera folder: '" << folder << "' (root='" << root << "', sub='" <<sub <<"')";
        }
    }

    connect(d->folderView, SIGNAL(signalFolderChanged(CameraFolderItem*)),
            this, SLOT(slotFolderPathSelectionChanged(CameraFolderItem*)));

    resize(500, 500);

    // make sure the ok button is properly set up
    enableButtonOk( d->folderView->currentItem() != 0 );
}

CameraFolderDialog::~CameraFolderDialog()
{
    delete d;
}

QString CameraFolderDialog::selectedFolderPath() const
{
    QTreeWidgetItem* item = d->folderView->currentItem();

    if (!item)
    {
        return QString();
    }

    CameraFolderItem* folderItem = dynamic_cast<CameraFolderItem*>(item);

    if (folderItem->isVirtualFolder())
    {
        return QString(d->rootPath);
    }

    // Case of Gphoto2 cameras. No need to duplicate root '/'.
    if (d->rootPath == QString("/"))
    {
        return(folderItem->folderPath());
    }

    return(d->rootPath + folderItem->folderPath());
}

void CameraFolderDialog::slotFolderPathSelectionChanged(CameraFolderItem* item)
{
    if (item)
    {
        enableButtonOk(true);
        kDebug() << "Camera folder path: " << selectedFolderPath();
    }
    else
    {
        enableButtonOk(false);
    }
}

}  // namespace Digikam

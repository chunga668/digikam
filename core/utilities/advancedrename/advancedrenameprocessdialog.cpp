/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-11-14
 * Description : process dialog for renaming files
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

#include "advancedrenameprocessdialog.moc"

// Qt includes

#include <QPixmap>
#include <QTimer>

// KDE includes

#include <klocale.h>

// Local includes

#include "thumbnailloadthread.h"
#include "imageviewutilities.h"

namespace Digikam
{

class AdvancedRenameProcessDialogPriv
{
public:

    AdvancedRenameProcessDialogPriv() :
        cancel(false),
        thumbLoadThread(0),
        utilities(0)
    {
    }

    bool                 cancel;
    ThumbnailLoadThread* thumbLoadThread;
    NewNamesList         newNameList;
    ImageViewUtilities*  utilities;
};

AdvancedRenameProcessDialog::AdvancedRenameProcessDialog(const NewNamesList& list)
    : DProgressDlg(0), d(new AdvancedRenameProcessDialogPriv)
{
    d->newNameList     = list;
    d->utilities       = new ImageViewUtilities(this);
    d->thumbLoadThread = ThumbnailLoadThread::defaultThread();

    connect(d->thumbLoadThread, SIGNAL(signalThumbnailLoaded(const LoadingDescription&, const QPixmap&)),
            this, SLOT(slotGotThumbnail(const LoadingDescription&, const QPixmap&)));

    connect(d->utilities, SIGNAL(imageRenameSucceeded(const KUrl&)),
            this, SLOT(slotRenameSuccess(const KUrl&)));

    connect(d->utilities, SIGNAL(imageRenameFailed(const KUrl&)),
            this, SLOT(slotRenameFailed(const KUrl&)));

    connect(d->utilities, SIGNAL(renamingAborted()),
            this, SLOT(slotCancel()));

    setModal(true);
    setValue(0);
    setCaption(i18n("Renaming images"));
    setLabel(i18n("<b>Renaming images. Please wait...</b>"));
    setButtonText(i18n("&Abort"));

    QTimer::singleShot(500, this, SLOT(slotRenameImages()));
}

AdvancedRenameProcessDialog::~AdvancedRenameProcessDialog()
{
    delete d;
}

void AdvancedRenameProcessDialog::slotRenameImages()
{
    setTitle(i18n("Processing..."));

    setMaximum(d->newNameList.count());

    if (d->newNameList.isEmpty())
    {
        slotCancel();
        return;
    }

    processOne();
}

void AdvancedRenameProcessDialog::processOne()
{
    if (d->cancel || d->newNameList.isEmpty())
    {
        return;
    }

    d->thumbLoadThread->find(d->newNameList.first().first.toLocalFile());
}

void AdvancedRenameProcessDialog::complete()
{
    done(Cancel);
}

void AdvancedRenameProcessDialog::slotGotThumbnail(const LoadingDescription& desc, const QPixmap& pix)
{
    if (d->cancel || d->newNameList.isEmpty())
    {
        return;
    }

    if (d->newNameList.first().first.toLocalFile() != desc.filePath)
    {
        return;
    }

    addedAction(pix, desc.filePath);
    advance(1);

    NewNameInfo info = d->newNameList.first();
    d->utilities->rename(info.first, info.second);
}

void AdvancedRenameProcessDialog::slotCancel()
{
    abort();
    done(Cancel);
}

void AdvancedRenameProcessDialog::slotRenameSuccess(const KUrl& src)
{
    if (d->cancel || d->newNameList.isEmpty())
    {
        return;
    }

    if (d->newNameList.first().first != src)
    {
        return;
    }

    if (!d->newNameList.isEmpty())
    {
        d->newNameList.removeFirst();
    }

    if (d->newNameList.isEmpty())
    {
        complete();
    }
    else
    {
        processOne();
    }
}

void AdvancedRenameProcessDialog::slotRenameFailed(const KUrl&)
{
    abort();
}

void AdvancedRenameProcessDialog::closeEvent(QCloseEvent* e)
{
    abort();
    e->accept();
}

void AdvancedRenameProcessDialog::abort()
{
    d->cancel = true;
    emit signalRebuildAllThumbsDone();
}

}  // namespace Digikam

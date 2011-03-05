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

#ifndef ADVANCEDRENAMEPROCESSDIALOG_H
#define ADVANCEDRENAMEPROCESSDIALOG_H

// Qt includes

#include <QCloseEvent>
#include <QPixmap>

// Local includes

#include "dprogressdlg.h"
#include "advancedrenamedialog.h"

class QPixmap;
class QWidget;

class KUrl;

namespace Digikam
{

class LoadingDescription;

class AdvancedRenameProcessDialogPriv;
class AdvancedRenameProcessDialog : public DProgressDlg
{
    Q_OBJECT

public:

    explicit AdvancedRenameProcessDialog(const NewNamesList& list);
    ~AdvancedRenameProcessDialog();

Q_SIGNALS:

    void signalRebuildAllThumbsDone();

private:

    void abort();
    void complete();
    void processOne();

protected:

    void closeEvent(QCloseEvent* e);

protected Q_SLOTS:

    void slotCancel();
    void slotRenameSuccess(const KUrl&);
    void slotRenameFailed(const KUrl&);

private Q_SLOTS:

    void slotRenameImages();
    void slotGotThumbnail(const LoadingDescription&, const QPixmap&);

private:

    AdvancedRenameProcessDialogPriv* const d;
};

}  // namespace Digikam

#endif /* ADVANCEDRENAMEPROCESSDIALOG_H */

/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-05-16
 * Description : finger-prints generator
 *
 * Copyright (C) 2008-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef FINGERPRINTSGENERATOR_H
#define FINGERPRINTSGENERATOR_H

// Qt includes

#include <QCloseEvent>

// Local includes

#include "dprogressdlg.h"

class QWidget;

class KUrl;

namespace Digikam
{

class DImg;
class LoadingDescription;

class FingerPrintsGenerator : public DProgressDlg
{
    Q_OBJECT

public:

    FingerPrintsGenerator(QWidget* parent, bool rebuildAll);
    ~FingerPrintsGenerator();

Q_SIGNALS:

    void signalRebuildAllFingerPrintsDone();

private:

    void abort();
    void complete();
    void processOne();

protected:

    void closeEvent(QCloseEvent* e);

protected Q_SLOTS:

    void slotCancel();

private Q_SLOTS:

    void slotRebuildFingerPrints();
    void slotGotImagePreview(const LoadingDescription&, const DImg&);

private:

    class FingerPrintsGeneratorPriv;
    FingerPrintsGeneratorPriv* const d;
};

}  // namespace Digikam

#endif /* FINGERPRINTSGENERATOR_H */

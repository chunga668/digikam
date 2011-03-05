/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-01-20
 * Description : image editor GUI saving context container
 *
 * Copyright (C) 2006-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2010 by Marcel Wiesweg <marcel.wiesweg@gmx.de>
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

#ifndef SAVINGCONTEXTCONTAINER_H
#define SAVINGCONTEXTCONTAINER_H

// Qt includes

#include <QString>

// KDE includes

#include <kurl.h>
#include <ktemporaryfile.h>

// Local includes

#include "digikam_export.h"
#include "versionfileoperation.h"

namespace Digikam
{

class DIGIKAM_EXPORT SavingContextContainer
{

public:

    SavingContextContainer()
    {
        savingState             = SavingStateNone;
        synchronizingState      = NormalSaving;
        saveTempFile            = 0;
        destinationExisted      = false;
        synchronousSavingResult = false;
        abortingSaving          = false;
        executedOperation       = SavingStateNone;
    }

    enum SavingState
    {
        SavingStateNone,
        SavingStateSave,
        SavingStateSaveAs,
        SavingStateVersion
    };

    enum SynchronizingState
    {
        NormalSaving,
        SynchronousSaving
    };

    SavingState         savingState;
    SynchronizingState  synchronizingState;
    bool                synchronousSavingResult;
    bool                destinationExisted;
    bool                abortingSaving;
    SavingState         executedOperation;

    QString             originalFormat;
    QString             format;

    KUrl                srcURL;
    KUrl                destinationURL;
    KUrl                moveSrcURL;

    KTemporaryFile*     saveTempFile;
    QString             saveTempFileName;

    VersionFileOperation versionFileOperation;
};

} // namespace Digikam

#endif /* SAVINGCONTEXTCONTAINER_H */

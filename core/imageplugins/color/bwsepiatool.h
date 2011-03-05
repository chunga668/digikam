/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-12-06
 * Description : Black and White conversion tool.
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
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

#ifndef BWSEPIATOOL_H
#define BWSEPIATOOL_H

// Local includes

#include "editortool.h"

using namespace Digikam;

namespace DigikamColorImagePlugin
{

class BWSepiaTool : public EditorToolThreaded
{
    Q_OBJECT

public:

    BWSepiaTool(QObject* parent);
    ~BWSepiaTool();

private Q_SLOTS:

    void slotInit();
    void slotSaveAsSettings();
    void slotLoadSettings();
    void slotResetSettings();

private:

    void readSettings();
    void writeSettings();
    void prepareEffect();
    void prepareFinal();
    void putPreviewData();
    void putFinalData();
    void slotScaleChanged();

private:

    class BWSepiaToolPriv;
    BWSepiaToolPriv* const d;
};

}  // namespace DigikamColorImagePlugin

#endif /* BWSEPIATOOL_H */

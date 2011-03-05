/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-08-24
 * Description : a plugin to reduce CCD noise.
 *
 * Copyright (C) 2004-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef NOISEREDUCTIONTOOL_H
#define NOISEREDUCTIONTOOL_H

// Local includes

#include "editortool.h"

using namespace Digikam;

namespace DigikamEnhanceImagePlugin
{

class NoiseReductionTool : public EditorToolThreaded
{
    Q_OBJECT

public:

    NoiseReductionTool(QObject* parent);
    ~NoiseReductionTool();

private:

    void readSettings();
    void writeSettings();
    void prepareEffect();
    void prepareFinal();
    void putPreviewData();
    void putFinalData();

private Q_SLOTS:

    void slotResetSettings();
    void slotLoadSettings();
    void slotSaveAsSettings();

private:

    class NoiseReductionToolPriv;
    NoiseReductionToolPriv* const d;
};

}  // namespace DigikamEnhanceImagePlugin

#endif /* NOISEREDUCTIONTOOL_H */

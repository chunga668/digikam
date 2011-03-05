/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-03-30
 * Description : a digiKam image editor plugin to inpaint
 *               a photograph
 *
 * Copyright (C) 2005-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef INPAINTINGTOOL_H
#define INPAINTINGTOOL_H

// Qt includes

#include <QString>

// KDE includes

#include <kpassivepopup.h>

// Local includes

#include "editortool.h"

using namespace Digikam;

namespace DigikamEnhanceImagePlugin
{

class InPaintingPassivePopup : public KPassivePopup
{
public:

    InPaintingPassivePopup(QWidget* parent) : KPassivePopup(parent), m_parent(parent) {}

protected:

    virtual void positionSelf()
    {
        move(m_parent->x() + 30, m_parent->y() + 30);
    }

private:

    QWidget* m_parent;
};

//-----------------------------------------------------------

class InPaintingTool : public EditorToolThreaded
{
    Q_OBJECT

public:

    InPaintingTool(QObject* parent);
    ~InPaintingTool();

private Q_SLOTS:

    void processCImgUrl(const QString&);
    void slotResetValues(int);
    void slotResetSettings();
    void slotSaveAsSettings();
    void slotLoadSettings();

private:

    void readSettings();
    void writeSettings();
    void prepareEffect();
    void prepareFinal();
    void putPreviewData();
    void putFinalData();

private:

    enum InPaintingFilteringPreset
    {
        NoPreset = 0,
        RemoveSmallArtefact,
        RemoveMediumArtefact,
        RemoveLargeArtefact
    };

private:

    class InPaintingToolPriv;
    InPaintingToolPriv* const d;
};

}  // namespace DigikamEnhanceImagePlugin

#endif /* INPAINTINGTOOL_H */

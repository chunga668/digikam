/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-06-06
 * Description : Red eyes correction tool for image editor
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
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

#ifndef REDEYETOOL_H
#define REDEYETOOL_H

// KDE includes

#include <kpassivepopup.h>

// Local includes

#include "editortool.h"

class QColor;

namespace Digikam
{
class DImg;
class DColor;
}

using namespace Digikam;

namespace DigikamEnhanceImagePlugin
{

class RedEyePassivePopup : public KPassivePopup
{
public:

    RedEyePassivePopup(QWidget* parent)
        : KPassivePopup(parent), m_parent(parent)
    {
    }

protected:

    virtual void positionSelf()
    {
        move(m_parent->x() + 30, m_parent->y() + 30);
    }

private:

    QWidget* m_parent;
};

class RedEyeTool : public EditorTool
{
    Q_OBJECT

public:

    RedEyeTool(QObject* parent);
    ~RedEyeTool();

private Q_SLOTS:

    void slotEffect();
    void slotResetSettings();
    void slotColorSelectedFromTarget(const DColor& color);
    void slotHSChanged(int h, int s);
    void slotVChanged(int v);

private:

    void readSettings();
    void writeSettings();
    void finalRendering();
    void redEyeFilter(DImg& selection);
    void setColor(const QColor& color);

private:

    enum RedThresold
    {
        Mild = 0,
        Aggressive
    };

    class RedEyeToolPriv;
    RedEyeToolPriv* const d;
};

}  // namespace DigikamEnhanceImagePlugin

#endif /* REDEYETOOL_H */

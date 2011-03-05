/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-02-11
 * Description : Color Balance batch tool.
 *
 * Copyright (C) 2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "colorbalance.moc"

// Qt includes

#include <QWidget>

// KDE includes

#include <kiconloader.h>
#include <klocale.h>
#include <kstandarddirs.h>

// Local includes

#include "dimg.h"
#include "cbfilter.h"
#include "cbsettings.h"

namespace Digikam
{

ColorBalance::ColorBalance(QObject* parent)
    : BatchTool("ColorBalance", ColorTool, parent)
{
    setToolTitle(i18n("Color Balance"));
    setToolDescription(i18n("A tool to adjust color balance."));
    setToolIcon(KIcon(SmallIcon("adjustrgb")));

    QWidget* box   = new QWidget;
    m_settingsView = new CBSettings(box);
    setSettingsWidget(box);

    connect(m_settingsView, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));
}

ColorBalance::~ColorBalance()
{
}

BatchToolSettings ColorBalance::defaultSettings()
{
    BatchToolSettings prm;
    CBContainer defaultPrm = m_settingsView->defaultSettings();

    prm.insert("Red",   (double)defaultPrm.red);
    prm.insert("Green", (double)defaultPrm.green);
    prm.insert("Blue",  (double)defaultPrm.blue);

    return prm;
}

void ColorBalance::slotAssignSettings2Widget()
{
    CBContainer prm;
    prm.red   = settings()["Red"].toDouble();
    prm.green = settings()["Green"].toDouble();
    prm.blue  = settings()["Blue"].toDouble();
    m_settingsView->setSettings(prm);
}

void ColorBalance::slotSettingsChanged()
{
    BatchToolSettings prm;
    CBContainer currentPrm = m_settingsView->settings();

    prm.insert("Red",   (double)currentPrm.red);
    prm.insert("Green", (double)currentPrm.green);
    prm.insert("Blue",  (double)currentPrm.blue);

    BatchTool::slotSettingsChanged(prm);
}

bool ColorBalance::toolOperations()
{
    if (!loadToDImg())
    {
        return false;
    }

    CBContainer prm;
    prm.red   = settings()["Red"].toDouble();
    prm.green = settings()["Green"].toDouble();
    prm.blue  = settings()["Blue"].toDouble();

    CBFilter cb(&image(), 0L, prm);
    cb.startFilterDirectly();
    image().putImageData(cb.getTargetImage().bits());

    return (savefromDImg());
}

}  // namespace Digikam

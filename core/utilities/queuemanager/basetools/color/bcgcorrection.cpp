/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-02-09
 * Description : Brightness/Contrast/Gamma batch tool.
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

#include "bcgcorrection.moc"

// Qt includes

#include <QWidget>

// KDE includes

#include <kiconloader.h>
#include <klocale.h>
#include <kstandarddirs.h>

// Local includes

#include "dimg.h"
#include "bcgfilter.h"
#include "bcgsettings.h"

namespace Digikam
{

BCGCorrection::BCGCorrection(QObject* parent)
    : BatchTool("BCGCorrection", ColorTool, parent)
{
    setToolTitle(i18n("BCG Correction"));
    setToolDescription(i18n("A tool to fix Brightness/Contrast/Gamma."));
    setToolIcon(KIcon(SmallIcon("contrast")));

    QWidget* box   = new QWidget;
    m_settingsView = new BCGSettings(box);
    setSettingsWidget(box);

    connect(m_settingsView, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));
}

BCGCorrection::~BCGCorrection()
{
}

BatchToolSettings BCGCorrection::defaultSettings()
{
    BatchToolSettings prm;
    BCGContainer defaultPrm = m_settingsView->defaultSettings();

    prm.insert("Brightness", (double)defaultPrm.brightness);
    prm.insert("Contrast",   (double)defaultPrm.contrast);
    prm.insert("Gamma",      (double)defaultPrm.gamma);

    return prm;
}

void BCGCorrection::slotAssignSettings2Widget()
{
    BCGContainer prm;
    prm.brightness = settings()["Brightness"].toDouble();
    prm.contrast   = settings()["Contrast"].toDouble();
    prm.gamma      = settings()["Gamma"].toDouble();
    m_settingsView->setSettings(prm);
}

void BCGCorrection::slotSettingsChanged()
{
    BatchToolSettings prm;
    BCGContainer currentPrm = m_settingsView->settings();

    prm.insert("Brightness", (double)currentPrm.brightness);
    prm.insert("Contrast",   (double)currentPrm.contrast);
    prm.insert("Gamma",      (double)currentPrm.gamma);

    BatchTool::slotSettingsChanged(prm);
}

bool BCGCorrection::toolOperations()
{
    if (!loadToDImg())
    {
        return false;
    }

    BCGContainer prm;
    prm.brightness = settings()["Brightness"].toDouble();
    prm.contrast   = settings()["Contrast"].toDouble();
    prm.gamma      = settings()["Gamma"].toDouble();

    BCGFilter bcg(&image(), 0L, prm);
    bcg.startFilterDirectly();
    image().putImageData(bcg.getTargetImage().bits());

    return (savefromDImg());
}

}  // namespace Digikam

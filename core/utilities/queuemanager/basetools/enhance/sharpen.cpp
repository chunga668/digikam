/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-05-02
 * Description : sharpen image batch tool.
 *
 * Copyright (C) 2009 by Matthias Welwarsky <matze at welwarsky dot de>
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

#include "sharpen.moc"

// Qt includes

#include <QLabel>
#include <QWidget>

// KDE includes

#include <kiconloader.h>
#include <klocale.h>
#include <kvbox.h>

// Local includes

#include "dimg.h"
#include "refocusfilter.h"
#include "sharpenfilter.h"
#include "unsharpmaskfilter.h"
#include "sharpsettings.h"

namespace Digikam
{

Sharpen::Sharpen(QObject* parent)
    : BatchTool("Sharpen", EnhanceTool, parent)
{
    setToolTitle(i18n("Sharpen Image"));
    setToolDescription(i18n("A tool to sharpen images"));
    setToolIcon(KIcon(SmallIcon("sharpenimage")));
}

Sharpen::~Sharpen()
{
}

QWidget* Sharpen::createSettingsWidget()
{
    QWidget* box   = new QWidget;
    m_settingsView = new SharpSettings(box);

    connect(m_settingsView, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));

    return box;
}

BatchToolSettings Sharpen::defaultSettings()
{
    BatchToolSettings settings;
    SharpContainer defaultPrm = m_settingsView->defaultSettings();

    // sharpen method
    settings.insert("SharpenFilterType",    (int)defaultPrm.method);

    // simple sharp
    settings.insert("SimpleSharpRadius",    (int)defaultPrm.ssRadius);

    // unsharp mask
    settings.insert("UnsharpMaskRadius",    (double)defaultPrm.umRadius);
    settings.insert("UnsharpMaskAmount",    (double)defaultPrm.umAmount);
    settings.insert("UnsharpMaskThreshold", (double)defaultPrm.umThreshold);

    // refocus
    settings.insert("RefocusRadius",        (double)defaultPrm.rfRadius);
    settings.insert("RefocusCorrelation",   (double)defaultPrm.rfCorrelation);
    settings.insert("RefocusNoise",         (double)defaultPrm.rfNoise);
    settings.insert("RefocusGauss",         (double)defaultPrm.rfGauss);
    settings.insert("RefocusMatrixSize",    (int)defaultPrm.rfMatrix);

    return settings;
}

void Sharpen::slotAssignSettings2Widget()
{
    SharpContainer prm;

    // sharpen method
    prm.method        = settings()["SharpenFilterType"].toInt();

    // simple sharp
    prm.ssRadius      = settings()["SimpleSharpRadius"].toInt();

    // unsharp mask
    prm.umRadius      = settings()["UnsharpMaskRadius"].toDouble();
    prm.umAmount      = settings()["UnsharpMaskAmount"].toDouble();
    prm.umThreshold   = settings()["UnsharpMaskThreshold"].toDouble();

    // refocus
    prm.rfRadius      = settings()["RefocusRadius"].toDouble();
    prm.rfCorrelation = settings()["RefocusCorrelation"].toDouble();
    prm.rfNoise       = settings()["RefocusNoise"].toDouble();
    prm.rfGauss       = settings()["RefocusGauss"].toDouble();
    prm.rfMatrix      = settings()["RefocusMatrixSize"].toInt();

    m_settingsView->setSettings(prm);
}

void Sharpen::slotSettingsChanged()
{
    BatchToolSettings settings;
    SharpContainer prm = m_settingsView->settings();

    // sharpen method
    settings.insert("SharpenFilterType",    (int)prm.method);

    // simple sharp
    settings.insert("SimpleSharpRadius",    (int)prm.ssRadius);

    // unsharp mask
    settings.insert("UnsharpMaskRadius",    (double)prm.umRadius);
    settings.insert("UnsharpMaskAmount",    (double)prm.umAmount);
    settings.insert("UnsharpMaskThreshold", (double)prm.umThreshold);

    // refocus
    settings.insert("RefocusRadius",        (double)prm.rfRadius);
    settings.insert("RefocusCorrelation",   (double)prm.rfCorrelation);
    settings.insert("RefocusNoise",         (double)prm.rfNoise);
    settings.insert("RefocusGauss",         (double)prm.rfGauss);
    settings.insert("RefocusMatrixSize",    (int)prm.rfMatrix);

    BatchTool::slotSettingsChanged(settings);
}

bool Sharpen::toolOperations()
{
    if (!loadToDImg())
    {
        return false;
    }

    int filterType  = settings()["SharpenFilterType"].toInt();

    switch (filterType)
    {
        case SharpContainer::SimpleSharp:
        {
            double radius = settings()["SimpleSharpRadius"].toInt() / 10.0;
            double sigma;

            if (radius < 1.0)
            {
                sigma = radius;
            }
            else
            {
                sigma = sqrt(radius);
            }

            SharpenFilter filter(&image(), 0L, radius, sigma);
            filter.startFilterDirectly();
            image().putImageData(filter.getTargetImage().bits());
            break;
        }

        case SharpContainer::UnsharpMask:
        {
            int r     = (int)settings()["UnsharpMaskRadius"].toDouble();
            double a  = settings()["UnsharpMaskAmount"].toDouble();
            double th = settings()["UnsharpMaskThreshold"].toDouble();

            UnsharpMaskFilter filter(&image(), 0L, r, a, th);
            filter.startFilterDirectly();
            image().putImageData(filter.getTargetImage().bits());
            break;
        }

        case SharpContainer::Refocus:
        {
            double radius      = settings()["RefocusRadius"].toDouble();
            double correlation = settings()["RefocusCorrelation"].toDouble();
            double noise       = settings()["RefocusNoise"].toDouble();
            double gauss       = settings()["RefocusGauss"].toDouble();
            int matrixSize     = settings()["RefocusMatrixSize"].toInt();

            RefocusFilter filter(&image(), 0L, matrixSize, radius, gauss, correlation, noise);
            filter.startFilterDirectly();
            image().putImageData(filter.getTargetImage().bits());
            break;
        }
    }

    return savefromDImg();
}

} // namespace Digikam

/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-06-18
 * Description : PGF image Converter batch tool.
 *
 * Copyright (C) 2009-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "convert2pgf.moc"

// Qt includes

#include <QFileInfo>
#include <QWidget>

// KDE includes

#include <kconfig.h>
#include <kconfiggroup.h>
#include <kiconloader.h>
#include <klocale.h>

// Local includes

#include "dimg.h"
#include "pgfsettings.h"

namespace Digikam
{

Convert2PGF::Convert2PGF(QObject* parent)
    : BatchTool("Convert2PGF", ConvertTool, parent)
{
    setToolTitle(i18n("Convert To PGF"));
    setToolDescription(i18n("A tool to convert images to PGF format."));
    setToolIcon(KIcon(SmallIcon("image-jp2")));

    m_settings = new PGFSettings();
    setSettingsWidget(m_settings);

    connect(m_settings, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotSettingsChanged()));
}

Convert2PGF::~Convert2PGF()
{
}

BatchToolSettings Convert2PGF::defaultSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("ImageViewer Settings");
    int compression           = group.readEntry("PGFCompression", 3);
    bool lossLessCompression  = group.readEntry("PGFLossLess", true);
    BatchToolSettings settings;
    settings.insert("quality",  compression);
    settings.insert("lossless", lossLessCompression);
    return settings;
}

void Convert2PGF::slotAssignSettings2Widget()
{
    m_settings->setCompressionValue(settings()["quality"].toInt());
    m_settings->setLossLessCompression(settings()["lossless"].toBool());
}

void Convert2PGF::slotSettingsChanged()
{
    BatchToolSettings settings;
    settings.insert("quality",  m_settings->getCompressionValue());
    settings.insert("lossless", m_settings->getLossLessCompression());
    BatchTool::slotSettingsChanged(settings);
}

QString Convert2PGF::outputSuffix() const
{
    return QString("pgf");
}

bool Convert2PGF::toolOperations()
{
    if (!loadToDImg())
    {
        return false;
    }

    bool lossless = settings()["lossless"].toBool();
    image().setAttribute("quality", lossless ? 0 : settings()["quality"].toInt());

    return (savefromDImg());
}

}  // namespace Digikam

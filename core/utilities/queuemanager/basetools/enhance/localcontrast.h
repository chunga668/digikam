/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-03-09
 * Description : LDR tonemapper batch tool.
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

#ifndef LOCALCONTRAST_H
#define LOCALCONTRAST_H

// Local includes

#include "batchtool.h"

namespace Digikam
{

class LocalContrastSettings;

class LocalContrast : public BatchTool
{
    Q_OBJECT

public:

    LocalContrast(QObject* parent=0);
    ~LocalContrast();

    BatchToolSettings defaultSettings();

private:

    bool toolOperations();
    QWidget* createSettingsWidget();

private Q_SLOTS:

    void slotAssignSettings2Widget();
    void slotSettingsChanged();

private:

    LocalContrastSettings* m_settingsView;
};

}  // namespace Digikam

#endif /* LOCALCONTRAST_H */

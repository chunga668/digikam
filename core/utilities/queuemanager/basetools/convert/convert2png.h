/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-11-28
 * Description : PNG image Converter batch tool.
 *
 * Copyright (C) 2008-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef CONVERT2PNG_H
#define CONVERT2PNG_H

// Local includes

#include "batchtool.h"

namespace Digikam
{

class PNGSettings;

class Convert2PNG : public BatchTool
{
    Q_OBJECT

public:

    Convert2PNG(QObject* parent=0);
    ~Convert2PNG();

    QString outputSuffix() const;
    BatchToolSettings defaultSettings();

private Q_SLOTS:

    void slotSettingsChanged();
    void slotAssignSettings2Widget();

private:

    bool toolOperations();

private:

    PNGSettings* m_settings;
};

}  // namespace Digikam

#endif /* CONVERT2PNG_H */

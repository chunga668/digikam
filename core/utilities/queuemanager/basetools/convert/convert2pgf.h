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

#ifndef CONVERT2PGF_H
#define CONVERT2PGF_H

// Local includes

#include "batchtool.h"

namespace Digikam
{

class PGFSettings;

class Convert2PGF : public BatchTool
{
    Q_OBJECT

public:

    Convert2PGF(QObject* parent=0);
    ~Convert2PGF();

    QString outputSuffix() const;
    BatchToolSettings defaultSettings();

private Q_SLOTS:

    void slotSettingsChanged();
    void slotAssignSettings2Widget();

private:

    bool toolOperations();

private:

    PGFSettings* m_settings;
};

}  // namespace Digikam

#endif /* CONVERT2PGF_H */

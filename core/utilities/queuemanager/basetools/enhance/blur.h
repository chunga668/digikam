/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-05-03
 * Description : blur image batch tool.
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

#ifndef BLUR_H_
#define BLUR_H_

#include "batchtool.h"

// LibKDcraw includes

#include <libkdcraw/rnuminput.h>

using namespace KDcrawIface;

namespace Digikam
{

class Blur : public BatchTool
{
    Q_OBJECT

public:

    Blur(QObject* parent=0);
    ~Blur();

    BatchToolSettings defaultSettings();

private:

    bool toolOperations();
    QWidget* createSettingsWidget();

private Q_SLOTS:

    void slotAssignSettings2Widget();
    void slotSettingsChanged();

private:

    RIntNumInput* m_radiusInput;
};

} // namespace Digikam

#endif /* BLUR_H_ */

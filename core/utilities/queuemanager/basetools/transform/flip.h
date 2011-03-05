/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-02-10
 * Description : flip image batch tool.
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

#ifndef FLIP_H
#define FLIP_H

// Local includes

#include "batchtool.h"

class KComboBox;

namespace Digikam
{

class Flip : public BatchTool
{
    Q_OBJECT

public:

    Flip(QObject* parent=0);
    ~Flip();

    BatchToolSettings defaultSettings();

private:

    bool toolOperations();

private Q_SLOTS:

    void slotAssignSettings2Widget();
    void slotSettingsChanged();

private:

    KComboBox* m_comboBox;
};

}  // namespace Digikam

#endif /* FLIP_H */

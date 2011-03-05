/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-06-06
 * Description : save PGF image options.
 *
 * Copyright (C) 2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef PGFSETTINGS_H
#define PGFSETTINGS_H

// KDE includes

#include <QWidget>

// Local includes

#include "digikam_export.h"

namespace Digikam
{

class PGFSettingsPriv;

class DIGIKAM_EXPORT PGFSettings : public QWidget
{
    Q_OBJECT

public:

    PGFSettings(QWidget* parent=0);
    ~PGFSettings();

    void setCompressionValue(int val);
    int  getCompressionValue();

    void setLossLessCompression(bool b);
    bool getLossLessCompression();

Q_SIGNALS:

    void signalSettingsChanged();

private Q_SLOTS:

    void slotTogglePGFLossLess(bool);

private:

    PGFSettingsPriv* const d;
};

}  // namespace Digikam

#endif /* PGFSETTINGS_H */

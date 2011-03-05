/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-05-22
 * Description : metadata information keys
 *
 * Copyright (C) 2009-2010 by Andi Clemens <andi dot clemens at gmx dot net>
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

#ifndef METADATAKEYS_H
#define METADATAKEYS_H

// local includes

#include "dbkeyscollection.h"
#include "parsesettings.h"

namespace Digikam
{

class MetadataKeys : public DbKeysCollection
{

public:

    MetadataKeys();
    virtual ~MetadataKeys() {};

protected:

    virtual QString getDbValue(const QString& key, ParseSettings& settings);
};

} // namespace Digikam

#endif /* METADATAKEYS_H */

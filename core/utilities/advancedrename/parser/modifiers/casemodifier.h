/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-09-14
 * Description : modifier to change the case of a renaming option
 *
 * Copyright (C) 2009 by Andi Clemens <andi dot clemens at gmx dot net>
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

#ifndef CASEMODIFIER_H
#define CASEMODIFIER_H

// Local includes

#include "modifier.h"

namespace Digikam
{

class CaseModifier : public Modifier
{
public:

    CaseModifier();
    virtual QString parseOperation(ParseSettings& settings);

private:

    QString firstupper(const QString& str2Modify);
    QString lower(const QString& str2Modify);
    QString upper(const QString& str2Modify);
};

} // namespace Digikam


#endif /* CASEMODIFIER_H */

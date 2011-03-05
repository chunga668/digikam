/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-08-08
 * Description : an abstract option class
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

#ifndef OPTION_H
#define OPTION_H

// Local includes

#include "parseable.h"
#include "parseresults.h"

class QRegExp;

namespace Digikam
{

class OptionPriv;

class Option : public Parseable
{
    Q_OBJECT

public:

    Option(const QString& name, const QString& description);
    Option(const QString& name, const QString& description, const QPixmap& icon);
    virtual ~Option();

    virtual ParseResults parse(ParseSettings& settings);

protected:

    virtual QString parseOperation(ParseSettings& settings) = 0;

private:

    OptionPriv* const d;
};

typedef QList<Option*> OptionsList;

} // namespace Digikam

#endif /* OPTION_H */

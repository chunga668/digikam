/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-04-21
 * Description : a kio-slave to process search on digiKam albums
 *
 * Copyright (C) 2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2007-2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef DIGIKAMSEARCH_H
#define DIGIKAMSEARCH_H

// KDE includes

#include <kio/slavebase.h>

class kio_digikamsearch : public KIO::SlaveBase
{

public:

    kio_digikamsearch(const QByteArray& pool_socket, const QByteArray& app_socket);
    ~kio_digikamsearch();

    void special(const QByteArray& data);
};

#endif /* DIGIKAMSEARCH_H */

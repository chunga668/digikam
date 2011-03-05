/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-09-09
 * Description : Hint data containers for the collection scanner
 *
 * Copyright (C) 2008 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef COLLECTIONSCANNERHINTS_H
#define COLLECTIONSCANNERHINTS_H

// Qt includes

#include <QList>
#include <QStringList>
#include <QDBusArgument>

// Local includes

#include "dbusutilities.h"
#include "digikam_export.h"

namespace Digikam
{

namespace CollectionScannerHints
{

class DIGIKAM_DATABASE_EXPORT Album
{
public:
    Album();
    Album(int albumRootId, int albumId);

    bool isNull() const;
    uint qHash() const;
    bool operator==(const Album& other) const;

    int albumRootId;
    int albumId;
};

// ---------------------------------------------------------------------------

class DIGIKAM_DATABASE_EXPORT DstPath
{
public:
    DstPath();
    DstPath(int albumRootId, const QString& relativePath);

    bool isNull() const;
    uint qHash() const;
    bool operator==(const DstPath& other) const;

    int albumRootId;
    QString relativePath;
};

// ---------------------------------------------------------------------------

class DIGIKAM_DATABASE_EXPORT Item
{
public:
    Item();
    Item(qlonglong id);

    bool isNull() const;
    uint qHash() const;
    bool operator==(const Item& other) const;

    qlonglong id;
};

inline uint qHash(const Album& src)
{
    return src.qHash();
}
inline uint qHash(const DstPath& dst)
{
    return dst.qHash();
}
inline uint qHash(const Item& item)
{
    return item.qHash();
}

} // namespace CollectionScannerHints

// ---------------------------------------------------------------------------

class DIGIKAM_DATABASE_EXPORT AlbumCopyMoveHint
{
public:

    /** An AlbumCopyMoveHint describes an existing album
     *  and a destination to which this album is expected to be
     *  copied, moved or renamed.
     */

    AlbumCopyMoveHint();
    AlbumCopyMoveHint(int srcAlbumRootId, int srcAlbum,
                      int dstAlbumRootId, const QString& dstRelativePath);

    int albumRootIdSrc() const;
    int albumIdSrc() const;
    bool isSrcAlbum(int albumRootId, int albumId) const;
    CollectionScannerHints::Album src() const
    {
        return m_src;
    }

    int albumRootIdDst() const;
    QString relativePathDst() const;
    bool isDstAlbum(int albumRootId, const QString& relativePath) const;
    CollectionScannerHints::DstPath dst() const
    {
        return m_dst;
    }

    uint qHash() const;

    bool operator==(const CollectionScannerHints::Album& src)
    {
        return src == m_src;
    }
    bool operator==(const CollectionScannerHints::DstPath& dst)
    {
        return dst == m_dst;
    }

    AlbumCopyMoveHint& operator<<(const QDBusArgument& argument);
    const AlbumCopyMoveHint& operator>>(QDBusArgument& argument) const;

    operator const CollectionScannerHints::Album& () const
    {
        return m_src;
    }
    operator const CollectionScannerHints::DstPath& () const
    {
        return m_dst;
    }

protected:

    CollectionScannerHints::Album   m_src;
    CollectionScannerHints::DstPath m_dst;
};

// ---------------------------------------------------------------------------

class DIGIKAM_DATABASE_EXPORT ItemCopyMoveHint
{
public:

    /** An ItemCopyMoveHint describes a list of existing items that will
     *  be copied, moved or renamed to an album given by album root id and album id.
     *  In the new album, the items will have the filenames given in dstNames.
     */

    ItemCopyMoveHint();
    ItemCopyMoveHint(QList<qlonglong> srcIds, int dstAlbumRootId, int albumId, QStringList dstNames);

    QList<qlonglong> srcIds() const;
    bool isSrcId(qlonglong id) const;

    int albumRootIdDst() const;
    int albumIdDst() const;
    bool isDstAlbum(int albumRootId, int albumId) const;
    CollectionScannerHints::Album dst() const
    {
        return m_dst;
    }
    QStringList dstNames() const;
    QString dstName(qlonglong id) const;

    bool operator==(const CollectionScannerHints::Album& dst)
    {
        return dst == m_dst;
    }

    ItemCopyMoveHint& operator<<(const QDBusArgument& argument);
    const ItemCopyMoveHint& operator>>(QDBusArgument& argument) const;

    operator const CollectionScannerHints::Album& () const
    {
        return m_dst;
    }

protected:

    QList<qlonglong>              m_srcIds;
    CollectionScannerHints::Album m_dst;
    QStringList                   m_dstNames;
};


// ---------------------------------------------------------------------------

class DIGIKAM_DATABASE_EXPORT ItemChangeHint
{
public:

    /** An ItemCopyMoveHint describes a list of existing items that
     *  should be updated although the modification date may not have changed.
     */

    enum ChangeType
    {
        ItemModified, /// treat as if modification date changed
        ItemRescan    /// reread metadata
    };

    ItemChangeHint();
    explicit ItemChangeHint(QList<qlonglong> srcIds, ChangeType type = ItemModified);

    QList<qlonglong> ids() const;
    bool isId(qlonglong id) const;
    ChangeType changeType() const;
    bool isModified() const
    {
        return changeType() == ItemModified;
    }
    bool needsRescan() const
    {
        return changeType() == ItemRescan;
    }

    ItemChangeHint& operator<<(const QDBusArgument& argument);
    const ItemChangeHint& operator>>(QDBusArgument& argument) const;

protected:

    QList<qlonglong>  m_ids;
    ChangeType        m_type;
};


inline uint qHash(const Digikam::AlbumCopyMoveHint& hint)
{
    return hint.qHash();
}

} // namespace Digikam

DECLARE_METATYPE_FOR_DBUS(Digikam::AlbumCopyMoveHint)
DECLARE_METATYPE_FOR_DBUS(Digikam::ItemCopyMoveHint)
DECLARE_METATYPE_FOR_DBUS(Digikam::ItemChangeHint)

#endif // COLLECTIONSCANNERHINTS_H

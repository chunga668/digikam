/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-07-05
 * Description : Access to the properties of an Image / Tag pair, i.e., a tag associated to an image
 *
 * Copyright (C) 2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "imagetagpair.h"

// Qt includes

#include <QSharedData>

// Local includes

#include "albumdb.h"
#include "databaseaccess.h"
#include "imageinfo.h"
#include "tagscache.h"

namespace Digikam
{

class ImageTagPairPriv : public QSharedData
{
public:

    ImageTagPairPriv()
    {
        tagId            = -1;
        isAssigned       = false;
        propertiesLoaded = false;
    }

    void init(const ImageInfo& info, int tagId);
    void checkProperties();

    ImageInfo info;
    int  tagId;
    bool isAssigned;

    bool propertiesLoaded;
    QMultiMap<QString, QString> properties;
};

class ImageTagPairPrivSharedNull : public QSharedDataPointer<ImageTagPairPriv>
{
public:
    ImageTagPairPrivSharedNull() : QSharedDataPointer<ImageTagPairPriv>(new ImageTagPairPriv) {}
};
K_GLOBAL_STATIC(ImageTagPairPrivSharedNull, imageTagPairPrivSharedNull)

void ImageTagPairPriv::init(const ImageInfo& i, int t)
{
    tagId = t;
    info  = i;
    isAssigned = info.tagIds().contains(tagId);
}

void ImageTagPairPriv::checkProperties()
{
    if (!propertiesLoaded)
    {
        QList<ImageTagProperty> props = DatabaseAccess().db()->getImageTagProperties(info.id(), tagId);
        foreach (const ImageTagProperty& p, props)
        {
            properties.insert(p.property, p.value);
        }
        propertiesLoaded = true;
    }
}

ImageTagPair::ImageTagPair()
    : d(*imageTagPairPrivSharedNull)
{
}

ImageTagPair::ImageTagPair(qlonglong imageId, int tagId)
    : d(new ImageTagPairPriv)
{
    d->init(ImageInfo(imageId), tagId);
}

ImageTagPair::ImageTagPair(const ImageInfo& info, int tagId)
    : d(new ImageTagPairPriv)
{
    d->init(info, tagId);
}

ImageTagPair::~ImageTagPair()
{
}

ImageTagPair::ImageTagPair(const ImageTagPair& other)
{
    d = other.d;
}

ImageTagPair& ImageTagPair::operator=(const ImageTagPair& other)
{
    d = other.d;
    return *this;
}

bool ImageTagPair::isNull() const
{
    return d == *imageTagPairPrivSharedNull;
}

QList<ImageTagPair> ImageTagPair::availablePairs(qlonglong imageId)
{
    return availablePairs(ImageInfo(imageId));
}

QList<ImageTagPair> ImageTagPair::availablePairs(const ImageInfo& info)
{
    QList<ImageTagPair> pairs;

    if (info.isNull())
    {
        return pairs;
    }

    QList<int> tagIds = DatabaseAccess().db()->getTagIdsWithProperties(info.id());
    foreach (int tagId, tagIds)
    {
        pairs << ImageTagPair(info, tagId);
    }
    return pairs;
}

qlonglong ImageTagPair::imageId() const
{
    return d->info.id();
}

int ImageTagPair::tagId() const
{
    return d->tagId;
}

bool ImageTagPair::isAssigned() const
{
    return d->isAssigned;
}

void ImageTagPair::assignTag()
{
    if (!d->isAssigned)
    {
        d->info.setTag(d->tagId);
        d->isAssigned = true;
    }
}

void ImageTagPair::unAssignTag()
{
    if (d->isAssigned)
    {
        d->info.removeTag(d->tagId);
        d->isAssigned = false;
    }
}

bool ImageTagPair::hasProperty(const QString& key) const
{
    d->checkProperties();
    return d->properties.contains(key);
}

bool ImageTagPair::hasAnyProperty(const QStringList& keys) const
{
    d->checkProperties();
    foreach (const QString& key, keys)
    {
        if (d->properties.contains(key))
        {
            return true;
        }
    }
    return false;
}

bool ImageTagPair::hasValue(const QString& key, const QString& value) const
{
    d->checkProperties();
    return d->properties.contains(key, value);
}

QString ImageTagPair::value(const QString& key) const
{
    d->checkProperties();
    return d->properties.value(key);
}

QStringList ImageTagPair::allValues(const QStringList& keys) const
{
    d->checkProperties();
    QStringList values;
    foreach (const QString& key, keys)
    {
        values << d->properties.values(key);
    }
    return values;
}

QStringList ImageTagPair::values(const QString& key) const
{
    d->checkProperties();
    return d->properties.values(key);
}


QStringList ImageTagPair::propertyKeys() const
{
    d->checkProperties();
    return d->properties.keys();
}

QMap<QString, QString> ImageTagPair::properties() const
{
    d->checkProperties();
    return d->properties;
}

void ImageTagPair::setProperty(const QString& key, const QString& value)
{
    if (d->info.isNull())
    {
        return;
    }

    d->checkProperties();

    // for single entries in db, this can of course be optimized using a single UPDATE WHERE
    removeProperties(key);
    d->properties.replace(key, value);
    DatabaseAccess().db()->addImageTagProperty(d->info.id(), d->tagId, key, value);
}

void ImageTagPair::addProperty(const QString& key, const QString& value)
{
    if (d->info.isNull())
    {
        return;
    }

    d->checkProperties();

    if (!d->properties.contains(key, value))
    {
        d->properties.insert(key, value);
        DatabaseAccess().db()->addImageTagProperty(d->info.id(), d->tagId, key, value);
    }
}

void ImageTagPair::removeProperty(const QString& key, const QString& value)
{
    if (d->info.isNull())
    {
        return;
    }

    d->checkProperties();

    if (d->properties.contains(key, value))
    {
        DatabaseAccess().db()->removeImageTagProperties(d->info.id(), d->tagId, key, value);
        d->properties.remove(key, value);
    }
}

void ImageTagPair::removeProperties(const QString& key)
{
    if (d->info.isNull())
    {
        return;
    }

    d->checkProperties();

    if (d->properties.contains(key))
    {
        DatabaseAccess().db()->removeImageTagProperties(d->info.id(), d->tagId, key);
        d->properties.remove(key);
    }
}

void ImageTagPair::clearProperties()
{
    if (d->info.isNull())
    {
        return;
    }

    if (d->propertiesLoaded && d->properties.isEmpty())
    {
        return;
    }

    DatabaseAccess().db()->removeImageTagProperties(d->info.id(), d->tagId);
    d->properties.clear();
    d->propertiesLoaded = true;
}

}


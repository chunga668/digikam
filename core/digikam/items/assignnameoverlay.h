/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-10-14
 * Description : overlay for assigning names to faces
 *
 * Copyright (C) 2010 by Aditya Bhatt <caulier dot gilles at gmail dot com>
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

#ifndef ASSIGNNAMEOVERLAY_H
#define ASSIGNNAMEOVERLAY_H

// Qt includes

#include <QAbstractItemView>

// Local includes

#include "imagedelegateoverlay.h"
#include "itemviewimagedelegate.h"

namespace Digikam
{

class AssignNameWidget;
class DatabaseFace;
class ImageInfo;
class TaggingAction;

class AssignNameOverlay : public AbstractWidgetDelegateOverlay
{
    Q_OBJECT
    REQUIRE_DELEGATE(ItemViewImageDelegate)

public:

    AssignNameOverlay(QObject* parent);
    ~AssignNameOverlay();

    AssignNameWidget* assignNameWidget() const;

Q_SIGNALS:

    void faceConfirmed(const DatabaseFace& face);
    void faceRejected(const DatabaseFace& face);

protected Q_SLOTS:

    void slotAssigned(const TaggingAction& action, const ImageInfo&, const QVariant& faceIdentifier);
    void slotRejected(const ImageInfo&, const QVariant& faceIdentifier);

protected:

    virtual QWidget* createWidget();
    virtual void setActive(bool);
    virtual void visualChange();
    virtual void slotEntered(const QModelIndex& index);
    virtual void hide();
    virtual bool checkIndex(const QModelIndex& index) const;
    virtual void viewportLeaveEvent(QObject* obj, QEvent* event);

    void updatePosition();
    void updateFace();

private:

    class AssignNameOverlayPriv;
    AssignNameOverlayPriv* const d;
};

} // namespace Digikam

#endif /* IMAGERATINGOVERLAY_H */

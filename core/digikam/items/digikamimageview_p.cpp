/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-10-03
 * Description : Private Qt item view for images
 *
 * Copyright (C) 2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009 by Johannes Wienke <languitar at semipol do de>
 *
 * This program is free software you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "digikamimageview_p.moc"

namespace Digikam
{

DigikamImageViewPriv::DigikamImageViewPriv(DigikamImageView* qq)
    : overlaysActive(false), q_ptr(qq)
{
    utilities          = 0;
    rotateLeftOverlay  = 0;
    rotateRightOverlay = 0;
    normalDelegate     = 0;
    faceDelegate       = 0;
    faceiface          = 0;
}

DigikamImageViewPriv::~DigikamImageViewPriv()
{
}

void DigikamImageViewPriv::updateOverlays()
{
    Q_Q(DigikamImageView);
    AlbumSettings* settings = AlbumSettings::instance();

    if (overlaysActive)
    {
        if (!settings->getIconShowOverlays())
        {
            disconnect(rotateLeftOverlay, SIGNAL(signalRotateLeft()),
                       q, SLOT(slotRotateLeft()));

            disconnect(rotateRightOverlay, SIGNAL(signalRotateRight()),
                       q, SLOT(slotRotateRight()));

            q->removeOverlay(rotateLeftOverlay);
            q->removeOverlay(rotateRightOverlay);

            overlaysActive = false;
        }
    }
    else
    {
        if (settings->getIconShowOverlays())
        {
            q->addOverlay(rotateLeftOverlay, normalDelegate);
            q->addOverlay(rotateRightOverlay, normalDelegate);

            connect(rotateLeftOverlay, SIGNAL(signalRotateLeft()),
                    q, SLOT(slotRotateLeft()));

            connect(rotateRightOverlay, SIGNAL(signalRotateRight()),
                    q, SLOT(slotRotateRight()));

            overlaysActive = true;
        }
    }
}

} // namespace Digikam

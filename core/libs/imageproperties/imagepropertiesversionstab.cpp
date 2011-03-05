/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-07-12
 * Description : tab for displaying image versions
 *
 * Copyright (C) 2010 by Martin Klapetek <martin dot klapetek at gmail dot com>
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

#include "imagepropertiesversionstab.moc"

// Qt includes

#include <QListView>
#include <QGridLayout>
#include <QLabel>
#include <QModelIndex>

// KDE includes

#include <KConfig>
#include <KConfigGroup>
#include <KGlobal>
#include <KDebug>
#include <KLocale>
#include <KIconLoader>
#include <KStandardGuiItem>
#include <KUrl>

// Local includes

#include "imageversionsmodel.h"
#include "dmetadata.h"
#include "dimagehistory.h"
#include "imageinfo.h"
#include "imageinfolist.h"
#include "versionswidget.h"
#include "filtershistorywidget.h"
#include "albumsettings.h"
#include "versionsoverlays.h"

namespace Digikam
{

class ImagePropertiesVersionsTab::ImagePropertiesVersionsTabPriv
{
public:

    ImagePropertiesVersionsTabPriv()
    {
        versionsWidget       = 0;
        filtersHistoryWidget = 0;
    }

    VersionsWidget*                versionsWidget;
    FiltersHistoryWidget*          filtersHistoryWidget;
    DImageHistory                  history;
    ImageInfo                      info;
    QString                        currentSelectedImagePath;
    int                            currentSelectedImageListPosition;
    qlonglong                      currentSelectedImageId;

    static const QString           configActiveTab;
};
const QString ImagePropertiesVersionsTab::ImagePropertiesVersionsTabPriv::configActiveTab("Version Properties Tab");

ImagePropertiesVersionsTab::ImagePropertiesVersionsTab(QWidget* parent)
    : KTabWidget(parent), d(new ImagePropertiesVersionsTabPriv)
{
    d->versionsWidget = new VersionsWidget(this);
    insertTab(0, d->versionsWidget, i18n("Versions"));

    d->filtersHistoryWidget = new FiltersHistoryWidget(this);
    insertTab(1, d->filtersHistoryWidget, i18n("Used Filters"));

    connect(d->versionsWidget, SIGNAL(imageSelected(const ImageInfo&)),
            this, SIGNAL(imageSelected(const ImageInfo&)));
}

ImagePropertiesVersionsTab::~ImagePropertiesVersionsTab()
{
    delete d;
}

void ImagePropertiesVersionsTab::readSettings(const KConfigGroup& group)
{
    QString tab = group.readEntry(d->configActiveTab, "versions");
    if (tab == "versions")
        setCurrentWidget(d->versionsWidget);
    else
        setCurrentWidget(d->filtersHistoryWidget);

    d->versionsWidget->readSettings(group);
}

void ImagePropertiesVersionsTab::writeSettings(KConfigGroup& group)
{
    group.writeEntry(d->configActiveTab, currentWidget() == d->versionsWidget ? "versions" : "filters");

    d->versionsWidget->writeSettings(group);
}

VersionsWidget* ImagePropertiesVersionsTab::versionsWidget() const
{
    return d->versionsWidget;
}

FiltersHistoryWidget* ImagePropertiesVersionsTab::filtersHistoryWidget() const
{
    return d->filtersHistoryWidget;
}

void ImagePropertiesVersionsTab::clear()
{
    d->filtersHistoryWidget->clearData();
    d->versionsWidget->setCurrentItem(ImageInfo());
}

void ImagePropertiesVersionsTab::setItem(const ImageInfo& info, const DImageHistory& history)
{
    clear();

    if (info.isNull())
    {
        return;
    }

    d->history = history;

    if (d->history.isNull())
    {
        d->history = info.imageHistory();
    }

    d->info = info;

    d->versionsWidget->setCurrentItem(info);
    d->filtersHistoryWidget->setHistory(d->history);
}

void ImagePropertiesVersionsTab::addShowHideOverlay()
{
    d->versionsWidget->addShowHideOverlay();
}

void ImagePropertiesVersionsTab::addOpenImageAction()
{
    ActionVersionsOverlay* overlay = d->versionsWidget->addActionOverlay(KStandardGuiItem::open());

    connect(overlay, SIGNAL(activated(const ImageInfo&)),
            this, SIGNAL(actionTriggered(const ImageInfo&)));
}

void ImagePropertiesVersionsTab::addOpenAlbumAction(const ImageModel* referenceModel)
{
    KGuiItem gui(i18n("Go To Albums"), "folder-image",
                 i18nc("@info:tooltip", "Go to the album of this image"));
    ActionVersionsOverlay* overlay = d->versionsWidget->addActionOverlay(gui);
    overlay->setReferenceModel(referenceModel);

    connect(overlay, SIGNAL(activated(const ImageInfo&)),
            this, SIGNAL(actionTriggered(const ImageInfo&)));
}

void ImagePropertiesVersionsTab::setEnabledHistorySteps(int count)
{
    d->filtersHistoryWidget->setEnabledEntries(count);
}

} // namespace Digikam

/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-08-03
 * Description : widget displaying all image versions in a list
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

#include "versionswidget.moc"

// Qt includes

#include <QButtonGroup>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPaintEvent>
#include <QToolButton>
#include <QTreeView>

// KDE includes

#include <KConfigGroup>
#include <KLocale>
#include <KIconLoader>
#include <KUrl>
#include <KDebug>

// Local includes

#include "albumsettings.h"
#include "dimagehistory.h"
#include "imagehistorygraphmodel.h"
#include "imageinfo.h"
#include "imagelistmodel.h"
#include "thumbnailloadthread.h"
#include "versionsdelegate.h"
#include "versionmanagersettings.h"
#include "versionsoverlays.h"
#include "versionstreeview.h"

namespace Digikam
{

class VersionsWidget::VersionsWidgetPriv
{
public:

    VersionsWidgetPriv()
        : view(0),
          model(0),
          delegate(0),
          showHideOverlay(0),
          viewButtonGroup(0),
          listModeButton(0),
          treeModeButton(0),
          combinedModeButton(0)
    {
    }

    VersionsTreeView*                view;
    ImageHistoryGraphModel*          model;
    VersionsDelegate*                delegate;

    ShowHideVersionsOverlay*         showHideOverlay;

    QButtonGroup*                    viewButtonGroup;
    QToolButton*                     listModeButton;
    QToolButton*                     treeModeButton;
    QToolButton*                     combinedModeButton;

    static const QString             configCurrentMode;
};
const QString VersionsWidget::VersionsWidgetPriv::configCurrentMode("Version Properties View Mode");


VersionsWidget::VersionsWidget(QWidget* parent)
    : QWidget(parent), d(new VersionsWidgetPriv)
{
    QGridLayout* layout      = new QGridLayout;

    d->viewButtonGroup    = new QButtonGroup(this);
    d->listModeButton     = new QToolButton;
    d->listModeButton->setIcon(SmallIcon("view-list-icons"));
    d->listModeButton->setCheckable(true);
    d->listModeButton->setToolTip(i18n("Show available versions in a list"));
    d->viewButtonGroup->addButton(d->listModeButton, ImageHistoryGraphModel::ImagesListMode);

    d->treeModeButton     = new QToolButton;
    d->treeModeButton->setIcon(SmallIcon("view-list-tree"));
    d->treeModeButton->setCheckable(true);
    d->treeModeButton->setToolTip(i18n("Show available versions as a tree"));
    d->viewButtonGroup->addButton(d->treeModeButton, ImageHistoryGraphModel::ImagesTreeMode);

    d->combinedModeButton = new QToolButton;
    d->combinedModeButton->setIcon(SmallIcon("view-list-details"));
    d->combinedModeButton->setCheckable(true);
    d->combinedModeButton->setToolTip(i18n("Show available version and the applied filters in a combined list"));
    d->viewButtonGroup->addButton(d->combinedModeButton, ImageHistoryGraphModel::CombinedTreeMode);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(d->listModeButton);
    buttonLayout->addWidget(d->treeModeButton);
    buttonLayout->addWidget(d->combinedModeButton);

    d->model                = new ImageHistoryGraphModel(this);
    d->model->imageModel()->setThumbnailLoadThread(ThumbnailLoadThread::defaultIconViewThread());

    d->view                 = new VersionsTreeView;
    d->view->setModel(d->model);
    d->view->setWordWrap(true);
    d->view->setRootIsDecorated(false);
    d->view->setHeaderHidden(true);
    d->view->setSelectionMode(QAbstractItemView::SingleSelection);
    //d->view->setFrameShape(QFrame::NoFrame);
    d->view->setFrameShadow(QFrame::Plain);

    layout->addLayout(buttonLayout,   0, 1);
    layout->addWidget(d->view,        1, 0, 1, 2);
    layout->setColumnStretch(0, 1);
    layout->setRowStretch(1, 1);
    setLayout(layout);

    connect(d->view->delegate(), SIGNAL(animationStateChanged()),
            d->view->viewport(), SLOT(update()));

    connect(d->view->selectionModel(), SIGNAL(currentChanged(const QModelIndex&,const QModelIndex&)),
            this, SLOT(slotViewCurrentChanged(const QModelIndex&,const QModelIndex&)));

    connect(d->viewButtonGroup, SIGNAL(buttonClicked(int)),
            this, SLOT(slotViewModeChanged(int)));

    connect(AlbumSettings::instance(), SIGNAL(setupChanged()),
            this, SLOT(slotSetupChanged()));

    slotSetupChanged();
}

VersionsWidget::~VersionsWidget()
{
    delete d;
}

void VersionsWidget::readSettings(const KConfigGroup& group)
{
    int mode = group.readEntry(d->configCurrentMode, (int)ImageHistoryGraphModel::CombinedTreeMode);
    switch (mode)
    {
        case ImageHistoryGraphModel::ImagesListMode:
            d->listModeButton->setChecked(true);
            break;
        case ImageHistoryGraphModel::ImagesTreeMode:
            d->treeModeButton->setChecked(true);
            break;
        default:
        case ImageHistoryGraphModel::CombinedTreeMode:
            d->combinedModeButton->setChecked(true);
            break;
    }
    slotViewModeChanged(mode);
}

VersionsTreeView *VersionsWidget::view() const
{
    return d->view;
}

VersionsDelegate *VersionsWidget::delegate() const
{
    return d->delegate;
}

ActionVersionsOverlay* VersionsWidget::addActionOverlay(const KGuiItem& item)
{
    ActionVersionsOverlay* overlay = new ActionVersionsOverlay(this, item);
    d->view->addOverlay(overlay);
    return overlay;
}

ShowHideVersionsOverlay* VersionsWidget::addShowHideOverlay()
{
    d->showHideOverlay = new ShowHideVersionsOverlay(this);
    d->showHideOverlay->setSettings(AlbumSettings::instance()->getVersionManagerSettings());
    d->view->addOverlay(d->showHideOverlay);
    return d->showHideOverlay;
}

void VersionsWidget::writeSettings(KConfigGroup& group)
{
    group.writeEntry(d->configCurrentMode, d->viewButtonGroup->checkedId());
}

void VersionsWidget::setCurrentItem(const ImageInfo& info)
{
    d->model->setHistory(info);
}

void VersionsWidget::slotViewCurrentChanged(const QModelIndex& current, const QModelIndex& previous)
{
    ImageInfo info = d->model->imageInfo(current);
    if (!info.isNull())
    {
        emit imageSelected(info);
    }

    switch (d->model->mode())
    {
        case ImageHistoryGraphModel::ImagesListMode:
        case ImageHistoryGraphModel::ImagesTreeMode:
            break;
        case ImageHistoryGraphModel::CombinedTreeMode:
            // toplevel image index
            if (!info.isNull() && !current.parent().isValid())
            {
                d->view->expand(current);
            }
            if (previous.isValid() && d->model->isImage(previous) && !previous.parent().isValid())
            {
                d->view->collapse(previous);
            }
            break;
    }
}

void VersionsWidget::slotViewModeChanged(int mode)
{
    d->model->setMode((ImageHistoryGraphModel::Mode)mode);

    switch (mode)
    {
        case ImageHistoryGraphModel::ImagesListMode:
            break;
        case ImageHistoryGraphModel::ImagesTreeMode:
            d->view->expandAll();
            break;
        case ImageHistoryGraphModel::CombinedTreeMode:
            d->view->collapseAll();
            break;
    }

    QModelIndex subjectIndex = d->model->indexForInfo(d->model->subject());
    d->view->scrollTo(subjectIndex, QAbstractItemView::PositionAtCenter);
    d->view->setCurrentIndex(subjectIndex);
}

void VersionsWidget::slotSetupChanged()
{
    d->view->setToolTipEnabled(AlbumSettings::instance()->showToolTipsIsValid());
    if (d->showHideOverlay)
    {
        d->showHideOverlay->setSettings(AlbumSettings::instance()->getVersionManagerSettings());
    }
}

} // namespace Digikam

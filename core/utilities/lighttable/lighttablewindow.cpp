/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-03-05
 * Description : digiKam light table GUI
 *
 * Copyright (C) 2007-2011 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "lighttablewindow.moc"
#include "lighttablewindow_p.h"

// Qt includes

#include <QFrame>
#include <QHBoxLayout>
#include <QVBoxLayout>

// KDE includes
#include <kaction.h>
#include <kactioncollection.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kedittoolbar.h>
#include <kglobal.h>
#include <klocale.h>
#include <kmenubar.h>
#include <kmessagebox.h>
#include <knotifyconfigwidget.h>
#include <kselectaction.h>
#include <kshortcutsdialog.h>
#include <kstandardaction.h>
#include <kstandardshortcut.h>
#include <kstatusbar.h>
#include <ktoggleaction.h>
#include <ktoolbar.h>
#include <ktoolinvocation.h>
#include <kwindowsystem.h>
#include <kxmlguifactory.h>

// Libkdcraw includes

#include <libkdcraw/version.h>
#include <libkdcraw/kdcraw.h>

// Local includes

#include "componentsinfo.h"
#include "digikamapp.h"
#include "themeengine.h"
#include "dimg.h"
#include "dlogoaction.h"
#include "dmetadata.h"
#include "metadatasettings.h"
#include "albumsettings.h"
#include "albummanager.h"
#include "loadingcacheinterface.h"
#include "deletedialog.h"
#include "imagewindow.h"
#include "slideshow.h"
#include "setup.h"
#include "syncjob.h"
#include "thumbbardock.h"
#include "thumbnailsize.h"
#include "lighttablepreview.h"
#include "uifilevalidator.h"
#include "albummodel.h"

namespace Digikam
{

LightTableWindow* LightTableWindow::m_instance = 0;

LightTableWindow* LightTableWindow::lightTableWindow()
{
    if (!m_instance)
    {
        new LightTableWindow();
    }

    return m_instance;
}

bool LightTableWindow::lightTableWindowCreated()
{
    return m_instance;
}

LightTableWindow::LightTableWindow()
    : KXmlGuiWindow(0), d(new LightTableWindowPriv)
{
    setXMLFile("lighttablewindowui.rc");

    // --------------------------------------------------------

    UiFileValidator validator(localXMLFile());

    if (!validator.isValid())
    {
        validator.fixConfigFile();
    }

    // --------------------------------------------------------

    m_instance = this;

    setWindowFlags(Qt::Window);
    setCaption(i18n("Light Table"));
    // We don't want to be deleted on close
    setAttribute(Qt::WA_DeleteOnClose, false);

    // -- Build the GUI -------------------------------

    setupUserArea();
    setupActions();
    setupStatusBar();

    // Make signals/slots connections

    setupConnections();

    //-------------------------------------------------------------

    slotSidebarTabTitleStyleChanged();

    readSettings();

    d->leftSideBar->populateTags();
    d->rightSideBar->populateTags();

    applySettings();
    setAutoSaveSettings("LightTable Settings", true);
}

LightTableWindow::~LightTableWindow()
{
    m_instance = 0;

    delete d->barView;
    delete d->rightSideBar;
    delete d->leftSideBar;
    delete d;
}

void LightTableWindow::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("LightTable Settings");

    d->hSplitter->restoreState(group, "Horizontal Splitter State");
    d->barViewDock->setShouldBeVisible(group.readEntry("Show Thumbbar", true));
    d->navigateByPairAction->setChecked(group.readEntry("Navigate By Pair", false));
    slotToggleNavigateByPair();

    d->leftSideBar->setConfigGroup(KConfigGroup(&group, "Left Sidebar"));
    d->leftSideBar->loadState();
    d->rightSideBar->setConfigGroup(KConfigGroup(&group, "Right Sidebar"));
    d->rightSideBar->loadState();
}

void LightTableWindow::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("LightTable Settings");
    d->hSplitter->saveState(group, "Horizontal Splitter State");
    group.writeEntry("Show Thumbbar", d->barViewDock->shouldBeVisible());
    group.writeEntry("Navigate By Pair", d->navigateByPairAction->isChecked());
    group.writeEntry("Clear On Close", d->clearOnCloseAction->isChecked());

    d->leftSideBar->setConfigGroup(KConfigGroup(&group, "Left Sidebar"));
    d->leftSideBar->saveState();
    d->rightSideBar->setConfigGroup(KConfigGroup(&group, "Right Sidebar"));
    d->rightSideBar->saveState();
    config->sync();
}

void LightTableWindow::applySettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("LightTable Settings");
    d->autoLoadOnRightPanel   = group.readEntry("Auto Load Right Panel",   true);
    d->autoSyncPreview        = group.readEntry("Auto Sync Preview",       true);
    d->fullScreenHideToolBar  = group.readEntry("FullScreen Hide ToolBar", false);
    d->clearOnCloseAction->setChecked(group.readEntry("Clear On Close", false));
    d->previewView->setLoadFullImageSize(group.readEntry("Load Full Image size", false));
    d->barView->applySettings();
    refreshView();
}

void LightTableWindow::refreshView()
{
    d->leftSideBar->refreshTagsView();
    d->rightSideBar->refreshTagsView();
}

void LightTableWindow::closeEvent(QCloseEvent* e)
{
    if (!e)
    {
        return;
    }

    if (d->clearOnCloseAction->isChecked())
    {
        slotClearItemsList();
    }

    // There is one nasty habit with the thumbnail bar if it is floating: it
    // doesn't close when the parent window does, so it needs to be manually
    // closed. If the light table is opened again, its original state needs to
    // be restored.
    // This only needs to be done when closing a visible window and not when
    // destroying a closed window, since the latter case will always report that
    // the thumbnail bar isn't visible.
    if (isVisible())
    {
        d->barViewDock->hide();
    }

    writeSettings();

    e->accept();
}

void LightTableWindow::showEvent(QShowEvent*)
{
    // Restore the visibility of the thumbbar and start autosaving again.
    d->barViewDock->restoreVisibility();
}

void LightTableWindow::setupUserArea()
{
    QWidget* mainW    = new QWidget(this);
    d->hSplitter      = new SidebarSplitter(Qt::Horizontal, mainW);
    QHBoxLayout* hlay = new QHBoxLayout(mainW);

    // The left sidebar
    d->leftSideBar    = new ImagePropertiesSideBarDB(mainW, d->hSplitter, KMultiTabBar::Left, true);

    // The central preview is wrapped in a KMainWindow so that the thumbnail
    // bar can float around it.
    KMainWindow* viewContainer = new KMainWindow(mainW, Qt::Widget);
    d->hSplitter->addWidget(viewContainer);
    d->previewView             = new LightTableView(viewContainer);
    viewContainer->setCentralWidget(d->previewView);

    // The right sidebar.
    d->rightSideBar = new ImagePropertiesSideBarDB(mainW, d->hSplitter, KMultiTabBar::Right, true);

    hlay->addWidget(d->leftSideBar);
    hlay->addWidget(d->hSplitter);
    hlay->addWidget(d->rightSideBar);
    hlay->setSpacing(0);
    hlay->setMargin(0);
    hlay->setStretchFactor(d->hSplitter, 10);

    d->hSplitter->setFrameStyle( QFrame::NoFrame );
    d->hSplitter->setFrameShadow( QFrame::Plain );
    d->hSplitter->setFrameShape( QFrame::NoFrame );
    d->hSplitter->setOpaqueResize(false);
    d->hSplitter->setStretchFactor(1, 10);      // set previewview+thumbbar container default size to max.

    // The thumb bar is placed in a detachable/dockable widget.
    d->barViewDock = new ThumbBarDock(viewContainer, Qt::Tool);
    d->barViewDock->setObjectName("lighttable_thumbbar");
    d->barView     = new LightTableBar(d->barViewDock, Qt::Horizontal,
                                       MetadataSettings::instance()->settings().exifRotate);
    d->barViewDock->setWidget(d->barView);
    viewContainer->addDockWidget(Qt::TopDockWidgetArea, d->barViewDock);
    d->barViewDock->setFloating(false);

    // Restore the previous state. This doesn't emit the proper signals to the
    // dock widget, so it has to be manually reinitialized.
    viewContainer->setAutoSaveSettings("LightTable Thumbbar", true);
    d->barViewDock->reInitialize();

    setCentralWidget(mainW);
}

void LightTableWindow::setupStatusBar()
{
    d->leftZoomBar = new DZoomBar(statusBar());
    d->leftZoomBar->setZoomToFitAction(d->leftZoomFitToWindowAction);
    d->leftZoomBar->setZoomTo100Action(d->leftZoomTo100percents);
    d->leftZoomBar->setZoomPlusAction(d->leftZoomPlusAction);
    d->leftZoomBar->setZoomMinusAction(d->leftZoomMinusAction);
    d->leftZoomBar->setBarMode(DZoomBar::PreviewZoomCtrl);
    d->leftZoomBar->setEnabled(false);
    statusBar()->addWidget(d->leftZoomBar, 1);

    d->leftFileName = new KSqueezedTextLabel(statusBar());
    d->leftFileName->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    statusBar()->addWidget(d->leftFileName, 10);

    d->statusProgressBar = new StatusProgressBar(statusBar());
    d->statusProgressBar->setAlignment(Qt::AlignCenter);
    statusBar()->addWidget(d->statusProgressBar, 10);

    d->rightFileName = new KSqueezedTextLabel(statusBar());
    d->rightFileName->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    statusBar()->addWidget(d->rightFileName, 10);

    d->rightZoomBar = new DZoomBar(statusBar());
    d->rightZoomBar->setZoomToFitAction(d->rightZoomFitToWindowAction);
    d->rightZoomBar->setZoomTo100Action(d->rightZoomTo100percents);
    d->rightZoomBar->setZoomPlusAction(d->rightZoomPlusAction);
    d->rightZoomBar->setZoomMinusAction(d->rightZoomMinusAction);
    d->rightZoomBar->setBarMode(DZoomBar::PreviewZoomCtrl);
    d->rightZoomBar->setEnabled(false);
    statusBar()->addWidget(d->rightZoomBar, 1);
}

void LightTableWindow::setupConnections()
{
    connect(d->statusProgressBar, SIGNAL(signalCancelButtonPressed()),
            this, SLOT(slotProgressBarCancelButtonPressed()));

    connect(ThemeEngine::instance(), SIGNAL(signalThemeChanged()),
            this, SLOT(slotThemeChanged()));

    connect(AlbumSettings::instance(), SIGNAL(setupChanged()),
            this, SLOT(slotSidebarTabTitleStyleChanged()));

    // Thumbs bar connections ---------------------------------------

    connect(d->barView, SIGNAL(signalSetItemOnLeftPanel(const ImageInfo&)),
            this, SLOT(slotSetItemOnLeftPanel(const ImageInfo&)));

    connect(d->barView, SIGNAL(signalSetItemOnRightPanel(const ImageInfo&)),
            this, SLOT(slotSetItemOnRightPanel(const ImageInfo&)));

    connect(d->barView, SIGNAL(signalRemoveItem(const ImageInfo&)),
            this, SLOT(slotRemoveItem(const ImageInfo&)));

    connect(d->barView, SIGNAL(signalEditItem(const ImageInfo&)),
            this, SLOT(slotEditItem(const ImageInfo&)));

    connect(d->barView, SIGNAL(signalClearAll()),
            this, SLOT(slotClearItemsList()));

    connect(d->barView, SIGNAL(signalLightTableBarItemSelected(const ImageInfo&)),
            this, SLOT(slotItemSelected(const ImageInfo&)));

    connect(d->barView, SIGNAL(signalDroppedItems(const ImageInfoList&)),
            this, SLOT(slotThumbbarDroppedItems(const ImageInfoList&)));

    // Zoom bars connections -----------------------------------------

    connect(d->leftZoomBar, SIGNAL(signalZoomSliderChanged(int)),
            d->previewView, SLOT(slotLeftZoomSliderChanged(int)));

    connect(d->leftZoomBar, SIGNAL(signalZoomValueEdited(double)),
            d->previewView, SLOT(setLeftZoomFactor(double)));

    connect(d->rightZoomBar, SIGNAL(signalZoomSliderChanged(int)),
            d->previewView, SLOT(slotRightZoomSliderChanged(int)));

    connect(d->rightZoomBar, SIGNAL(signalZoomValueEdited(double)),
            d->previewView, SLOT(setRightZoomFactor(double)));

    // View connections ---------------------------------------------

    connect(d->previewView, SIGNAL(signalLeftPopupTagsView()),
            d->leftSideBar, SLOT(slotPopupTagsView()));

    connect(d->previewView, SIGNAL(signalRightPopupTagsView()),
            d->rightSideBar, SLOT(slotPopupTagsView()));

    connect(d->previewView, SIGNAL(signalLeftZoomFactorChanged(double)),
            this, SLOT(slotLeftZoomFactorChanged(double)));

    connect(d->previewView, SIGNAL(signalRightZoomFactorChanged(double)),
            this, SLOT(slotRightZoomFactorChanged(double)));

    connect(d->previewView, SIGNAL(signalEditItem(const ImageInfo&)),
            this, SLOT(slotEditItem(const ImageInfo&)));

    connect(d->previewView, SIGNAL(signalDeleteItem(const ImageInfo&)),
            this, SLOT(slotDeleteItem(const ImageInfo&)));

    connect(d->previewView, SIGNAL(signalSlideShow()),
            this, SLOT(slotToggleSlideShow()));

    connect(d->previewView, SIGNAL(signalLeftDroppedItems(const ImageInfoList&)),
            this, SLOT(slotLeftDroppedItems(const ImageInfoList&)));

    connect(d->previewView, SIGNAL(signalRightDroppedItems(const ImageInfoList&)),
            this, SLOT(slotRightDroppedItems(const ImageInfoList&)));

    connect(d->previewView, SIGNAL(signalToggleOnSyncPreview(bool)),
            this, SLOT(slotToggleOnSyncPreview(bool)));

    connect(d->previewView, SIGNAL(signalLeftPreviewLoaded(bool)),
            this, SLOT(slotLeftPreviewLoaded(bool)));

    connect(d->previewView, SIGNAL(signalRightPreviewLoaded(bool)),
            this, SLOT(slotRightPreviewLoaded(bool)));

    connect(d->previewView, SIGNAL(signalLeftPanelLeftButtonClicked()),
            this, SLOT(slotLeftPanelLeftButtonClicked()));

    connect(d->previewView, SIGNAL(signalRightPanelLeftButtonClicked()),
            this, SLOT(slotRightPanelLeftButtonClicked()));

    connect(this, SIGNAL(signalWindowHasMoved()),
            d->leftZoomBar, SLOT(slotUpdateTrackerPos()));

    connect(this, SIGNAL(signalWindowHasMoved()),
            d->rightZoomBar, SLOT(slotUpdateTrackerPos()));

    // -- FileWatch connections ------------------------------

    LoadingCacheInterface::connectToSignalFileChanged(this, SLOT(slotFileChanged(const QString&)));
}

void LightTableWindow::setupActions()
{
    // -- Standard 'File' menu actions ---------------------------------------------

    d->backwardAction = KStandardAction::back(this, SLOT(slotBackward()), this);
    actionCollection()->addAction("lighttable_backward", d->backwardAction);
    d->backwardAction->setShortcut(KShortcut(Qt::Key_PageUp, Qt::Key_Backspace));

    d->forwardAction = KStandardAction::forward(this, SLOT(slotForward()), this);
    actionCollection()->addAction("lighttable_forward", d->forwardAction);
    d->forwardAction->setEnabled(false);
    d->forwardAction->setShortcut(KShortcut(Qt::Key_PageDown, Qt::Key_Space));

    d->firstAction = new KAction(KIcon("go-first"), i18n("&First"), this);
    d->firstAction->setShortcut(KStandardShortcut::begin());
    d->firstAction->setEnabled(false);
    connect(d->firstAction, SIGNAL(triggered()), this, SLOT(slotFirst()));
    actionCollection()->addAction("lighttable_first", d->firstAction);

    d->lastAction = new KAction(KIcon("go-last"), i18n("&Last"), this);
    d->lastAction->setShortcut(KStandardShortcut::end());
    d->lastAction->setEnabled(false);
    connect(d->lastAction, SIGNAL(triggered()), this, SLOT(slotLast()));
    actionCollection()->addAction("lighttable_last", d->lastAction);

    d->setItemLeftAction = new KAction(KIcon("arrow-left"), i18n("On left"), this);
    d->setItemLeftAction->setShortcut(KShortcut(Qt::CTRL+Qt::Key_L));
    d->setItemLeftAction->setEnabled(false);
    d->setItemLeftAction->setWhatsThis(i18n("Show item on left panel"));
    connect(d->setItemLeftAction, SIGNAL(triggered()), this, SLOT(slotSetItemLeft()));
    actionCollection()->addAction("lighttable_setitemleft", d->setItemLeftAction);

    d->setItemRightAction = new KAction(KIcon("arrow-right"), i18n("On right"), this);
    d->setItemRightAction->setShortcut(KShortcut(Qt::CTRL+Qt::Key_R));
    d->setItemRightAction->setEnabled(false);
    d->setItemRightAction->setWhatsThis(i18n("Show item on right panel"));
    connect(d->setItemRightAction, SIGNAL(triggered()), this, SLOT(slotSetItemRight()));
    actionCollection()->addAction("lighttable_setitemright", d->setItemRightAction);

    d->editItemAction = new KAction(KIcon("editimage"), i18n("Edit"), this);
    d->editItemAction->setShortcut(KShortcut(Qt::Key_F4));
    d->editItemAction->setEnabled(false);
    connect(d->editItemAction, SIGNAL(triggered()), this, SLOT(slotEditItem()));
    actionCollection()->addAction("lighttable_edititem", d->editItemAction);

    d->removeItemAction = new KAction(KIcon("list-remove"), i18n("Remove item from LightTable"), this);
    d->removeItemAction->setShortcut(KShortcut(Qt::CTRL+Qt::Key_K));
    d->removeItemAction->setEnabled(false);
    connect(d->removeItemAction, SIGNAL(triggered()), this, SLOT(slotRemoveItem()));
    actionCollection()->addAction("lighttable_removeitem", d->removeItemAction);

    d->clearListAction = new KAction(KIcon("edit-clear"), i18n("Remove all items from LightTable"), this);
    d->clearListAction->setShortcut(KShortcut(Qt::CTRL+Qt::SHIFT+Qt::Key_K));
    d->clearListAction->setEnabled(false);
    connect(d->clearListAction, SIGNAL(triggered()), this, SLOT(slotClearItemsList()));
    actionCollection()->addAction("lighttable_clearlist", d->clearListAction);

    d->fileDeleteAction = new KAction(KIcon("user-trash"), i18nc("Non-pluralized", "Move to Trash"), this);
    d->fileDeleteAction->setShortcut(KShortcut(Qt::Key_Delete));
    d->fileDeleteAction->setEnabled(false);
    connect(d->fileDeleteAction, SIGNAL(triggered()), this, SLOT(slotDeleteItem()));
    actionCollection()->addAction("lighttable_filedelete", d->fileDeleteAction);

    d->fileDeleteFinalAction = new KAction(KIcon("edit-delete"), i18n("Delete immediately"), this);
    d->fileDeleteFinalAction->setShortcut(KShortcut(Qt::SHIFT + Qt::Key_Delete));
    d->fileDeleteFinalAction->setEnabled(false);
    connect(d->fileDeleteFinalAction, SIGNAL(triggered()), this, SLOT(slotDeleteFinalItem()));
    actionCollection()->addAction("lighttable_filefinaldelete", d->fileDeleteFinalAction);

    KAction* closeAction = KStandardAction::close(this, SLOT(close()), this);
    actionCollection()->addAction("lighttable_close", closeAction);

    // -- Standard 'View' menu actions ---------------------------------------------

    d->syncPreviewAction = new KToggleAction(KIcon("view-split-left-right"), i18n("Synchronize"), this);
    d->syncPreviewAction->setShortcut(KShortcut(Qt::CTRL+Qt::SHIFT+Qt::Key_Y));
    d->syncPreviewAction->setEnabled(false);
    d->syncPreviewAction->setWhatsThis(i18n("Synchronize preview from left and right panels"));
    connect(d->syncPreviewAction, SIGNAL(triggered()), this, SLOT(slotToggleSyncPreview()));
    actionCollection()->addAction("lighttable_syncpreview", d->syncPreviewAction);

    d->navigateByPairAction = new KToggleAction(KIcon("system-run"), i18n("By Pair"), this);
    d->navigateByPairAction->setShortcut(KShortcut(Qt::CTRL+Qt::SHIFT+Qt::Key_P));
    d->navigateByPairAction->setEnabled(false);
    d->navigateByPairAction->setWhatsThis(i18n("Navigate by pairs with all items"));
    connect(d->navigateByPairAction, SIGNAL(triggered()), this, SLOT(slotToggleNavigateByPair()));
    actionCollection()->addAction("lighttable_navigatebypair", d->navigateByPairAction);

    d->clearOnCloseAction = new KToggleAction(KIcon("edit-clear"), i18n("Clear On Close"), this);
    d->clearOnCloseAction->setShortcut(KShortcut(Qt::CTRL+Qt::SHIFT+Qt::Key_C));
    d->clearOnCloseAction->setEnabled(true);
    d->clearOnCloseAction->setToolTip(i18n("Clear light table when it is closed"));
    d->clearOnCloseAction->setWhatsThis(i18n("Remove all images from the light table when it is closed"));
    actionCollection()->addAction("lighttable_clearonclose", d->clearOnCloseAction);

    d->showThumbBarAction = d->barViewDock->getToggleAction(this);
    actionCollection()->addAction("lighttable_showthumbbar", d->showThumbBarAction);

    d->fullScreenAction = actionCollection()->addAction(KStandardAction::FullScreen,
                          "lighttable_fullscreen", this, SLOT(slotToggleFullScreen()));

    d->slideShowAction = new KAction(KIcon("view-presentation"), i18n("Slideshow"), this);
    d->slideShowAction->setShortcut(KShortcut(Qt::Key_F9));
    connect(d->slideShowAction, SIGNAL(triggered()), this, SLOT(slotToggleSlideShow()));
    actionCollection()->addAction("lighttable_slideshow", d->slideShowAction);

    // Left Panel Zoom Actions

    d->leftZoomPlusAction  = KStandardAction::zoomIn(d->previewView, SLOT(slotIncreaseLeftZoom()), this);
    d->leftZoomPlusAction->setEnabled(false);
    KShortcut leftKeysPlus = d->leftZoomPlusAction->shortcut();
    leftKeysPlus.setAlternate(Qt::Key_Plus);
    d->leftZoomPlusAction->setShortcut(leftKeysPlus);
    actionCollection()->addAction("lighttable_zoomplus_left", d->leftZoomPlusAction);

    d->leftZoomMinusAction  = KStandardAction::zoomOut(d->previewView, SLOT(slotDecreaseLeftZoom()), this);
    d->leftZoomMinusAction->setEnabled(false);
    KShortcut leftKeysMinus = d->leftZoomMinusAction->shortcut();
    leftKeysMinus.setAlternate(Qt::Key_Minus);
    d->leftZoomMinusAction->setShortcut(leftKeysMinus);
    actionCollection()->addAction("lighttable_zoomminus_left", d->leftZoomMinusAction);

    d->leftZoomTo100percents = new KAction(KIcon("zoom-original"), i18n("Zoom to 100%"), this);
    d->leftZoomTo100percents->setShortcut(KShortcut(Qt::ALT + Qt::CTRL + Qt::Key_0));    // NOTE: Photoshop 7 use ALT+CTRL+0
    connect(d->leftZoomTo100percents, SIGNAL(triggered()), d->previewView, SLOT(slotLeftZoomTo100()));
    actionCollection()->addAction("lighttable_zoomto100percents_left", d->leftZoomTo100percents);

    d->leftZoomFitToWindowAction = new KAction(KIcon("zoom-fit-best"), i18n("Fit to &Window"), this);
    d->leftZoomFitToWindowAction->setShortcut(KShortcut(Qt::ALT + Qt::CTRL + Qt::Key_E));
    connect(d->leftZoomFitToWindowAction, SIGNAL(triggered()), d->previewView, SLOT(slotLeftFitToWindow()));
    actionCollection()->addAction("lighttable_zoomfit2window_left", d->leftZoomFitToWindowAction);

    // Right Panel Zoom Actions

    d->rightZoomPlusAction  = KStandardAction::zoomIn(d->previewView, SLOT(slotIncreaseRightZoom()), this);
    d->rightZoomPlusAction->setEnabled(false);
    KShortcut rightKeysPlus = d->rightZoomPlusAction->shortcut();
    rightKeysPlus.setPrimary(Qt::SHIFT + Qt::CTRL + Qt::Key_Plus);
    rightKeysPlus.setAlternate(Qt::SHIFT + Qt::Key_Plus);
    d->rightZoomPlusAction->setShortcut(rightKeysPlus);
    actionCollection()->addAction("lighttable_zoomplus_right", d->rightZoomPlusAction);

    d->rightZoomMinusAction  = KStandardAction::zoomOut(d->previewView, SLOT(slotDecreaseRightZoom()), this);
    d->rightZoomMinusAction->setEnabled(false);
    KShortcut rightKeysMinus = d->rightZoomMinusAction->shortcut();
    rightKeysMinus.setPrimary(Qt::SHIFT + Qt::CTRL + Qt::Key_Minus);
    rightKeysMinus.setAlternate(Qt::SHIFT + Qt::Key_Minus);
    d->rightZoomMinusAction->setShortcut(rightKeysMinus);
    actionCollection()->addAction("lighttable_zoomminus_right", d->rightZoomMinusAction);

    d->rightZoomTo100percents = new KAction(KIcon("zoom-original"), i18n("Zoom to 100%"), this);
    d->rightZoomTo100percents->setShortcut(KShortcut(Qt::SHIFT + Qt::CTRL + Qt::Key_0));
    connect(d->rightZoomTo100percents, SIGNAL(triggered()), d->previewView, SLOT(slotRightZoomTo100()));
    actionCollection()->addAction("lighttable_zoomto100percents_right", d->rightZoomTo100percents);

    d->rightZoomFitToWindowAction = new KAction(KIcon("zoom-fit-best"), i18n("Fit to &Window"), this);
    d->rightZoomFitToWindowAction->setShortcut(KShortcut(Qt::SHIFT + Qt::CTRL + Qt::Key_E));
    connect(d->rightZoomFitToWindowAction, SIGNAL(triggered()), d->previewView, SLOT(slotRightFitToWindow()));
    actionCollection()->addAction("lighttable_zoomfit2window_right", d->rightZoomFitToWindowAction);

    // -- Standard 'Configure' menu actions ----------------------------------------

    d->showMenuBarAction = KStandardAction::showMenubar(this, SLOT(slotShowMenuBar()), actionCollection());

    KStandardAction::keyBindings(this,            SLOT(slotEditKeys()),          actionCollection());
    KStandardAction::configureToolbars(this,      SLOT(slotConfToolbars()),      actionCollection());
    KStandardAction::configureNotifications(this, SLOT(slotConfNotifications()), actionCollection());
    KStandardAction::preferences(this,            SLOT(slotSetup()),             actionCollection());

    // ---------------------------------------------------------------------------------

    d->themeMenuAction = new KSelectAction(i18n("&Themes"), this);
    connect(d->themeMenuAction, SIGNAL(triggered(const QString&)),
            this, SLOT(slotChangeTheme(const QString&)));
    actionCollection()->addAction("theme_menu", d->themeMenuAction);

    d->themeMenuAction->setItems(ThemeEngine::instance()->themeNames());
    slotThemeChanged();

    // -- Standard 'Help' menu actions ---------------------------------------------

    d->about = new DAboutData(this);
    d->about->registerHelpActions();

    d->libsInfoAction = new KAction(KIcon("help-about"), i18n("Components Information"), this);
    connect(d->libsInfoAction, SIGNAL(triggered()), this, SLOT(slotComponentsInfo()));
    actionCollection()->addAction("lighttable_librariesinfo", d->libsInfoAction);

    d->dbStatAction = new KAction(KIcon("network-server-database"), i18n("Database Statistics"), this);
    connect(d->dbStatAction, SIGNAL(triggered()), this, SLOT(slotDBStat()));
    actionCollection()->addAction("lighttable_dbstat", d->dbStatAction);

    // Provides a menu entry that allows showing/hiding the toolbar(s)
    setStandardToolBarMenuEnabled(true);

    // Provides a menu entry that allows showing/hiding the statusbar
    createStandardStatusBarAction();

    // -- Keyboard-only actions added to <MainWindow> ------------------------------

    KAction* altBackwardAction = new KAction(i18n("Previous Image"), this);
    actionCollection()->addAction("lighttable_backward_shift_space", altBackwardAction);
    altBackwardAction->setShortcut( KShortcut(Qt::SHIFT+Qt::Key_Space) );
    connect(altBackwardAction, SIGNAL(triggered()), this, SLOT(slotBackward()));

    // ---------------------------------------------------------------------------------

    actionCollection()->addAction("logo_action", new DLogoAction(this));

    createGUI(xmlFile());

    d->showMenuBarAction->setChecked(!menuBar()->isHidden());  // NOTE: workaround for B.K.O #171080
}

// Deal with items dropped onto the thumbbar (e.g. from the Album view)
void LightTableWindow::slotThumbbarDroppedItems(const ImageInfoList& list)
{
    // Setting the third parameter of loadImageInfos to true
    // means that the images are added to the presently available images.
    loadImageInfos(list, ImageInfo(), true);
}

// We get here either
// - via CTRL+L (from the albumview)
//     a) digikamapp.cpp:  CTRL+key_L leads to slotImageLightTable())
//     b) digikamview.cpp: void DigikamView::slotImageLightTable()
//          calls d->iconView->insertToLightTable(list, info);
//     c) albumiconview.cpp: AlbumIconView::insertToLightTable
//          calls ltview->loadImageInfos(list, current);
// - via drag&drop, i.e. calls issued by the ...Dropped... routines
void LightTableWindow::loadImageInfos(const ImageInfoList& list,
                                      const ImageInfo& givenImageInfoCurrent,
                                      bool addTo)
{
    // Clear all items before adding new images to the light table.
    kDebug() << "Clearing LT" << (!addTo);
    if (!addTo)
    {
        slotClearItemsList();
    }

    ImageInfoList l = list;
    ImageInfo imageInfoCurrent = givenImageInfoCurrent;

    if (imageInfoCurrent.isNull() && !l.isEmpty())
    {
        imageInfoCurrent = l.first();
    }

    d->barView->blockSignals(true);

    for (ImageInfoList::const_iterator it = l.constBegin(); it != l.constEnd(); ++it)
    {
        if (!d->barView->findItemByInfo(*it))
        {
            new LightTableBarItem(d->barView, *it);
        }
    }

    d->barView->blockSignals(false);

    refreshStatusBar();
}

bool LightTableWindow::isEmpty() const
{
    return d->barView->countItems() == 0;
}

void LightTableWindow::refreshStatusBar()
{
    d->statusProgressBar->progressBarMode(StatusProgressBar::TextMode,
                                          i18np("%1 item on Light Table", "%1 items on Light Table",
                                                  d->barView->countItems()));
}

void LightTableWindow::slotFileChanged(const QString& path)
{
    KUrl url = KUrl::fromPath(path);
    d->barView->reloadThumbs(url);

    if (!d->previewView->leftImageInfo().isNull())
    {
        if (d->previewView->leftImageInfo().fileUrl() == url)
        {
            d->previewView->leftReload();
            d->leftSideBar->itemChanged(d->previewView->leftImageInfo());
        }
    }

    if (!d->previewView->rightImageInfo().isNull())
    {
        if (d->previewView->rightImageInfo().fileUrl() == url)
        {
            d->previewView->rightReload();
            d->rightSideBar->itemChanged(d->previewView->rightImageInfo());
        }
    }
}

void LightTableWindow::slotLeftPanelLeftButtonClicked()
{
    if (d->navigateByPairAction->isChecked())
    {
        return;
    }

    d->barView->setSelectedItem(d->barView->findItemByInfo(d->previewView->leftImageInfo()));
}

void LightTableWindow::slotRightPanelLeftButtonClicked()
{
    // With navigate by pair option, only the left panel can be selected.
    if (d->navigateByPairAction->isChecked())
    {
        return;
    }

    d->barView->setSelectedItem(d->barView->findItemByInfo(d->previewView->rightImageInfo()));
}

void LightTableWindow::slotLeftPreviewLoaded(bool b)
{
    d->leftZoomBar->setEnabled(b);
    d->leftFileName->clear();

    if (b)
    {
        d->leftFileName->setText(d->previewView->leftImageInfo().name());
        d->previewView->checkForSelection(d->barView->currentItemImageInfo());
        d->barView->setOnLeftPanel(d->previewView->leftImageInfo());

        LightTableBarItem* item = dynamic_cast<LightTableBarItem*>(d->barView->findItemByInfo(d->previewView->leftImageInfo()));

        if (item)
        {
            item->setOnLeftPanel(true);
        }

        if (d->navigateByPairAction->isChecked() && item)
        {
            LightTableBarItem* next = dynamic_cast<LightTableBarItem*>(item->next());

            if (next)
            {
                d->barView->setOnRightPanel(next->info());
                slotSetItemOnRightPanel(next->info());
            }
            else
            {
                LightTableBarItem* first = dynamic_cast<LightTableBarItem*>(d->barView->firstItem());
                slotSetItemOnRightPanel(first ? first->info() : ImageInfo());
            }
        }
    }
}

void LightTableWindow::slotRightPreviewLoaded(bool b)
{
    d->rightZoomBar->setEnabled(b);
    d->rightFileName->clear();

    if (b)
    {
        d->rightFileName->setText(d->previewView->rightImageInfo().name());
        d->previewView->checkForSelection(d->barView->currentItemImageInfo());
        d->barView->setOnRightPanel(d->previewView->rightImageInfo());

        LightTableBarItem* item = dynamic_cast<LightTableBarItem*>(d->barView->findItemByInfo(d->previewView->rightImageInfo()));

        if (item)
        {
            item->setOnRightPanel(true);
        }
    }
}

void LightTableWindow::slotItemSelected(const ImageInfo& info)
{
    bool hasInfo = !info.isNull();

    d->setItemLeftAction->setEnabled(hasInfo);
    d->setItemRightAction->setEnabled(hasInfo);
    d->editItemAction->setEnabled(hasInfo);
    d->removeItemAction->setEnabled(hasInfo);
    d->clearListAction->setEnabled(hasInfo);
    d->fileDeleteAction->setEnabled(hasInfo);
    d->fileDeleteFinalAction->setEnabled(hasInfo);
    d->backwardAction->setEnabled(hasInfo);
    d->forwardAction->setEnabled(hasInfo);
    d->firstAction->setEnabled(hasInfo);
    d->lastAction->setEnabled(hasInfo);
    d->syncPreviewAction->setEnabled(hasInfo);
    d->navigateByPairAction->setEnabled(hasInfo);
    d->slideShowAction->setEnabled(hasInfo);

    if (hasInfo)
    {
        LightTableBarItem* curr = dynamic_cast<LightTableBarItem*>(d->barView->findItemByInfo(info));

        if (curr)
        {
            if (!curr->prev())
            {
                d->firstAction->setEnabled(false);
            }

            if (!curr->next())
            {
                d->lastAction->setEnabled(false);
            }

            if (d->navigateByPairAction->isChecked())
            {
                d->setItemLeftAction->setEnabled(false);
                d->setItemRightAction->setEnabled(false);

                d->barView->setOnLeftPanel(info);
                slotSetItemOnLeftPanel(info);
            }
            else if (d->autoLoadOnRightPanel && !curr->isOnLeftPanel())
            {
                d->barView->setOnRightPanel(info);
                slotSetItemOnRightPanel(info);
            }
        }
    }

    d->previewView->checkForSelection(info);
}

// Deal with one (or more) items dropped onto the left panel
void LightTableWindow::slotLeftDroppedItems(const ImageInfoList& list)
{
    ImageInfo info = list.first();
    // add the image to the existing images
    loadImageInfos(list, info, true);

    // We will check if first item from list is already stored in thumbbar
    // Note that the thumbbar stores all ImageInfo reference
    // in memory for preview object.
    LightTableBarItem* item = dynamic_cast<LightTableBarItem*>(d->barView->findItemByInfo(info));

    if (item)
    {
        slotSetItemOnLeftPanel(item->info());
    }
}

// Deal with one (or more) items dropped onto the right panel
void LightTableWindow::slotRightDroppedItems(const ImageInfoList& list)
{
    ImageInfo info = list.first();
    // add the image to the existing images
    loadImageInfos(list, info, true);

    // We will check if first item from list is already stored in thumbbar
    // Note that the thumbbar stores all ImageInfo reference
    // in memory for preview object.
    LightTableBarItem* item = dynamic_cast<LightTableBarItem*>(d->barView->findItemByInfo(info));

    if (item)
    {
        slotSetItemOnRightPanel(item->info());
        // Make this item the current one.
        d->barView->setSelectedItem(item);
    }
}

// Set the images for the left and right panel.
void LightTableWindow::setLeftRightItems(const ImageInfoList& list, bool addTo)
{
    ImageInfoList l = list;

    if (l.count() == 0)
    {
        return;
    }

    ImageInfo info            = l.first();
    LightTableBarItem* ltItem = dynamic_cast<LightTableBarItem*>(d->barView->findItemByInfo(info));

    if (l.count() == 1 && !addTo)
    {
        // Just one item; this is used for the left panel.
        d->barView->setOnLeftPanel(info);
        slotSetItemOnLeftPanel(info);
        d->barView->setSelectedItem(ltItem);
        d->barView->ensureItemVisible(ltItem);
        return;
    }

    if (ltItem)
    {
        // The first item is used for the left panel.
        if (!addTo)
        {
            d->barView->setOnLeftPanel(info);
            slotSetItemOnLeftPanel(info);
        }

        // The subsequent item is used for the right panel.
        LightTableBarItem* next = dynamic_cast<LightTableBarItem*>(ltItem->next());

        if (next && !addTo)
        {
            d->barView->setOnRightPanel(next->info());
            slotSetItemOnRightPanel(next->info());

            if (!d->navigateByPairAction->isChecked())
            {
                d->barView->setSelectedItem(next);
                // ensure that the selected item is visible
                // FIXME: this does not work:
                d->barView->ensureItemVisible(next);
            }
        }

        // If navigate by pairs is active, the left panel item is selected.
        // (Fixes parts of bug #150296)
        if (d->navigateByPairAction->isChecked())
        {
            d->barView->setSelectedItem(ltItem);
            d->barView->ensureItemVisible(ltItem);
        }
    }
}

void LightTableWindow::slotSetItemLeft()
{
    if (!d->barView->currentItemImageInfo().isNull())
    {
        slotSetItemOnLeftPanel(d->barView->currentItemImageInfo());
    }
}

void LightTableWindow::slotSetItemRight()
{
    if (!d->barView->currentItemImageInfo().isNull())
    {
        slotSetItemOnRightPanel(d->barView->currentItemImageInfo());
    }
}

void LightTableWindow::slotSetItemOnLeftPanel(const ImageInfo& info)
{
    d->previewView->setLeftImageInfo(info);

    if (!info.isNull())
    {
        d->leftSideBar->itemChanged(info);
    }
    else
    {
        d->leftSideBar->slotNoCurrentItem();
    }
}

void LightTableWindow::slotSetItemOnRightPanel(const ImageInfo& info)
{
    d->previewView->setRightImageInfo(info);

    if (!info.isNull())
    {
        d->rightSideBar->itemChanged(info);
    }
    else
    {
        d->rightSideBar->slotNoCurrentItem();
    }
}

void LightTableWindow::slotClearItemsList()
{
    if (!d->previewView->leftImageInfo().isNull())
    {
        d->previewView->setLeftImageInfo();
        d->leftSideBar->slotNoCurrentItem();
    }

    if (!d->previewView->rightImageInfo().isNull())
    {
        d->previewView->setRightImageInfo();
        d->rightSideBar->slotNoCurrentItem();
    }

    d->barView->clear();
    refreshStatusBar();
}

void LightTableWindow::slotDeleteItem()
{
    deleteItem(false);
}

void LightTableWindow::slotDeleteItem(const ImageInfo& info)
{
    deleteItem(info, false);
}

void LightTableWindow::slotDeleteFinalItem()
{
    deleteItem(true);
}

void LightTableWindow::slotDeleteFinalItem(const ImageInfo& info)
{
    deleteItem(info, true);
}

void LightTableWindow::deleteItem(bool permanently)
{
    if (!d->barView->currentItemImageInfo().isNull())
    {
        deleteItem(d->barView->currentItemImageInfo(), permanently);
    }
}

void LightTableWindow::deleteItem(const ImageInfo& info, bool permanently)
{
    KUrl u = info.fileUrl();
    PAlbum* palbum = AlbumManager::instance()->findPAlbum(u.directory());

    if (!palbum)
    {
        return;
    }

    // Provide a digikamalbums:// URL to KIO
    KUrl kioURL  = info.databaseUrl();
    KUrl fileURL = u;

    bool useTrash;
    bool preselectDeletePermanently = permanently;

    DeleteDialog dialog(this);

    KUrl::List urlList;
    urlList.append(u);

    if (!dialog.confirmDeleteList(urlList, DeleteDialogMode::Files, preselectDeletePermanently ?
                                  DeleteDialogMode::NoChoiceDeletePermanently : DeleteDialogMode::NoChoiceTrash))
    {
        return;
    }

    useTrash = !dialog.shouldDelete();

    // trash does not like non-local URLs, put is not implemented
    if (useTrash)
    {
        kioURL = fileURL;
    }

    SyncJobResult deleteResult = SyncJob::del(kioURL, useTrash);

    if (!deleteResult)
    {
        KMessageBox::error(this, deleteResult.errorString);
        return;
    }

    emit signalFileDeleted(u);

    slotRemoveItem(info);
}

void LightTableWindow::slotRemoveItem()
{
    if (!d->barView->currentItemImageInfo().isNull())
    {
        slotRemoveItem(d->barView->currentItemImageInfo());
    }
}

void LightTableWindow::slotRemoveItem(const ImageInfo& info)
{
    /*
        if (!d->previewView->leftImageInfo().isNull())
        {
            if (d->previewView->leftImageInfo() == info)
            {
                d->previewView->setLeftImageInfo();
                d->leftSideBar->slotNoCurrentItem();
            }
        }

        if (!d->previewView->rightImageInfo().isNull())
        {
            if (d->previewView->rightImageInfo() == info)
            {
                d->previewView->setRightImageInfo();
                d->rightSideBar->slotNoCurrentItem();
            }
        }

        d->barView->removeItemByInfo(info);
        d->barView->setSelected(d->barView->currentItem());
    */

    // When either the image from the left or right panel is removed,
    // there are various situations to account for.
    // To describe them, 4 images A B C D are used
    // and the subscript _L and _ R  mark the currently
    // active item on the left and right panel

    bool leftPanelActive = false;
    ImageInfo curr_linfo = d->previewView->leftImageInfo();
    ImageInfo curr_rinfo = d->previewView->rightImageInfo();
    ImageInfo new_linfo;
    ImageInfo new_rinfo;

    qint64 infoId = info.id();

    // First determine the next images to the current left and right image:
    ImageInfo next_linfo;
    ImageInfo next_rinfo;

    if (!curr_linfo.isNull())
    {
        LightTableBarItem* ltItem = dynamic_cast<LightTableBarItem*>(d->barView->findItemByInfo(curr_linfo));

        if (ltItem)
        {
            LightTableBarItem* next = dynamic_cast<LightTableBarItem*>(ltItem->next());

            if (next)
            {
                next_linfo = next->info();
            }
        }
    }

    if (!curr_rinfo.isNull())
    {
        LightTableBarItem* ltItem = dynamic_cast<LightTableBarItem*>(d->barView->findItemByInfo(curr_rinfo));

        if (ltItem)
        {
            LightTableBarItem* next = dynamic_cast<LightTableBarItem*>(ltItem->next());

            if (next)
            {
                next_rinfo = next->info();
            }
        }
    }

    d->barView->removeItemByInfo(info);

    // Make sure that next_linfo and next_rinfo are still available:
    if (!d->barView->findItemByInfo(next_linfo))
    {
        next_linfo = ImageInfo();
    }

    if (!d->barView->findItemByInfo(next_rinfo))
    {
        next_rinfo = ImageInfo();
    }

    // removal of the left panel item?
    if (!curr_linfo.isNull())
    {
        if ( curr_linfo.id() == infoId )
        {
            leftPanelActive = true;
            // Delete the item A_L of the left panel:
            // 1)  A_L  B_R  C    D   ->   B_L  C_R  D
            // 2)  A_L  B    C_R  D   ->   B    C_L  D_R
            // 3)  A_L  B    C    D_R ->   B_R  C    D_L
            // 4)  A_L  B_R           ->   A_L
            // some more corner cases:
            // 5)  A    B_L  C_R  D   ->   A    C_L  D_R
            // 6)  A    B_L  C_R      ->   A_R  C_L
            // 7)  A_LR B    C    D   ->   B_L    C_R  D  (does not yet work)
            // I.e. in 3) we wrap around circularly.

            // When removing the left panel image,
            // put the right panel image into the left panel.
            // Check if this one is not the same (i.e. also removed).
            if (!curr_rinfo.isNull())
            {
                if (curr_rinfo.id() != infoId)
                {
                    new_linfo = curr_rinfo;
                    // Set the right panel to the next image:
                    new_rinfo = next_rinfo;

                    // set the right panel active, but not in pair mode
                    if (!d->navigateByPairAction->isChecked())
                    {
                        leftPanelActive = false;
                    }
                }
            }
        }
    }

    // removal of the right panel item?
    if (!curr_rinfo.isNull())
    {
        if (curr_rinfo.id() == infoId)
        {
            // Leave the left panel as the current one
            new_linfo = curr_linfo;
            // Set the right panel to the next image
            new_rinfo = next_rinfo;
        }
    }

    // Now we deal with the corner cases, where no left or right item exists.
    // If the right panel would be set, but not the left-one, then swap
    if (new_linfo.isNull() && !new_rinfo.isNull())
    {
        new_linfo       = new_rinfo;
        new_rinfo       = ImageInfo();
        leftPanelActive = true;
    }

    if (new_linfo.isNull())
    {
        if (d->barView->countItems() > 0)
        {
            LightTableBarItem* first = dynamic_cast<LightTableBarItem*>(d->barView->firstItem());
            new_linfo = first->info();
        }
    }

    // Make sure that new_linfo and new_rinfo exist.
    // This addresses a crash occurring if the last image is removed
    // in the navigate by pairs mode.
    if (!d->barView->findItemByInfo(new_linfo))
    {
        new_linfo = ImageInfo();
    }

    if (!d->barView->findItemByInfo(new_rinfo))
    {
        new_rinfo = ImageInfo();
    }

    // no right item defined?
    if (new_rinfo.isNull())
    {
        // If there are at least two items, we can find reasonable right image.
        if (d->barView->countItems() > 1)
        {
            // See if there is an item next to the left one:
            LightTableBarItem* ltItem = dynamic_cast<LightTableBarItem*>(d->barView->findItemByInfo(new_linfo));
            LightTableBarItem* next   = 0;

            if (ltItem)
            {
                next = dynamic_cast<LightTableBarItem*>(ltItem->next());
            }

            if (next)
            {
                new_rinfo = next->info();
            }
            else
            {
                // If there is no item to the right of new_linfo
                // then we can choose the first item for new_rinfo
                // (as we made sure that there are at least two items)
                LightTableBarItem* first = dynamic_cast<LightTableBarItem*>(d->barView->firstItem());
                new_rinfo               = first->info();
            }
        }
    }

    // Check if left and right are set to the same
    if (!new_linfo.isNull() && !new_rinfo.isNull())
    {
        if (new_linfo.id() == new_rinfo.id())
        {
            // Only keep the left one
            new_rinfo = ImageInfo();
        }
    }

    // If the right panel would be set, but not the left-one, then swap
    // (note that this has to be done here again!)
    if (new_linfo.isNull() && !new_rinfo.isNull())
    {
        new_linfo       = new_rinfo;
        new_rinfo       = ImageInfo();
        leftPanelActive = true;
    }

    // set the image for the left panel
    if (!new_linfo.isNull())
    {
        d->barView->setOnLeftPanel(new_linfo);
        slotSetItemOnLeftPanel(new_linfo);

        //  make this the selected item if the left was active before
        if ( leftPanelActive)
        {
            LightTableBarItem* ltItem = dynamic_cast<LightTableBarItem*>(d->barView->findItemByInfo(new_linfo));
            d->barView->setSelectedItem(ltItem);
        }
    }
    else
    {
        d->previewView->setLeftImageInfo();
        d->leftSideBar->slotNoCurrentItem();
    }

    // set the image for the right panel
    if (!new_rinfo.isNull())
    {
        d->barView->setOnRightPanel(new_rinfo);
        slotSetItemOnRightPanel(new_rinfo);

        //  make this the selected item if the left was active before
        if (!leftPanelActive)
        {
            LightTableBarItem* ltItem = dynamic_cast<LightTableBarItem*>(d->barView->findItemByInfo(new_rinfo));
            d->barView->setSelectedItem(ltItem);
        }
    }
    else
    {
        d->previewView->setRightImageInfo();
        d->rightSideBar->slotNoCurrentItem();
    }

    refreshStatusBar();
}

void LightTableWindow::slotEditItem()
{
    if (!d->barView->currentItemImageInfo().isNull())
    {
        slotEditItem(d->barView->currentItemImageInfo());
    }
}

void LightTableWindow::slotEditItem(const ImageInfo& info)
{
    ImageWindow* im    = ImageWindow::imageWindow();
    ImageInfoList list = d->barView->itemsImageInfoList();

    im->loadImageInfos(list, info, i18n("Light Table"));

    if (im->isHidden())
    {
        im->show();
    }
    else
    {
        im->raise();
    }

    im->setFocus();
}

void LightTableWindow::slotToggleSlideShow()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group("ImageViewer Settings");
    bool startWithCurrent     = group.readEntry("SlideShowStartCurrent", false);

    SlideShowSettings settings;
    settings.exifRotate           = MetadataSettings::instance()->settings().exifRotate;
    settings.delay                = group.readEntry("SlideShowDelay", 5) * 1000;
    settings.printName            = group.readEntry("SlideShowPrintName", true);
    settings.printDate            = group.readEntry("SlideShowPrintDate", false);
    settings.printApertureFocal   = group.readEntry("SlideShowPrintApertureFocal", false);
    settings.printExpoSensitivity = group.readEntry("SlideShowPrintExpoSensitivity", false);
    settings.printMakeModel       = group.readEntry("SlideShowPrintMakeModel", false);
    settings.printComment         = group.readEntry("SlideShowPrintComment", false);
    settings.printLabels          = group.readEntry("SlideShowPrintLabels", false);
    settings.loop                 = group.readEntry("SlideShowLoop", false);
    slideShow(startWithCurrent, settings);
}

void LightTableWindow::slideShow(bool startWithCurrent, SlideShowSettings& settings)
{
    if (!d->barView->countItems())
    {
        return;
    }

    int              i = 0;
    d->cancelSlideShow = false;

    d->statusProgressBar->progressBarMode(StatusProgressBar::CancelProgressBarMode,
                                          i18n("Preparing slideshow. Please wait..."));

    ImageInfoList list = d->barView->itemsImageInfoList();

    for (ImageInfoList::const_iterator it = list.constBegin();
         !d->cancelSlideShow && it != list.constEnd() ; ++it)
    {
        SlidePictureInfo pictInfo;
        pictInfo.comment    = (*it).comment();
        pictInfo.rating     = (*it).rating();
        pictInfo.colorLabel = (*it).colorLabel();
        pictInfo.pickLabel  = (*it).pickLabel();
        pictInfo.photoInfo  = (*it).photoInfoContainer();
        settings.pictInfoMap.insert((*it).fileUrl(), pictInfo);
        settings.fileList.append((*it).fileUrl());

        d->statusProgressBar->setProgressValue((int)((i++/(float)list.count())*100.0));
        kapp->processEvents();
    }

    d->statusProgressBar->progressBarMode(StatusProgressBar::TextMode, QString());
    refreshStatusBar();

    if (!d->cancelSlideShow)
    {
        settings.exifRotate = MetadataSettings::instance()->settings().exifRotate;

        SlideShow* slide = new SlideShow(settings);

        if (startWithCurrent)
        {
            slide->setCurrent(d->barView->currentItemImageInfo().fileUrl());
        }

        connect(slide, SIGNAL(signalRatingChanged(const KUrl&, int)),
                d->barView, SLOT(slotRatingChanged(const KUrl&, int)));

        connect(slide, SIGNAL(signalColorLabelChanged(const KUrl&, int)),
                d->barView, SLOT(slotColorLabelChanged(const KUrl&, int)));

        connect(slide, SIGNAL(signalPickLabelChanged(const KUrl&, int)),
                d->barView, SLOT(slotPickLabelChanged(const KUrl&, int)));

        slide->show();
    }
}

void LightTableWindow::slotProgressBarCancelButtonPressed()
{
    d->cancelSlideShow = true;
}

void LightTableWindow::slotToggleFullScreen()
{
    if (d->fullScreen) // out of fullscreen
    {
        setWindowState( windowState() & ~Qt::WindowFullScreen ); // reset

        slotShowMenuBar();
        statusBar()->show();
        showToolBars();

        if (d->removeFullScreenButton)
        {
            QList<KToolBar*> toolbars = toolBars();
            foreach (KToolBar* toolbar, toolbars)
            {
                // name is set in ui.rc XML file
                if (toolbar->objectName() == "ToolBar")
                {
                    toolbar->removeAction(d->fullScreenAction);
                    break;
                }
            }
        }

        d->leftSideBar->restore();
        d->rightSideBar->restore();

        d->fullScreen = false;
    }
    else  // go to fullscreen
    {
        // hide the menubar and the statusbar
        menuBar()->hide();
        statusBar()->hide();

        if (d->fullScreenHideToolBar)
        {
            hideToolBars();
        }
        else
        {
            showToolBars();

            QList<KToolBar*> toolbars = toolBars();
            KToolBar* mainToolbar     = 0;
            foreach (KToolBar* toolbar, toolbars)
            {
                if (toolbar->objectName() == "ToolBar")
                {
                    mainToolbar = toolbar;
                    break;
                }
            }

            // add fullscreen action if necessary
            if ( mainToolbar && !mainToolbar->actions().contains(d->fullScreenAction) )
            {
                mainToolbar->addAction(d->fullScreenAction);
                d->removeFullScreenButton=true;
            }
            else
            {
                // If FullScreen button is enabled in toolbar settings,
                // we shall not remove it when leaving of fullscreen mode.
                d->removeFullScreenButton=false;
            }
        }

        d->leftSideBar->backup();
        d->rightSideBar->backup();

        setWindowState( windowState() | Qt::WindowFullScreen ); // set
        d->fullScreen = true;
    }
}

void LightTableWindow::slotEscapePressed()
{
    if (d->fullScreen)
    {
        d->fullScreenAction->activate(QAction::Trigger);
    }
}

void LightTableWindow::showToolBars()
{
    QList<KToolBar*> toolbars = toolBars();
    foreach(KToolBar* toolbar, toolbars)
    {
        toolbar->show();
    }
}

void LightTableWindow::hideToolBars()
{
    QList<KToolBar*> toolbars = toolBars();
    foreach(KToolBar* toolbar, toolbars)
    {
        toolbar->hide();
    }
}

void LightTableWindow::slotEditKeys()
{
    KShortcutsDialog dialog(KShortcutsEditor::AllActions,
                            KShortcutsEditor::LetterShortcutsAllowed, this);
    dialog.addCollection( actionCollection(), i18n( "General" ) );
    dialog.configure();
}

void LightTableWindow::slotConfToolbars()
{
    saveMainWindowSettings(KGlobal::config()->group("LightTable Settings"));
    KEditToolBar dlg(factory(), this);

    connect(&dlg, SIGNAL(newToolbarConfig()),
            this, SLOT(slotNewToolbarConfig()));

    dlg.exec();
}

void LightTableWindow::slotConfNotifications()
{
    KNotifyConfigWidget::configure(this);
}

void LightTableWindow::slotNewToolbarConfig()
{
    applyMainWindowSettings(KGlobal::config()->group("LightTable Settings"));
}

void LightTableWindow::slotSetup()
{
    Setup::exec(this);
}

void LightTableWindow::slotLeftZoomFactorChanged(double zoom)
{
    double zmin = d->previewView->leftZoomMin();
    double zmax = d->previewView->leftZoomMax();
    d->leftZoomBar->setZoom(zoom, zmin, zmax);

    d->leftZoomPlusAction->setEnabled(!d->previewView->leftMaxZoom());
    d->leftZoomMinusAction->setEnabled(!d->previewView->leftMinZoom());
}

void LightTableWindow::slotRightZoomFactorChanged(double zoom)
{
    double zmin = d->previewView->rightZoomMin();
    double zmax = d->previewView->rightZoomMax();
    d->rightZoomBar->setZoom(zoom, zmin, zmax);

    d->rightZoomPlusAction->setEnabled(!d->previewView->rightMaxZoom());
    d->rightZoomMinusAction->setEnabled(!d->previewView->rightMinZoom());
}

void LightTableWindow::slotToggleSyncPreview()
{
    d->previewView->setSyncPreview(d->syncPreviewAction->isChecked());
}

void LightTableWindow::slotToggleOnSyncPreview(bool t)
{
    d->syncPreviewAction->setEnabled(t);

    if (!t)
    {
        d->syncPreviewAction->setChecked(false);
    }
    else
    {
        if (d->autoSyncPreview)
        {
            d->syncPreviewAction->setChecked(true);
        }
    }
}

void LightTableWindow::slotBackward()
{
    ThumbBarItem* curr = d->barView->currentItem();
    ThumbBarItem* last = d->barView->lastItem();

    if (curr)
    {
        if (curr->prev())
        {
            d->barView->setSelected(curr->prev());
        }
        else
        {
            d->barView->setSelected(last);
        }
    }
}

void LightTableWindow::slotForward()
{
    ThumbBarItem* curr = d->barView->currentItem();
    ThumbBarItem* first = d->barView->firstItem();

    if (curr)
    {
        if (curr->next())
        {
            d->barView->setSelected(curr->next());
        }
        else
        {
            d->barView->setSelected(first);
        }
    }
}

void LightTableWindow::slotFirst()
{
    d->barView->setSelected( d->barView->firstItem() );
}

void LightTableWindow::slotLast()
{
    d->barView->setSelected( d->barView->lastItem() );
}

void LightTableWindow::slotToggleNavigateByPair()
{
    d->barView->setNavigateByPair(d->navigateByPairAction->isChecked());
    d->previewView->setNavigateByPair(d->navigateByPairAction->isChecked());
    slotItemSelected(d->barView->currentItemImageInfo());
}

void LightTableWindow::slotThemeChanged()
{
    QStringList themes(ThemeEngine::instance()->themeNames());
    int index = themes.indexOf(AlbumSettings::instance()->getCurrentTheme());

    if (index == -1)
    {
        index = themes.indexOf(i18n("Default"));
    }

    d->themeMenuAction->setCurrentItem(index);
}

void LightTableWindow::slotChangeTheme(const QString& theme)
{
    // Theme menu entry is returned with keyboard accelerator. We remove it.
    QString name = theme;
    name.remove(QChar('&'));
    AlbumSettings::instance()->setCurrentTheme(theme);
    ThemeEngine::instance()->slotChangeTheme(theme);
}

void LightTableWindow::slotComponentsInfo()
{
    showDigikamComponentsInfo();
}

void LightTableWindow::slotDBStat()
{
    showDigikamDatabaseStat();
}

void LightTableWindow::slotShowMenuBar()
{
    menuBar()->setVisible(d->showMenuBarAction->isChecked());
}

void LightTableWindow::slotSidebarTabTitleStyleChanged()
{
    d->leftSideBar->setStyle(AlbumSettings::instance()->getSidebarTitleStyle());
    d->rightSideBar->setStyle(AlbumSettings::instance()->getSidebarTitleStyle());

    /// @todo Which part of the settings has to be reloaded?
//     d->rightSideBar->applySettings();
}

void LightTableWindow::moveEvent(QMoveEvent* e)
{
    Q_UNUSED(e)
    emit signalWindowHasMoved();
}

void LightTableWindow::toggleTag(int tagID)
{
    d->barView->toggleTag(tagID);
}

void LightTableWindow::slotAssignPickLabel(int pickId)
{
    d->barView->slotAssignPickLabel(pickId);
}

void LightTableWindow::slotAssignColorLabel(int colorId)
{
    d->barView->slotAssignColorLabel(colorId);
}

void LightTableWindow::slotAssignRating(int rating)
{
    d->barView->slotAssignRating(rating);
}

}  // namespace Digikam

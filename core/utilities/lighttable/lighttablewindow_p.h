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

#ifndef LIGHTTABLEWINDOWPRIVATE_H
#define LIGHTTABLEWINDOWPRIVATE_H

// Qt includes

#include <QSplitter>

// KDE includes

#include <kaction.h>
#include <kselectaction.h>
#include <ksqueezedtextlabel.h>

// Local includes

#include "imagepropertiessidebardb.h"
#include "statusprogressbar.h"
#include "daboutdata.h"
#include "dzoombar.h"
#include "lighttableview.h"
#include "lighttablebar.h"
#include "thumbbardock.h"
#include "albummodel.h"

namespace Digikam
{

class LightTableWindow::LightTableWindowPriv
{

public:

    LightTableWindowPriv() :
        autoLoadOnRightPanel(true),
        autoSyncPreview(true),
        fullScreenHideToolBar(false),
        fullScreen(false),
        removeFullScreenButton(false),
        cancelSlideShow(false),
        setItemLeftAction(0),
        setItemRightAction(0),
        clearListAction(0),
        editItemAction(0),
        removeItemAction(0),
        fileDeleteAction(0),
        fileDeleteFinalAction(0),
        slideShowAction(0),
        fullScreenAction(0),
        leftZoomPlusAction(0),
        leftZoomMinusAction(0),
        leftZoomTo100percents(0),
        leftZoomFitToWindowAction(0),
        rightZoomPlusAction(0),
        rightZoomMinusAction(0),
        rightZoomTo100percents(0),
        rightZoomFitToWindowAction(0),
        forwardAction(0),
        backwardAction(0),
        firstAction(0),
        lastAction(0),
        libsInfoAction(0),
        dbStatAction(0),
        themeMenuAction(0),
        showThumbBarAction(0),
        syncPreviewAction(0),
        navigateByPairAction(0),
        showMenuBarAction(0),
        clearOnCloseAction(0),
        leftFileName(0),
        rightFileName(0),
        hSplitter(0),
        barViewDock(0),
        barView(0),
        previewView(0),
        leftZoomBar(0),
        rightZoomBar(0),
        statusProgressBar(0),
        leftSideBar(0),
        rightSideBar(0),
        about(0)
    {
    }

    bool                      autoLoadOnRightPanel;
    bool                      autoSyncPreview;
    bool                      fullScreenHideToolBar;
    bool                      fullScreen;
    bool                      removeFullScreenButton;
    bool                      cancelSlideShow;

    KAction*                  setItemLeftAction;
    KAction*                  setItemRightAction;
    KAction*                  clearListAction;
    KAction*                  editItemAction;
    KAction*                  removeItemAction;
    KAction*                  fileDeleteAction;
    KAction*                  fileDeleteFinalAction;
    KAction*                  slideShowAction;
    KAction*                  fullScreenAction;
    KAction*                  leftZoomPlusAction;
    KAction*                  leftZoomMinusAction;
    KAction*                  leftZoomTo100percents;
    KAction*                  leftZoomFitToWindowAction;
    KAction*                  rightZoomPlusAction;
    KAction*                  rightZoomMinusAction;
    KAction*                  rightZoomTo100percents;
    KAction*                  rightZoomFitToWindowAction;

    KAction*                  forwardAction;
    KAction*                  backwardAction;
    KAction*                  firstAction;
    KAction*                  lastAction;
    KAction*                  donateMoneyAction;
    KAction*                  contributeAction;
    KAction*                  rawCameraListAction;
    KAction*                  libsInfoAction;
    KAction*                  dbStatAction;

    KSelectAction*            themeMenuAction;

    KToggleAction*            showThumbBarAction;
    KToggleAction*            syncPreviewAction;
    KToggleAction*            navigateByPairAction;
    KToggleAction*            showMenuBarAction;
    KToggleAction*            clearOnCloseAction;

    KSqueezedTextLabel*       leftFileName;
    KSqueezedTextLabel*       rightFileName;

    SidebarSplitter*          hSplitter;
    ThumbBarDock*             barViewDock;
    LightTableBar*            barView;

    LightTableView*           previewView;

    DZoomBar*                 leftZoomBar;
    DZoomBar*                 rightZoomBar;

    StatusProgressBar*        statusProgressBar;

    ImagePropertiesSideBarDB* leftSideBar;
    ImagePropertiesSideBarDB* rightSideBar;

    DAboutData*               about;
};

}  // namespace Digikam

#endif /* LIGHTTABLEWINDOWPRIVATE_H */

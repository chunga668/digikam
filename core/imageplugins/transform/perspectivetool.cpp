/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-02-17
 * Description : a plugin to change image perspective .
 *
 * Copyright (C) 2005-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2006-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#include "perspectivetool.moc"

// Qt includes

#include <QCheckBox>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

// KDE includes

#include <kapplication.h>
#include <kcolorbutton.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kcursor.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kpushbutton.h>
#include <kseparator.h>
#include <kstandarddirs.h>

// Local includes

#include "dimg.h"
#include "editortoolsettings.h"
#include "imageiface.h"
#include "perspectivewidget.h"

namespace DigikamTransformImagePlugin
{

class PerspectiveTool::PerspectiveToolPriv
{
public:

    PerspectiveToolPriv() :
        newWidthLabel(0),
        newHeightLabel(0),
        topLeftAngleLabel(0),
        topRightAngleLabel(0),
        bottomLeftAngleLabel(0),
        bottomRightAngleLabel(0),
        drawWhileMovingCheckBox(0),
        drawGridCheckBox(0),
        inverseTransformation(0),
        previewWidget(0),
        gboxSettings(0)
    {}

    static const QString configGroupName;
    static const QString configDrawWhileMovingEntry;
    static const QString configDrawGridEntry;
    static const QString configInverseTransformationEntry;

    QLabel*              newWidthLabel;
    QLabel*              newHeightLabel;
    QLabel*              topLeftAngleLabel;
    QLabel*              topRightAngleLabel;
    QLabel*              bottomLeftAngleLabel;
    QLabel*              bottomRightAngleLabel;

    QCheckBox*           drawWhileMovingCheckBox;
    QCheckBox*           drawGridCheckBox;
    QCheckBox*           inverseTransformation;

    PerspectiveWidget*   previewWidget;
    EditorToolSettings*  gboxSettings;
};
const QString PerspectiveTool::PerspectiveToolPriv::configGroupName("perspective Tool");
const QString PerspectiveTool::PerspectiveToolPriv::configDrawWhileMovingEntry("Draw While Moving");
const QString PerspectiveTool::PerspectiveToolPriv::configDrawGridEntry("Draw Grid");
const QString PerspectiveTool::PerspectiveToolPriv::configInverseTransformationEntry("Inverse Transformation");

// --------------------------------------------------------

PerspectiveTool::PerspectiveTool(QObject* parent)
    : EditorTool(parent),
      d(new PerspectiveToolPriv)
{
    setObjectName("perspective");
    setToolName(i18n("Perspective"));
    setToolIcon(SmallIcon("perspective"));

    // -------------------------------------------------------------

    QFrame* frame    = new QFrame(0);
    frame->setFrameStyle(QFrame::Panel|QFrame::Sunken);
    QVBoxLayout* l   = new QVBoxLayout(frame);
    d->previewWidget = new PerspectiveWidget(525, 350, frame);
    l->addWidget(d->previewWidget);
    d->previewWidget->setWhatsThis(i18n("This is the perspective transformation operation preview. "
                                        "You can use the mouse for dragging the corner to adjust the "
                                        "perspective transformation area."));
    setToolView(frame);

    // -------------------------------------------------------------

    QString temp;
    ImageIface iface(0, 0);

    d->gboxSettings = new EditorToolSettings;
    d->gboxSettings->setTools(EditorToolSettings::ColorGuide);

    // -------------------------------------------------------------

    QLabel* label1   = new QLabel(i18n("New width:"));
    d->newWidthLabel = new QLabel(temp.setNum( iface.originalWidth()) + i18n(" px"));
    d->newWidthLabel->setAlignment( Qt::AlignBottom | Qt::AlignRight );

    QLabel* label2    = new QLabel(i18n("New height:"));
    d->newHeightLabel = new QLabel(temp.setNum( iface.originalHeight()) + i18n(" px"));
    d->newHeightLabel->setAlignment( Qt::AlignBottom | Qt::AlignRight );

    // -------------------------------------------------------------

    KSeparator* line         = new KSeparator (Qt::Horizontal);
    QLabel* angleLabel       = new QLabel(i18n("Angles (in degrees):"));
    QLabel* label3           = new QLabel(i18n("  Top left:"));
    d->topLeftAngleLabel     = new QLabel;
    QLabel* label4           = new QLabel(i18n("  Top right:"));
    d->topRightAngleLabel    = new QLabel;
    QLabel* label5           = new QLabel(i18n("  Bottom left:"));
    d->bottomLeftAngleLabel  = new QLabel;
    QLabel* label6           = new QLabel(i18n("  Bottom right:"));
    d->bottomRightAngleLabel = new QLabel;

    // -------------------------------------------------------------

    KSeparator* line2          = new KSeparator (Qt::Horizontal);
    d->drawWhileMovingCheckBox = new QCheckBox(i18n("Draw preview while moving"));
    d->drawGridCheckBox        = new QCheckBox(i18n("Draw grid"));
    d->inverseTransformation   = new QCheckBox(i18n("Inverse transformation"));

    // -------------------------------------------------------------

    QGridLayout* grid = new QGridLayout;
    grid->addWidget(label1,                       0, 0, 1, 1);
    grid->addWidget(d->newWidthLabel,             0, 1, 1, 2);
    grid->addWidget(label2,                       1, 0, 1, 1);
    grid->addWidget(d->newHeightLabel,            1, 1, 1, 2);
    grid->addWidget(line,                         2, 0, 1, 3);
    grid->addWidget(angleLabel,                   3, 0, 1, 3);
    grid->addWidget(label3,                       4, 0, 1, 1);
    grid->addWidget(d->topLeftAngleLabel,         4, 1, 1, 2);
    grid->addWidget(label4,                       5, 0, 1, 1);
    grid->addWidget(d->topRightAngleLabel,        5, 1, 1, 2);
    grid->addWidget(label5,                       6, 0, 1, 1);
    grid->addWidget(d->bottomLeftAngleLabel,      6, 1, 1, 2);
    grid->addWidget(label6,                       7, 0, 1, 1);
    grid->addWidget(d->bottomRightAngleLabel,     7, 1, 1, 2);
    grid->addWidget(line2,                        8, 0, 1, 3);
    grid->addWidget(d->drawWhileMovingCheckBox,   9, 0, 1, 3);
    grid->addWidget(d->drawGridCheckBox,         10, 0, 1, 3);
    grid->addWidget(d->inverseTransformation,    11, 0, 1, 3);
    grid->setColumnStretch(1, 10);
    grid->setRowStretch(12, 10);
    grid->setMargin(d->gboxSettings->spacingHint());
    grid->setSpacing(d->gboxSettings->spacingHint());
    d->gboxSettings->plainPage()->setLayout(grid);

    // -------------------------------------------------------------

    setToolSettings(d->gboxSettings);
    init();

    // -------------------------------------------------------------

    connect(d->previewWidget, SIGNAL(signalPerspectiveChanged(const QRect&, float, float, float, float, bool)),
            this, SLOT(slotUpdateInfo(const QRect&, float, float, float, float, bool)));

    connect(d->drawWhileMovingCheckBox, SIGNAL(toggled(bool)),
            d->previewWidget, SLOT(slotToggleDrawWhileMoving(bool)));

    connect(d->drawGridCheckBox, SIGNAL(toggled(bool)),
            d->previewWidget, SLOT(slotToggleDrawGrid(bool)));

    connect(d->inverseTransformation, SIGNAL(toggled(bool)),
            this, SLOT(slotInverseTransformationChanged(bool)));

    connect(d->gboxSettings, SIGNAL(signalColorGuideChanged()),
            this, SLOT(slotColorGuideChanged()));
}

PerspectiveTool::~PerspectiveTool()
{
    delete d;
}

void PerspectiveTool::slotColorGuideChanged()
{
    d->previewWidget->slotChangeGuideColor(d->gboxSettings->guideColor());
    d->previewWidget->slotChangeGuideSize(d->gboxSettings->guideSize());
}

void PerspectiveTool::readSettings()
{
    QColor defaultGuideColor(Qt::red);
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);

    d->drawWhileMovingCheckBox->setChecked(group.readEntry( d->configDrawWhileMovingEntry,       true));
    d->drawGridCheckBox->setChecked(group.readEntry(        d->configDrawGridEntry,              false));
    d->inverseTransformation->setChecked(group.readEntry(   d->configInverseTransformationEntry, false));

    d->previewWidget->slotToggleDrawWhileMoving(d->drawWhileMovingCheckBox->isChecked());
    d->previewWidget->slotToggleDrawGrid(d->drawGridCheckBox->isChecked());
}

void PerspectiveTool::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);

    group.writeEntry(d->configDrawWhileMovingEntry,       d->drawWhileMovingCheckBox->isChecked());
    group.writeEntry(d->configDrawGridEntry,              d->drawGridCheckBox->isChecked());
    group.writeEntry(d->configInverseTransformationEntry, d->inverseTransformation->isChecked());
    config->sync();
}

void PerspectiveTool::slotResetSettings()
{
    d->previewWidget->reset();
}

void PerspectiveTool::finalRendering()
{
    kapp->setOverrideCursor(Qt::WaitCursor);
    d->previewWidget->applyPerspectiveAdjustment();
    kapp->restoreOverrideCursor();
}

void PerspectiveTool::slotUpdateInfo(const QRect& newSize, float topLeftAngle, float topRightAngle,
                                     float bottomLeftAngle, float bottomRightAngle, bool valid)
{
    QString temp;
    d->newWidthLabel->setText(temp.setNum(newSize.width()) + i18n(" px"));
    d->newHeightLabel->setText(temp.setNum(newSize.height()) + i18n(" px"));

    d->topLeftAngleLabel->setText(temp.setNum(topLeftAngle, 'f', 1));
    d->topRightAngleLabel->setText(temp.setNum(topRightAngle, 'f', 1));
    d->bottomLeftAngleLabel->setText(temp.setNum(bottomLeftAngle, 'f', 1));
    d->bottomRightAngleLabel->setText(temp.setNum(bottomRightAngle, 'f', 1));

    d->gboxSettings->button(EditorToolSettings::Ok)->setEnabled(valid);
}

void PerspectiveTool::slotInverseTransformationChanged(bool b)
{
    d->drawWhileMovingCheckBox->setEnabled(!b);
    d->drawGridCheckBox->setEnabled(!b);
    d->previewWidget->slotInverseTransformationChanged(b);
}

}  // namespace DigikamTransformImagePlugin
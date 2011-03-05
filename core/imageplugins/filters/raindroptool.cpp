/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-09-30
 * Description : a plugin to add rain drop over an image
 *
 * Copyright (C) 2004-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "raindroptool.moc"

// Qt includes

#include <QFrame>
#include <QGridLayout>
#include <QImage>
#include <QLabel>

// KDE includes

#include <kapplication.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <knuminput.h>
#include <kstandarddirs.h>

// LibKDcraw includes

#include <libkdcraw/rnuminput.h>

// Local includes

#include "editortoolsettings.h"
#include "imageiface.h"
#include "imageguidewidget.h"
#include "raindropfilter.h"

using namespace KDcrawIface;

namespace DigikamFxFiltersImagePlugin
{

class RainDropTool::RainDropToolPriv
{
public:

    RainDropToolPriv() :
        dropInput(0),
        amountInput(0),
        coeffInput(0),
        previewWidget(0),
        gboxSettings(0)
    {}

    static const QString configGroupName;
    static const QString configDropAdjustmentEntry;
    static const QString configAmountAdjustmentEntry;
    static const QString configCoeffAdjustmentEntry;

    RIntNumInput*        dropInput;
    RIntNumInput*        amountInput;
    RIntNumInput*        coeffInput;

    ImageGuideWidget*    previewWidget;
    EditorToolSettings*  gboxSettings;
};
const QString RainDropTool::RainDropToolPriv::configGroupName("raindrops Tool");
const QString RainDropTool::RainDropToolPriv::configDropAdjustmentEntry("DropAdjustment");
const QString RainDropTool::RainDropToolPriv::configAmountAdjustmentEntry("AmountAdjustment");
const QString RainDropTool::RainDropToolPriv::configCoeffAdjustmentEntry("CoeffAdjustment");

// --------------------------------------------------------

RainDropTool::RainDropTool(QObject* parent)
    : EditorToolThreaded(parent),
      d(new RainDropToolPriv)
{
    setObjectName("raindrops");
    setToolName(i18n("Raindrops"));
    setToolIcon(SmallIcon("raindrop"));

    d->previewWidget = new ImageGuideWidget(0, false, ImageGuideWidget::HVGuideMode);
    d->previewWidget->setWhatsThis(i18n("This is the preview of the Raindrop effect."
                                        "<p>Note: if you have previously selected an area in the editor, "
                                        "this will be unaffected by the filter. You can use this method to "
                                        "disable the Raindrops effect on a human face, for example.</p>"));

    setToolView(d->previewWidget);
    setPreviewModeMask(PreviewToolBar::AllPreviewModes);

    // -------------------------------------------------------------

    d->gboxSettings = new EditorToolSettings;
    d->gboxSettings->setButtons(EditorToolSettings::Default|
                                EditorToolSettings::Ok|
                                EditorToolSettings::Try|
                                EditorToolSettings::Cancel);


    // -------------------------------------------------------------

    QLabel* label1 = new QLabel(i18n("Drop size:"));
    d->dropInput   = new RIntNumInput;
    d->dropInput->setRange(0, 200, 1);
    d->dropInput->setSliderEnabled(true);
    d->dropInput->setDefaultValue(80);
    d->dropInput->setWhatsThis( i18n("Set here the raindrops' size."));

    // -------------------------------------------------------------

    QLabel* label2 = new QLabel(i18n("Number:"));
    d->amountInput = new RIntNumInput;
    d->amountInput->setRange(1, 500, 1);
    d->amountInput->setSliderEnabled(true);
    d->amountInput->setDefaultValue(150);
    d->amountInput->setWhatsThis( i18n("This value controls the maximum number of raindrops."));

    // -------------------------------------------------------------

    QLabel* label3 = new QLabel(i18n("Fish eyes:"));
    d->coeffInput  = new RIntNumInput;
    d->coeffInput->setRange(1, 100, 1);
    d->coeffInput->setSliderEnabled(true);
    d->coeffInput->setDefaultValue(30);
    d->coeffInput->setWhatsThis( i18n("This value is the fish-eye-effect optical "
                                      "distortion coefficient."));

    // -------------------------------------------------------------

    QGridLayout* mainLayout = new QGridLayout;
    mainLayout->addWidget(label1,         0, 0, 1, 3);
    mainLayout->addWidget(d->dropInput,   1, 0, 1, 3);
    mainLayout->addWidget(label2,         2, 0, 1, 3);
    mainLayout->addWidget(d->amountInput, 3, 0, 1, 3);
    mainLayout->addWidget(label3,         4, 0, 1, 3);
    mainLayout->addWidget(d->coeffInput,  5, 0, 1, 3);
    mainLayout->setRowStretch(6, 10);
    mainLayout->setMargin(d->gboxSettings->spacingHint());
    mainLayout->setSpacing(d->gboxSettings->spacingHint());
    d->gboxSettings->plainPage()->setLayout(mainLayout);

    // -------------------------------------------------------------

    setToolSettings(d->gboxSettings);
    init();
}

RainDropTool::~RainDropTool()
{
    delete d;
}

void RainDropTool::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);

    blockWidgetSignals(true);

    d->dropInput->setValue(group.readEntry(d->configDropAdjustmentEntry,     d->dropInput->defaultValue()));
    d->amountInput->setValue(group.readEntry(d->configAmountAdjustmentEntry, d->amountInput->defaultValue()));
    d->coeffInput->setValue(group.readEntry(d->configCoeffAdjustmentEntry,   d->coeffInput->defaultValue()));

    blockWidgetSignals(false);
}

void RainDropTool::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);
    group.writeEntry(d->configDropAdjustmentEntry,   d->dropInput->value());
    group.writeEntry(d->configAmountAdjustmentEntry, d->amountInput->value());
    group.writeEntry(d->configCoeffAdjustmentEntry,  d->coeffInput->value());

    group.sync();
}

void RainDropTool::slotResetSettings()
{
    blockWidgetSignals(true);

    d->dropInput->slotReset();
    d->amountInput->slotReset();
    d->coeffInput->slotReset();

    blockWidgetSignals(false);

    slotEffect();
}

void RainDropTool::prepareEffect()
{
    int drop   = d->dropInput->value();
    int amount = d->amountInput->value();
    int coeff  = d->coeffInput->value();

    ImageIface* iface = d->previewWidget->imageIface();

    // Selected data from the image
    QRect selection( iface->selectedXOrg(), iface->selectedYOrg(),
                     iface->selectedWidth(), iface->selectedHeight() );

    setFilter(new RainDropFilter(iface->getOriginalImg(), this, drop, amount, coeff, &selection));
}

void RainDropTool::prepareFinal()
{
    int drop   = d->dropInput->value();
    int amount = d->amountInput->value();
    int coeff  = d->coeffInput->value();

    ImageIface iface(0, 0);

    // Selected data from the image
    QRect selection( iface.selectedXOrg(), iface.selectedYOrg(),
                     iface.selectedWidth(), iface.selectedHeight() );

    setFilter(new RainDropFilter(iface.getOriginalImg(), this, drop, amount, coeff, &selection));
}

void RainDropTool::putPreviewData()
{
    ImageIface* iface = d->previewWidget->imageIface();
    DImg imDest       = filter()->getTargetImage().smoothScale(iface->previewWidth(), iface->previewHeight());
    iface->putPreviewImage(imDest.bits());

    d->previewWidget->updatePreview();
}

void RainDropTool::putFinalData()
{
    ImageIface iface(0, 0);
    iface.putOriginalImage(i18n("RainDrop"), filter()->filterAction(), filter()->getTargetImage().bits());
}

void RainDropTool::blockWidgetSignals(bool b)
{
    d->dropInput->blockSignals(b);
    d->amountInput->blockSignals(b);
    d->coeffInput->blockSignals(b);
}

}  // namespace DigikamFxFiltersImagePlugin

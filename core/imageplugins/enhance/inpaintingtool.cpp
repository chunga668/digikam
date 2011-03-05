/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2005-03-30
 * Description : a digiKam image editor plugin to inpaint
 *               a photograph
 *
 * Copyright (C) 2005-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "inpaintingtool.moc"

// C++ includes

#include <cstdio>
#include <cmath>
#include <cstring>

// Qt includes

#include <QBrush>
#include <QCheckBox>
#include <QEvent>
#include <QFile>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPainter>
#include <QPixmap>
#include <QPushButton>
#include <QTimer>

// KDE includes

#include <kapplication.h>
#include <kcombobox.h>
#include <kcursor.h>
#include <kfiledialog.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kpassivepopup.h>
#include <kstandarddirs.h>
#include <ktabwidget.h>
#include <ktoolinvocation.h>
#include <kurllabel.h>

// Local includes

#include "editortoolsettings.h"
#include "greycstorationfilter.h"
#include "greycstorationsettings.h"
#include "imageiface.h"
#include "imageguidewidget.h"

namespace DigikamEnhanceImagePlugin
{

class InPaintingTool::InPaintingToolPriv
{
public:

    InPaintingToolPriv() :
        isComputed(false),
        mainTab(0),
        inpaintingTypeCB(0),
        settingsWidget(0),
        previewWidget(0),
        gboxSettings(0)
    {}

    static const QString    configGroupName;
    static const QString    configFastApproxEntry;
    static const QString    configInterpolationEntry;
    static const QString    configAmplitudeEntry;
    static const QString    configSharpnessEntry;
    static const QString    configAnisotropyEntry;
    static const QString    configAlphaEntry;
    static const QString    configSigmaEntry;
    static const QString    configGaussPrecEntry;
    static const QString    configDlEntry;
    static const QString    configDaEntry;
    static const QString    configIterationEntry;
    static const QString    configTileEntry;
    static const QString    configBTileEntry;
    static const QString    configPresetEntry;

    bool                    isComputed;

    QRect                   maskRect;

    QImage                  maskImage;

    KTabWidget*             mainTab;

    KComboBox*              inpaintingTypeCB;

    DImg                    originalImage;
    DImg                    cropImage;

    GreycstorationSettings* settingsWidget;

    ImageGuideWidget*       previewWidget;

    EditorToolSettings*     gboxSettings;
};
const QString InPaintingTool::InPaintingToolPriv::configGroupName("inpainting Tool");
const QString InPaintingTool::InPaintingToolPriv::configFastApproxEntry("FastApprox");
const QString InPaintingTool::InPaintingToolPriv::configInterpolationEntry("Interpolation");
const QString InPaintingTool::InPaintingToolPriv::configAmplitudeEntry("Amplitude");
const QString InPaintingTool::InPaintingToolPriv::configSharpnessEntry("Sharpness");
const QString InPaintingTool::InPaintingToolPriv::configAnisotropyEntry("Anisotropy");
const QString InPaintingTool::InPaintingToolPriv::configAlphaEntry("Alpha");
const QString InPaintingTool::InPaintingToolPriv::configSigmaEntry("Sigma");
const QString InPaintingTool::InPaintingToolPriv::configGaussPrecEntry("GaussPrec");
const QString InPaintingTool::InPaintingToolPriv::configDlEntry("Dl");
const QString InPaintingTool::InPaintingToolPriv::configDaEntry("Da");
const QString InPaintingTool::InPaintingToolPriv::configIterationEntry("Iteration");
const QString InPaintingTool::InPaintingToolPriv::configTileEntry("Tile");
const QString InPaintingTool::InPaintingToolPriv::configBTileEntry("BTile");
const QString InPaintingTool::InPaintingToolPriv::configPresetEntry("Preset");

// --------------------------------------------------------

InPaintingTool::InPaintingTool(QObject* parent)
    : EditorToolThreaded(parent),
      d(new InPaintingToolPriv)
{
    setObjectName("inpainting");
    setToolName(i18n("In-painting"));
    setToolIcon(SmallIcon("inpainting"));

    // -------------------------------------------------------------

    d->gboxSettings = new EditorToolSettings;
    d->gboxSettings->setButtons(EditorToolSettings::Default|
                                EditorToolSettings::Ok|
                                EditorToolSettings::Try|
                                EditorToolSettings::Load|
                                EditorToolSettings::SaveAs|
                                EditorToolSettings::Cancel);

    // -------------------------------------------------------------

    d->mainTab               = new KTabWidget( d->gboxSettings->plainPage());
    QWidget* firstPage       = new QWidget(d->mainTab);

    KUrlLabel* cimgLogoLabel = new KUrlLabel();
    cimgLogoLabel->setText(QString());
    cimgLogoLabel->setUrl("http://cimg.sourceforge.net");
    cimgLogoLabel->setPixmap(QPixmap(KStandardDirs::locate("data", "digikam/data/logo-cimg.png")));
    cimgLogoLabel->setToolTip(i18n("Visit CImg library website"));

    QLabel* typeLabel   = new QLabel(i18n("Filtering type:"));
    typeLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    d->inpaintingTypeCB = new KComboBox();
    d->inpaintingTypeCB->addItem(i18nc("no inpainting type", "None"));
    d->inpaintingTypeCB->addItem(i18n("Remove Small Artifact"));
    d->inpaintingTypeCB->addItem(i18n("Remove Medium Artifact"));
    d->inpaintingTypeCB->addItem(i18n("Remove Large Artifact"));
    d->inpaintingTypeCB->setWhatsThis(i18n("<p>Select the filter preset to use for photograph restoration here:</p>"
                                           "<p><b>None</b>: Most common values. Puts settings to default.<br/>"
                                           "<b>Remove Small Artifact</b>: in-paint small image artifacts, such as image glitches.<br/>"
                                           "<b>Remove Medium Artifact</b>: in-paint medium image artifacts.<br/>"
                                           "<b>Remove Large Artifact</b>: in-paint large image artifacts, such as unwanted objects.</p>"));

    // -------------------------------------------------------------

    QGridLayout* firstPageLayout  = new QGridLayout();
    firstPageLayout->addWidget(cimgLogoLabel,       0, 1, 1, 1);
    firstPageLayout->addWidget(typeLabel,           1, 0, 1, 1);
    firstPageLayout->addWidget(d->inpaintingTypeCB, 1, 1, 1, 1);
    firstPageLayout->setRowStretch(1, 10);
    firstPageLayout->setMargin(d->gboxSettings->spacingHint());
    firstPageLayout->setSpacing(d->gboxSettings->spacingHint());
    firstPage->setLayout(firstPageLayout);

    d->mainTab->addTab(firstPage, i18n("Preset"));

    // -------------------------------------------------------------

    QLabel* spacer    = new QLabel();
    d->settingsWidget = new GreycstorationSettings(d->mainTab);

    // -------------------------------------------------------------

    QGridLayout* mainLayout = new QGridLayout(d->gboxSettings->plainPage());
    mainLayout->addWidget(d->mainTab,   0, 1, 1, 1);
    mainLayout->addWidget(spacer,       1, 1, 1, 1);
    mainLayout->setRowStretch(1, 10);
    mainLayout->setMargin(d->gboxSettings->spacingHint());
    mainLayout->setSpacing(d->gboxSettings->spacingHint());
    d->gboxSettings->plainPage()->setLayout(mainLayout);

    // -------------------------------------------------------------

    d->previewWidget = new ImageGuideWidget(0, false, ImageGuideWidget::HVGuideMode, Qt::red, 1, false, true);
    d->previewWidget->setWhatsThis(i18n("The image selection preview with in-painting applied "
                                        "is shown here."));

    setToolSettings(d->gboxSettings);
    setToolView(d->previewWidget);
    setPreviewModeMask(PreviewToolBar::UnSplitPreviewModes);
    init();

    // -------------------------------------------------------------

    connect(cimgLogoLabel, SIGNAL(leftClickedUrl(const QString&)),
            this, SLOT(processCImgUrl(const QString&)));

    connect(d->inpaintingTypeCB, SIGNAL(activated(int)),
            this, SLOT(slotResetValues(int)));

    // -------------------------------------------------------------

    GreycstorationContainer defaults;
    defaults.setInpaintingDefaultSettings();
    d->settingsWidget->setDefaultSettings(defaults);
}

InPaintingTool::~InPaintingTool()
{
    delete d;
}

void InPaintingTool::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);

    GreycstorationContainer prm;
    GreycstorationContainer defaults;
    defaults.setInpaintingDefaultSettings();

    prm.fastApprox = group.readEntry(d->configFastApproxEntry,    defaults.fastApprox);
    prm.interp     = group.readEntry(d->configInterpolationEntry, defaults.interp);
    prm.amplitude  = group.readEntry(d->configAmplitudeEntry,     (double)defaults.amplitude);
    prm.sharpness  = group.readEntry(d->configSharpnessEntry,     (double)defaults.sharpness);
    prm.anisotropy = group.readEntry(d->configAnisotropyEntry,    (double)defaults.anisotropy);
    prm.alpha      = group.readEntry(d->configAlphaEntry,         (double)defaults.alpha);
    prm.sigma      = group.readEntry(d->configSigmaEntry,         (double)defaults.sigma);
    prm.gaussPrec  = group.readEntry(d->configGaussPrecEntry,     (double)defaults.gaussPrec);
    prm.dl         = group.readEntry(d->configDlEntry,            (double)defaults.dl);
    prm.da         = group.readEntry(d->configDaEntry,            (double)defaults.da);
    prm.nbIter     = group.readEntry(d->configIterationEntry,     defaults.nbIter);
    prm.tile       = group.readEntry(d->configTileEntry,          defaults.tile);
    prm.btile      = group.readEntry(d->configBTileEntry,         defaults.btile);
    d->settingsWidget->setSettings(prm);

    int p = group.readEntry(d->configPresetEntry, (int)NoPreset);
    d->inpaintingTypeCB->setCurrentIndex(p);

    if (p == NoPreset)
    {
        d->settingsWidget->setEnabled(true);
    }
    else
    {
        d->settingsWidget->setEnabled(false);
    }
}

void InPaintingTool::writeSettings()
{
    GreycstorationContainer prm = d->settingsWidget->settings();
    KSharedConfig::Ptr config   = KGlobal::config();
    KConfigGroup group          = config->group(d->configGroupName);

    group.writeEntry(d->configPresetEntry,        d->inpaintingTypeCB->currentIndex());
    group.writeEntry(d->configFastApproxEntry,    prm.fastApprox);
    group.writeEntry(d->configInterpolationEntry, prm.interp);
    group.writeEntry(d->configAmplitudeEntry,     (double)prm.amplitude);
    group.writeEntry(d->configSharpnessEntry,     (double)prm.sharpness);
    group.writeEntry(d->configAnisotropyEntry,    (double)prm.anisotropy);
    group.writeEntry(d->configAlphaEntry,         (double)prm.alpha);
    group.writeEntry(d->configSigmaEntry,         (double)prm.sigma);
    group.writeEntry(d->configGaussPrecEntry,     (double)prm.gaussPrec);
    group.writeEntry(d->configDlEntry,            (double)prm.dl);
    group.writeEntry(d->configDaEntry,            (double)prm.da);
    group.writeEntry(d->configIterationEntry,     prm.nbIter);
    group.writeEntry(d->configTileEntry,          prm.tile);
    group.writeEntry(d->configBTileEntry,         prm.btile);

    config->sync();
}

void InPaintingTool::slotResetValues(int i)
{
    if (i == NoPreset)
    {
        d->settingsWidget->setEnabled(true);
    }
    else
    {
        d->settingsWidget->setEnabled(false);
    }

    slotResetSettings();
}

void InPaintingTool::slotResetSettings()
{
    GreycstorationContainer settings;
    settings.setInpaintingDefaultSettings();

    switch (d->inpaintingTypeCB->currentIndex())
    {
        case RemoveSmallArtefact:
            // We use default settings here.
            break;

        case RemoveMediumArtefact:
        {
            settings.amplitude = 50.0;
            settings.nbIter    = 50;
            break;
        }

        case RemoveLargeArtefact:
        {
            settings.amplitude = 100.0;
            settings.nbIter    = 100;
            break;
        }
    }

    d->settingsWidget->setSettings(settings);
}

void InPaintingTool::processCImgUrl(const QString& url)
{
    KToolInvocation::invokeBrowser(url);
}

void InPaintingTool::prepareEffect()
{
    ImageIface iface(0, 0);
    uchar* data      = iface.getOriginalImage();
    d->originalImage = DImg(iface.originalWidth(), iface.originalHeight(),
                            iface.originalSixteenBit(), iface.originalHasAlpha(), data);
    delete [] data;

    // Selected area from the image and mask creation:
    //
    // We optimize the computation time to use the current selected area in image editor
    // and to create an inpainting mask with it. Because inpainting is done by interpolation
    // neighbor pixels which can be located far from the selected area, we need to adjust the
    // mask size in according with the parameter algorithms, especially 'amplitude'.
    // Mask size is computed like this :
    //
    // (image_size_x + 2*amplitude , image_size_y + 2*amplitude)


    QRect selectionRect = QRect(iface.selectedXOrg(), iface.selectedYOrg(),
                                iface.selectedWidth(), iface.selectedHeight());

    QPixmap inPaintingMask(iface.originalWidth(), iface.originalHeight());
    inPaintingMask.fill(Qt::black);
    QPainter p(&inPaintingMask);
    p.fillRect( selectionRect, QBrush(Qt::white) );
    p.end();

    GreycstorationContainer settings = d->settingsWidget->settings();

    int x1 = (int)(selectionRect.left()   - 2*settings.amplitude);
    int y1 = (int)(selectionRect.top()    - 2*settings.amplitude);
    int x2 = (int)(selectionRect.right()  + 2*settings.amplitude);
    int y2 = (int)(selectionRect.bottom() + 2*settings.amplitude);
    d->maskRect = QRect(x1, y1, x2-x1, y2-y1);

    // Mask area normalization.
    // We need to check if mask area is out of image size else inpainting give strange results.

    if (d->maskRect.left()   < 0)
    {
        d->maskRect.setLeft(0);
    }

    if (d->maskRect.top()    < 0)
    {
        d->maskRect.setTop(0);
    }

    if (d->maskRect.right()  > iface.originalWidth())
    {
        d->maskRect.setRight(iface.originalWidth());
    }

    if (d->maskRect.bottom() > iface.originalHeight())
    {
        d->maskRect.setBottom(iface.originalHeight());
    }

    d->maskImage = inPaintingMask.toImage().copy(d->maskRect);
    d->cropImage = d->originalImage.copy(d->maskRect);

    setFilter(new GreycstorationFilter(&d->cropImage,
                                       settings,
                                       GreycstorationFilter::InPainting,
                                       0, 0,
                                       d->maskImage, this));
}

void InPaintingTool::prepareFinal()
{
    if (!d->isComputed)
    {
        prepareEffect();
    }
    else
    {
        slotFilterFinished(true);
    }
}

void InPaintingTool::putPreviewData()
{
    ImageIface* iface                = d->previewWidget->imageIface();
    GreycstorationContainer settings = d->settingsWidget->settings();

    d->cropImage = filter()->getTargetImage();
    QRect cropSel((int)(2*settings.amplitude), (int)(2*settings.amplitude),
                  iface->selectedWidth(), iface->selectedHeight());
    DImg imDest = d->cropImage.copy(cropSel);

    iface->putPreviewImage((imDest.smoothScale(iface->previewWidth(),
                            iface->previewHeight())).bits());
    d->previewWidget->updatePreview();
    d->isComputed = true;
}

void InPaintingTool::putFinalData()
{
    ImageIface iface(0, 0);

    if (!d->isComputed)
    {
        d->cropImage = filter()->getTargetImage();
    }

    d->originalImage.bitBltImage(&d->cropImage, d->maskRect.left(), d->maskRect.top());

    iface.putOriginalImage(i18n("In-Painting"), filter()->filterAction(), d->originalImage.bits());
}

void InPaintingTool::slotLoadSettings()
{
    KUrl loadInpaintingFile = KFileDialog::getOpenUrl(KGlobalSettings::documentPath(),
                              QString( "*" ), kapp->activeWindow(),
                              QString( i18n("Photograph In-Painting Settings File to Load")) );

    if ( loadInpaintingFile.isEmpty() )
    {
        return;
    }

    QFile file(loadInpaintingFile.toLocalFile());

    if ( file.open(QIODevice::ReadOnly) )
    {
        if (!d->settingsWidget->loadSettings(file, QString("# Photograph Inpainting Configuration File V2")))
        {
            KMessageBox::error(kapp->activeWindow(),
                               i18n("\"%1\" is not a Photograph In-Painting settings text file.",
                                    loadInpaintingFile.fileName()));
            file.close();
            return;
        }
    }
    else
    {
        KMessageBox::error(kapp->activeWindow(), i18n("Cannot load settings from the Photograph In-Painting text file."));
    }

    file.close();
    d->inpaintingTypeCB->blockSignals(true);
    d->inpaintingTypeCB->setCurrentIndex(NoPreset);
    d->inpaintingTypeCB->blockSignals(false);
    d->settingsWidget->setEnabled(true);
}

void InPaintingTool::slotSaveAsSettings()
{
    KUrl saveRestorationFile = KFileDialog::getSaveUrl(KGlobalSettings::documentPath(),
                               QString( "*" ), kapp->activeWindow(),
                               QString( i18n("Photograph In-Painting Settings File to Save")) );

    if ( saveRestorationFile.isEmpty() )
    {
        return;
    }

    QFile file(saveRestorationFile.toLocalFile());

    if ( file.open(QIODevice::WriteOnly) )
    {
        d->settingsWidget->saveSettings(file, QString("# Photograph Inpainting Configuration File V2"));
    }
    else
    {
        KMessageBox::error(kapp->activeWindow(), i18n("Cannot save settings to the Photograph In-Painting text file."));
    }

    file.close();
}

}  // namespace DigikamEnhanceImagePlugin

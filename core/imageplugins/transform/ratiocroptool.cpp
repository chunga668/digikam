/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-12-06
 * Description : digiKam image editor Ratio Crop tool
 *
 * Copyright (C) 2007 by Jaromir Malenko <malenko at email dot cz>
 * Copyright (C) 2008 by Roberto Castagnola <roberto dot castagnola at gmail dot com>
 * Copyright (C) 2004-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "ratiocroptool.moc"

// Qt includes

#include <QCheckBox>
#include <QFrame>
#include <QGridLayout>
#include <QGroupBox>
#include <QImage>
#include <QLabel>
#include <QPixmap>
#include <QRect>
#include <QSpinBox>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>

// KDE includes

#include <kapplication.h>
#include <kcolorbutton.h>
#include <kconfig.h>
#include <kconfiggroup.h>
#include <kcursor.h>
#include <klocale.h>
#include <kpushbutton.h>
#include <kstandarddirs.h>
#include <kstandardguiitem.h>

// LibKDcraw includes

#include <libkdcraw/rcombobox.h>
#include <libkdcraw/rnuminput.h>
#include <libkdcraw/rexpanderbox.h>

// Local includes

#include "editortoolsettings.h"
#include "imageiface.h"
#include "imageselectionwidget.h"

using namespace KDcrawIface;

namespace DigikamTransformImagePlugin
{

class RatioCropTool::RatioCropToolPriv
{
public:

    RatioCropToolPriv() :
        originalIsLandscape(false),
        customLabel(0),
        orientLabel(0),
        colorGuideLabel(0),
        centerWidth(0),
        centerHeight(0),
        goldenSectionBox(0),
        goldenSpiralSectionBox(0),
        goldenSpiralBox(0),
        goldenTriangleBox(0),
        flipHorBox(0),
        flipVerBox(0),
        autoOrientation(0),
        preciseCrop(0),
        ratioCB(0),
        orientCB(0),
        guideLinesCB(0),
        customRatioDInput(0),
        customRatioNInput(0),
        guideSize(0),
        heightInput(0),
        widthInput(0),
        xInput(0),
        yInput(0),
        guideColorBt(0),
        imageSelectionWidget(0),
        expbox(0),
        gboxSettings(0)
    {}

    static const QString  configGroupName;
    static const QString  configHorOrientedAspectRatioEntry;
    static const QString  configHorOrientedAspectRatioOrientationEntry;
    static const QString  configHorOrientedCustomAspectRatioNumEntry;
    static const QString  configHorOrientedCustomAspectRatioDenEntry;
    static const QString  configHorOrientedCustomAspectRatioXposEntry;
    static const QString  configHorOrientedCustomAspectRatioYposEntry;
    static const QString  configHorOrientedCustomAspectRatioWidthEntry;
    static const QString  configHorOrientedCustomAspectRatioHeightEntry;
    static const QString  configVerOrientedAspectRatioEntry;
    static const QString  configVerOrientedAspectRatioOrientationEntry;
    static const QString  configVerOrientedCustomAspectRatioNumEntry;
    static const QString  configVerOrientedCustomAspectRatioDenEntry;
    static const QString  configVerOrientedCustomAspectRatioXposEntry;
    static const QString  configVerOrientedCustomAspectRatioYposEntry;
    static const QString  configVerOrientedCustomAspectRatioWidthEntry;
    static const QString  configVerOrientedCustomAspectRatioHeightEntry;
    static const QString  configPreciseAspectRatioCropEntry;
    static const QString  configAutoOrientationEntry;
    static const QString  configGuideLinesTypeEntry;
    static const QString  configGoldenSectionEntry;
    static const QString  configGoldenSpiralSectionEntry;
    static const QString  configGoldenSpiralEntry;
    static const QString  configGoldenTriangleEntry;
    static const QString  configGoldenFlipHorizontalEntry;
    static const QString  configGoldenFlipVerticalEntry;
    static const QString  configGuideColorEntry;
    static const QString  configGuideWidthEntry;

    bool                  originalIsLandscape;

    QLabel*               customLabel;
    QLabel*               orientLabel;
    QLabel*               colorGuideLabel;

    QToolButton*          centerWidth;
    QToolButton*          centerHeight;

    QCheckBox*            goldenSectionBox;
    QCheckBox*            goldenSpiralSectionBox;
    QCheckBox*            goldenSpiralBox;
    QCheckBox*            goldenTriangleBox;
    QCheckBox*            flipHorBox;
    QCheckBox*            flipVerBox;
    QCheckBox*            autoOrientation;
    QCheckBox*            preciseCrop;

    RComboBox*            ratioCB;
    RComboBox*            orientCB;
    RComboBox*            guideLinesCB;

    RIntNumInput*         customRatioDInput;
    RIntNumInput*         customRatioNInput;
    RIntNumInput*         guideSize;
    RIntNumInput*         heightInput;
    RIntNumInput*         widthInput;
    RIntNumInput*         xInput;
    RIntNumInput*         yInput;

    KColorButton*         guideColorBt;

    ImageSelectionWidget* imageSelectionWidget;
    RExpanderBox*         expbox;
    EditorToolSettings*   gboxSettings;
};
const QString RatioCropTool::RatioCropToolPriv::configGroupName("aspectratiocrop Tool");
const QString RatioCropTool::RatioCropToolPriv::configHorOrientedAspectRatioEntry("Hor.Oriented Aspect Ratio");
const QString RatioCropTool::RatioCropToolPriv::configHorOrientedAspectRatioOrientationEntry("Hor.Oriented Aspect Ratio Orientation");
const QString RatioCropTool::RatioCropToolPriv::configHorOrientedCustomAspectRatioNumEntry("Hor.Oriented Custom Aspect Ratio Num");
const QString RatioCropTool::RatioCropToolPriv::configHorOrientedCustomAspectRatioDenEntry("Hor.Oriented Custom Aspect Ratio Den");
const QString RatioCropTool::RatioCropToolPriv::configHorOrientedCustomAspectRatioXposEntry("Hor.Oriented Custom Aspect Ratio Xpos");
const QString RatioCropTool::RatioCropToolPriv::configHorOrientedCustomAspectRatioYposEntry("Hor.Oriented Custom Aspect Ratio Ypos");
const QString RatioCropTool::RatioCropToolPriv::configHorOrientedCustomAspectRatioWidthEntry("Hor.Oriented Custom Aspect Ratio Width");
const QString RatioCropTool::RatioCropToolPriv::configHorOrientedCustomAspectRatioHeightEntry("Hor.Oriented Custom Aspect Ratio Height");
const QString RatioCropTool::RatioCropToolPriv::configVerOrientedAspectRatioEntry("Ver.Oriented Aspect Ratio");
const QString RatioCropTool::RatioCropToolPriv::configVerOrientedAspectRatioOrientationEntry("Ver.Oriented Aspect Ratio Orientation");
const QString RatioCropTool::RatioCropToolPriv::configVerOrientedCustomAspectRatioNumEntry("Ver.Oriented Custom Aspect Ratio Num");
const QString RatioCropTool::RatioCropToolPriv::configVerOrientedCustomAspectRatioDenEntry("Ver.Oriented Custom Aspect Ratio Den");
const QString RatioCropTool::RatioCropToolPriv::configVerOrientedCustomAspectRatioXposEntry("Ver.Oriented Custom Aspect Ratio Xpos");
const QString RatioCropTool::RatioCropToolPriv::configVerOrientedCustomAspectRatioYposEntry("Ver.Oriented Custom Aspect Ratio Ypos");
const QString RatioCropTool::RatioCropToolPriv::configVerOrientedCustomAspectRatioWidthEntry("Ver.Oriented Custom Aspect Ratio Width");
const QString RatioCropTool::RatioCropToolPriv::configVerOrientedCustomAspectRatioHeightEntry("Ver.Oriented Custom Aspect Ratio Height");
const QString RatioCropTool::RatioCropToolPriv::configPreciseAspectRatioCropEntry("Precise Aspect Ratio Crop");
const QString RatioCropTool::RatioCropToolPriv::configAutoOrientationEntry("Auto Orientation");
const QString RatioCropTool::RatioCropToolPriv::configGuideLinesTypeEntry("Guide Lines Type");
const QString RatioCropTool::RatioCropToolPriv::configGoldenSectionEntry("Golden Section");
const QString RatioCropTool::RatioCropToolPriv::configGoldenSpiralSectionEntry("Golden Spiral Section");
const QString RatioCropTool::RatioCropToolPriv::configGoldenSpiralEntry("Golden Spiral");
const QString RatioCropTool::RatioCropToolPriv::configGoldenTriangleEntry("Golden Triangle");
const QString RatioCropTool::RatioCropToolPriv::configGoldenFlipHorizontalEntry("Golden Flip Horizontal");
const QString RatioCropTool::RatioCropToolPriv::configGoldenFlipVerticalEntry("Golden Flip Vertical");
const QString RatioCropTool::RatioCropToolPriv::configGuideColorEntry("Guide Color");
const QString RatioCropTool::RatioCropToolPriv::configGuideWidthEntry("Guide Width");

// --------------------------------------------------------

RatioCropTool::RatioCropTool(QObject* parent)
    : EditorTool(parent),
      d(new RatioCropToolPriv)
{
    setObjectName("aspectratiocrop");
    setToolName(i18n("Aspect Ratio Crop"));
    setToolIcon(SmallIcon("ratiocrop"));
    setToolHelp("ratiocroptool.anchor");

    // -------------------------------------------------------------

    // Important: Deactivate drawing of the selection now, we will enable it later.
    d->imageSelectionWidget = new ImageSelectionWidget(480, 320, false);
    d->imageSelectionWidget->setWhatsThis(i18n("<p>Here you can see the aspect ratio selection preview "
                                          "used for cropping. You can use the mouse to move and "
                                          "resize the crop area.</p>"
                                          "<p>Press and hold the <b>CTRL</b> key to move the opposite corner too.</p>"
                                          "<p>Press and hold the <b>SHIFT</b> key to move the closest corner to the "
                                          "mouse pointer.</p>"));

    d->originalIsLandscape = ((d->imageSelectionWidget->getOriginalImageWidth()) >
                              (d->imageSelectionWidget->getOriginalImageHeight()));

    setToolView(d->imageSelectionWidget);

    // -------------------------------------------------------------

    d->gboxSettings = new EditorToolSettings;
    d->gboxSettings->setButtons(EditorToolSettings::Default|
                                EditorToolSettings::Ok|
                                EditorToolSettings::Try|
                                EditorToolSettings::Cancel);

    // -------------------------------------------------------------

    // need to set the button to a KStdGuiItem that has no icon
    KPushButton* tryBtn = d->gboxSettings->button(EditorToolSettings::Try);
    tryBtn->setGuiItem(KStandardGuiItem::Test);
    tryBtn->setText(i18n("Max. Aspect"));
    tryBtn->setToolTip(i18n("Set selection area to the maximum size according "
                            "to the current ratio."));

    // -------------------------------------------------------------

    QVBoxLayout* vlay      = new QVBoxLayout(d->gboxSettings->plainPage());
    d->expbox              = new RExpanderBox(d->gboxSettings->plainPage());
    d->expbox->setObjectName("RatioCropTool Expander");
    QWidget* cropSelection = new QWidget(d->expbox);

    QLabel* label = new QLabel(i18n("Aspect ratio:"), cropSelection);
    d->ratioCB    = new RComboBox(cropSelection);
    d->ratioCB->addItem(i18nc("custom aspect ratio crop settings", "Custom"));
    // NOTE: Order is important there. Look ImageSelectionWidget::RatioAspect for details.
    d->ratioCB->addItem("1:1");
    d->ratioCB->addItem("2:3");
    d->ratioCB->addItem("3:4");
    d->ratioCB->addItem("4:5");
    d->ratioCB->addItem("5:7");
    d->ratioCB->addItem("7:10");
    d->ratioCB->addItem("8:5");
    d->ratioCB->addItem(i18n("Golden Ratio"));
    d->ratioCB->addItem(i18n("Current Image Aspect Ratio"));
    d->ratioCB->addItem(i18nc("no crop mode", "None"));
    d->ratioCB->setDefaultIndex(ImageSelectionWidget::RATIO03X04);
    setRatioCBText(ImageSelectionWidget::Landscape);
    d->ratioCB->setWhatsThis( i18n("<p>Select your constrained aspect ratio for cropping. "
                                   "Aspect Ratio Crop tool uses a relative ratio. That means it "
                                   "is the same if you use centimeters or inches and it does not "
                                   "specify the physical size.</p>"
                                   "<p>You can see below a correspondence list of traditional photographic "
                                   "paper sizes and aspect ratio crop:</p>"
                                   "<p><b>2:3</b>: 10x15cm, 20x30cm, 30x45cm, 4x6\", 8x12\", "
                                   "12x18\", 16x24\", 20x30\"</p>"
                                   "<p><b>3:4</b>: 6x8cm, 15x20cm, 18x24cm, 30x40cm, 3.75x5\", 4.5x6\", "
                                   "6x8\", 7.5x10\", 9x12\"</p>"
                                   "<p><b>4:5</b>: 20x25cm, 40x50cm, 8x10\", 16x20\"</p>"
                                   "<p><b>5:7</b>: 15x21cm, 30x42cm, 5x7\"</p>"
                                   "<p><b>7:10</b>: 21x30cm, 42x60cm, 3.5x5\"</p>"
                                   "<p><b>8:5</b>: common widescreen monitor (as 1680x1050)</p>"
                                   "<p>The <b>Golden Ratio</b> is 1:1.618. A composition following this rule "
                                   "is considered visually harmonious but can be unadapted to print on "
                                   "standard photographic paper.</p>"
                                   "<p>The <b>Current Aspect Ratio</b> takes aspect ratio from the currently "
                                   "opened image.</p>"));

    d->preciseCrop = new QCheckBox(cropSelection);
    d->preciseCrop->setToolTip(i18n("Exact aspect"));
    d->preciseCrop->setWhatsThis(i18n("Enable this option to force exact aspect ratio crop."));

    d->orientLabel = new QLabel(i18n("Orientation:"), cropSelection);
    d->orientCB    = new RComboBox( cropSelection );
    d->orientCB->addItem( i18n("Landscape") );
    d->orientCB->addItem( i18n("Portrait") );
    d->orientCB->setWhatsThis(i18n("Select constrained aspect ratio orientation."));

    d->autoOrientation = new QCheckBox(cropSelection);
    d->autoOrientation->setToolTip(i18n("Auto"));
    d->autoOrientation->setWhatsThis( i18n("Enable this option to automatically set the orientation."));

    // -------------------------------------------------------------

    d->customLabel       = new QLabel(i18n("Custom ratio:"), cropSelection);
    d->customLabel->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);
    d->customRatioNInput = new RIntNumInput(cropSelection);
    d->customRatioNInput->setRange(1, 10000, 1);
    d->customRatioNInput->setDefaultValue(1);
    d->customRatioNInput->setSliderEnabled(false);
    d->customRatioNInput->setWhatsThis( i18n("Set here the desired custom aspect numerator value."));

    d->customRatioDInput = new RIntNumInput(cropSelection);
    d->customRatioDInput->setRange(1, 10000, 1);
    d->customRatioDInput->setDefaultValue(1);
    d->customRatioDInput->setSliderEnabled(false);
    d->customRatioDInput->setWhatsThis( i18n("Set here the desired custom aspect denominator value."));

    // -------------------------------------------------------------

    d->xInput = new RIntNumInput(cropSelection);
    d->xInput->setWhatsThis( i18n("Set here the top left selection corner position for cropping."));
    d->xInput->input()->setLabel(i18nc("top left corner position for cropping", "X:"), Qt::AlignLeft|Qt::AlignVCenter);
    d->xInput->setRange(0, d->imageSelectionWidget->getOriginalImageWidth(), 1);
    d->xInput->setSliderEnabled(true);
    d->xInput->setDefaultValue(50);

    d->widthInput = new RIntNumInput(cropSelection);
    d->widthInput->input()->setLabel(i18n("Width:"), Qt::AlignLeft|Qt::AlignVCenter);
    d->widthInput->setWhatsThis( i18n("Set here the width selection for cropping."));
    d->widthInput->setRange(d->imageSelectionWidget->getMinWidthRange(),
                            d->imageSelectionWidget->getMaxWidthRange(),
                            d->imageSelectionWidget->getWidthStep());
    d->widthInput->setSliderEnabled(true);
    d->widthInput->setDefaultValue(800);

    d->centerWidth = new QToolButton(cropSelection);
    d->centerWidth->setIcon(QPixmap(KStandardDirs::locate("data", "digikam/data/centerwidth.png")));
    d->centerWidth->setWhatsThis( i18n("Set width position to center."));

    // -------------------------------------------------------------

    d->yInput = new RIntNumInput(cropSelection);
    d->yInput->input()->setLabel(i18n("Y:"), Qt::AlignLeft|Qt::AlignVCenter);
    d->yInput->setWhatsThis( i18n("Set here the top left selection corner position for cropping."));
    d->yInput->setRange(0, d->imageSelectionWidget->getOriginalImageWidth(), 1);
    d->yInput->setSliderEnabled(true);
    d->yInput->setDefaultValue(50);

    d->heightInput = new RIntNumInput(cropSelection);
    d->heightInput->input()->setLabel(i18n("Height:"), Qt::AlignLeft|Qt::AlignVCenter);
    d->heightInput->setWhatsThis( i18n("Set here the height selection for cropping."));
    d->heightInput->setRange(d->imageSelectionWidget->getMinHeightRange(),
                             d->imageSelectionWidget->getMaxHeightRange(),
                             d->imageSelectionWidget->getHeightStep());
    d->heightInput->setSliderEnabled(true);
    d->heightInput->setDefaultValue(600);

    d->centerHeight = new QToolButton(cropSelection);
    d->centerHeight->setIcon(QPixmap(KStandardDirs::locate("data", "digikam/data/centerheight.png")));
    d->centerHeight->setWhatsThis( i18n("Set height position to center."));

    // -------------------------------------------------------------

    QGridLayout* mainLayout = new QGridLayout(cropSelection);
    mainLayout->addWidget(label,                0, 0, 1, 1);
    mainLayout->addWidget(d->ratioCB,           0, 1, 1, 3);
    mainLayout->addWidget(d->preciseCrop,       0, 4, 1, 1);
    mainLayout->addWidget(d->customLabel,       1, 0, 1, 1);
    mainLayout->addWidget(d->customRatioNInput, 1, 1, 1, 1);
    mainLayout->addWidget(d->customRatioDInput, 2, 1, 1, 1);
    mainLayout->addWidget(d->orientLabel,       3, 0, 1, 1);
    mainLayout->addWidget(d->orientCB,          3, 1, 1, 3);
    mainLayout->addWidget(d->autoOrientation,   3, 4, 1, 1);
    mainLayout->addWidget(d->xInput,            4, 0, 1, 4);
    mainLayout->addWidget(d->widthInput,        5, 0, 1, 4);
    mainLayout->addWidget(d->centerWidth,       5, 4, 1, 1);
    mainLayout->addWidget(d->yInput,            6, 0, 1, 4);
    mainLayout->addWidget(d->heightInput,       7, 0, 1, 4);
    mainLayout->addWidget(d->centerHeight,      7, 4, 1, 1);
    mainLayout->setMargin(d->gboxSettings->spacingHint());
    mainLayout->setSpacing(d->gboxSettings->spacingHint());

    d->expbox->addItem(cropSelection, SmallIcon("transform-crop-and-resize"),
                       i18n("Crop Settings"), QString("CropSelection"), true);

    // -------------------------------------------------------------

    QWidget* compositionGuide = new QWidget(d->expbox);
    QGridLayout* grid2        = new QGridLayout(compositionGuide);

    QLabel* labelGuideLines = new QLabel(i18n("Geometric form:"), compositionGuide);
    d->guideLinesCB         = new RComboBox(compositionGuide);
    d->guideLinesCB->addItem(i18n("Rules of Thirds"));
    d->guideLinesCB->addItem(i18n("Diagonal Method"));
    d->guideLinesCB->addItem(i18n("Harmonious Triangles"));
    d->guideLinesCB->addItem(i18n("Golden Mean"));
    d->guideLinesCB->addItem(i18nc("no geometric form", "None"));
    d->guideLinesCB->setDefaultIndex(ImageSelectionWidget::GuideNone);
    d->guideLinesCB->setCurrentIndex(3);
    d->guideLinesCB->setWhatsThis( i18n("With this option, you can display guide lines "
                                        "to help compose your photograph."));

    d->goldenSectionBox = new QCheckBox(i18n("Golden sections"), compositionGuide);
    d->goldenSectionBox->setWhatsThis(i18n("Enable this option to show golden sections."));

    d->goldenSpiralSectionBox = new QCheckBox(i18n("Golden spiral sections"), compositionGuide);
    d->goldenSpiralSectionBox->setWhatsThis(i18n("Enable this option to show golden spiral sections."));

    d->goldenSpiralBox = new QCheckBox(i18n("Golden spiral"), compositionGuide);
    d->goldenSpiralBox->setWhatsThis(i18n("Enable this option to show a golden spiral guide."));

    d->goldenTriangleBox = new QCheckBox(i18n("Golden triangles"), compositionGuide);
    d->goldenTriangleBox->setWhatsThis(i18n("Enable this option to show golden triangles."));

    d->flipHorBox = new QCheckBox(i18n("Flip horizontally"), compositionGuide);
    d->flipHorBox->setWhatsThis(i18n("Enable this option to flip the guidelines horizontally."));

    d->flipVerBox = new QCheckBox(i18n("Flip vertically"), compositionGuide);
    d->flipVerBox->setWhatsThis(i18n("Enable this option to flip the guidelines vertically."));

    d->colorGuideLabel = new QLabel(i18n("Color and width:"), compositionGuide);
    d->guideColorBt    = new KColorButton( QColor( 250, 250, 255 ), compositionGuide );
    d->guideSize       = new RIntNumInput(compositionGuide);
    d->guideSize->setRange(1, 5, 1);
    d->guideSize->setSliderEnabled(false);
    d->guideSize->setDefaultValue(1);
    d->guideColorBt->setWhatsThis(i18n("Set here the color used to draw composition guides."));
    d->guideSize->setWhatsThis(i18n("Set here the width in pixels used to draw composition guides."));

    // -------------------------------------------------------------

    grid2->addWidget(labelGuideLines,           0, 0, 1, 1);
    grid2->addWidget(d->guideLinesCB,           0, 1, 1, 2);
    grid2->addWidget(d->goldenSectionBox,       1, 0, 1, 3);
    grid2->addWidget(d->goldenSpiralSectionBox, 2, 0, 1, 3);
    grid2->addWidget(d->goldenSpiralBox,        3, 0, 1, 3);
    grid2->addWidget(d->goldenTriangleBox,      4, 0, 1, 3);
    grid2->addWidget(d->flipHorBox,             5, 0, 1, 3);
    grid2->addWidget(d->flipVerBox,             6, 0, 1, 3);
    grid2->addWidget(d->colorGuideLabel,        7, 0, 1, 1);
    grid2->addWidget(d->guideColorBt,           7, 1, 1, 1);
    grid2->addWidget(d->guideSize,              7, 2, 1, 1);
    grid2->setMargin(d->gboxSettings->spacingHint());
    grid2->setSpacing(d->gboxSettings->spacingHint());

    d->expbox->addItem(compositionGuide, SmallIcon("tools-wizard"),
                       i18n("Composition Guides"), QString("CompositionGuide"), true);
    d->expbox->addStretch();

    // -------------------------------------------------------------

    vlay->addWidget(d->expbox, 10);
    vlay->addStretch();
    vlay->setMargin(0);
    vlay->setSpacing(0);

    // -------------------------------------------------------------

    setToolSettings(d->gboxSettings);
    init();

    // -------------------------------------------------------------

    connect(d->ratioCB, SIGNAL(activated(int)),
            this, SLOT(slotRatioChanged(int)));

    connect(d->preciseCrop, SIGNAL(toggled(bool)),
            this, SLOT(slotPreciseCropChanged(bool)));

    connect(d->orientCB, SIGNAL(activated(int)),
            this, SLOT(slotOrientChanged(int)));

    connect(d->autoOrientation, SIGNAL(toggled(bool)),
            this, SLOT(slotAutoOrientChanged(bool)));

    connect(d->xInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotXChanged(int)));

    connect(d->yInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotYChanged(int)));

    connect(d->customRatioNInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotCustomRatioChanged()));

    connect(d->customRatioDInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotCustomRatioChanged()));

    connect(d->guideLinesCB, SIGNAL(activated(int)),
            this, SLOT(slotGuideTypeChanged(int)));

    connect(d->goldenSectionBox, SIGNAL(toggled(bool)),
            this, SLOT(slotGoldenGuideTypeChanged()));

    connect(d->goldenSpiralSectionBox, SIGNAL(toggled(bool)),
            this, SLOT(slotGoldenGuideTypeChanged()));

    connect(d->goldenSpiralBox, SIGNAL(toggled(bool)),
            this, SLOT(slotGoldenGuideTypeChanged()));

    connect(d->goldenTriangleBox, SIGNAL(toggled(bool)),
            this, SLOT(slotGoldenGuideTypeChanged()));

    connect(d->flipHorBox, SIGNAL(toggled(bool)),
            this, SLOT(slotGoldenGuideTypeChanged()));

    connect(d->flipVerBox, SIGNAL(toggled(bool)),
            this, SLOT(slotGoldenGuideTypeChanged()));

    connect(d->guideColorBt, SIGNAL(changed(const QColor&)),
            d->imageSelectionWidget, SLOT(slotChangeGuideColor(const QColor&)));

    connect(d->guideSize, SIGNAL(valueChanged(int)),
            d->imageSelectionWidget, SLOT(slotChangeGuideSize(int)));

    connect(d->widthInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotWidthChanged(int)));

    connect(d->heightInput, SIGNAL(valueChanged(int)),
            this, SLOT(slotHeightChanged(int)));

    connect(d->imageSelectionWidget, SIGNAL(signalSelectionChanged(const QRect&)),
            this, SLOT(slotSelectionChanged(const QRect&)));

    connect(d->imageSelectionWidget, SIGNAL(signalSelectionMoved(const QRect&)),
            this, SLOT(slotSelectionChanged(const QRect&)));

    connect(d->imageSelectionWidget, SIGNAL(signalSelectionOrientationChanged(int)),
            this, SLOT(slotSelectionOrientationChanged(int)));

    connect(d->centerWidth, SIGNAL(clicked()),
            this, SLOT(slotCenterWidth()));

    connect(d->centerHeight, SIGNAL(clicked()),
            this, SLOT(slotCenterHeight()));

    // we need to disconnect the standard connection of the Try button first
    disconnect(d->gboxSettings, SIGNAL(signalTryClicked()),
               this, SLOT(slotEffect()));

    connect(d->gboxSettings, SIGNAL(signalTryClicked()),
            this, SLOT(slotMaxAspectRatio()));
}

RatioCropTool::~RatioCropTool()
{
    delete d;
}

void RatioCropTool::readSettings()
{
    QColor defaultGuideColor(250, 250, 255);
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);

    // --------------------------------------------------------

    // Note: the selection widget has been setup to NOT draw the selection at the moment.
    // This is necessary to avoid jumping of the selection when reading the settings.
    // The drawing must be activated later on in this method to have a working selection.

    d->expbox->readSettings();

    // No guide lines per default.
    d->guideLinesCB->setCurrentIndex(group.readEntry(d->configGuideLinesTypeEntry,
                                     (int)ImageSelectionWidget::GuideNone));
    d->goldenSectionBox->setChecked(group.readEntry(d->configGoldenSectionEntry,             true));
    d->goldenSpiralSectionBox->setChecked(group.readEntry(d->configGoldenSpiralSectionEntry, false));
    d->goldenSpiralBox->setChecked(group.readEntry(d->configGoldenSpiralEntry,               false));
    d->goldenTriangleBox->setChecked(group.readEntry(d->configGoldenTriangleEntry,           false));
    d->flipHorBox->setChecked(group.readEntry(d->configGoldenFlipHorizontalEntry,            false));
    d->flipVerBox->setChecked(group.readEntry(d->configGoldenFlipVerticalEntry,              false));
    d->guideColorBt->setColor(group.readEntry(d->configGuideColorEntry,                      defaultGuideColor));
    d->guideSize->setValue(group.readEntry(d->configGuideWidthEntry,                         d->guideSize->defaultValue()));

    d->imageSelectionWidget->slotGuideLines(d->guideLinesCB->currentIndex());
    d->imageSelectionWidget->slotChangeGuideColor(d->guideColorBt->color());

    d->preciseCrop->setChecked( group.readEntry(d->configPreciseAspectRatioCropEntry, false) );
    d->imageSelectionWidget->setPreciseCrop( d->preciseCrop->isChecked() );

    // Empty selection so it can be moved w/out size constraint
    d->widthInput->setValue(0);
    d->heightInput->setValue(0);

    d->xInput->setValue(group.readEntry(d->configHorOrientedCustomAspectRatioXposEntry,
                                        d->xInput->defaultValue()));
    d->yInput->setValue(group.readEntry(d->configHorOrientedCustomAspectRatioYposEntry,
                                        d->yInput->defaultValue()));

    d->widthInput->setValue(group.readEntry(d->configHorOrientedCustomAspectRatioWidthEntry,
                                            d->widthInput->defaultValue()));
    d->heightInput->setValue(group.readEntry(d->configHorOrientedCustomAspectRatioHeightEntry,
                             d->heightInput->defaultValue()));

    d->imageSelectionWidget->setSelectionOrientation(d->orientCB->currentIndex());

    d->customRatioNInput->setValue(group.readEntry(d->configHorOrientedCustomAspectRatioNumEntry,
                                   d->customRatioNInput->defaultValue()));
    d->customRatioDInput->setValue(group.readEntry(d->configHorOrientedCustomAspectRatioDenEntry,
                                   d->customRatioDInput->defaultValue()));
    d->ratioCB->setCurrentIndex(group.readEntry(d->configHorOrientedAspectRatioEntry,
                                d->ratioCB->defaultIndex()));

    if (d->originalIsLandscape)
    {
        d->orientCB->setCurrentIndex(group.readEntry(d->configHorOrientedAspectRatioOrientationEntry,
                                     (int)ImageSelectionWidget::Landscape));
        d->orientCB->setDefaultIndex(ImageSelectionWidget::Landscape);
    }
    else
    {
        d->orientCB->setCurrentIndex(group.readEntry(d->configVerOrientedAspectRatioOrientationEntry,
                                     (int)ImageSelectionWidget::Portrait));
        d->orientCB->setDefaultIndex(ImageSelectionWidget::Portrait);
    }

    d->autoOrientation->setChecked(group.readEntry(d->configAutoOrientationEntry, false));
    slotAutoOrientChanged( d->autoOrientation->isChecked() );
    applyRatioChanges(d->ratioCB->currentIndex());

    slotXChanged(d->xInput->value());
    slotYChanged(d->yInput->value());
    slotWidthChanged(d->widthInput->value());

    // For the last setting to be applied, activate drawing in the selectionWidget,
    // so that we can see the results.
    d->imageSelectionWidget->setIsDrawingSelection(true);
    slotHeightChanged(d->heightInput->value());

    slotGuideTypeChanged(d->guideLinesCB->currentIndex());
}

void RatioCropTool::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group = config->group(d->configGroupName);

    if (d->originalIsLandscape)
    {
        group.writeEntry(d->configHorOrientedAspectRatioEntry,             d->ratioCB->currentIndex());
        group.writeEntry(d->configHorOrientedAspectRatioOrientationEntry,  d->orientCB->currentIndex());
        group.writeEntry(d->configHorOrientedCustomAspectRatioNumEntry,    d->customRatioNInput->value());
        group.writeEntry(d->configHorOrientedCustomAspectRatioDenEntry,    d->customRatioDInput->value());

        group.writeEntry(d->configHorOrientedCustomAspectRatioXposEntry,   d->xInput->value());
        group.writeEntry(d->configHorOrientedCustomAspectRatioYposEntry,   d->yInput->value());
        group.writeEntry(d->configHorOrientedCustomAspectRatioWidthEntry,  d->widthInput->value());
        group.writeEntry(d->configHorOrientedCustomAspectRatioHeightEntry, d->heightInput->value());
    }
    else
    {
        group.writeEntry(d->configVerOrientedAspectRatioEntry,             d->ratioCB->currentIndex());
        group.writeEntry(d->configVerOrientedAspectRatioOrientationEntry,  d->orientCB->currentIndex());
        group.writeEntry(d->configVerOrientedCustomAspectRatioNumEntry,    d->customRatioNInput->value());
        group.writeEntry(d->configVerOrientedCustomAspectRatioDenEntry,    d->customRatioDInput->value());

        group.writeEntry(d->configVerOrientedCustomAspectRatioXposEntry,   d->xInput->value());
        group.writeEntry(d->configVerOrientedCustomAspectRatioYposEntry,   d->yInput->value());
        group.writeEntry(d->configVerOrientedCustomAspectRatioWidthEntry,  d->widthInput->value());
        group.writeEntry(d->configVerOrientedCustomAspectRatioHeightEntry, d->heightInput->value());
    }

    group.writeEntry(d->configPreciseAspectRatioCropEntry, d->preciseCrop->isChecked());
    group.writeEntry(d->configAutoOrientationEntry,        d->autoOrientation->isChecked());
    group.writeEntry(d->configGuideLinesTypeEntry,         d->guideLinesCB->currentIndex());
    group.writeEntry(d->configGoldenSectionEntry,          d->goldenSectionBox->isChecked());
    group.writeEntry(d->configGoldenSpiralSectionEntry,    d->goldenSpiralSectionBox->isChecked());
    group.writeEntry(d->configGoldenSpiralEntry,           d->goldenSpiralBox->isChecked());
    group.writeEntry(d->configGoldenTriangleEntry,         d->goldenTriangleBox->isChecked());
    group.writeEntry(d->configGoldenFlipHorizontalEntry,   d->flipHorBox->isChecked());
    group.writeEntry(d->configGoldenFlipVerticalEntry,     d->flipVerBox->isChecked());
    group.writeEntry(d->configGuideColorEntry,             d->guideColorBt->color());
    group.writeEntry(d->configGuideWidthEntry,             d->guideSize->value());
    group.sync();
}

void RatioCropTool::slotResetSettings()
{
    d->imageSelectionWidget->resetSelection();
}

void RatioCropTool::slotMaxAspectRatio()
{
    d->imageSelectionWidget->maxAspectSelection();
}

void RatioCropTool::slotCenterWidth()
{
    d->imageSelectionWidget->setCenterSelection(ImageSelectionWidget::CenterWidth);
}

void RatioCropTool::slotCenterHeight()
{
    d->imageSelectionWidget->setCenterSelection(ImageSelectionWidget::CenterHeight);
}

void RatioCropTool::slotSelectionChanged(const QRect& rect)
{
    blockWidgetSignals(true);

    d->xInput->setRange(0, d->imageSelectionWidget->getOriginalImageWidth() - rect.width(), 1);
    d->yInput->setRange(0, d->imageSelectionWidget->getOriginalImageHeight() - rect.height(), 1);
    d->widthInput->setRange(d->imageSelectionWidget->getMinWidthRange(),
                            d->imageSelectionWidget->getMaxWidthRange(),
                            d->imageSelectionWidget->getWidthStep());
    d->heightInput->setRange(d->imageSelectionWidget->getMinHeightRange(),
                             d->imageSelectionWidget->getMaxHeightRange(),
                             d->imageSelectionWidget->getHeightStep());

    d->xInput->setValue(rect.x());
    d->yInput->setValue(rect.y());
    d->widthInput->setValue(rect.width());
    d->heightInput->setValue(rect.height());

    d->gboxSettings->enableButton(EditorToolSettings::Ok,
                                  rect.isValid());

    d->preciseCrop->setEnabled(d->imageSelectionWidget->preciseCropAvailable());

    blockWidgetSignals(false);
}

void RatioCropTool::setRatioCBText(int orientation)
{
    int item = d->ratioCB->currentIndex();
    d->ratioCB->blockSignals(true);
    d->ratioCB->combo()->clear();
    d->ratioCB->addItem(i18nc("custom ratio crop settings", "Custom"));
    d->ratioCB->addItem("1:1");

    if (orientation == ImageSelectionWidget::Landscape)
    {
        d->ratioCB->addItem("3:2");
        d->ratioCB->addItem("4:3");
        d->ratioCB->addItem("5:4");
        d->ratioCB->addItem("7:5");
        d->ratioCB->addItem("10:7");
        d->ratioCB->addItem("5:8");
    }
    else
    {
        d->ratioCB->addItem("2:3");
        d->ratioCB->addItem("3:4");
        d->ratioCB->addItem("4:5");
        d->ratioCB->addItem("5:7");
        d->ratioCB->addItem("7:10");
        d->ratioCB->addItem("8:5");
    }

    d->ratioCB->addItem(i18n("Golden Ratio"));
    d->ratioCB->addItem(i18n("Current aspect ratio"));
    d->ratioCB->addItem(i18nc("no aspect ratio", "None"));
    d->ratioCB->setCurrentIndex(item);
    d->ratioCB->blockSignals(false);
}

void RatioCropTool::slotSelectionOrientationChanged(int newOrientation)
{
    // Change text for Aspect ratio ComboBox

    setRatioCBText(newOrientation);

    // Change Orientation ComboBox

    d->orientCB->setCurrentIndex(newOrientation);

    // Reverse custom values

    if ( ( d->customRatioNInput->value() < d->customRatioDInput->value() &&
           newOrientation == ImageSelectionWidget::Landscape ) ||
         ( d->customRatioNInput->value() > d->customRatioDInput->value() &&
           newOrientation == ImageSelectionWidget::Portrait ) )
    {
        d->customRatioNInput->blockSignals(true);
        d->customRatioDInput->blockSignals(true);

        int tmp = d->customRatioNInput->value();
        d->customRatioNInput->setValue(d->customRatioDInput->value());
        d->customRatioDInput->setValue(tmp);

        d->customRatioNInput->blockSignals(false);
        d->customRatioDInput->blockSignals(false);
    }
}

void RatioCropTool::slotXChanged(int x)
{
    d->imageSelectionWidget->setSelectionX(x);
}

void RatioCropTool::slotYChanged(int y)
{
    d->imageSelectionWidget->setSelectionY(y);
}

void RatioCropTool::slotWidthChanged(int w)
{
    d->imageSelectionWidget->setSelectionWidth(w);
}

void RatioCropTool::slotHeightChanged(int h)
{
    d->imageSelectionWidget->setSelectionHeight(h);
}

void RatioCropTool::slotPreciseCropChanged(bool a)
{
    d->imageSelectionWidget->setPreciseCrop(a);
}

void RatioCropTool::slotOrientChanged(int o)
{
    d->imageSelectionWidget->setSelectionOrientation(o);

    // Reset selection area.
    slotResetSettings();
}

void RatioCropTool::slotAutoOrientChanged(bool a)
{
    d->orientCB->setEnabled(!a /*|| d->ratioCB->currentIndex() == ImageSelectionWidget::RATIONONE*/);
    d->imageSelectionWidget->setAutoOrientation(a);
}

void RatioCropTool::slotRatioChanged(int a)
{
    applyRatioChanges(a);

    // Reset selection area.
    slotResetSettings();
}

void RatioCropTool::applyRatioChanges(int a)
{
    d->imageSelectionWidget->setSelectionAspectRatioType(a);

    if (a == ImageSelectionWidget::RATIOCUSTOM)
    {
        d->customLabel->setEnabled(true);
        d->customRatioNInput->setEnabled(true);
        d->customRatioDInput->setEnabled(true);
        d->orientLabel->setEnabled(true);
        d->orientCB->setEnabled(!d->autoOrientation->isChecked());
        d->autoOrientation->setEnabled(true);
        slotCustomRatioChanged();
    }
    else if (a == ImageSelectionWidget::RATIONONE)
    {
        d->orientLabel->setEnabled(false);
        d->orientCB->setEnabled(false);
        d->autoOrientation->setEnabled(false);
        d->customLabel->setEnabled(false);
        d->customRatioNInput->setEnabled(false);
        d->customRatioDInput->setEnabled(false);
    }
    else // Pre-config ratio selected.
    {
        d->orientLabel->setEnabled(true);
        d->orientCB->setEnabled(!d->autoOrientation->isChecked());
        d->autoOrientation->setEnabled(true);
        d->customLabel->setEnabled(false);
        d->customRatioNInput->setEnabled(false);
        d->customRatioDInput->setEnabled(false);
    }
}

void RatioCropTool::slotGuideTypeChanged(int t)
{
    d->goldenSectionBox->setEnabled(false);
    d->goldenSpiralSectionBox->setEnabled(false);
    d->goldenSpiralBox->setEnabled(false);
    d->goldenTriangleBox->setEnabled(false);
    d->flipHorBox->setEnabled(false);
    d->flipVerBox->setEnabled(false);
    d->colorGuideLabel->setEnabled(true);
    d->guideColorBt->setEnabled(true);
    d->guideSize->setEnabled(true);

    switch (t)
    {
        case ImageSelectionWidget::GuideNone:
            d->colorGuideLabel->setEnabled(false);
            d->guideColorBt->setEnabled(false);
            d->guideSize->setEnabled(false);
            break;
        case ImageSelectionWidget::HarmoniousTriangles:
            d->flipHorBox->setEnabled(true);
            d->flipVerBox->setEnabled(true);
            break;
        case ImageSelectionWidget::GoldenMean:
            d->flipHorBox->setEnabled(true);
            d->flipVerBox->setEnabled(true);
            d->goldenSectionBox->setEnabled(true);
            d->goldenSpiralSectionBox->setEnabled(true);
            d->goldenSpiralBox->setEnabled(true);
            d->goldenTriangleBox->setEnabled(true);
            break;
    }

    d->imageSelectionWidget->setGoldenGuideTypes(d->goldenSectionBox->isChecked(),
            d->goldenSpiralSectionBox->isChecked(),
            d->goldenSpiralBox->isChecked(),
            d->goldenTriangleBox->isChecked(),
            d->flipHorBox->isChecked(),
            d->flipVerBox->isChecked());
    d->imageSelectionWidget->slotGuideLines(t);
}

void RatioCropTool::slotGoldenGuideTypeChanged()
{
    slotGuideTypeChanged(d->guideLinesCB->currentIndex());
}

void RatioCropTool::slotCustomNRatioChanged(int a)
{
    if ( ! d->autoOrientation->isChecked() )
    {
        if ( ( d->orientCB->currentIndex() == ImageSelectionWidget::Portrait &&
               d->customRatioDInput->value() < a ) ||
             ( d->orientCB->currentIndex() == ImageSelectionWidget::Landscape &&
               d->customRatioDInput->value() > a ) )
        {
            d->customRatioDInput->blockSignals(true);
            d->customRatioDInput->setValue(a);
            d->customRatioDInput->blockSignals(false);
        }
    }

    slotCustomRatioChanged();
}

void RatioCropTool::slotCustomDRatioChanged(int a)
{
    if ( ! d->autoOrientation->isChecked() )
    {
        if ( ( d->orientCB->currentIndex() == ImageSelectionWidget::Landscape &&
               d->customRatioNInput->value() < a ) ||
             ( d->orientCB->currentIndex() == ImageSelectionWidget::Portrait &&
               d->customRatioNInput->value() > a ) )
        {
            d->customRatioNInput->blockSignals(true);
            d->customRatioNInput->setValue(a);
            d->customRatioNInput->blockSignals(false);
        }
    }

    slotCustomRatioChanged();
}

void RatioCropTool::slotCustomRatioChanged()
{
    d->imageSelectionWidget->setSelectionAspectRatioValue(d->customRatioNInput->value(),
            d->customRatioDInput->value());

    // Reset selection area.
    slotResetSettings();
}

void RatioCropTool::finalRendering()
{
    kapp->setOverrideCursor( Qt::WaitCursor );

    QRect currentRegion    = d->imageSelectionWidget->getRegionSelection();
    ImageIface* iface      = d->imageSelectionWidget->imageIface();
    uchar* data            = iface->getOriginalImage();
    int w                  = iface->originalWidth();
    int h                  = iface->originalHeight();
    bool a                 = iface->originalHasAlpha();
    bool sb                = iface->originalSixteenBit();
    QRect normalizedRegion = currentRegion.normalized();

    if (normalizedRegion.right() > w)
    {
        normalizedRegion.setRight(w);
    }

    if (normalizedRegion.bottom() > h)
    {
        normalizedRegion.setBottom(h);
    }

    DImg imOrg(w, h, sb, a, data);
    delete [] data;
    imOrg.crop(normalizedRegion);

    FilterAction action("digikam:RatioCrop", 1);
    action.setDisplayableName(i18n("Aspect Ratio Crop"));

    action.addParameter("x", currentRegion.x());
    action.addParameter("y", currentRegion.y());
    action.addParameter("width", currentRegion.width());
    action.addParameter("height", currentRegion.height());

    iface->putOriginalImage(i18n("Aspect Ratio Crop"), action, imOrg.bits(), imOrg.width(), imOrg.height());

    kapp->restoreOverrideCursor();
    writeSettings();
}

void RatioCropTool::blockWidgetSignals(bool b)
{
    d->customRatioDInput->blockSignals(b);
    d->customRatioNInput->blockSignals(b);
    d->flipHorBox->blockSignals(b);
    d->flipVerBox->blockSignals(b);
    d->goldenSectionBox->blockSignals(b);
    d->goldenSpiralBox->blockSignals(b);
    d->goldenSpiralSectionBox->blockSignals(b);
    d->goldenTriangleBox->blockSignals(b);
    d->guideLinesCB->blockSignals(b);
    d->heightInput->blockSignals(b);
    d->imageSelectionWidget->blockSignals(b);
    d->preciseCrop->blockSignals(b);
    d->widthInput->blockSignals(b);
    d->xInput->blockSignals(b);
    d->yInput->blockSignals(b);
}

}  // namespace DigikamTransformImagePlugin

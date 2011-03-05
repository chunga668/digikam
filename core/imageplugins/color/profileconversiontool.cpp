/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-09-19
 * Description : a tool for color space conversion
 *
 * Copyright (C) 2009-2010 by Marcel Wiesweg <marcel.wiesweg@gmx.de>
 * Copyright (C) 2009-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "profileconversiontool.moc"

// Qt includes

#include <QGridLayout>
#include <QLabel>
#include <QPushButton>

// KDE includes

#include <kapplication.h>
#include <kconfig.h>
#include <kcursor.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <klocale.h>

// Local includes

#include "editortoolsettings.h"
#include "iccprofileinfodlg.h"
#include "iccprofilesettings.h"
#include "iccsettings.h"
#include "iccsettingscontainer.h"
#include "icctransform.h"
#include "icctransformfilter.h"
#include "histogramwidget.h"
#include "imageiface.h"
#include "imageregionwidget.h"
#include "dmetadata.h"

namespace DigikamColorImagePlugin
{

class ProfileConversionTool::ProfileConversionToolPriv
{
public:

    ProfileConversionToolPriv() :
        destinationPreviewData(0),
        profilesBox(0),
        previewWidget(0),
        gboxSettings(0)
    {
    }

    static const QString configGroupName;
    static const QString configProfileEntry;

    uchar*               destinationPreviewData;

    IccProfilesSettings* profilesBox;

    ImageRegionWidget*   previewWidget;
    EditorToolSettings*  gboxSettings;

    IccProfile           currentProfile;

    IccTransform         transform;

public:

    static IccTransform getTransform(const IccProfile& in, const IccProfile& out);
};
const QString ProfileConversionTool::ProfileConversionToolPriv::configGroupName("Profile Conversion Tool");
const QString ProfileConversionTool::ProfileConversionToolPriv::configProfileEntry("Profile");

// --------------------------------------------------------

IccTransform ProfileConversionTool::ProfileConversionToolPriv::getTransform(const IccProfile& in, const IccProfile& out)
{
    ICCSettingsContainer settings = IccSettings::instance()->settings();

    IccTransform transform;
    transform.setIntent(settings.renderingIntent);
    transform.setUseBlackPointCompensation(settings.useBPC);

    transform.setInputProfile(in);
    transform.setOutputProfile(out);

    return transform;
}

// ----------------------------------------------------------------------------

ProfileConversionTool::ProfileConversionTool(QObject* parent)
    : EditorToolThreaded(parent),
      d(new ProfileConversionToolPriv)
{
    setObjectName("profile conversion");
    setToolName(i18n("Color Profile Conversion"));
    setToolIcon(SmallIcon("colormanagement"));
    //TODO setToolHelp("colormanagement.anchor");

    // -------------------------------------------------------------

    ImageIface iface(0, 0);
    d->currentProfile = iface.getOriginalIccProfile();

    d->gboxSettings = new EditorToolSettings;
    d->gboxSettings->setButtons(EditorToolSettings::Ok|
                                EditorToolSettings::Cancel);

    d->gboxSettings->setTools(EditorToolSettings::Histogram);
    d->gboxSettings->setHistogramType(LRGBA);

    // -------------------------------------------------------------

    QGridLayout* grid            = new QGridLayout(d->gboxSettings->plainPage());
    QLabel* currentProfileTitle  = new QLabel;
    QLabel* currentProfileDesc   = new QLabel;
    QPushButton* currentProfInfo = new QPushButton(i18n("Info..."));
    d->profilesBox               = new IccProfilesSettings;

    currentProfileTitle->setText(i18n("Current Color Space:"));
    currentProfileDesc->setText(QString("<b>%1</b>").arg(d->currentProfile.description()));
    currentProfileDesc->setWordWrap(true);

    grid->addWidget(currentProfileTitle, 0, 0, 1, 5);
    grid->addWidget(currentProfileDesc,  1, 0, 1, 5);
    grid->addWidget(currentProfInfo,     2, 0, 1, 1);
    grid->addWidget(d->profilesBox,      3, 0, 1, 5);
    grid->setRowStretch(4, 10);
    grid->setMargin(d->gboxSettings->spacingHint());
    grid->setSpacing(d->gboxSettings->spacingHint());

    // -------------------------------------------------------------

    d->previewWidget = new ImageRegionWidget;

    setToolSettings(d->gboxSettings);
    setToolView(d->previewWidget);
    setPreviewModeMask(PreviewToolBar::AllPreviewModes);

    init();

    // -------------------------------------------------------------

    connect(d->previewWidget, SIGNAL(signalOriginalClipFocusChanged()),
            this, SLOT(slotTimer()));

    connect(currentProfInfo, SIGNAL(clicked()),
            this, SLOT(slotCurrentProfInfo()));

    connect(d->profilesBox, SIGNAL(signalSettingsChanged()),
            this, SLOT(slotProfileChanged()));
}

ProfileConversionTool::~ProfileConversionTool()
{
    if (d->destinationPreviewData)
    {
        delete [] d->destinationPreviewData;
    }

    delete d;
}

void ProfileConversionTool::slotCurrentProfInfo()
{
    ICCProfileInfoDlg infoDlg(kapp->activeWindow(), QString(), d->currentProfile);
    infoDlg.exec();
}

void ProfileConversionTool::slotProfileChanged()
{
    d->gboxSettings->enableButton(EditorToolSettings::Ok, !d->profilesBox->currentProfile().isNull());
    updateTransform();
    slotTimer();
}

void ProfileConversionTool::updateTransform()
{
    d->transform = d->getTransform(d->currentProfile, d->profilesBox->currentProfile());
}

void ProfileConversionTool::readSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);
    d->profilesBox->setCurrentProfile(group.readPathEntry(d->configProfileEntry, d->currentProfile.filePath()));
    d->profilesBox->readSettings(group);
}

void ProfileConversionTool::writeSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d->configGroupName);
    group.writePathEntry(d->configProfileEntry, d->profilesBox->currentProfile().filePath());
    d->profilesBox->writeSettings(group);
    config->sync();
}

void ProfileConversionTool::slotResetSettings()
{
    d->profilesBox->resetToDefault();
}

void ProfileConversionTool::prepareEffect()
{
    DImg preview = d->previewWidget->getOriginalRegionImage(true);
    setFilter(new IccTransformFilter(&preview, this, d->transform));
}

void ProfileConversionTool::putPreviewData()
{
    DImg preview = filter()->getTargetImage();
    d->previewWidget->setPreviewImage(preview);

    // Update histogram.

    if (d->destinationPreviewData)
    {
        delete [] d->destinationPreviewData;
    }

    d->destinationPreviewData = preview.copyBits();
    d->gboxSettings->histogramBox()->histogram()->updateData(d->destinationPreviewData,
            preview.width(), preview.height(), preview.sixteenBit(),
            0, 0, 0, false);
}

void ProfileConversionTool::prepareFinal()
{
    ImageIface iface(0, 0);
    setFilter(new IccTransformFilter(iface.getOriginalImg(), this, d->transform));
}

void ProfileConversionTool::putFinalData()
{
    ImageIface iface(0, 0);
    DImg imDest = filter()->getTargetImage();

    iface.putOriginalImage(i18n("Color Profile Conversion"), filter()->filterAction(), imDest.bits());
    iface.putOriginalIccProfile(imDest.getIccProfile());

    DMetadata meta(iface.getOriginalMetadata());
    meta.removeExifColorSpace();
    iface.setOriginalMetadata(meta.data());
}

// Static Methods.

QStringList ProfileConversionTool::favoriteProfiles()
{
    ProfileConversionToolPriv d;
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(d.configGroupName);
    return IccProfilesSettings::favoriteProfiles(group);
}

void ProfileConversionTool::fastConversion(const IccProfile& profile)
{
    ImageIface iface(0, 0);

    IccProfile currentProfile = iface.getOriginalIccProfile();
    IccTransform transform    = ProfileConversionToolPriv::getTransform(currentProfile, profile);
    IccTransformFilter filter(iface.getOriginalImg(), 0, transform);
    filter.startFilterDirectly();

    DImg imDest               = filter.getTargetImage();
    iface.putOriginalImage(i18n("Color Profile Conversion"), filter.filterAction(), imDest.bits());
    iface.putOriginalIccProfile(imDest.getIccProfile());

    DMetadata meta(iface.getOriginalMetadata());
    meta.removeExifColorSpace();
    iface.setOriginalMetadata(meta.data());
}

}  // namespace DigikamColorImagePlugin

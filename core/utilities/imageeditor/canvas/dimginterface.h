/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2003-01-15
 * Description : DImg interface for image editor
 *
 * Copyright (C) 2004-2005 by Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Copyright (C) 2004-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef DIMGINTERFACE_H
#define DIMGINTERFACE_H

// Qt includes

#include <QObject>
#include <QString>
#include <QByteArray>

// Local includes

#include "digikam_export.h"
#include "dimg.h"
#include "dimagehistory.h"
#include "versionmanager.h"

class QWidget;
class QPixmap;

namespace Digikam
{

class ICCSettingsContainer;
class ExposureSettingsContainer;
class IOFileSettingsContainer;
class LoadingDescription;
class DImgInterfacePrivate;
class DImgBuiltinFilter;
class UndoAction;
class VersionFileOperation;

class DIGIKAM_EXPORT DImgInterface : public QObject
{
    Q_OBJECT

public:

    static DImgInterface* defaultInterface();
    static void setDefaultInterface(DImgInterface* defaultInterface);

    DImgInterface();
    ~DImgInterface();

    void   load(const QString& filename, IOFileSettingsContainer* iofileSettings);
    void   applyTransform(const IccTransform& transform);
    void   updateColorManagement();
    void   setSoftProofingEnabled(bool enabled);

    void   setICCSettings(const ICCSettingsContainer& cmSettings);
    ICCSettingsContainer getICCSettings();

    void   setExposureSettings(ExposureSettingsContainer* expoSettings);
    ExposureSettingsContainer* getExposureSettings();

    void   setExifOrient(bool exifOrient);
    void   setDisplayingWidget(QWidget* widget);

    void   undo();
    void   redo();
    void   restore();
    void   rollbackToOrigin();

    void   saveAs(const QString& file, IOFileSettingsContainer* iofileSettings,
                  bool setExifOrientationTag, const QString& mimeType=QString());
    void   saveAs(const QString& file, IOFileSettingsContainer* iofileSettings,
                  bool setExifOrientationTag, const QString& mimeType,
                  const VersionFileOperation& operation);

    void   setHistoryIsBranch(bool isBranching);
    void   setLastSaved(const QString& filePath);
    void   switchToLastSaved(const DImageHistory& resolvedCurrentHistory = DImageHistory());
    void   abortSaving();
    void   setModified();
    void   readMetadataFromFile(const QString& file);
    void   clearUndoManager();
    void   setUndoManagerOrigin();
    void   updateUndoState();
    void   resetImage();
    bool   hasChangesToSave();
    QString ensureHasCurrentUuid();
    void   provideCurrentUuid(const QString& uuid);

    void   zoom(double val);

    void   paintOnDevice(QPaintDevice* p,
                         int sx, int sy, int sw, int sh,
                         int dx, int dy, int dw, int dh,
                         int antialias);
    void   paintOnDevice(QPaintDevice* p,
                         int sx, int sy, int sw, int sh,
                         int dx, int dy, int dw, int dh,
                         int mx, int my, int mw, int mh,
                         int antialias);

    bool   imageValid();
    int    width();
    int    height();
    int    origWidth();
    int    origHeight();
    int    bytesDepth();
    bool   hasAlpha();
    bool   sixteenBit();
    bool   exifRotated();
    bool   isReadOnly();

    void   setSelectedArea(int x, int y, int w, int h);
    void   getSelectedArea(int& x, int& y, int& w, int& h);

    void   rotate90();
    void   rotate180();
    void   rotate270();

    void   flipHoriz();
    void   flipVert();

    void   crop(int x, int y, int w, int h);

    void   resize(int w, int h);

    void   convertDepth(int depth);

    QStringList getUndoHistory() const;
    QStringList getRedoHistory() const;
    int    availableUndoSteps() const;
    int    availableRedoSteps() const;

    DImg*  getImg();
    uchar* getImage();

    void   putImage(const QString& caller, const FilterAction& action, uchar* data, int w, int h);
    void   putImage(const QString& caller, const FilterAction& action, uchar* data, int w, int h, bool sixteenBit);

    uchar* getImageSelection();
    void   putImageSelection(const QString& caller, const FilterAction& action, uchar* data);

    void   putIccProfile(const IccProfile& profile);

    /// For internal usage by UndoManager
    void   setUndoImageData(const DImageHistory& history, uchar* data, int w, int h, bool sixteenBit);
    void   imageUndoChanged(const DImageHistory& history);
    void   setFileOriginData(const QVariant& data);

    /** Convert a DImg image to a pixmap for screen using color
        managed view if necessary */
    QPixmap               convertToPixmap(DImg& img);

    IccProfile            getEmbeddedICC();
    KExiv2Data            getMetadata();
    DImageHistory         getImageHistory();
    DImageHistory         getInitialImageHistory();
    DImageHistory         getImageHistoryOfFullRedo();
    DImageHistory         getResolvedInitialHistory();
    void                  setResolvedInitialHistory(const DImageHistory& history);

    QString               getImageFileName();
    QString               getImageFilePath();
    QString               getImageFormat();

protected Q_SLOTS:

    void   slotImageLoaded(const LoadingDescription& loadingDescription, const DImg& img);
    void   slotImageSaved(const QString& filePath, bool success);
    void   slotLoadingProgress(const LoadingDescription& loadingDescription, float progress);
    void   slotSavingProgress(const QString& filePath, float progress);

Q_SIGNALS:

    void   signalModified();
    void   signalUndoStateChanged(bool moreUndo, bool moreRedo, bool canSave);
    void   signalFileOriginChanged(const QString& filePath);

    void   signalLoadingStarted(const QString& filename);
    void   signalLoadingProgress(const QString& filePath, float progress);
    void   signalImageLoaded(const QString& filePath, bool success);
    void   signalSavingStarted(const QString& filename);
    void   signalSavingProgress(const QString& filePath, float progress);
    void   signalImageSaved(const QString& filePath, bool success);

private Q_SLOTS:

    void slotLoadRawFromTool();
    void slotLoadRaw();

private:

    void   putImageData(uchar* data, int w, int h, bool sixteenBit);
    void   applyBuiltinFilter(const DImgBuiltinFilter& filter, UndoAction* action);
    void   applyReversibleBuiltinFilter(const DImgBuiltinFilter& filter);

    void   load(const LoadingDescription& description);
    void   loadCurrent();
    void   resetValues();
    void   saveNext();
    QMap<QString,QVariant> ioAttributes(IOFileSettingsContainer* iofileSettings, const QString& givenMimeType);

private:

    static DImgInterface* m_defaultInterface;

    DImgInterfacePrivate* const d;
};

}  // namespace Digikam

#endif /* DIMGINTERFACE_H */

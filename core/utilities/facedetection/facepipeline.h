/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-09-03
 * Description : Integrated, multithread face detection / recognition
 *
 * Copyright (C) 2010-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef FACEDETECTOR_H
#define FACEDETECTOR_H

// Qt includes

#include <QFlags>

// KDE includes

#include <kdebug.h>

// libkface includes

#include <libkface/face.h>

// Local includes

#include "databaseface.h"
#include "dimg.h"
#include "imageinfo.h"

namespace Digikam
{

class FaceIface;

class FacePipelineDatabaseFace : public DatabaseFace
{
public:

    enum Role
    {
        NoRole             = 0,

        /// Source
        GivenAsArgument    = 1 << 0,
        ReadFromDatabase   = 1 << 1,
        DetectedFromImage  = 1 << 2,

        /// Task
        ForConfirmation    = 1 << 10,
        ForTraining        = 1 << 11,
        ForRemoval         = 1 << 12,

        /// Executed action (task is cleared)
        Confirmed          = 1 << 20,
        Trained            = 1 << 21,
        Removed            = 1 << 22
    };
    Q_DECLARE_FLAGS(Roles, Role)

public:

    FacePipelineDatabaseFace();
    FacePipelineDatabaseFace(const DatabaseFace& face);

public:

    Roles     roles;
    int       assignedTagId;
    TagRegion assignedRegion;
};

class FacePipelineDatabaseFaceList : public QList<FacePipelineDatabaseFace>
{
public:

    FacePipelineDatabaseFaceList();
    FacePipelineDatabaseFaceList(const QList<DatabaseFace>& faces);

    FacePipelineDatabaseFaceList& operator=(const QList<DatabaseFace>& faces);

    void setRole(FacePipelineDatabaseFace::Roles role);
    void clearRole(FacePipelineDatabaseFace::Roles role);
    void replaceRole(FacePipelineDatabaseFace::Roles remove, FacePipelineDatabaseFace::Roles add);

    QList<DatabaseFace> toDatabaseFaceList() const;

    FacePipelineDatabaseFaceList facesForRole(FacePipelineDatabaseFace::Roles role) const;
};

class FacePipelinePackage
{
public:

    enum ProcessFlag
    {
        NotProcessed            = 0,
        PreviewImageLoaded      = 1 << 0,
        ProcessedByDetector     = 1 << 1,
        ProcessedByRecognizer   = 1 << 2,
        WrittenToDatabase       = 1 << 3,
        ProcessedByTrainer      = 1 << 4
    };
    Q_DECLARE_FLAGS(ProcessFlags, ProcessFlag)

public:

    FacePipelinePackage()
        : processFlags(NotProcessed)
    {
    }

public:

    ImageInfo                    info;
    DImg                         image;
    QList<KFaceIface::Face>      faces;
    FacePipelineDatabaseFaceList databaseFaces;

    ProcessFlags                 processFlags;
};

// ------------------------------------------------------------------------------------

class FacePipeline : public QObject
{
    Q_OBJECT

public:

    FacePipeline();
    ~FacePipeline();

    enum FilterMode
    {
        /// Will read any given image
        ScanAll,
        /// Will skip any image that is already marked as scanned
        SkipAlreadyScanned,
        /// Will read unconfirmed faces for recognition
        ReadUnconfirmedFaces,
        /// Will read faces marked for training
        ReadFacesForTraining,
        /// Will read faces which are confirmed
        ReadConfirmedFaces
    };

    enum WriteMode
    {
        /// Write results. Merge with existing entries.
        NormalWrite,
        /// Add new results. Previous unconfirmed results will be cleared.
        OverwriteUnconfirmed
    };

    /**
     * You can plug these four different steps in the working pipeline.
     * 1) Call any of the four plug...() methods. See below for supported combinations.
     * 2) Call construct() to set up the pipeline.
     *
     * - Database filter: Prepares database records and/or filters out items.
     *   See FilterMode for specification.
     * - Preview loader: If no preview loader is plugged, you must provide
     *   a DImg for face detection and recognition
     * - Face Detector: If no recognizer is plugged, all detected face are marked
     *   as the unknown person
     * - Face Recognizer: If no detector is plugged, only already scanned faces
     *   marked as unknown will be processed. They are implicitly read from the database.
     * - DatabaseWriter: Writes the detection and recognition results to the database.
     *   The trainer works on a completely different storage and is not affected by the database writer.
     * - DatabaseEditor: Can confirm or reject faces
     *
     * PlugParallel: You can call this instead of the simple plugging method.
     * Depending on the number of processor cores of the machine and the memory cost,
     * more than one element may be plugged and process parallelly for this part of the pipeline.
     *
     * Supported combinations:
     *  (Database Filter ->) (Preview Loader ->) Detector -> Recognizer (-> DatabaseWriter)
     *  (Database Filter ->) (Preview Loader ->) Detector (-> DatabaseWriter)
     *  (Database Filter ->) (Preview Loader ->) Recognizer (-> DatabaseWriter)
     *  DatabaseEditor
     *  Trainer
     *  DatabaseEditor -> Trainer
     */

    void plugDatabaseFilter(FilterMode mode);
    void plugRetrainingDatabaseFilter();
    void plugPreviewLoader();
    void plugFaceDetector();
    void plugParallelFaceDetectors();
    void plugFaceRecognizer();
    void plugDatabaseWriter(WriteMode mode);
    void plugDatabaseEditor();
    void plugTrainer();
    void construct();

    /** Cancels all processing */
    void cancel();

    bool hasFinished() const;

public Q_SLOTS:

    /**
     * Processes the given image info. If a filter is installed,
     * returns false if the info is skipped, or true if it is processed.
     * If no preview loader is plugged, you must provide a DImg for detection or recognition.
     * Any of the signals below will only be emitted if true is returned.
     */
    bool process(const ImageInfo& info);
    bool process(const ImageInfo& info, const DImg& image);

    /**
     * Confirm the face. Pass the original face, and additionally tag id or region
     * if they changed. Returns the confirmed face entry immediately purely for convenience,
     * it is not yet in the database (connect to signal processed() to react when the processing finished).
     * If a trainer is plugged, the face will be trained.
     */
    DatabaseFace confirm(const ImageInfo& info, const DatabaseFace& face,
                         int assignedTagId = 0, const TagRegion& assignedRegion = TagRegion());
    DatabaseFace confirm(const ImageInfo& info, const DatabaseFace& face, const DImg& image,
                         int assignedTagId = 0, const TagRegion& assignedRegion = TagRegion());
    /**
     * Train the given faces.
     */
    void train(const ImageInfo& info, const QList<DatabaseFace> &faces);
    void train(const ImageInfo& info, const QList<DatabaseFace> &faces, const DImg& image);
    /**
     * Remove the given face.
     */
    void remove(const ImageInfo& info, const DatabaseFace& face);

    /**
     * Batch processing. If a filter is installed, the skipped() signal
     * will inform about skipped infos. Filtering is done in a thread, returns immediately.
     * Some of the signals below will be emitted in any case.
     */
    void process(const QList<ImageInfo>& infos);

    void setDetectionAccuracy(double accuracy);
    void setDetectionSpecificity(double specificity);

Q_SIGNALS:

    /// Emitted when one package has finished processing
    void processed(const FacePipelinePackage& package);
    /// Emitted when the last package has finished processing
    void finished();
    /// Emitted when one or several packages were skipped, usually because they have already been scanned.
    void skipped(const QList<ImageInfo>& skippedInfos);

public:

    class FacePipelinePriv;

private:

    FacePipelinePriv* const d;
    friend class FacePipelinePriv;
};

} // namespace Digikam

Q_DECLARE_METATYPE(Digikam::FacePipelinePackage)
Q_DECLARE_OPERATORS_FOR_FLAGS(Digikam::FacePipelineDatabaseFace::Roles)
Q_DECLARE_OPERATORS_FOR_FLAGS(Digikam::FacePipelinePackage::ProcessFlags)

#endif // FACEDETECTOR_H

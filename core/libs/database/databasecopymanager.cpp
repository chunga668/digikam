/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-11-14
 * Description : database migration dialog
 *
 * Copyright (C) 2009-2010 by Holger Foerster <Hamsi2k at freenet dot de>
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

#include "databasecopymanager.moc"

// QT includes

#include <QSqlError>
#include <QSqlDriver>

// KDE Includes

#include "kdebug.h"
#include "klocale.h"

// Local includes

#include "databasecorebackend.h"
#include "databaseparameters.h"
#include "albumdb.h"
#include "schemaupdater.h"

namespace Digikam
{

DatabaseCopyManager::DatabaseCopyManager()
{
}

DatabaseCopyManager::~DatabaseCopyManager()
{
}

void DatabaseCopyManager::stopProcessing()
{
    isStopProcessing=true;
}

void DatabaseCopyManager::copyDatabases(DatabaseParameters fromDBParameters, DatabaseParameters toDBParameters)
{
    isStopProcessing = false;
    DatabaseLocking fromLocking;
    DatabaseBackend fromDBbackend(&fromLocking, "MigrationFromDatabase");

    if (!fromDBbackend.open(fromDBParameters))
    {
        emit finished(DatabaseCopyManager::failed, i18n("Error while opening the source database."));
        return;
    }

    DatabaseLocking toLocking;
    DatabaseBackend toDBbackend(&toLocking, "MigrationToDatabase");

    if (!toDBbackend.open(toDBParameters))
    {
        emit finished(DatabaseCopyManager::failed, i18n("Error while opening the target database."));
        fromDBbackend.close();
        return;
    }

    QMap<QString, QVariant> bindingMap;

    // Delete all tables
    if (toDBbackend.execDBAction(toDBbackend.getDBAction("Migrate_Cleanup_DB"), bindingMap) != DatabaseCoreBackend::NoErrors)
    {
        emit finished(DatabaseCopyManager::failed, i18n("Error while scrubbing the target database."));
        fromDBbackend.close();
        toDBbackend.close();
        return;
    }

    // then create the schema
    AlbumDB       albumDB(&toDBbackend);
    SchemaUpdater updater(&albumDB, &toDBbackend, toDBParameters);

    emit stepStarted(i18n("Create Schema..."));

    if (!updater.update())
    {
        emit finished(DatabaseCopyManager::failed, i18n("Error while creating the database schema."));
        fromDBbackend.close();
        toDBbackend.close();
        return;
    }

    emit stepStarted(i18n("Copy AlbumRoots..."));

    // now perform the copy action
    if (isStopProcessing || !copyTable(fromDBbackend, QString("Migrate_Read_AlbumRoots"), toDBbackend, QString("Migrate_Write_AlbumRoots")))
    {
        handleClosing(isStopProcessing, fromDBbackend, toDBbackend);
        return;
    }

    emit stepStarted(i18n("Copy Albums..."));

    if (isStopProcessing || !copyTable(fromDBbackend, QString("Migrate_Read_Albums"), toDBbackend, QString("Migrate_Write_Albums")))
    {
        handleClosing(isStopProcessing, fromDBbackend, toDBbackend);
        return;
    }

    emit stepStarted(i18n("Copy Images..."));

    if (isStopProcessing || !copyTable(fromDBbackend, QString("Migrate_Read_Images"), toDBbackend, QString("Migrate_Write_Images")))
    {
        handleClosing(isStopProcessing, fromDBbackend, toDBbackend);
        return;
    }

    emit stepStarted(i18n("Copy ImageHaarMatrix..."));

    if (isStopProcessing || !copyTable(fromDBbackend, QString("Migrate_Read_ImageHaarMatrix"), toDBbackend, QString("Migrate_Write_ImageHaarMatrix")))
    {
        handleClosing(isStopProcessing, fromDBbackend, toDBbackend);
        return;
    }

    emit stepStarted(i18n("Copy ImageInformation..."));

    if (isStopProcessing || !copyTable(fromDBbackend, QString("Migrate_Read_ImageInformation"), toDBbackend, QString("Migrate_Write_ImageInformation")))
    {
        handleClosing(isStopProcessing, fromDBbackend, toDBbackend);
        return;
    }

    emit stepStarted(i18n("Copy ImageMetadata..."));

    if (isStopProcessing || !copyTable(fromDBbackend, QString("Migrate_Read_ImageMetadata"), toDBbackend, QString("Migrate_Write_ImageMetadata")))
    {
        handleClosing(isStopProcessing, fromDBbackend, toDBbackend);
        return;
    }

    emit stepStarted(i18n("Copy ImagePositions..."));

    if (isStopProcessing || !copyTable(fromDBbackend, QString("Migrate_Read_ImagePositions"), toDBbackend, QString("Migrate_Write_ImagePositions")))
    {
        handleClosing(isStopProcessing, fromDBbackend, toDBbackend);
        return;
    }

    emit stepStarted(i18n("Copy ImageComments..."));

    if (isStopProcessing || !copyTable(fromDBbackend, QString("Migrate_Read_ImageComments"), toDBbackend, QString("Migrate_Write_ImageComments")))
    {
        handleClosing(isStopProcessing, fromDBbackend, toDBbackend);
        return;
    }

    emit stepStarted(i18n("Copy ImageCopyright..."));

    if (isStopProcessing || !copyTable(fromDBbackend, QString("Migrate_Read_ImageCopyright"), toDBbackend, QString("Migrate_Write_ImageCopyright")))
    {
        handleClosing(isStopProcessing, fromDBbackend, toDBbackend);
        return;
    }

    emit stepStarted(i18n("Copy Tags..."));

    if (isStopProcessing || !copyTable(fromDBbackend, QString("Migrate_Read_Tags"), toDBbackend, QString("Migrate_Write_Tags")))
    {
        handleClosing(isStopProcessing, fromDBbackend, toDBbackend);
        return;
    }

    emit stepStarted(i18n("Copy ImageTags..."));

    if (isStopProcessing || !copyTable(fromDBbackend, QString("Migrate_Read_ImageTags"), toDBbackend, QString("Migrate_Write_ImageTags")))
    {
        handleClosing(isStopProcessing, fromDBbackend, toDBbackend);
        return;
    }

    emit stepStarted(i18n("Copy ImageProperties..."));

    if (isStopProcessing || !copyTable(fromDBbackend, QString("Migrate_Read_ImageProperties"), toDBbackend, QString("Migrate_Write_ImageProperties")))
    {
        handleClosing(isStopProcessing, fromDBbackend, toDBbackend);
        return;
    }

    emit stepStarted(i18n("Copy Searches..."));

    if (isStopProcessing || !copyTable(fromDBbackend, QString("Migrate_Read_Searches"), toDBbackend, QString("Migrate_Write_Searches")))
    {
        handleClosing(isStopProcessing, fromDBbackend, toDBbackend);
        return;
    }

    emit stepStarted(i18n("Copy DownloadHistory..."));

    if (isStopProcessing || !copyTable(fromDBbackend, QString("Migrate_Read_DownloadHistory"), toDBbackend, QString("Migrate_Write_DownloadHistory")))
    {
        handleClosing(isStopProcessing, fromDBbackend, toDBbackend);
        return;
    }

    /*
        if (isStopThread || !copyTable(fromDBbackend, QString("Migrate_Read_Settings"), toDBbackend, QString("Migrate_Write_Settings")))
        {
            handleClosing(isStopThread, fromDBbackend, toDBbackend);
            return;
        }
    */
    fromDBbackend.close();
    toDBbackend.close();

    emit finished(DatabaseCopyManager::success, QString());
}

bool DatabaseCopyManager::copyTable(DatabaseBackend& fromDBbackend, QString fromActionName, DatabaseBackend& toDBbackend, QString toActionName)
{
    kDebug(50003) << "Trying to copy contents from DB with ActionName: [" << fromActionName
                  << "] to DB with ActionName [" << toActionName << "]";

    QMap<QString, QVariant> bindingMap;
    // now perform the copy action
    QList<QString> columnNames;
    QSqlQuery result = fromDBbackend.execDBActionQuery(fromDBbackend.getDBAction(fromActionName), bindingMap) ;

    int resultSize = -1;

    if (result.driver()->hasFeature(QSqlDriver::QuerySize))
    {
        resultSize=result.size();
    }
    else
    {
        kDebug(50003) << "Driver doesn't support query size. We try to go to the last row and back to the current.";
        result.last();
        /*
         * Now get the current row. If this is not possible, a value lower than 0 will be returned.
         * To not confuse the log reading user, we reset this value to 0.
         */
        resultSize = result.at()<0 ? 0 : result.at();
        result.first();
    }

    kDebug(50003) << "Result size: ["<< resultSize << "]";

    /*
     * If the sql query is forward only - perform the query again.
     * This is not atomic, so it can be tend to different results between
     * real database entries copied and shown at the progressbar.
     */
    if (result.isForwardOnly())
    {
        result.finish();
        result = fromDBbackend.execDBActionQuery(fromDBbackend.getDBAction(fromActionName), bindingMap) ;
    }

    int columnCount = result.record().count();

    for (int i=0; i<columnCount; i++)
    {
        //            kDebug(50003) << "Column: ["<< result.record().fieldName(i) << "]";
        columnNames.append(result.record().fieldName(i));
    }

    int resultCounter=0;

    while (result.next())
    {
        kDebug(50003) << "Query isOnValidRow ["<< result.isValid() <<"] isActive ["<< result.isActive() <<"] result size: ["<< result.size() << "]";

        if (isStopProcessing==true)
        {
            return false;
        }

        // Send a signal to the GUI to entertain the user
        emit smallStepStarted(++resultCounter, resultSize);

        // read the values from the fromDB into a hash
        QMap<QString, QVariant> tempBindingMap;
        int i=0;

        foreach (QString columnName, columnNames)
        {
            kDebug(50003) << "Column: ["<< columnName << "] value ["<<result.value(i)<<"]";
            tempBindingMap.insert(columnName.insert(0, ':'), result.value(i));
            i++;
        }

        // insert the previous requested values to the toDB
        DatabaseAction action = toDBbackend.getDBAction(toActionName);
        DatabaseCoreBackend::QueryState queryStateResult = toDBbackend.execDBAction(action, tempBindingMap);

        if (queryStateResult != DatabaseCoreBackend::NoErrors && toDBbackend.lastSQLError().isValid() && toDBbackend.lastSQLError().number()!=0)
        {
            kDebug(50003) << "Error while converting table data. Details: " << toDBbackend.lastSQLError();
            QString errorMsg = i18n("Error while converting the database. \n Details: %1", toDBbackend.lastSQLError().databaseText());
            emit finished(DatabaseCopyManager::failed, errorMsg);
            return false;
        }

    }

    return true;
}

void DatabaseCopyManager::handleClosing(bool isStopThread, DatabaseBackend& fromDBbackend, DatabaseBackend& toDBbackend)
{
    if (isStopThread)
    {
        emit finished(DatabaseCopyManager::canceled, "");
    }

    fromDBbackend.close();
    toDBbackend.close();
}

} // namespace Digikam

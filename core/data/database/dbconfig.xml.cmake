<?xml version="1.0" encoding="UTF-8"?>
<!-- 
 * ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-07-28
 * Description : Database statements
 *
 * Copyright (C) 2009 by Holger Foerster <hamsi2k at freenet dot de>
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
 * ============================================================
 -->

<databaseconfig>
    <defaultDB>QMYSQL</defaultDB>

    <!-- Increment this version number whenever you change this file.
         The number is defined in the toplevel CMakeList.txt file.
         On version mismatch, users will be warned.
    -->
    <version>${DBCONFIG_XML_VERSION}</version>

    <database name="QSQLITE">
        <hostName>TestHost</hostName>
        <databaseName>DatabaseName</databaseName>
        <userName>UserName</userName>
        <password>Password</password>
        <port>Port</port>
        <connectoptions>ConnectOptions</connectoptions>
        <dbservercmd></dbservercmd>
        <dbinitcmd></dbinitcmd>

        <dbactions>
           	<dbaction name="CheckPriv_CREATE_TRIGGER"><statement mode="plain">
        		CREATE TRIGGER privcheck_trigger DELETE ON PrivCheck
        			BEGIN
                                SELECT * FROM PrivCheck;
  				END;
        	</statement>
        	</dbaction>

        	<dbaction name="CheckPriv_DROP_TRIGGER"><statement mode="plain">
        		DROP TRIGGER privcheck_trigger;
        	</statement>
        	</dbaction>
        	        	
        	<dbaction name="CheckPriv_CREATE_TABLE"><statement mode="plain">
        		CREATE TABLE PrivCheck
        		(
 				   id   INT,
    			           name VARCHAR(35)
				);
        	</statement>
        	</dbaction>

        	<dbaction name="CheckPriv_ALTER_TABLE"><statement mode="plain">
        		ALTER TABLE PrivCheck ADD COLUMN addedColumn;
        	</statement>
			</dbaction>        	
        	
        	<dbaction name="CheckPriv_DROP_TABLE"><statement mode="plain">
        		DROP TABLE PrivCheck;
        	</statement>
            </dbaction>
            
            <dbaction name="CheckPriv_Cleanup"><statement mode="plain">
                DROP TABLE IF EXISTS PrivCheck;
            </statement>
        	</dbaction>
        
        
            <dbaction name="CreateDB" mode="transaction"><statement mode="plain">CREATE TABLE AlbumRoots
                            (id INTEGER PRIMARY KEY,
                            label TEXT,
                            status INTEGER NOT NULL,
                            type INTEGER NOT NULL,
                            identifier TEXT,
                            specificPath TEXT,
                            UNIQUE(identifier, specificPath))</statement>
            <statement mode="plain">CREATE TABLE Albums
                            (id INTEGER PRIMARY KEY,
                            albumRoot INTEGER NOT NULL,
                            relativePath TEXT NOT NULL,
                            date DATE,
                            caption TEXT,
                            collection TEXT,
                            icon INTEGER,
                            UNIQUE(albumRoot, relativePath))</statement>
            <statement mode="plain"> CREATE TABLE Images
                            (id INTEGER PRIMARY KEY,
                            album INTEGER,
                            name TEXT NOT NULL,
                            status INTEGER NOT NULL,
                            category INTEGER NOT NULL,
                            modificationDate DATETIME,
                            fileSize INTEGER,
                            uniqueHash TEXT,
                            UNIQUE (album, name))</statement>
            <statement mode="plain">CREATE TABLE ImageHaarMatrix
                            (imageid INTEGER PRIMARY KEY,
                            modificationDate DATETIME,
                            uniqueHash TEXT,
                            matrix BLOB)</statement>
            <statement mode="plain">CREATE TABLE ImageInformation
                            (imageid INTEGER PRIMARY KEY,
                            rating INTEGER,
                            creationDate DATETIME,
                            digitizationDate DATETIME,
                            orientation INTEGER,
                            width INTEGER,
                            height INTEGER,
                            format TEXT,
                            colorDepth INTEGER,
                colorModel INTEGER);</statement>
            <statement mode="plain"> CREATE TABLE ImageMetadata
                            (imageid INTEGER PRIMARY KEY,
                            make TEXT,
                            model TEXT,
                            lens TEXT,
                            aperture REAL,
                            focalLength REAL,
                            focalLength35 REAL,
                            exposureTime REAL,
                            exposureProgram INTEGER,
                            exposureMode INTEGER,
                            sensitivity INTEGER,
                            flash INTEGER,
                            whiteBalance INTEGER,
                            whiteBalanceColorTemperature INTEGER,
                            meteringMode INTEGER,
                            subjectDistance REAL,
                            subjectDistanceCategory INTEGER)</statement>
            <statement mode="plain">CREATE TABLE ImagePositions
                            (imageid INTEGER PRIMARY KEY,
                            latitude TEXT,
                            latitudeNumber REAL,
                            longitude TEXT,
                            longitudeNumber REAL,
                            altitude REAL,
                            orientation REAL,
                            tilt REAL,
                            roll REAL,
                            accuracy REAL,
                            description TEXT)</statement>
            <statement mode="plain">CREATE TABLE ImageComments
                            (id INTEGER PRIMARY KEY,
                            imageid INTEGER,
                            type INTEGER,
                            language TEXT,
                            author TEXT,
                            date DATETIME,
                            comment TEXT,
                            UNIQUE(imageid, type, language, author))</statement>
            <statement mode="plain"> CREATE TABLE ImageCopyright
                            (id INTEGER PRIMARY KEY,
                            imageid INTEGER,
                            property TEXT,
                            value TEXT,
                            extraValue TEXT,
                            UNIQUE(imageid, property, value, extraValue))</statement>
            <statement mode="plain">CREATE TABLE IF NOT EXISTS Tags
                            (id INTEGER PRIMARY KEY,
                            pid INTEGER,
                            name TEXT NOT NULL,
                            icon INTEGER,
                            iconkde TEXT,
                            UNIQUE (name, pid))</statement>
            <statement mode="plain"> CREATE TABLE IF NOT EXISTS TagsTree
                            (id INTEGER NOT NULL,
                            pid INTEGER NOT NULL,
                            UNIQUE (id, pid))</statement>
            <statement mode="plain">CREATE TABLE IF NOT EXISTS ImageTags
                            (imageid INTEGER NOT NULL,
                            tagid INTEGER NOT NULL,
                            UNIQUE (imageid, tagid))</statement>
            <statement mode="plain">CREATE TABLE IF NOT EXISTS ImageProperties
                            (imageid  INTEGER NOT NULL,
                            property TEXT    NOT NULL,
                            value    TEXT    NOT NULL,
                            UNIQUE (imageid, property))</statement>
            <statement mode="plain">CREATE TABLE IF NOT EXISTS Searches
                            (id INTEGER PRIMARY KEY,
                            type INTEGER,
                            name TEXT NOT NULL,
                            query TEXT NOT NULL)</statement>
            <statement mode="plain">CREATE TABLE DownloadHistory
                            (id  INTEGER PRIMARY KEY,
                            identifier TEXT,
                            filename TEXT,
                            filesize INTEGER,
                            filedate DATETIME,
                            UNIQUE(identifier, filename, filesize, filedate))</statement>
            <statement mode="plain"> CREATE TABLE IF NOT EXISTS Settings
                            (keyword TEXT NOT NULL UNIQUE,
                            value TEXT)</statement>
            <statement mode="plain">CREATE TABLE ImageHistory
                            (imageid INTEGER PRIMARY KEY,
                             uuid TEXT,
                             history TEXT);</statement>
            <statement mode="plain">CREATE TABLE ImageRelations
                            (subject INTEGER,
                             object INTEGER,
                             type INTEGER,
                             UNIQUE(subject, object, type));</statement>
            <statement mode="plain">CREATE TABLE TagProperties
                            (tagid INTEGER,
                             property TEXT,
                             value TEXT);</statement>
            <statement mode="plain">CREATE TABLE ImageTagProperties
                            (imageid INTEGER,
                             tagid INTEGER,
                             property TEXT,
                             value TEXT);</statement>
            </dbaction>

            <!-- Indices -->
            <dbaction name="CreateIndices" mode="transaction">
                <statement mode="plain">CREATE INDEX dir_index  ON Images (album);</statement>
                <statement mode="plain">CREATE INDEX hash_index ON Images (uniqueHash);</statement>
                <statement mode="plain">CREATE INDEX tag_index  ON ImageTags (tagid);</statement>
                <statement mode="plain">CREATE INDEX tag_id_index  ON ImageTags (imageid);</statement>
                <statement mode="plain">CREATE INDEX image_name_index ON Images (name);</statement>
                <statement mode="plain">CREATE INDEX creationdate_index ON ImageInformation (creationDate);</statement>
                <statement mode="plain">CREATE INDEX comments_imageid_index ON ImageComments (imageid);</statement>
                <statement mode="plain">CREATE INDEX copyright_imageid_index ON ImageCopyright (imageid);</statement>
                <statement mode="plain">CREATE INDEX uuid_index ON ImageHistory (uuid);</statement>
                <statement mode="plain">CREATE INDEX subject_relations_index ON ImageRelations (subject);</statement>
                <statement mode="plain">CREATE INDEX object_relations_index ON ImageRelations (object);</statement>
                <statement mode="plain">CREATE INDEX tagproperties_index ON TagProperties (tagid);</statement>
                <statement mode="plain">CREATE INDEX imagetagproperties_index ON ImageTagProperties (imageid, tagid);</statement>
                <statement mode="plain">CREATE INDEX imagetagproperties_imageid_index ON ImageTagProperties (imageid);</statement>
                <statement mode="plain">CREATE INDEX imagetagproperties_tagid_index ON ImageTagProperties (tagid);</statement>
            </dbaction>

            <!-- Triggers -->
            <dbaction name="CreateTriggers" mode="transaction">
                <statement mode="plain">CREATE TRIGGER delete_albumroot DELETE ON AlbumRoots
                BEGIN
                DELETE FROM Albums
                WHERE Albums.albumRoot = OLD.id;
                END;</statement>
                <statement mode="plain">CREATE TRIGGER delete_album DELETE ON Albums
                BEGIN
                DELETE FROM Images
                WHERE Images.album = OLD.id;
                END;</statement>
                <statement mode="plain">CREATE TRIGGER delete_image DELETE ON Images
                    BEGIN
                        DELETE FROM ImageTags          WHERE imageid=OLD.id;
                        DELETE From ImageHaarMatrix    WHERE imageid=OLD.id;
                        DELETE From ImageInformation   WHERE imageid=OLD.id;
                        DELETE From ImageMetadata      WHERE imageid=OLD.id;
                        DELETE From ImagePositions     WHERE imageid=OLD.id;
                        DELETE From ImageComments      WHERE imageid=OLD.id;
                        DELETE From ImageCopyright     WHERE imageid=OLD.id;
                        DELETE From ImageProperties    WHERE imageid=OLD.id;
                        DELETE From ImageHistory       WHERE imageid=OLD.id;
                        DELETE FROM ImageRelations     WHERE subject=OLD.id OR object=OLD.id;
                        DELETE FROM ImageTagProperties WHERE imageid=OLD.id;
                        UPDATE Albums SET icon=null    WHERE icon=OLD.id;
                        UPDATE Tags SET icon=null      WHERE icon=OLD.id;
                    END;
                </statement>
                <statement mode="plain">CREATE TRIGGER delete_tag DELETE ON Tags
                    BEGIN
                        DELETE FROM ImageTags WHERE tagid=OLD.id;
                        DELETE FROM TagProperties WHERE tagid=OLD.id;
                        DELETE FROM ImageTagProperties WHERE tagid=OLD.id;
                    END;
                </statement>
                <statement mode="plain">CREATE TRIGGER insert_tagstree AFTER INSERT ON Tags
                BEGIN
                INSERT INTO TagsTree
                    SELECT NEW.id, NEW.pid
                    UNION
                    SELECT NEW.id, pid FROM TagsTree WHERE id=NEW.pid;
                END;</statement>
                <statement mode="plain">CREATE TRIGGER delete_tagstree DELETE ON Tags
                BEGIN
                DELETE FROM Tags
                WHERE id  IN (SELECT id FROM TagsTree WHERE pid=OLD.id);
                DELETE FROM TagsTree
                WHERE id IN (SELECT id FROM TagsTree WHERE pid=OLD.id);
                DELETE FROM TagsTree
                    WHERE id=OLD.id;
                END;</statement>
                <statement mode="plain">CREATE TRIGGER move_tagstree UPDATE OF pid ON Tags
                BEGIN
                DELETE FROM TagsTree
                    WHERE
                    ((id = OLD.id)
                    OR
                    id IN (SELECT id FROM TagsTree WHERE pid=OLD.id))
                    AND
                    pid IN (SELECT pid FROM TagsTree WHERE id=OLD.id);
                INSERT INTO TagsTree
                    SELECT NEW.id, NEW.pid
                    UNION
                    SELECT NEW.id, pid FROM TagsTree WHERE id=NEW.pid
                    UNION
                    SELECT id, NEW.pid FROM TagsTree WHERE pid=NEW.id
                    UNION
                    SELECT A.id, B.pid FROM TagsTree A, TagsTree B
                    WHERE
                    A.pid = NEW.id AND B.id = NEW.pid;
                END;</statement>
            </dbaction>
            
            <dbaction name="getItemURLsInAlbumByItemName">
                <statement mode="query">SELECT Albums.relativePath, Images.name FROM Images INNER JOIN Albums ON Albums.id=Images.album WHERE Albums.id=:albumID ORDER BY Images.name COLLATE NOCASE;</statement>
            </dbaction>

            <!--Don't collate on the path - this is to maintain the same behavior
                that happens when sort order is "By Path"
            -->
            <dbaction name="getItemURLsInAlbumByItemPath">
                <statement mode="query">SELECT Albums.relativePath, Images.name FROM Images INNER JOIN Albums ON Albums.id=Images.album WHERE Albums.id=:albumID ORDER BY Albums.relativePath,Images.name;</statement>
            </dbaction>

            <dbaction name="getItemURLsInAlbumByItemDate">
                <statement mode="query">SELECT Albums.relativePath, Images.name FROM Images INNER JOIN Albums ON Albums.id=Images.album INNER JOIN ImageInformation ON ImageInformation.imageid=Images.id WHERE Albums.id=:albumID ORDER BY ImageInformation.creationDate;</statement>
            </dbaction>

            <dbaction name="getItemURLsInAlbumByItemRating">
                <statement mode="query">SELECT Albums.relativePath, Images.name FROM Images INNER JOIN Albums ON Albums.id=Images.album INNER JOIN ImageInformation ON ImageInformation.imageid=Images.id WHERE Albums.id=:albumID ORDER BY ImageInformation.rating DESC;</statement>
            </dbaction>

            <dbaction name="getItemURLsInAlbumNoItemSorting">
                <statement mode="query">SELECT Albums.relativePath, Images.name FROM Images INNER JOIN Albums ON Albums.id=Images.album WHERE Albums.id=:albumID;</statement>
            </dbaction>

            <dbaction name="changeImageInformation" mode="transaction">
            <statement mode="query">INSERT OR IGNORE INTO ImageInformation ( imageid, :fieldList ) VALUES ( :id, :valueList );</statement>
            <statement mode="query">UPDATE ImageInformation SET :fieldValueList WHERE imageid=:id;</statement>
            </dbaction>

            <dbaction name="changeImageHistory" mode="transaction">
            <statement mode="query">INSERT OR IGNORE INTO ImageHistory ( imageid, :fieldList ) VALUES ( :id, :valueList );</statement>
            <statement mode="query">UPDATE ImageHistory SET :fieldValueList WHERE imageid=:id;</statement>
            </dbaction>

            <dbaction name="InsertTag">
                <statement mode="query">INSERT INTO Tags (pid, name) VALUES( :tagPID, :tagname);</statement>
            </dbaction>

            <dbaction name="DeleteTag"><statement mode="query">DELETE FROM Tags WHERE id=:tagID;</statement></dbaction>

            <dbaction name="deleteAlbumRoot">
                <statement mode="query">DELETE FROM Albums WHERE albumRoot=:albumRoot;</statement>
            </dbaction>

            <dbaction name="deleteAlbumRootPath">
                <statement mode="query">DELETE FROM Albums WHERE albumRoot=:albumRoot AND relativePath=:relativePath;</statement>
            </dbaction>

            <dbaction name="deleteAlbumID">
            <statement mode="query">DELETE FROM Albums WHERE Albums.id=:albumId;</statement>
            </dbaction>

            <dbaction name="GetItemURLsInTagRecursive">
            <statement mode="query">SELECT Albums.albumRoot, Albums.relativePath, Images.name
                            FROM Images JOIN Albums ON Albums.id=Images.album
                            WHERE Images.status=1 AND Images.id IN (SELECT imageid FROM ImageTags WHERE tagid=:tagID OR tagid IN (SELECT id FROM TagsTree WHERE pid=:tagID)  );
            </statement>
            </dbaction>

            <dbaction name="GetItemURLsInTag">
            <statement mode="query">SELECT Albums.albumRoot, Albums.relativePath, Images.name
                            FROM Images JOIN Albums ON Albums.id=Images.album
                            WHERE Images.status=1 AND Images.id IN (SELECT imageid FROM ImageTags WHERE tagid=:tagID);
            </statement>
            </dbaction>

            <dbaction name="getItemIDsInTagRecursive">
            <statement mode="query">SELECT imageid FROM ImageTags JOIN Images ON ImageTags.imageid=Images.id
                                WHERE Images.status=1 AND
                                ( tagid=:tagID
                                OR tagid IN (SELECT id FROM TagsTree WHERE pid=:tagPID) );
            </statement>
            </dbaction>

            <dbaction name="getItemIDsInTag">
            <statement mode="query">SELECT imageid FROM ImageTags JOIN Images ON ImageTags.imageid=Images.id
                                WHERE Images.status=1 AND tagid=:tagID;
            </statement>
            </dbaction>

            <dbaction name="listTagRecursive">
            <statement mode="query">  SELECT DISTINCT Images.id, Images.name, Images.album,
                                    Albums.albumRoot,
                                    ImageInformation.rating, Images.category,
                                    ImageInformation.format, ImageInformation.creationDate,
                                    Images.modificationDate, Images.fileSize,
                                    ImageInformation.width, ImageInformation.height
                            FROM Images
                                    INNER JOIN ImageInformation ON Images.id=ImageInformation.imageid
                                    INNER JOIN Albums ON Albums.id=Images.album
                            WHERE Images.status=1 AND Images.id IN
                                    (SELECT imageid FROM ImageTags
                                    WHERE tagid=:tagID OR tagid IN (SELECT id FROM TagsTree WHERE pid=:tagPID));
            </statement>
            </dbaction>

            <dbaction name="listTag">
            <statement mode="query">  SELECT DISTINCT Images.id, Images.name, Images.album,
                                    Albums.albumRoot,
                                    ImageInformation.rating, Images.category,
                                    ImageInformation.format, ImageInformation.creationDate,
                                    Images.modificationDate, Images.fileSize,
                                    ImageInformation.width, ImageInformation.height
                            FROM Images
                                    INNER JOIN ImageInformation ON Images.id=ImageInformation.imageid
                                    INNER JOIN Albums ON Albums.id=Images.album
                            WHERE Images.status=1 AND Images.id IN
                                    (SELECT imageid FROM ImageTags
                                    WHERE tagid=:tagID );
            </statement>
            </dbaction>

            <!-- Thumbnails Schema DB -->
            <dbaction name="CreateThumbnailsDB" mode="transaction">
                    <statement mode="plain">CREATE TABLE Thumbnails
                            (id INTEGER PRIMARY KEY,
                            type INTEGER,
                            modificationDate DATETIME,
                            orientationHint INTEGER,
                            data BLOB)</statement>
                    <statement mode="plain">CREATE TABLE UniqueHashes
                            (uniqueHash TEXT,
                            fileSize INTEGER,
                            thumbId INTEGER,
                            UNIQUE(uniqueHash, fileSize))</statement>
                    <statement mode="plain">CREATE TABLE FilePaths
                            (path TEXT,
                            thumbId INTEGER,
                            UNIQUE(path))</statement>
                    <statement mode="plain">CREATE TABLE CustomIdentifiers
                            (identifier TEXT,
                            thumbId INTEGER,
                            UNIQUE(identifier))
                    </statement>
                    <statement mode="plain">CREATE TABLE IF NOT EXISTS Settings
                            (keyword TEXT NOT NULL UNIQUE,
                            value TEXT)
                    </statement>
                </dbaction>

                <!-- Thumbnails Indexes DB -->
                <dbaction name="CreateThumbnailsDBIndices" mode="transaction">
                    <statement mode="plain">CREATE INDEX id_uniqueHashes ON UniqueHashes (thumbId);</statement>
                    <statement mode="plain">CREATE INDEX id_filePaths ON FilePaths (thumbId);</statement>
                    <statement mode="plain">CREATE INDEX id_customIdentifiers ON CustomIdentifiers (thumbId);</statement>
                </dbaction>

                <!-- Thumbnails Trigger DB -->
                <dbaction name="CreateThumbnailsDBTrigger" mode="transaction">
                    <statement mode="plain">CREATE TRIGGER delete_thumbnails DELETE ON Thumbnails
                                BEGIN
                                DELETE FROM UniqueHashes WHERE UniqueHashes.thumbId = OLD.id;
                                DELETE FROM FilePaths WHERE FilePaths.thumbId = OLD.id;
                                DELETE FROM CustomIdentifiers WHERE CustomIdentifiers.thumbId = OLD.id;
                                END;
                    </statement>
                </dbaction>

            <!-- Migration Statements -->
            <dbaction name="Migrate_Cleanup_DB" mode="query">
                <statement mode="plain">DROP TABLE IF EXISTS AlbumRoots</statement>
                <statement mode="plain">DROP TABLE IF EXISTS Albums</statement>
                <statement mode="plain">DROP TABLE IF EXISTS DownloadHistory</statement>
                <statement mode="plain">DROP TABLE IF EXISTS FilePaths</statement>
                <statement mode="plain">DROP TABLE IF EXISTS ImageComments</statement>
                <statement mode="plain">DROP TABLE IF EXISTS ImageCopyright</statement>
                <statement mode="plain">DROP TABLE IF EXISTS ImageHaarMatrix</statement>
                <statement mode="plain">DROP TABLE IF EXISTS ImageInformation</statement>
                <statement mode="plain">DROP TABLE IF EXISTS ImageMetadata</statement>
                <statement mode="plain">DROP TABLE IF EXISTS ImagePositions</statement>
                <statement mode="plain">DROP TABLE IF EXISTS ImageProperties</statement>
                <statement mode="plain">DROP TABLE IF EXISTS ImageTags</statement>
                <statement mode="plain">DROP TABLE IF EXISTS Images</statement>
                <statement mode="plain">DROP TABLE IF EXISTS Searches</statement>
                <statement mode="plain">DROP TABLE IF EXISTS Settings</statement>
                <statement mode="plain">DROP TABLE IF EXISTS Tags</statement>
                <statement mode="plain">DROP TABLE IF EXISTS TagsTree</statement>
                <statement mode="plain">DROP TABLE IF EXISTS Thumbnails</statement>
                <statement mode="plain">DROP TABLE IF EXISTS UniqueHashes</statement>
            </dbaction>

            <dbaction name="Migrate_Read_AlbumRoots"><statement mode="query">
                SELECT id, label, status, type, identifier, specificPath FROM AlbumRoots;
            </statement></dbaction>
            <dbaction name="Migrate_Write_AlbumRoots"><statement mode="query">
                INSERT INTO AlbumRoots (id, label, status, type, identifier, specificPath) VALUES (:id, :label, :status, :type, :identifier, :specificPath);
            </statement></dbaction>

            <dbaction name="Migrate_Read_Albums"><statement mode="query">
                SELECT id, albumRoot, relativePath, date, caption, collection, icon FROM Albums;
            </statement></dbaction>
            <dbaction name="Migrate_Write_Albums"><statement mode="query">
                INSERT INTO Albums (id, albumRoot, relativePath, date, caption, collection, icon) VALUES (:id, :albumRoot, :relativePath, :date, :caption, :collection, :icon);
            </statement></dbaction>

            <dbaction name="Migrate_Read_Images"><statement mode="query">
                SELECT id, album, name, status, category, modificationDate, fileSize, uniqueHash FROM Images;
            </statement></dbaction>
            <dbaction name="Migrate_Write_Images"><statement mode="query">
                INSERT INTO Images (id, album, name, status, category, modificationDate, fileSize, uniqueHash) VALUES (:id, :album, :name, :status, :category, :modificationDate, :fileSize, :uniqueHash);
            </statement></dbaction>

            <dbaction name="Migrate_Read_ImageHaarMatrix"><statement mode="query">
                SELECT imageid, modificationDate, uniqueHash, matrix FROM ImageHaarMatrix;
            </statement></dbaction>
            <dbaction name="Migrate_Write_ImageHaarMatrix"><statement mode="query">
                INSERT INTO ImageHaarMatrix (imageid, modificationDate, uniqueHash, matrix) VALUES (:imageid, :modificationDate, :uniqueHash, :matrix);
            </statement></dbaction>

            <dbaction name="Migrate_Read_ImageInformation"><statement mode="query">
                SELECT imageid, rating, creationDate, digitizationDate, orientation, width, height, format, colorDepth, colorModel FROM ImageInformation;
            </statement></dbaction>
            <dbaction name="Migrate_Write_ImageInformation"><statement mode="query">
                INSERT INTO ImageInformation (imageid, rating, creationDate, digitizationDate, orientation, width, height, format, colorDepth, colorModel) VALUES (:imageid, :rating, :creationDate, :digitizationDate, :orientation, :width, :height, :format, :colorDepth, :colorModel);
            </statement></dbaction>

            <dbaction name="Migrate_Read_ImageMetadata"><statement mode="query">
                SELECT imageid, make, model, lens, aperture, focalLength, focalLength35, exposureTime, exposureProgram, exposureMode, sensitivity, flash, whiteBalance, whiteBalanceColorTemperature, meteringMode, subjectDistance, subjectDistanceCategory FROM ImageMetadata;
            </statement></dbaction>
            <dbaction name="Migrate_Write_ImageMetadate"><statement mode="query">
                INSERT INTO ImageMetadate (imageid, make, model, lens, aperture, focalLength, focalLength35, exposureTime, exposureProgram, exposureMode, sensitivity, flash, whiteBalance, whiteBalanceColorTemperature, meteringMode, subjectDistance, subjectDistanceCategory) VALUES (:imageid, :make, :model, :lens, :aperture, :focalLength, :focalLength35, :exposureTime, :exposureProgram, :exposureMode, :sensitivity, :flash, :whiteBalance, :whiteBalanceColorTemperature, :meteringMode, :subjectDistance, :subjectDistanceCategory);
            </statement></dbaction>

            <dbaction name="Migrate_Read_ImagePositions"><statement mode="query">
                SELECT  imageid, latitude, latitudeNumber, longitude, longitudeNumber, altitude, orientation, tilt, roll, accuracy, description FROM ImagePositions;
            </statement></dbaction>
            <dbaction name="Migrate_Write_ImagePositions"><statement mode="query">
                INSERT INTO ImagePositions (imageid, latitude, latitudeNumber, longitude, longitudeNumber, altitude, orientation, tilt, roll, accuracy, description) VALUES (:imageid, :latitude, :latitudeNumber, :longitude, :longitudeNumber, :altitude, :orientation, :tilt, :roll, :accuracy, :description);
            </statement></dbaction>

            <dbaction name="Migrate_Read_ImageComments"><statement mode="query">
                SELECT  id, imageid, type, language, author, date, comment FROM ImageComments;
            </statement></dbaction>
            <dbaction name="Migrate_Write_ImageComments"><statement mode="query">
                INSERT INTO ImageComments (id, imageid, type, language, author, date, comment) VALUES (:id, :imageid, :type, :language, :author, :date, :comment);
            </statement></dbaction>

            <dbaction name="Migrate_Read_ImageCopyright"><statement mode="query">
                SELECT  id, imageid, property, value, extraValue FROM ImageCopyright;
            </statement></dbaction>
            <dbaction name="Migrate_Write_ImageCopyright"><statement mode="query">
                INSERT INTO ImageCopyright (id, imageid, property, value, extraValue) VALUES (:id, :imageid, :property, :value, :extraValue);
            </statement></dbaction>

            <dbaction name="Migrate_Read_Tags"><statement mode="query">
                SELECT  id, pid, name, icon, iconkde FROM Tags;
            </statement></dbaction>
            <dbaction name="Migrate_Write_Tags"><statement mode="query">
                INSERT INTO Tags (id, pid, name, icon, iconkde) VALUES (:id, :pid, :name, :icon, :iconkde);
            </statement></dbaction>

            <dbaction name="Migrate_Read_ImageTags"><statement mode="query">
                SELECT  imageid, tagid FROM ImageTags;
            </statement></dbaction>
            <dbaction name="Migrate_Write_ImageTags"><statement mode="query">
                INSERT INTO ImageTags (imageid, tagid) VALUES (:imageid, :tagid);
            </statement></dbaction>

            <dbaction name="Migrate_Read_ImageProperties"><statement mode="query">
                SELECT  imageid, property, value FROM ImageProperties;
            </statement></dbaction>
            <dbaction name="Migrate_Write_ImageProperties"><statement mode="query">
                INSERT INTO ImageProperties (imageid, property, value) VALUES (:imageid, :property, :value);
            </statement></dbaction>

            <dbaction name="Migrate_Read_Searches"><statement mode="query">
                SELECT  id, type, name, query FROM Searches;
            </statement></dbaction>
            <dbaction name="Migrate_Write_Searches"><statement mode="query">
                INSERT INTO Searches (id, type, name, query) VALUES (:id, :type, :name, :query);
            </statement></dbaction>

            <dbaction name="Migrate_Read_DownloadHistory"><statement mode="query">
                SELECT id, identifier, filename, filesize, filedate FROM DownloadHistory;
            </statement></dbaction>
            <dbaction name="Migrate_Write_DownloadHistory"><statement mode="query">
                INSERT INTO DownloadHistory (id, identifier, filename, filesize, filedate) VALUES (:id, :identifier, :filename, :filesize, :filedate);
            </statement></dbaction>

            <dbaction name="Migrate_Read_Settings"><statement mode="query">
                SELECT keyword, value FROM Settings;
            </statement></dbaction>
            <dbaction name="Migrate_Write_Settings"><statement mode="query">
                INSERT INTO Settings (keyword, value) VALUES (:keyword, :value);
            </statement></dbaction>

            <dbaction name="Delete_Thumbnail_ByPath"><statement mode="query">
                DELETE FROM Thumbnails WHERE id IN (SELECT thumbId FROM FilePaths WHERE path=:path);
            </statement></dbaction>

            <dbaction name="Delete_Thumbnail_ByUniqueHashId"><statement mode="query">
                DELETE FROM Thumbnails WHERE id IN (SELECT thumbId FROM UniqueHashes WHERE uniqueHash=:uniqueHash AND fileSize=:filesize);
            </statement></dbaction>

            <dbaction name="Delete_Thumbnail_ByCustomIdentifier"><statement mode="query">
                DELETE FROM Thumbnails WHERE id IN (SELECT thumbId FROM CustomIdentifiers WHERE identifier=:identifier);
            </statement></dbaction>

            <!-- Migration from DB Version 5 (0.10 - 1.4) to Version 6 (1.5-) -->
            <dbaction name="UpdateSchemaFromV5ToV6" mode="transaction">
                <statement mode="plain">CREATE TABLE ImageHistory
                            (imageid INTEGER PRIMARY KEY,
                             uuid TEXT,
                             history TEXT);</statement>
                <statement mode="plain">CREATE TABLE ImageRelations
                            (subject INTEGER,
                             object INTEGER,
                             type INTEGER,
                             UNIQUE(subject, object, type));</statement>
                <statement mode="plain">CREATE TABLE TagProperties
                            (tagid INTEGER,
                             property TEXT,
                             value TEXT);</statement>
                <statement mode="plain">CREATE TABLE ImageTagProperties
                            (imageid INTEGER,
                             tagid INTEGER,
                             property TEXT,
                             value TEXT);</statement>
                <statement mode="plain">CREATE INDEX tag_id_index  ON ImageTags (imageid);</statement>
                <statement mode="plain">CREATE INDEX image_name_index ON Images (name);</statement>
                <statement mode="plain">CREATE INDEX creationdate_index ON ImageInformation (creationDate);</statement>
                <statement mode="plain">CREATE INDEX comments_imageid_index ON ImageComments (imageid);</statement>
                <statement mode="plain">CREATE INDEX copyright_imageid_index ON ImageCopyright (imageid);</statement>
                <statement mode="plain">CREATE INDEX uuid_index ON ImageHistory (uuid);</statement>
                <statement mode="plain">CREATE INDEX subject_relations_index ON ImageRelations (subject);</statement>
                <statement mode="plain">CREATE INDEX object_relations_index ON ImageRelations (object);</statement>
                <statement mode="plain">CREATE INDEX tagproperties_index ON TagProperties (tagid);</statement>
                <statement mode="plain">CREATE INDEX imagetagproperties_index ON ImageTagProperties (imageid, tagid);</statement>
                <statement mode="plain">CREATE INDEX imagetagproperties_imageid_index ON ImageTagProperties (imageid);</statement>
                <statement mode="plain">CREATE INDEX imagetagproperties_tagid_index ON ImageTagProperties (tagid);</statement>
                <statement mode="plain">DROP TRIGGER delete_image;</statement>
                <statement mode="plain">CREATE TRIGGER delete_image DELETE ON Images
                    BEGIN
                        DELETE FROM ImageTags          WHERE imageid=OLD.id;
                        DELETE From ImageHaarMatrix    WHERE imageid=OLD.id;
                        DELETE From ImageInformation   WHERE imageid=OLD.id;
                        DELETE From ImageMetadata      WHERE imageid=OLD.id;
                        DELETE From ImagePositions     WHERE imageid=OLD.id;
                        DELETE From ImageComments      WHERE imageid=OLD.id;
                        DELETE From ImageCopyright     WHERE imageid=OLD.id;
                        DELETE From ImageProperties    WHERE imageid=OLD.id;
                        DELETE From ImageHistory       WHERE imageid=OLD.id;
                        DELETE FROM ImageRelations     WHERE subject=OLD.id OR object=OLD.id;
                        DELETE FROM ImageTagProperties WHERE imageid=OLD.id;
                        UPDATE Albums SET icon=null    WHERE icon=OLD.id;
                        UPDATE Tags SET icon=null      WHERE icon=OLD.id;
                    END;
                </statement>
                <statement mode="plain">DROP TRIGGER delete_tag;</statement>
                <statement mode="plain">CREATE TRIGGER delete_tag DELETE ON Tags
                            BEGIN
                            DELETE FROM ImageTags WHERE tagid=OLD.id;
                            DELETE FROM TagProperties WHERE tagid=OLD.id;
                            DELETE FROM ImageTagProperties WHERE tagid=OLD.id;
                            END;
                </statement>
            </dbaction>
            <dbaction name="UpdateThumbnailsDBSchemaFromV1ToV2" mode="transaction">
                <statement mode="plain">CREATE TABLE CustomIdentifiers
                        (identifier TEXT,
                        thumbId INTEGER,
                        UNIQUE(identifier))
                </statement>
                <statement mode="plain">CREATE INDEX id_customIdentifiers ON CustomIdentifiers (thumbId);</statement>
                <statement mode="plain">DROP TRIGGER delete_thumbnails;</statement>
                <statement mode="plain">CREATE TRIGGER delete_thumbnails DELETE ON Thumbnails
                            BEGIN
                            DELETE FROM UniqueHashes WHERE UniqueHashes.thumbId = OLD.id;
                            DELETE FROM FilePaths WHERE FilePaths.thumbId = OLD.id;
                            DELETE FROM CustomIdentifiers WHERE CustomIdentifiers.thumbId = OLD.id;
                            END;
                </statement>
            </dbaction>

        </dbactions>
    </database>
        <database name="QMYSQL">
        <hostName></hostName>
        <databaseName>digikam</databaseName>
        <userName>root</userName>
        <password></password>
        <port>1</port>
        <connectoptions>UNIX_SOCKET=$$DBMISCPATH$$/mysql.socket</connectoptions>
        <dbservercmd>${SERVERCMD_MYSQL}</dbservercmd>
        <dbinitcmd>${INITCMD_MYSQL}</dbinitcmd>

        <dbactions>
        	<dbaction name="CheckPriv_CREATE_TRIGGER"><statement mode="plain">
        		CREATE TRIGGER privcheck_trigger AFTER DELETE ON PrivCheck
 				 FOR EACH ROW BEGIN
  				END;
        	</statement>
        	</dbaction>

        	<dbaction name="CheckPriv_DROP_TRIGGER"><statement mode="plain">
        		DROP TRIGGER privcheck_trigger;
        	</statement>
        	</dbaction>
        	        	
        	<dbaction name="CheckPriv_CREATE_TABLE"><statement mode="plain">
        		CREATE TABLE PrivCheck
        		(
 				   id   INT,
    			   name VARCHAR(35)
				);
        	</statement>
        	</dbaction>

        	<dbaction name="CheckPriv_ALTER_TABLE"><statement mode="plain">
        		ALTER TABLE PrivCheck DROP COLUMN name;
        	</statement>
			</dbaction>        	
        	
        	<dbaction name="CheckPriv_DROP_TABLE"><statement mode="plain">
        		DROP TABLE PrivCheck;
        	</statement>
        	</dbaction>
        	
        	<dbaction name="CheckPriv_Cleanup"><statement mode="plain">
                DROP TABLE IF EXISTS PrivCheck;
            </statement>
            </dbaction>
        
            <dbaction name="CreateDB" mode="transaction"><statement mode="plain">  CREATE TABLE AlbumRoots
            (id INTEGER PRIMARY KEY NOT NULL AUTO_INCREMENT,
            label LONGTEXT,
            status INTEGER NOT NULL,
            type INTEGER NOT NULL,
            identifier LONGTEXT,
            specificPath LONGTEXT,
            UNIQUE(identifier(127), specificPath(128)));</statement>
            <statement mode="plain">CREATE TABLE Albums
                            (id INTEGER PRIMARY KEY NOT NULL AUTO_INCREMENT,
                            albumRoot INTEGER NOT NULL,
                            relativePath LONGTEXT CHARACTER SET utf8 NOT NULL,
                            date DATE,
                            caption LONGTEXT CHARACTER SET utf8,
                            collection LONGTEXT CHARACTER SET utf8,
                            icon INTEGER,
                            UNIQUE(albumRoot, relativePath(255)));</statement>
            <statement mode="plain">CREATE TABLE Images
                            (id INTEGER PRIMARY KEY NOT NULL AUTO_INCREMENT,
                            album INTEGER,
                            name LONGTEXT CHARACTER SET utf8 NOT NULL,
                            status INTEGER NOT NULL,
                            category INTEGER NOT NULL,
                            modificationDate DATETIME,
                            fileSize INTEGER,
                            uniqueHash VARCHAR(128),
                            UNIQUE (album, name(255)));</statement>
            <statement mode="plain">CREATE TABLE ImageHaarMatrix
                            (imageid INTEGER PRIMARY KEY,
                            modificationDate DATETIME,
                            uniqueHash LONGTEXT CHARACTER SET utf8,
                            matrix LONGBLOB);</statement>
            <statement mode="plain">CREATE TABLE ImageInformation
                            (imageid INTEGER PRIMARY KEY,
                            rating INTEGER,
                            creationDate DATETIME,
                            digitizationDate DATETIME,
                            orientation INTEGER,
                            width INTEGER,
                            height INTEGER,
                            format LONGTEXT CHARACTER SET utf8,
                            colorDepth INTEGER,
                            colorModel INTEGER);</statement>
            <statement mode="plain">CREATE TABLE ImageMetadata
                            (imageid INTEGER PRIMARY KEY,
                            make LONGTEXT CHARACTER SET utf8,
                            model LONGTEXT CHARACTER SET utf8,
                            lens LONGTEXT CHARACTER SET utf8,
                            aperture REAL,
                            focalLength REAL,
                            focalLength35 REAL,
                            exposureTime REAL,
                            exposureProgram INTEGER,
                            exposureMode INTEGER,
                            sensitivity INTEGER,
                            flash INTEGER,
                            whiteBalance INTEGER,
                            whiteBalanceColorTemperature INTEGER,
                            meteringMode INTEGER,
                            subjectDistance REAL,
                            subjectDistanceCategory INTEGER);</statement>
            <statement mode="plain">CREATE TABLE ImagePositions
                            (imageid INTEGER PRIMARY KEY,
                            latitude LONGTEXT CHARACTER SET utf8,
                            latitudeNumber REAL,
                            longitude LONGTEXT CHARACTER SET utf8,
                            longitudeNumber REAL,
                            altitude REAL,
                            orientation REAL,
                            tilt REAL,
                            roll REAL,
                            accuracy REAL,
                            description LONGTEXT CHARACTER SET utf8);</statement>
            <statement mode="plain">CREATE TABLE ImageComments
                            (id INTEGER PRIMARY KEY NOT NULL AUTO_INCREMENT,
                            imageid INTEGER,
                            type INTEGER,
                            language VARCHAR(128) CHARACTER SET utf8,
                            author LONGTEXT CHARACTER SET utf8,
                            date DATETIME,
                            comment LONGTEXT CHARACTER SET utf8,
                            UNIQUE(imageid, type, language, author(202)));</statement>
            <statement mode="plain">CREATE TABLE ImageCopyright
                            (id INTEGER PRIMARY KEY NOT NULL AUTO_INCREMENT,
                            imageid INTEGER,
                            property LONGTEXT CHARACTER SET utf8,
                            value LONGTEXT CHARACTER SET utf8,
                            extraValue LONGTEXT CHARACTER SET utf8,
                            UNIQUE(imageid, property(110), value(111), extraValue(111)));</statement>
            <statement mode="plain">CREATE TABLE Tags
                            (id INTEGER PRIMARY KEY NOT NULL AUTO_INCREMENT,
                pid INTEGER,
                            name LONGTEXT CHARACTER SET utf8 NOT NULL,
                            icon INTEGER,
                            iconkde LONGTEXT CHARACTER SET utf8,
                            lft INT NOT NULL,
                rgt INT NOT NULL
                            );</statement>
            <statement mode="plain">CREATE TABLE IF NOT EXISTS TagsTree
                            (id INTEGER NOT NULL NOT NULL AUTO_INCREMENT,
                            pid INTEGER NOT NULL,
                            UNIQUE (id, pid));</statement>
            <statement mode="plain">CREATE TABLE IF NOT EXISTS ImageTags
                            (imageid INTEGER NOT NULL,
                            tagid INTEGER NOT NULL,
                            UNIQUE (imageid, tagid));</statement>
            <statement mode="plain"> CREATE TABLE IF NOT EXISTS ImageProperties
                            (imageid  INTEGER NOT NULL,
                            property LONGTEXT CHARACTER SET utf8    NOT NULL,
                            value    LONGTEXT CHARACTER SET utf8    NOT NULL,
                            UNIQUE (imageid, property(255)));</statement>
            <statement mode="plain">CREATE TABLE IF NOT EXISTS Searches
                            (id INTEGER PRIMARY KEY NOT NULL AUTO_INCREMENT,
                            type INTEGER,
                            name LONGTEXT CHARACTER SET utf8 NOT NULL,
                            query LONGTEXT CHARACTER SET utf8 NOT NULL);</statement>
            <statement mode="plain">CREATE TABLE DownloadHistory
                            (id  INTEGER PRIMARY KEY NOT NULL AUTO_INCREMENT,
                            identifier LONGTEXT CHARACTER SET utf8,
                            filename LONGTEXT CHARACTER SET utf8,
                            filesize INTEGER,
                            filedate DATETIME,
                            UNIQUE(identifier(164), filename(165), filesize, filedate));</statement>
            <statement mode="plain">CREATE TABLE IF NOT EXISTS Settings
                            (keyword LONGTEXT CHARACTER SET utf8 NOT NULL,
                            value LONGTEXT CHARACTER SET utf8,
                            UNIQUE(keyword(255)));</statement>
            <statement mode="plain">CREATE TABLE ImageHistory
                            (imageid INTEGER PRIMARY KEY,
                             uuid VARCHAR(128),
                             history LONGTEXT CHARACTER SET utf8);</statement>
            <statement mode="plain">CREATE TABLE ImageRelations
                            (subject INTEGER,
                             object INTEGER,
                             type INTEGER,
                             UNIQUE(subject, object, type));</statement>
            <statement mode="plain">CREATE TABLE TagProperties
                            (tagid INTEGER,
                             property TEXT CHARACTER SET utf8,
                             value LONGTEXT CHARACTER SET utf8);</statement>
            <statement mode="plain">CREATE TABLE ImageTagProperties
                            (imageid INTEGER,
                             tagid INTEGER,
                             property TEXT CHARACTER SET utf8,
                             value LONGTEXT CHARACTER SET utf8);</statement>
            </dbaction>


            <!-- Indices -->
            <dbaction name="CreateIndices" mode="transaction">
                <statement mode="plain">CREATE INDEX dir_index  ON Images (album);</statement>
                <statement mode="plain">CREATE INDEX hash_index ON Images (uniqueHash);</statement>
                <statement mode="plain">CREATE INDEX tag_index  ON ImageTags (tagid);</statement>
                <statement mode="plain">CREATE INDEX tag_id_index  ON ImageTags (imageid);</statement>
                <statement mode="plain">CREATE INDEX image_name_index ON Images (name(996));</statement>
                <statement mode="plain">CREATE INDEX creationdate_index ON ImageInformation (creationDate);</statement>
                <statement mode="plain">CREATE INDEX comments_imageid_index ON ImageComments (imageid);</statement>
                <statement mode="plain">CREATE INDEX copyright_imageid_index ON ImageCopyright (imageid);</statement>
                <statement mode="plain">CREATE INDEX uuid_index ON ImageHistory (uuid);</statement>
                <statement mode="plain">CREATE INDEX subject_relations_index ON ImageRelations (subject);</statement>
                <statement mode="plain">CREATE INDEX object_relations_index ON ImageRelations (object);</statement>
                <statement mode="plain">CREATE INDEX tagproperties_index ON TagProperties (tagid);</statement>
                <statement mode="plain">CREATE INDEX imagetagproperties_index ON ImageTagProperties (imageid, tagid);</statement>
                <statement mode="plain">CREATE INDEX imagetagproperties_imageid_index ON ImageTagProperties (imageid);</statement>
                <statement mode="plain">CREATE INDEX imagetagproperties_tagid_index ON ImageTagProperties (tagid);</statement>
            </dbaction>

            <!-- Triggers -->
            <dbaction name="CreateTriggers" mode="transaction">
            <statement mode="plain">CREATE TRIGGER delete_image AFTER DELETE ON Images
                    FOR EACH ROW BEGIN
                        DELETE FROM ImageTags          WHERE imageid=OLD.id;
                        DELETE From ImageHaarMatrix    WHERE imageid=OLD.id;
                        DELETE From ImageInformation   WHERE imageid=OLD.id;
                        DELETE From ImageMetadata      WHERE imageid=OLD.id;
                        DELETE From ImagePositions     WHERE imageid=OLD.id;
                        DELETE From ImageComments      WHERE imageid=OLD.id;
                        DELETE From ImageCopyright     WHERE imageid=OLD.id;
                        DELETE From ImageProperties    WHERE imageid=OLD.id;
                        DELETE From ImageHistory       WHERE imageid=OLD.id;
                        DELETE FROM ImageRelations     WHERE subject=OLD.id OR object=OLD.id;
                        DELETE FROM ImageTagProperties WHERE imageid=OLD.id;
                        UPDATE Albums SET icon=null    WHERE icon=OLD.id;
                        UPDATE Tags SET icon=null      WHERE icon=OLD.id;
                    END;
            </statement>
            <statement mode="plain">CREATE TRIGGER delete_tag AFTER DELETE ON Tags
            FOR EACH ROW BEGIN
                DELETE FROM ImageTags          WHERE tagid=OLD.id;
                DELETE FROM TagProperties      WHERE tagid=OLD.id;
                DELETE FROM ImageTagProperties WHERE tagid=OLD.id;
                DELETE FROM TagsTree;
                REPLACE INTO TagsTree
                SELECT node.id, parent.pid
                FROM Tags AS node, Tags AS parent
                WHERE node.lft BETWEEN parent.lft AND parent.rgt
                ORDER BY parent.lft;
            END;
            </statement>
            <statement mode="plain">CREATE TRIGGER move_tagstree AFTER UPDATE ON Tags
            FOR EACH ROW BEGIN
            DELETE FROM TagsTree;
            REPLACE INTO TagsTree
            SELECT node.id, parent.pid
            FROM Tags AS node, Tags AS parent
            WHERE node.lft BETWEEN parent.lft AND parent.rgt
            ORDER BY parent.lft;
            END;</statement>
            </dbaction>
                       
            <dbaction name="checkIfDatabaseExists">
                <statement mode="query">SELECT Albums.relativePath, Images.name FROM Images INNER JOIN Albums ON Albums.id=Images.album WHERE Albums.id=:albumID ORDER BY Images.name;</statement>
            </dbaction>

            <dbaction name="getItemURLsInAlbumByItemName">
                <statement mode="query">SELECT Albums.relativePath, Images.name FROM Images INNER JOIN Albums ON Albums.id=Images.album WHERE Albums.id=:albumID ORDER BY Images.name;</statement>
            </dbaction>

            <!--Don't collate on the path - this is to maintain the same behavior
                that happens when sort order is "By Path"
            -->
            <dbaction name="getItemURLsInAlbumByItemPath">
                <statement mode="query">SELECT Albums.relativePath, Images.name FROM Images INNER JOIN Albums ON Albums.id=Images.album WHERE Albums.id=:albumID ORDER BY Albums.relativePath,Images.name;</statement>
            </dbaction>

            <dbaction name="getItemURLsInAlbumByItemDate">
                <statement mode="query">SELECT Albums.relativePath, Images.name FROM Images INNER JOIN Albums ON Albums.id=Images.album INNER JOIN ImageInformation ON ImageInformation.imageid=Images.id WHERE Albums.id=:albumID ORDER BY ImageInformation.creationDate;</statement>
            </dbaction>

            <dbaction name="getItemURLsInAlbumByItemRating">
                <statement mode="query">SELECT Albums.relativePath, Images.name FROM Images INNER JOIN Albums ON Albums.id=Images.album INNER JOIN ImageInformation ON ImageInformation.imageid=Images.id WHERE Albums.id=:albumID ORDER BY ImageInformation.rating DESC;</statement>
            </dbaction>

            <dbaction name="getItemURLsInAlbumNoItemSorting">
                <statement mode="query">SELECT Albums.relativePath, Images.name FROM Images INNER JOIN Albums ON Albums.id=Images.album WHERE Albums.id=:albumID;</statement>
            </dbaction>

            <dbaction name="changeImageInformation">
            <statement mode="query">INSERT INTO ImageInformation ( imageid, :fieldList ) VALUES ( :id, :valueList ) ON DUPLICATE KEY UPDATE :fieldValueList;</statement>
            </dbaction>

            <dbaction name="changeImageHistory">
            <statement mode="query">INSERT INTO ImageHistory( imageid, :fieldList ) VALUES ( :id, :valueList ) ON DUPLICATE KEY UPDATE :fieldValueList;</statement>
            </dbaction>

            <dbaction name="InsertTag" mode="transaction">
                <statement mode="plain">LOCK TABLE Tags WRITE;</statement>
                <statement mode="query">SELECT @myLeft := lft FROM Tags WHERE id = :tagPID;</statement>
                <statement mode="query">SELECT @myLeft := IF (@myLeft is null, 0, @myLeft);</statement>
                <statement mode="query">UPDATE Tags SET rgt = rgt + 2 WHERE rgt > @myLeft;</statement>
                <statement mode="query">UPDATE Tags SET lft = lft + 2 WHERE lft > @myLeft;</statement>
                <statement mode="query">INSERT INTO Tags(name, pid, lft, rgt) VALUES(:tagname, :tagPID, @myLeft + 1, @myLeft + 2);</statement>
                <statement mode="plain">UNLOCK TABLES;</statement>
            </dbaction>

            <dbaction name="DeleteTag" mode="transaction">
            <statement mode="plain">LOCK TABLE Tags WRITE;</statement>
            <statement mode="query">SELECT @myLeft := lft, @myRight := rgt, @myWidth := rgt - lft + 1
                        FROM Tags
                        WHERE id = :tagID;</statement>
            <statement mode="query">DELETE FROM Tags WHERE lft BETWEEN @myLeft AND @myRight;</statement>
            <statement mode="query">UPDATE Tags SET rgt = rgt - @myWidth WHERE rgt > @myRight;</statement>
            <statement mode="query">UPDATE Tags SET lft = lft - @myWidth WHERE lft > @myRight;</statement>
            <statement mode="plain">UNLOCK TABLES;</statement></dbaction>

            <dbaction name="deleteAlbumRoot" mode="transaction">
                <statement mode="query">SELECT @albumID:=id FROM Albums WHERE albumRoot=:albumRoot;</statement>
                <statement mode="query">DELETE FROM Albums WHERE albumRoot=:albumRoot;</statement>
                <statement mode="query">DELETE FROM Images WHERE Images.album=@albumID;</statement>
            </dbaction>

            <dbaction name="deleteAlbumRootPath" mode="transaction">
                <statement mode="query">SELECT @albumID:=id FROM Albums WHERE albumRoot=:albumRoot AND relativePath=:relativePath;</statement>
                <statement mode="query">DELETE FROM Albums WHERE albumRoot=:albumRoot AND relativePath=:relativePath;</statement>
                <statement mode="query">DELETE FROM Images WHERE Images.album=@albumID;</statement>
            </dbaction>

            <dbaction name="deleteAlbumID" mode="transaction">
            <statement mode="query">SELECT @albumID:=id FROM Albums WHERE Albums.id=:albumId;</statement>
            <statement mode="query">DELETE FROM Albums WHERE Albums.id=:albumId;</statement>
            <statement mode="query">DELETE FROM Images WHERE Images.album=@albumID;</statement>
            </dbaction>

            <dbaction name="GetItemURLsInTagRecursive">
            <statement mode="query">SELECT Albums.albumRoot, Albums.relativePath, Images.name
                            FROM Images JOIN Albums ON Albums.id=Images.album
                            WHERE Images.status=1 AND Images.id IN (SELECT imageid FROM ImageTags WHERE tagid=:tagID OR tagid IN (SELECT id FROM Tags WHERE lft BETWEEN (SELECT lft FROM Tags WHERE id=:tagID) AND (SELECT rgt FROM Tags WHERE id=:tagID)) );
            </statement>
            </dbaction>

            <dbaction name="GetItemURLsInTag">
            <statement mode="query">SELECT Albums.albumRoot, Albums.relativePath, Images.name
                            FROM Images JOIN Albums ON Albums.id=Images.album
                            WHERE Images.status=1 AND Images.id IN (SELECT imageid FROM ImageTags WHERE tagid=:tagID);
            </statement>
            </dbaction>

            <dbaction name="getItemIDsInTagRecursive">
            <statement mode="query">SELECT imageid FROM ImageTags JOIN Images ON ImageTags.imageid=Images.id
                                WHERE Images.status=1 AND
                                ( tagid=:tagID
                                OR tagid IN (SELECT id FROM Tags WHERE lft BETWEEN (SELECT lft FROM Tags WHERE id=:tagID) AND (SELECT rgt FROM Tags WHERE id=:tagID)) );
            </statement>
            </dbaction>

            <dbaction name="getItemIDsInTag">
            <statement mode="query">SELECT imageid FROM ImageTags JOIN Images ON ImageTags.imageid=Images.id
                                WHERE Images.status=1 AND tagid=:tagID;
            </statement>
            </dbaction>

            <dbaction name="listTagRecursive">
            <statement mode="query">  SELECT DISTINCT Images.id, Images.name, Images.album,
                                    Albums.albumRoot,
                                    ImageInformation.rating, Images.category,
                                    ImageInformation.format, ImageInformation.creationDate,
                                    Images.modificationDate, Images.fileSize,
                                    ImageInformation.width, ImageInformation.height
                            FROM Images
                                    INNER JOIN ImageInformation ON Images.id=ImageInformation.imageid
                                    INNER JOIN Albums ON Albums.id=Images.album
                            WHERE Images.status=1 AND Images.id IN
                                    (SELECT imageid FROM ImageTags
                                    WHERE tagid=:tagID OR tagid IN (SELECT id FROM Tags WHERE lft BETWEEN (SELECT lft FROM Tags WHERE id=:tagID) AND (SELECT rgt FROM Tags WHERE id=:tagID)) );
            </statement>
            </dbaction>

            <dbaction name="listTag">
            <statement mode="query">  SELECT DISTINCT Images.id, Images.name, Images.album,
                                    Albums.albumRoot,
                                    ImageInformation.rating, Images.category,
                                    ImageInformation.format, ImageInformation.creationDate,
                                    Images.modificationDate, Images.fileSize,
                                    ImageInformation.width, ImageInformation.height
                            FROM Images
                                    INNER JOIN ImageInformation ON Images.id=ImageInformation.imageid
                                    INNER JOIN Albums ON Albums.id=Images.album
                            WHERE Images.status=1 AND Images.id IN
                                    (SELECT imageid FROM ImageTags
                                    WHERE tagid=:tagID );
            </statement>
            </dbaction>

            <!-- Thumbnails Schema DB -->
            <dbaction name="CreateThumbnailsDB" mode="transaction">
                <statement mode="plain">CREATE TABLE Thumbnails
                            (id INTEGER PRIMARY KEY AUTO_INCREMENT,
                            type INTEGER,
                            modificationDate DATETIME,
                            orientationHint INTEGER,
                            data LONGBLOB)
                </statement>
                <statement mode="plain">CREATE TABLE UniqueHashes
                            (uniqueHash VARCHAR(128),
                            fileSize INTEGER,
                            thumbId INTEGER,
                            UNIQUE(uniqueHash, fileSize))
                </statement>
                <statement mode="plain">CREATE TABLE FilePaths
                            (path LONGTEXT CHARACTER SET utf8,
                            thumbId INTEGER,
                            UNIQUE(path(255)))
                </statement>
                <statement mode="plain">CREATE TABLE CustomIdentifiers
                            (identifier LONGTEXT CHARACTER SET utf8,
                            thumbId INTEGER,
                            UNIQUE(identifier(255)))
                </statement>
                <statement mode="plain">CREATE TABLE IF NOT EXISTS Settings
                            (keyword LONGTEXT CHARACTER SET utf8 NOT NULL,
                            value LONGTEXT CHARACTER SET utf8,
                            UNIQUE(keyword(255)))
                </statement>
            </dbaction>
            <!-- Thumbnails Indexes DB -->
            <dbaction name="CreateThumbnailsDBIndices" mode="transaction">
                <statement mode="plain">CREATE INDEX id_uniqueHashes ON UniqueHashes (thumbId);</statement>
                <statement mode="plain">CREATE INDEX id_filePaths ON FilePaths (thumbId);</statement>
                <statement mode="plain">CREATE INDEX id_customIdentifiers ON CustomIdentifiers (thumbId);</statement>
            </dbaction>

            <!-- Thumbnails Trigger DB -->
            <dbaction name="CreateThumbnailsDBTrigger" mode="transaction"></dbaction>

            <!-- Migration Statements -->
            <dbaction name="Migrate_Cleanup_DB"><statement mode="plain">
                DROP TABLE IF EXISTS AlbumRoots, Albums, DownloadHistory, FilePaths, ImageComments, ImageCopyright, ImageHaarMatrix, ImageInformation, ImageMetadata, ImagePositions, ImageProperties, ImageTags, Images, Searches, Settings, Tags, TagsTree, Thumbnails, UniqueHashes;
            </statement></dbaction>

            <dbaction name="Migrate_Read_AlbumRoots"><statement mode="query">
                SELECT id, label, status, type, identifier, specificPath FROM AlbumRoots;
            </statement></dbaction>
            <dbaction name="Migrate_Write_AlbumRoots" mode="transaction"><statement mode="query">
                INSERT INTO AlbumRoots (id, label, status, type, identifier, specificPath) VALUES (:id, :label, :status, :type, :identifier, :specificPath);
            </statement></dbaction>
            <dbaction name="Migrate_Read_Albums"><statement mode="query">
                SELECT id, albumRoot, relativePath, date, caption, collection, icon FROM Albums;
            </statement></dbaction>
            <dbaction name="Migrate_Write_Albums" mode="transaction"><statement mode="query">
                INSERT INTO Albums (id, albumRoot, relativePath, date, caption, collection, icon) VALUES (:id, :albumRoot, :relativePath, :date, :caption, :collection, :icon);
            </statement></dbaction>
            <dbaction name="Migrate_Read_Images"><statement mode="query">
                SELECT id, album, name, status, category, modificationDate, fileSize, uniqueHash FROM Images;
            </statement></dbaction>
            <dbaction name="Migrate_Write_Images" mode="transaction"><statement mode="query">
                INSERT INTO Images (id, album, name, status, category, modificationDate, fileSize, uniqueHash) VALUES (:id, :album, :name, :status, :category, :modificationDate, :fileSize, :uniqueHash);
            </statement></dbaction>

            <dbaction name="Migrate_Read_ImageHaarMatrix"><statement mode="query">
                SELECT imageid, modificationDate, uniqueHash, matrix FROM ImageHaarMatrix;
            </statement></dbaction>
            <dbaction name="Migrate_Write_ImageHaarMatrix" mode="transaction"><statement mode="query">
                INSERT INTO ImageHaarMatrix (imageid, modificationDate, uniqueHash, matrix) VALUES (:imageid, :modificationDate, :uniqueHash, :matrix);
            </statement></dbaction>

            <dbaction name="Migrate_Read_ImageInformation"><statement mode="query">
                SELECT imageid, rating, creationDate, digitizationDate, orientation, width, height, format, colorDepth, colorModel FROM ImageInformation;
            </statement></dbaction>
            <dbaction name="Migrate_Write_ImageInformation" mode="transaction"><statement mode="query">
                INSERT INTO ImageInformation (imageid, rating, creationDate, digitizationDate, orientation, width, height, format, colorDepth, colorModel) VALUES (:imageid, :rating, :creationDate, :digitizationDate, :orientation, :width, :height, :format, :colorDepth, :colorModel);
            </statement></dbaction>

            <dbaction name="Migrate_Read_ImageMetadata"><statement mode="query">
                SELECT imageid, make, model, lens, aperture, focalLength, focalLength35, exposureTime, exposureProgram, exposureMode, sensitivity, flash, whiteBalance, whiteBalanceColorTemperature, meteringMode, subjectDistance, subjectDistanceCategory FROM ImageMetadata;
            </statement></dbaction>
            <dbaction name="Migrate_Write_ImageMetadate" mode="transaction"><statement mode="query">
                INSERT INTO ImageMetadate (imageid, make, model, lens, aperture, focalLength, focalLength35, exposureTime, exposureProgram, exposureMode, sensitivity, flash, whiteBalance, whiteBalanceColorTemperature, meteringMode, subjectDistance, subjectDistanceCategory) VALUES (:imageid, :make, :model, :lens, :aperture, :focalLength, :focalLength35, :exposureTime, :exposureProgram, :exposureMode, :sensitivity, :flash, :whiteBalance, :whiteBalanceColorTemperature, :meteringMode, :subjectDistance, :subjectDistanceCategory);
            </statement></dbaction>

            <dbaction name="Migrate_Read_ImagePositions"><statement mode="query">
                SELECT  imageid, latitude, latitudeNumber, longitude, longitudeNumber, altitude, orientation, tilt, roll, accuracy, description FROM ImagePositions;
            </statement></dbaction>
            <dbaction name="Migrate_Write_ImagePositions" mode="transaction"><statement mode="query">
                INSERT INTO ImagePositions (imageid, latitude, latitudeNumber, longitude, longitudeNumber, altitude, orientation, tilt, roll, accuracy, description) VALUES (:imageid, :latitude, :latitudeNumber, :longitude, :longitudeNumber, :altitude, :orientation, :tilt, :roll, :accuracy, :description);
            </statement></dbaction>

            <dbaction name="Migrate_Read_ImageComments"><statement mode="query">
                SELECT  id, imageid, type, language, author, date, comment FROM ImageComments;
            </statement></dbaction>
            <dbaction name="Migrate_Write_ImageComments" mode="transaction"><statement mode="query">
                INSERT INTO ImageComments (id, imageid, type, language, author, date, comment) VALUES (:id, :imageid, :type, :language, :author, :date, :comment);
            </statement></dbaction>

            <dbaction name="Migrate_Read_ImageCopyright"><statement mode="query">
                SELECT  id, imageid, property, value, extraValue FROM ImageCopyright;
            </statement></dbaction>
            <dbaction name="Migrate_Write_ImageCopyright" mode="transaction"><statement mode="query">
                INSERT INTO ImageCopyright (id, imageid, property, value, extraValue) VALUES (:id, :imageid, :property, :value, :extraValue);
            </statement></dbaction>

            <dbaction name="Migrate_Read_Tags"><statement mode="query">
                SELECT  id, pid, name, icon, iconkde FROM Tags;
            </statement></dbaction>
            <dbaction name="Migrate_Write_Tags" mode="transaction">
                <statement mode="plain">LOCK TABLE Tags WRITE;</statement>
                <statement mode="query">SELECT @myLeft := lft FROM Tags WHERE id = :pid;</statement>
                <statement mode="query">SELECT @myLeft := IF (@myLeft is null, 0, @myLeft);</statement>
                <statement mode="query">UPDATE Tags SET rgt = rgt + 2 WHERE rgt > @myLeft;</statement>
                <statement mode="query">UPDATE Tags SET lft = lft + 2 WHERE lft > @myLeft;</statement>
                <statement mode="query">INSERT INTO Tags(id, pid, name, icon, iconkde, lft, rgt) VALUES(:id, :pid, :name, :icon, :iconkde, @myLeft + 1, @myLeft + 2);</statement>
                <statement mode="plain">UNLOCK TABLES;</statement>
            </dbaction>

            <dbaction name="Migrate_Read_ImageTags"><statement mode="query">
                SELECT  imageid, tagid FROM ImageTags;
            </statement></dbaction>
            <dbaction name="Migrate_Write_ImageTags" mode="transaction"><statement mode="query">
                INSERT INTO ImageTags (imageid, tagid) VALUES (:imageid, :tagid);
            </statement></dbaction>

            <dbaction name="Migrate_Read_ImageProperties"><statement mode="query">
                SELECT  imageid, property, value FROM ImageProperties;
            </statement></dbaction>
            <dbaction name="Migrate_Write_ImageProperties" mode="transaction"><statement mode="query">
                INSERT INTO ImageProperties (imageid, property, value) VALUES (:imageid, :property, :value);
            </statement></dbaction>

            <dbaction name="Migrate_Read_Searches"><statement mode="query">
                SELECT  id, type, name, query FROM Searches;
            </statement></dbaction>
            <dbaction name="Migrate_Write_Searches" mode="transaction"><statement mode="query">
                INSERT INTO Searches (id, type, name, query) VALUES (:id, :type, :name, :query);
            </statement></dbaction>

            <dbaction name="Migrate_Read_DownloadHistory"><statement mode="query">
                SELECT id, identifier, filename, filesize, filedate FROM DownloadHistory;
            </statement></dbaction>
            <dbaction name="Migrate_Write_DownloadHistory" mode="transaction"><statement mode="query">
                INSERT INTO DownloadHistory (id, identifier, filename, filesize, filedate) VALUES (:id, :identifier, :filename, :filesize, :filedate);
            </statement></dbaction>

            <dbaction name="Migrate_Read_Settings"><statement mode="query">
                SELECT keyword, value FROM Settings;
            </statement></dbaction>
            <dbaction name="Migrate_Write_Settings" mode="transaction"><statement mode="query">
                INSERT INTO Settings (keyword, value) VALUES (:keyword, :value);
            </statement></dbaction>

            <dbaction name="Delete_Thumbnail_ByPath" mode="query">
                <statement mode="query">
                    SELECT @thumbsId := thumbId FROM FilePaths WHERE path=:path
                </statement>
                <statement mode="query">
                    DELETE FROM UniqueHashes WHERE UniqueHashes.thumbId = @thumbsId;
                </statement>
                <statement mode="query">
                    DELETE FROM FilePaths WHERE FilePaths.thumbId = @thumbsId;
                </statement>
                <statement mode="query">
                    DELETE FROM Thumbnails WHERE id = @thumbsId;
                </statement>
            </dbaction>

            <dbaction name="Delete_Thumbnail_ByUniqueHashId" mode="query">
                <statement mode="query">
                    SELECT @thumbsId := thumbId FROM UniqueHashes WHERE uniqueHash=:uniqueHash AND fileSize=:filesize
                </statement>
                <statement mode="query">
                    DELETE FROM UniqueHashes WHERE UniqueHashes.thumbId = @thumbsId;
                </statement>
                <statement mode="query">
                    DELETE FROM FilePaths WHERE FilePaths.thumbId = @thumbsId;
                </statement>
                <statement mode="query">
                    DELETE FROM Thumbnails WHERE id = @thumbsId;
                </statement>
            </dbaction>

            <dbaction name="Delete_Thumbnail_ByCustomIdentifier" mode="query">
                <statement mode="query">
                    SELECT @thumbsId := thumbId FROM CustomIdentifiers WHERE identifier=:identifier
                </statement>
                <statement mode="query">
                    DELETE FROM UniqueHashes WHERE UniqueHashes.thumbId = @thumbsId;
                </statement>
                <statement mode="query">
                    DELETE FROM FilePaths WHERE FilePaths.thumbId = @thumbsId;
                </statement>
                <statement mode="query">
                    DELETE FROM CustomIdentifiers WHERE CustomIdentifiers.thumbId = @thumbsId;
                </statement>
                <statement mode="query">
                    DELETE FROM Thumbnails WHERE id = @thumbsId;
                </statement>
            </dbaction>


            <!-- Migration from DB Version 5 (0.10 - 1.4) to Version 6 (1.5-) -->
            <dbaction name="UpdateSchemaFromV5ToV6" mode="transaction">
            <statement mode="plain">CREATE TABLE ImageHistory
                            (imageid INTEGER PRIMARY KEY,
                             uuid VARCHAR(128),
                             history LONGTEXT CHARACTER SET utf8);</statement>
            <statement mode="plain">CREATE TABLE ImageRelations
                            (subject INTEGER,
                             object INTEGER,
                             type INTEGER,
                             UNIQUE(subject, object, type));</statement>
            <statement mode="plain">CREATE TABLE TagProperties
                            (tagid INTEGER,
                             property TEXT CHARACTER SET utf8,
                             value LONGTEXT CHARACTER SET utf8);</statement>
            <statement mode="plain">CREATE TABLE ImageTagProperties
                            (imageid INTEGER,
                             tagid INTEGER,
                             property TEXT CHARACTER SET utf8,
                             value LONGTEXT CHARACTER SET utf8);</statement>
            <statement mode="plain">CREATE INDEX tag_id_index  ON ImageTags (imageid);</statement>
            <statement mode="plain">CREATE INDEX image_name_index ON Images (name(996));</statement>
            <statement mode="plain">CREATE INDEX creationdate_index ON ImageInformation (creationDate);</statement>
            <statement mode="plain">CREATE INDEX comments_imageid_index ON ImageComments (imageid);</statement>
            <statement mode="plain">CREATE INDEX copyright_imageid_index ON ImageCopyright (imageid);</statement>
            <statement mode="plain">CREATE INDEX uuid_index ON ImageHistory (uuid);</statement>
            <statement mode="plain">CREATE INDEX subject_relations_index ON ImageRelations (subject);</statement>
            <statement mode="plain">CREATE INDEX object_relations_index ON ImageRelations (object);</statement>
            <statement mode="plain">CREATE INDEX tagproperties_index ON TagProperties (tagid);</statement>
            <statement mode="plain">CREATE INDEX imagetagproperties_index ON ImageTagProperties (imageid, tagid);</statement>
            <statement mode="plain">CREATE INDEX imagetagproperties_imageid_index ON ImageTagProperties (imageid);</statement>
            <statement mode="plain">CREATE INDEX imagetagproperties_tagid_index ON ImageTagProperties (tagid);</statement>
            <statement mode="plain">ALTER TABLE Images CHANGE uniqueHash uniqueHash VARCHAR(128);</statement>
            <statement mode="plain">DROP TRIGGER IF EXISTS delete_image;</statement>
            <statement mode="plain">CREATE TRIGGER delete_image AFTER DELETE ON Images
                    FOR EACH ROW BEGIN
                        DELETE FROM ImageTags          WHERE imageid=OLD.id;
                        DELETE From ImageHaarMatrix    WHERE imageid=OLD.id;
                        DELETE From ImageInformation   WHERE imageid=OLD.id;
                        DELETE From ImageMetadata      WHERE imageid=OLD.id;
                        DELETE From ImagePositions     WHERE imageid=OLD.id;
                        DELETE From ImageComments      WHERE imageid=OLD.id;
                        DELETE From ImageCopyright     WHERE imageid=OLD.id;
                        DELETE From ImageProperties    WHERE imageid=OLD.id;
                        DELETE From ImageHistory       WHERE imageid=OLD.id;
                        DELETE FROM ImageRelations     WHERE subject=OLD.id OR object=OLD.id;
                        DELETE FROM ImageTagProperties WHERE imageid=OLD.id;
                        UPDATE Albums SET icon=null    WHERE icon=OLD.id;
                        UPDATE Tags SET icon=null      WHERE icon=OLD.id;
                    END;
            </statement>
            <statement mode="plain">DROP TRIGGER IF EXISTS delete_tag;</statement>
            <statement mode="plain">CREATE TRIGGER delete_tag AFTER DELETE ON Tags
                FOR EACH ROW BEGIN
                    DELETE FROM ImageTags          WHERE tagid=OLD.id;
                    DELETE FROM TagProperties      WHERE tagid=OLD.id;
                    DELETE FROM ImageTagProperties WHERE tagid=OLD.id;
                    DELETE FROM TagsTree;
                    REPLACE INTO TagsTree
                    SELECT node.id, parent.pid
                    FROM Tags AS node, Tags AS parent
                    WHERE node.lft BETWEEN parent.lft AND parent.rgt
                    ORDER BY parent.lft;
                END;
            </statement>
            </dbaction>
            <dbaction name="UpdateThumbnailsDBSchemaFromV1ToV2" mode="transaction">
                <statement mode="plain">ALTER TABLE UniqueHashes CHANGE uniqueHash uniqueHash VARCHAR(128);</statement>
                <statement mode="plain">CREATE TABLE CustomIdentifiers
                            (identifier LONGTEXT CHARACTER SET utf8,
                            thumbId INTEGER,
                            UNIQUE(identifier(333)))
                </statement>
                <statement mode="plain">CREATE INDEX id_customIdentifiers ON CustomIdentifiers (thumbId);</statement>
            </dbaction>

        </dbactions>
    </database>
</databaseconfig>

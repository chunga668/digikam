/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-08-08
 * Description : a class that manages the files to be renamed
 *
 * Copyright (C) 2009-2010 by Andi Clemens <andi dot clemens at gmx dot net>
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

#ifndef ADVANCEDRENAMEMANAGER_H
#define ADVANCEDRENAMEMANAGER_H

// KDE includes

#include <KUrl>

// Qt includes

#include <QString>
#include <QStringList>

namespace Digikam
{

class AdvancedRenameWidget;
class Parser;
class ParseSettings;

class AdvancedRenameManager
{
public:

    enum ParserType
    {
        DefaultParser = 0,
        ImportParser
    };

    enum SortType
    {
        SortAscending = 0,
        SortDescending,
        SortCustom
    };

public:

    AdvancedRenameManager();
    AdvancedRenameManager(const QList<ParseSettings>& files, SortType sort = SortCustom);
    virtual ~AdvancedRenameManager();

    void addFiles(const QList<ParseSettings>& files, SortType sort = SortCustom);
    void reset();

    void parseFiles();
    void parseFiles(ParseSettings& settings);
    void parseFiles(const QString& parseString);
    void parseFiles(const QString& parseString, ParseSettings& settings);

    void setParserType(ParserType type);
    Parser* getParser();

    void setSortType(SortType type);

    void setStartIndex(int index);

    void setWidget(AdvancedRenameWidget* widget);

    int indexOfFile(const QString& filename);
    int indexOfFolder(const QString& filename);
    int indexOfFileGroup(const QString& filename);
    QString newName(const QString& filename);

    QStringList            fileList();
    QMap<QString, QString> newFileList();

private:

    void addFile(const QString& filename);
    void addFile(const QString& filename, const QDateTime& datetime);
    bool initialize();

    QString fileGroupKey(const QString& filename);

    void clearMappings();
    void clearAll();

    QStringList sortListCaseInsensitive(const QStringList& list);

private:

    class ParseManagerPriv;
    ParseManagerPriv* const d;
};

} // namespace Digikam

#endif /* ADVANCEDRENAMEMANAGER_H */

/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-11-15
 * Description : collections setup tab model/view
 *
 * Copyright (C) 2008-2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2005-2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef SETUPCOLLECTIONVIEW_H
#define SETUPCOLLECTIONVIEW_H

// Qt includes

#include <QAbstractItemModel>
#include <QAbstractItemDelegate>
#include <QList>
#include <QTreeView>

// KDE includes

#include <kwidgetitemdelegate.h>

// Local includes

#include "collectionlocation.h"

namespace Digikam
{

class SetupCollectionModelPriv;

class SetupCollectionModel : public QAbstractItemModel
{
    Q_OBJECT

public:

    /** SetupCollectionModel is a model specialized for use in
     *  SetupCollectionTreeView. It provides a reads the current collections
     *  from CollectionManager, displays them in three categories,
     *  and supports adding and removing collections
     */

    enum SetupCollectionDataRole
    {
        /// Returns true if the model index is the index of a category
        IsCategoryRole             = Qt::UserRole,
        /// The text for the category button
        CategoryButtonDisplayRole  = Qt::UserRole + 1,
        CategoryButtonMapId        = Qt::UserRole + 2,
        /// Returns true if the model index is the index of a button
        IsButtonRole               = Qt::UserRole + 3,
        /// The pixmap of the button
        ButtonDecorationRole       = Qt::UserRole + 4,
        ButtonMapId                = Qt::UserRole + 5
    };

    enum Columns
    {
        ColumnStatus       = 0,
        ColumnName         = 1,
        ColumnPath         = 2,
        ColumnDeleteButton = 3,
        NumberOfColumns
    };

    enum Category
    {
        CategoryLocal      = 0,
        CategoryRemovable  = 1,
        CategoryRemote     = 2,
        NumberOfCategories
    };

    SetupCollectionModel(QObject* parent = 0);
    ~SetupCollectionModel();

    /// Read collections from CollectionManager
    void loadCollections();
    /// Set a widget used as parent for dialogs and message boxes
    void setParentWidgetForDialogs(QWidget* widget);
    /// Apply the changed settings to CollectionManager
    void apply();

    // QAbstractItemModel implementation
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const;
    virtual Qt::ItemFlags flags(const QModelIndex& index) const;
    virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole );
    virtual QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
    virtual QModelIndex parent(const QModelIndex& index) const;

    /*
    virtual Qt::DropActions supportedDropActions() const;
    virtual QStringList mimeTypes() const;
    virtual bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex& parent);
    virtual QMimeData * mimeData(const QModelIndexList& indexes) const;
    */

    QModelIndex indexForCategory(Category category) const;
    QList<QModelIndex> categoryIndexes() const;

Q_SIGNALS:

    /// Emitted when all collections were loaded and the model reset in loadCollections
    void collectionsLoaded();

public Q_SLOTS:

    /** Forward category button clicked signals to this slot.
     *  mappedId is retrieved with the CategoryButtonMapId role
     *  for the model index of the button */
    void slotCategoryButtonPressed(int mappedId);
    /** Forward button clicked signals to this slot.
     *  mappedId is retrieved with the ButtonMapId role
     *  for the model index of the button */
    void slotButtonPressed(int mappedId);

protected Q_SLOTS:

    void addCollection(int category);
    void deleteCollection(int internalId);

protected:

    QModelIndex indexForId(int id, int column) const;

    int categoryButtonMapId(const QModelIndex& index) const;
    int buttonMapId(const QModelIndex& index) const;

    static Category typeToCategory(CollectionLocation::Type type);

    class Item
    {
    public:

        Item();
        Item(const CollectionLocation& location);
        Item(const QString& path, const QString& label, SetupCollectionModel::Category category);

        CollectionLocation location;
        QString            label;
        QString            path;
        int                parentId;
        bool               deleted;
    };

    QList<Item>  m_collections;
    QWidget*     m_dialogParentWidget;
};

// -----------------------------------------------------------------------

class SetupCollectionTreeView : public QTreeView
{
    Q_OBJECT

public:

    SetupCollectionTreeView(QWidget* parent = 0);

    void setModel(SetupCollectionModel* model);

protected Q_SLOTS:

    void modelLoadedCollections();

private:

    void setModel(QAbstractItemModel* model)
    {
        setModel(static_cast<SetupCollectionModel*>(model));
    }
};

} // namespace Digikam

#endif /* SETUPCOLLECTIONVIEW_H */
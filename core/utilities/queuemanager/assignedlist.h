/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2008-11-27
 * Description : batch tools list assigned to an queued item.
 *
 * Copyright (C) 2008-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef ASSIGNED_LIST_H
#define ASSIGNED_LIST_H

// Qt includes

#include <QTreeWidget>
#include <QWidget>
#include <QIcon>

// Local includes

#include "batchtool.h"

namespace Digikam
{

class QueueSettings;

class AssignedListViewItem : public QTreeWidgetItem
{

public:

    AssignedListViewItem(QTreeWidget* parent, const BatchToolSet& set);
    AssignedListViewItem(QTreeWidget* parent, QTreeWidgetItem* preceding, const BatchToolSet& set);
    virtual ~AssignedListViewItem();

    void setToolSet(const BatchToolSet& set);
    BatchToolSet toolSet();

    void setProgressIcon(const QPixmap& icon);

    void setCanceled();
    void setFailed();
    void setDone();
    void reset();

private:

    BatchToolSet m_set;
};

// -------------------------------------------------------------------------

class AssignedListView : public QTreeWidget
{
    Q_OBJECT

public:

    AssignedListView(QWidget* parent);
    ~AssignedListView();

    int assignedCount();
    AssignedBatchTools assignedList();

    AssignedListViewItem* insertTool(AssignedListViewItem* preceding, const BatchToolSet& set);
    AssignedListViewItem* moveTool(AssignedListViewItem* preceding, const BatchToolSet& set);
    AssignedListViewItem* addTool(int index, const BatchToolSet& set);
    AssignedListViewItem* findTool(int index);
    bool removeTool(const BatchToolSet& set);

    void setCurrentTool(int index);
    void reset();
    void setBusy(bool b);

Q_SIGNALS:

    void signalToolSelected(const BatchToolSet&);
    void signalAssignedToolsChanged(const AssignedBatchTools&);

public Q_SLOTS:

    void slotMoveCurrentToolUp();
    void slotMoveCurrentToolDown();
    void slotRemoveCurrentTool();
    void slotClearToolsList();
    void slotQueueSelected(int, const QueueSettings&, const AssignedBatchTools&);
    void slotSettingsChanged(const BatchToolSet&);
    void slotAssignTools(const QMap<int, QString>&);

protected:

    void keyPressEvent(QKeyEvent*);

private Q_SLOTS:

    void slotSelectionChanged();
    void slotContextMenu();

private:

    AssignedListViewItem* findTool(const BatchToolSet& set);
    void assignTools(const QMap<int, QString>& map, AssignedListViewItem* preceding);

    Qt::DropActions supportedDropActions() const;
    QStringList mimeTypes() const;
    QMimeData* mimeData(const QList<QTreeWidgetItem*> items) const;

    void dragEnterEvent(QDragEnterEvent*);
    void dragMoveEvent(QDragMoveEvent*);
    void dropEvent(QDropEvent*);
};

}  // namespace Digikam

#endif // ASSIGNED_LIST_H

/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-03-25
 * Description : Tree View for album models
 *
 * Copyright (C) 2009-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2010 by Andi Clemens <andi dot clemens at gmx dot net>
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

#include "albumtreeview.moc"

// Qt includes

#include <QMouseEvent>
#include <QScrollBar>
#include <QStyledItemDelegate>
#include <QTimer>

// KDE includes

#include <kdebug.h>
#include <kiconloader.h>
#include <kio/job.h>
#include <kio/jobuidelegate.h>
#include <kmenu.h>

// Local includes

#include "albumdragdrop.h"
#include "albummanager.h"
#include "albummodeldragdrophandler.h"
#include "albumsettings.h"
#include "albumthumbnailloader.h"
#include "contextmenuhelper.h"
#include "metadatamanager.h"
#include "tagdragdrop.h"
#include "tagmodificationhelper.h"

namespace Digikam
{

template <class A>
static inline A* currentAlbum(QItemSelectionModel* selModel, AlbumFilterModel* filterModel)
{
    return static_cast<A*>(filterModel->albumForIndex(selModel->currentIndex()));
}

template <class A>
static QList<A*> selectedAlbums(QItemSelectionModel* selModel, AlbumFilterModel* filterModel)
{
    QList<QModelIndex> indexes = selModel->selectedIndexes();
    QList<A*> albums;
    foreach (const QModelIndex& index, indexes)
    {
        albums << static_cast<A*>(filterModel->albumForIndex(index));
    }
    return albums;
}

struct State
{
    State() :
        selected(false), expanded(false), currentIndex(false)
    {
    }
    bool selected;
    bool expanded;
    bool currentIndex;
};

class AlbumTreeViewDelegate : public QStyledItemDelegate
{
public:
    AlbumTreeViewDelegate(AbstractAlbumTreeView* treeView = 0)
        : QStyledItemDelegate(treeView),
          m_treeView(treeView), m_height(0)
    {
        updateHeight();
    }

    virtual QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
    {
        QSize size = QStyledItemDelegate::sizeHint(option, index);
        size.setHeight(qMax(size.height(), m_height));
        return size;
    }

    void updateHeight()
    {
        int h = qMax(AlbumThumbnailLoader::instance()->thumbnailSize() + 2, m_treeView->fontMetrics().height());

        if (h % 2 > 0)
        {
            ++h;
        }

        setHeight(h);
    }

    void setHeight(int height)
    {
        if (m_height == height)
        {
            return;
        }

        m_height = height;
        emit sizeHintChanged(QModelIndex());
    }

protected:

    AbstractAlbumTreeView* m_treeView;
    int m_height;
};

class AbstractAlbumTreeViewPriv
{

public:
    AbstractAlbumTreeViewPriv() :
        delegate(0),
        expandOnSingleClick(false),
        expandNewCurrent(false),
        selectAlbumOnClick(false),
        selectOnContextMenu(true),
        enableContextMenu(false),
        setInAlbumManager(false),
        resizeColumnsTimer(0)
    {
    }

    AlbumTreeViewDelegate* delegate;

    bool expandOnSingleClick;
    bool expandNewCurrent;
    bool selectAlbumOnClick;
    bool selectOnContextMenu;
    bool enableContextMenu;
    bool setInAlbumManager;

    QMap<int, State>     statesByAlbumId;
    QMap<int, State>     searchBackup;

    static const QString configSelectionEntry;
    static const QString configExpansionEntry;
    static const QString configCurrentIndexEntry;
    static const QString configSortColumnEntry;
    static const QString configSortOrderEntry;

    QTimer*              resizeColumnsTimer;

    AlbumPointer<Album>  lastSelectedAlbum;

};
const QString AbstractAlbumTreeViewPriv::configSelectionEntry("Selection");
const QString AbstractAlbumTreeViewPriv::configExpansionEntry("Expansion");
const QString AbstractAlbumTreeViewPriv::configCurrentIndexEntry("CurrentIndex");
const QString AbstractAlbumTreeViewPriv::configSortColumnEntry("SortColumn");
const QString AbstractAlbumTreeViewPriv::configSortOrderEntry("SortOrder");

// --------------------------------------------------------

AbstractAlbumTreeView::AbstractAlbumTreeView(QWidget* parent, Flags flags)
    : QTreeView(parent), StateSavingObject(this),
      m_albumModel(0), m_albumFilterModel(0), m_dragDropHandler(0),
      m_checkOnMiddleClick(false), m_restoreCheckState(false), m_flags(flags),
      d(new AbstractAlbumTreeViewPriv)
{
    if (flags & CreateDefaultDelegate)
    {
        d->delegate = new AlbumTreeViewDelegate(this);
        setItemDelegate(d->delegate);
        setUniformRowHeights(true);
    }

    d->resizeColumnsTimer = new QTimer(this);
    d->resizeColumnsTimer->setInterval(200);
    d->resizeColumnsTimer->setSingleShot(true);
    connect(d->resizeColumnsTimer, SIGNAL(timeout()),
            this, SLOT(adaptColumnsToContent()));

    connect(AlbumSettings::instance(), SIGNAL(setupChanged()),
            this, SLOT(albumSettingsChanged()));
    connect(this, SIGNAL(currentAlbumChanged(Album*)),
            this, SLOT(currentAlbumChangedForBackupSelection(Album*)));

    if (flags & CreateDefaultFilterModel)
    {
        setAlbumFilterModel(new AlbumFilterModel(this));
    }
}

AbstractAlbumTreeView::~AbstractAlbumTreeView()
{
    delete d;
}

void AbstractAlbumTreeView::setAlbumModel(AbstractSpecificAlbumModel* model)
{
    if (m_albumModel == model)
    {
        return;
    }

    if (m_albumModel)
    {
        disconnect(m_albumModel, 0, this, 0);
    }

    m_albumModel = model;

    if (m_albumFilterModel)
    {
        m_albumFilterModel->setSourceAlbumModel(m_albumModel);
    }

    if (m_albumModel)
    {
        if (!m_albumModel->rootAlbum())
        {
            connect(m_albumModel, SIGNAL(rootAlbumAvailable()),
                    this, SLOT(slotRootAlbumAvailable()));
        }

        if (m_albumFilterModel)
        {
            expand(m_albumFilterModel->rootAlbumIndex());
        }
    }
}

void AbstractAlbumTreeView::setAlbumFilterModel(AlbumFilterModel* filterModel)
{
    if (filterModel == m_albumFilterModel)
    {
        return;
    }

    if (m_albumFilterModel)
    {
        disconnect(m_albumFilterModel);
    }

    if (selectionModel())
    {
        disconnect(selectionModel());
    }

    m_albumFilterModel = filterModel;
    setModel(m_albumFilterModel);

    if (m_albumFilterModel)
    {
        m_albumFilterModel->setSourceAlbumModel(m_albumModel);

        connect(m_albumFilterModel, SIGNAL(searchTextSettingsAboutToChange(bool, bool)),
                this, SLOT(slotSearchTextSettingsAboutToChange(bool, bool)));
        connect(m_albumFilterModel, SIGNAL(searchTextSettingsChanged(bool, bool)),
                this, SLOT(slotSearchTextSettingsChanged(bool, bool)));


        connect(selectionModel(), SIGNAL(currentChanged(const QModelIndex&, const QModelIndex&)),
                this, SLOT(slotCurrentChanged()));

        connect(selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)),
                this, SLOT(slotCurrentChanged()));

        connect(m_albumFilterModel, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)),
                this, SLOT(adaptColumnsOnDataChange(const QModelIndex&, const QModelIndex&)));
        connect(m_albumFilterModel, SIGNAL(rowsInserted(const QModelIndex&, int, int)),
                this, SLOT(adaptColumnsOnRowChange(const QModelIndex&, int, int)));
        connect(m_albumFilterModel, SIGNAL(rowsRemoved(const QModelIndex&, int, int)),
                this, SLOT(adaptColumnsOnRowChange(const QModelIndex&, int, int)));
        connect(m_albumFilterModel, SIGNAL(layoutChanged()),
                this, SLOT(adaptColumnsOnLayoutChange()));
        adaptColumnsToContent();

        if (m_albumModel)
        {
            expand(m_albumFilterModel->rootAlbumIndex());
        }
    }
}

AbstractSpecificAlbumModel* AbstractAlbumTreeView::albumModel() const
{
    return m_albumModel;
}

AlbumFilterModel* AbstractAlbumTreeView::albumFilterModel() const
{
    return m_albumFilterModel;
}

void AbstractAlbumTreeView::setExpandOnSingleClick(bool doThat)
{
    d->expandOnSingleClick = doThat;
}

void AbstractAlbumTreeView::setExpandNewCurrentItem(bool doThat)
{
    d->expandNewCurrent = doThat;
}

void AbstractAlbumTreeView::setSelectAlbumOnClick(bool selectOnClick)
{
    d->selectAlbumOnClick = selectOnClick;
}

QModelIndex AbstractAlbumTreeView::indexVisuallyAt(const QPoint& p)
{
    if (viewport()->rect().contains(p))
    {
        QModelIndex index = indexAt(p);

        if (index.isValid() && visualRect(index).contains(p))
        {
            return index;
        }
    }

    return QModelIndex();
}

void AbstractAlbumTreeView::slotSearchTextSettingsAboutToChange(bool searched, bool willSearch)
{

    // backup before we begin searching
    if (!searched && willSearch && d->searchBackup.isEmpty())
    {

        kDebug() << "Searching started, backing up state";

        QList<int> selection, expansion;
        saveStateRecursive(QModelIndex(), selection, expansion);

        // selection is ignored here because the user may have changed this
        // while searching
        foreach (const int& expandedId, expansion)
        {
            d->searchBackup[expandedId].expanded = true;
        }

        // also backup the last selected album in case this didn't work via the
        // slot
        Album* current = currentAlbum<Album>(selectionModel(), m_albumFilterModel);
        d->lastSelectedAlbum = current;

    }

}

void AbstractAlbumTreeView::slotSearchTextSettingsChanged(bool wasSearching, bool searched)
{

    // ensure that all search results are visible if there is currently a search
    // working
    if (searched)
    {
        kDebug() << "Searched, expanding all results";
        expandMatches(QModelIndex());
    }

    // restore the tree view state if searching finished
    if (wasSearching && !searched && !d->searchBackup.isEmpty())
    {

        kDebug() << "Searching finished, restoring tree view state";

        collapseAll();
        restoreStateForHierarchy(QModelIndex(), d->searchBackup);
        d->searchBackup.clear();

        if (d->lastSelectedAlbum)
        {
            setCurrentAlbum(d->lastSelectedAlbum, false);
            // doing this twice somehow ensures that all parents are expanded
            // and we are at the right position. Maybe a hack... ;)
            scrollTo(m_albumFilterModel->indexForAlbum(d->lastSelectedAlbum));
            scrollTo(m_albumFilterModel->indexForAlbum(d->lastSelectedAlbum));
        }

    }

}

void AbstractAlbumTreeView::currentAlbumChangedForBackupSelection(Album* currentAlbum)
{
    d->lastSelectedAlbum = currentAlbum;
}

void AbstractAlbumTreeView::slotRootAlbumAvailable()
{
    expand(m_albumFilterModel->rootAlbumIndex());
}

bool AbstractAlbumTreeView::expandMatches(const QModelIndex& index)
{
    bool anyMatch        = false;

    // expand index if a child matches
    QModelIndex source_index = m_albumFilterModel->mapToSource(index);
    AlbumFilterModel::MatchResult result = m_albumFilterModel->matchResult(source_index);

    switch (result)
    {
        case AlbumFilterModel::NoMatch:
            if (index != rootIndex())
                return false;
        case AlbumFilterModel::ParentMatch:
            // Does not rule out additional child match, return value is unknown
            break;
        case AlbumFilterModel::DirectMatch:
            // Does not rule out additional child match, but we know we will return true
            anyMatch = true;
            break;
        case AlbumFilterModel::ChildMatch:
        case AlbumFilterModel::SpecialMatch:
            // We know already to expand, and we know already we will return true.
            anyMatch = true;
            expand(index);
            break;
    }

    // Recurse. Expand if children if have an (indirect) match
    const int rows = m_albumFilterModel->rowCount(index);
    for (int i = 0; i < rows; ++i)
    {
        QModelIndex child = m_albumFilterModel->index(i, 0, index);
        bool childResult  = expandMatches(child);

        if (childResult)
        {
            anyMatch = true;
            // if there is a direct match _and_ a child match, dont forget to expand the parent
            expand(index);
        }
    }

    return anyMatch;
}

void AbstractAlbumTreeView::setSearchTextSettings(const SearchTextSettings& settings)
{
    m_albumFilterModel->setSearchTextSettings(settings);
}

void AbstractAlbumTreeView::setAlbumManagerCurrentAlbum(bool set)
{
    d->setInAlbumManager = set;
}

void AbstractAlbumTreeView::setCurrentAlbum(Album* album, bool selectInAlbumManager)
{
    if (!model())
    {
        return;
    }

    setCurrentIndex(albumFilterModel()->indexForAlbum(album));

    // check local and global flag
    if (selectInAlbumManager && d->setInAlbumManager)
    {
        AlbumManager::instance()->setCurrentAlbum(album);
    }
}

void AbstractAlbumTreeView::slotCurrentChanged()
{
    emit currentAlbumChanged(currentAlbum<Album>(selectionModel(), m_albumFilterModel));
}

void AbstractAlbumTreeView::slotSelectionChanged()
{
    emit selectedAlbumsChanged(selectedAlbums<Album>(selectionModel(), m_albumFilterModel));
}

void AbstractAlbumTreeView::mousePressEvent(QMouseEvent* e)
{

    if (d->selectAlbumOnClick && e->button() == Qt::LeftButton)
    {
        QModelIndex index = indexVisuallyAt(e->pos());

        if (index.isValid())
        {
            Album* album = albumFilterModel()->albumForIndex(index);

            if (album && d->setInAlbumManager)
            {
                AlbumManager::instance()->setCurrentAlbum(album);
            }
        }
    }

    if ((d->expandOnSingleClick || d->expandNewCurrent) && e->button() == Qt::LeftButton)
    {
        QModelIndex index = indexVisuallyAt(e->pos());

        if (index.isValid())
        {
            if (d->expandOnSingleClick)
            {
                // See B.K.O #126871: collapse/expand treeview using left mouse button single click.
                // Exception: If a newly selected item is already expanded, do not collapse on selection.
                bool expanded = isExpanded(index);

                if (index == currentIndex() || !expanded)
                {
                    setExpanded(index, !expanded);
                }
            }
            else
            {
                if (index != currentIndex())
                {
                    expand(index);
                }
            }
        }
    }
    else if (m_checkOnMiddleClick && e->button() == Qt::MidButton)
    {
        Album* a = m_albumFilterModel->albumForIndex(indexAt(e->pos()));

        if (a)
        {
            middleButtonPressed(a);
        }
    }

    QTreeView::mousePressEvent(e);
}

void AbstractAlbumTreeView::middleButtonPressed(Album*)
{
    // reimplement if needed
}

void AbstractAlbumTreeView::startDrag(Qt::DropActions supportedActions)
{
    QModelIndexList indexes = selectedIndexes();

    if (indexes.count() > 0)
    {
        QMimeData* data = m_albumFilterModel->mimeData(indexes);

        if (!data)
        {
            return;
        }

        QStyleOptionViewItem option = viewOptions();
        option.rect = viewport()->rect();
        QPixmap pixmap = /*m_delegate->*/pixmapForDrag(option, indexes);
        QDrag* drag = new QDrag(this);
        drag->setPixmap(pixmap);
        drag->setMimeData(data);
        drag->exec(supportedActions, Qt::IgnoreAction);
    }
}

//TODO: Move to delegate, when we have one.
//      Copy code from image delegate for creating icons when dragging multiple items
QPixmap AbstractAlbumTreeView::pixmapForDrag(const QStyleOptionViewItem&, QList<QModelIndex> indexes)
{
    if (indexes.isEmpty())
    {
        return QPixmap();
    }

    QVariant decoration = indexes.first().data(Qt::DecorationRole);
    return decoration.value<QPixmap>();
}

void AbstractAlbumTreeView::dragEnterEvent(QDragEnterEvent* e)
{
    AlbumModelDragDropHandler* handler = m_albumModel->dragDropHandler();

    if (handler && handler->acceptsMimeData(e->mimeData()))
    {
        setState(DraggingState);
        e->accept();
    }
    else
    {
        e->ignore();
    }
}

void AbstractAlbumTreeView::dragMoveEvent(QDragMoveEvent* e)
{
    QTreeView::dragMoveEvent(e);
    AlbumModelDragDropHandler* handler = m_albumModel->dragDropHandler();

    if (handler)
    {
        QModelIndex index = indexVisuallyAt(e->pos());
        Qt::DropAction action = handler->accepts(e, m_albumFilterModel->mapToSourceAlbumModel(index));

        if (action == Qt::IgnoreAction)
        {
            e->ignore();
        }
        else
        {
            e->setDropAction(action);
            e->accept();
        }
    }
}

void AbstractAlbumTreeView::dragLeaveEvent(QDragLeaveEvent* e)
{
    QTreeView::dragLeaveEvent(e);
}

void AbstractAlbumTreeView::dropEvent(QDropEvent* e)
{
    QTreeView::dropEvent(e);
    AlbumModelDragDropHandler* handler = m_albumModel->dragDropHandler();

    if (handler)
    {
        QModelIndex index = indexVisuallyAt(e->pos());

        if (handler->dropEvent(this, e, m_albumFilterModel->mapToSourceAlbumModel(index)))
        {
            e->accept();
        }
    }
}

bool AbstractAlbumTreeView::viewportEvent(QEvent* event)
{
    return QTreeView::viewportEvent(event);
}

void AbstractAlbumTreeView::doLoadState()
{

    KConfigGroup configGroup = getConfigGroup();

    //kDebug() << "Loading view state from " << this << configGroup.name() << objectName();

    // extract the selection from the config
    const QStringList selection = configGroup.readEntry(entryName(d->configSelectionEntry),
                                  QStringList());
    //kDebug() << "selection: " << selection;
    foreach (const QString& key, selection)
    {
        bool validId;
        int id = key.toInt(&validId);

        if (validId)
        {
            d->statesByAlbumId[id].selected = true;
        }
    }

    // extract expansion state from config
    const QStringList expansion = configGroup.readEntry(entryName(d->configExpansionEntry),
                                  QStringList());
    //kDebug() << "expansion: " << expansion;
    foreach (const QString& key, expansion)
    {
        bool validId;
        int id = key.toInt(&validId);

        if (validId)
        {
            d->statesByAlbumId[id].expanded = true;
        }
    }

    // extract current index from config
    const QString key = configGroup.readEntry(entryName(d->configCurrentIndexEntry), QString());
    //kDebug() << "currentIndex: " << key;
    bool validId;
    const int id = key.toInt(&validId);

    if (validId)
    {
        d->statesByAlbumId[id].currentIndex = true;
    }

    /*
    for (QMap<int, Digikam::State>::iterator it = d->statesByAlbumId.begin(); it
        != d->statesByAlbumId.end(); ++it)
    {
        kDebug() << "id = " << it.key() << ": recovered state (selected = "
        << it.value().selected << ", expanded = "
        << it.value().expanded << ", currentIndex = "
        << it.value().currentIndex << ")";
    }
    */


    // initial restore run, for everything already loaded
    //kDebug() << "initial restore run with " << model()->rowCount() << " rows";
    restoreStateForHierarchy(QModelIndex(), d->statesByAlbumId);

    // also restore the sorting order
    sortByColumn(configGroup.readEntry(entryName(d->configSortColumnEntry), 0),
                 (Qt::SortOrder) configGroup.readEntry(entryName(d->configSortOrderEntry), (int) Qt::AscendingOrder));

    // use a timer to scroll to the first possible selected album
    QTimer* selectCurrentTimer = new QTimer(this);
    selectCurrentTimer->setInterval(200);
    selectCurrentTimer->setSingleShot(true);
    connect(selectCurrentTimer, SIGNAL(timeout()),
            this, SLOT(scrollToSelectedAlbum()));

}

void AbstractAlbumTreeView::restoreStateForHierarchy(const QModelIndex& index, QMap<int, Digikam::State> &stateStore)
{
    restoreState(index, stateStore);

    // do a recursive call of the state restoration
    for (int i = 0; i < model()->rowCount(index); ++i)
    {
        const QModelIndex child = model()->index(i, 0, index);
        restoreStateForHierarchy(child, stateStore);
    }
}

void AbstractAlbumTreeView::restoreState(const QModelIndex& index, QMap<int, Digikam::State> &stateStore)
{
    Album* album = albumFilterModel()->albumForIndex(index);

    if (album && stateStore.contains(album->id()))
    {

        Digikam::State state = stateStore.value(album->id());

        /*
        kDebug() << "Trying to restore state of album " << album->title() << "(" <<album->id() << ")"
                 << ": state(selected = " << state.selected
                 << ", expanded = " << state.expanded
                 << ", currentIndex = " << state.currentIndex << ")" << this;
        */
        if (state.selected)
        {
            //kDebug() << "Selecting" << album->title();
            selectionModel()->select(index, QItemSelectionModel::SelectCurrent
                                     | QItemSelectionModel::Rows);
        }

        // restore expansion state but ensure that the root album is always
        // expanded
        if (!album->isRoot())
        {
            setExpanded(index, state.expanded);
        }
        else
        {
            setExpanded(index, true);
        }

        // restore the current index
        if (state.currentIndex)
        {
            //kDebug() << "Setting current index" << album->title() << "(" << album->id() << ")";
            selectionModel()->setCurrentIndex(index, QItemSelectionModel::SelectCurrent
                                              | QItemSelectionModel::Rows);
        }
    }
}

void AbstractAlbumTreeView::rowsInserted(const QModelIndex& parent, int start, int end)
{
    QTreeView::rowsInserted(parent, start, end);

    if (!d->statesByAlbumId.isEmpty())
    {
        //kDebug() << "slot rowInserted called with index = " << index
        //         << ", start = " << start << ", end = " << end << "remaining ids" << d->statesByAlbumId.keys();

        // Restore state for parent a second time - expansion can only be restored if there are children
        restoreState(parent, d->statesByAlbumId);

        for (int i = start; i <= end; ++i)
        {
            const QModelIndex child = model()->index(i, 0, parent);
            restoreState(child, d->statesByAlbumId);
        }
    }
}

void AbstractAlbumTreeView::rowsAboutToBeRemoved(const QModelIndex& parent, int start, int end)
{
    QTreeView::rowsAboutToBeRemoved(parent, start, end);

    // Clean up map if album id is reused for a new album
    if (!d->statesByAlbumId.isEmpty())
    {
        for (int i = start; i <= end; ++i)
        {
            const QModelIndex child = model()->index(i, 0, parent);
            Album* album = albumModel()->albumForIndex(child);

            if (album)
            {
                d->statesByAlbumId.remove(album->id());
            }
        }
    }
}

void AbstractAlbumTreeView::adaptColumnsToContent()
{
    resizeColumnToContents(0);
}

void AbstractAlbumTreeView::scrollToSelectedAlbum()
{
    QModelIndexList selected = selectedIndexes();

    if (!selected.isEmpty())
    {
        scrollTo(selected.first(), PositionAtCenter);
    }
}

void AbstractAlbumTreeView::expandEverything(const QModelIndex& index)
{

    for (int row = 0; row < albumFilterModel()->rowCount(index); ++row)
    {
        QModelIndex rowIndex = albumFilterModel()->index(row, 0, index);
        expand(rowIndex);
        expandEverything(rowIndex);
    }

}

void AbstractAlbumTreeView::adaptColumnsOnDataChange(const QModelIndex& topLeft, const QModelIndex& bottomRight)
{
    Q_UNUSED(topLeft);
    Q_UNUSED(bottomRight);

    if (!d->resizeColumnsTimer->isActive())
    {
        d->resizeColumnsTimer->start();
    }
}

void AbstractAlbumTreeView::adaptColumnsOnRowChange(const QModelIndex& parent, int start, int end)
{
    Q_UNUSED(parent);
    Q_UNUSED(start);
    Q_UNUSED(end);

    if (!d->resizeColumnsTimer->isActive())
    {
        d->resizeColumnsTimer->start();
    }
}

void AbstractAlbumTreeView::adaptColumnsOnLayoutChange()
{
    if (!d->resizeColumnsTimer->isActive())
    {
        d->resizeColumnsTimer->start();
    }
}

void AbstractAlbumTreeView::doSaveState()
{

    KConfigGroup configGroup = getConfigGroup();

    QList<int> selection, expansion;

    for (int i = 0; i < model()->rowCount(); ++i)
    {
        const QModelIndex index = model()->index(i, 0);
        saveStateRecursive(index, selection, expansion);
    }

    Album* selectedAlbum = albumFilterModel()->albumForIndex(selectionModel()->currentIndex());
    QString currentIndex;

    if (selectedAlbum)
    {
        currentIndex = QString::number(selectedAlbum->id());
    }

    //    kDebug() << "selection: " << selection;
    //    kDebug() << "expansion: " << expansion;
    //    kDebug() << "currentIndex: " << currentIndex;

    configGroup.writeEntry(entryName(d->configSelectionEntry), selection);
    configGroup.writeEntry(entryName(d->configExpansionEntry), expansion);
    configGroup.writeEntry(entryName(d->configCurrentIndexEntry), currentIndex);
    configGroup.writeEntry(entryName(d->configSortColumnEntry), albumFilterModel()->sortColumn());
    configGroup.writeEntry(entryName(d->configSortOrderEntry), (int) albumFilterModel()->sortOrder());

}

void AbstractAlbumTreeView::saveStateRecursive(const QModelIndex& index,
        QList<int> &selection, QList<int> &expansion)
{

    Album* album = albumFilterModel()->albumForIndex(index);

    if (album)
    {
        const int id = album->id();

        if (selectionModel()->isSelected(index))
        {
            selection.append(id);
        }

        if (isExpanded(index))
        {
            expansion.append(id);
        }
    }

    for (int i = 0; i < model()->rowCount(index); ++i)
    {
        const QModelIndex child = model()->index(i, 0, index);
        saveStateRecursive(child, selection, expansion);
    }

}

void AbstractAlbumTreeView::setEnableContextMenu(bool enable)
{
    d->enableContextMenu = enable;
}

bool AbstractAlbumTreeView::showContextMenuAt(QContextMenuEvent* event, Album* albumForEvent)
{
    Q_UNUSED(event);
    return albumForEvent;
}

QPixmap AbstractAlbumTreeView::contextMenuIcon() const
{
    return SmallIcon("digikam");
}

QString AbstractAlbumTreeView::contextMenuTitle() const
{
    return i18n("Context menu");
}

void AbstractAlbumTreeView::contextMenuEvent(QContextMenuEvent* event)
{

    if (!d->enableContextMenu)
    {
        return;
    }

    Album* album = albumFilterModel()->albumForIndex(indexAt(event->pos()));

    if (!showContextMenuAt(event, album))
    {
        return;
    }

    // switch to the selected album if need
    if (d->selectOnContextMenu && album)
    {
        setCurrentAlbum(album);
    }

    // --------------------------------------------------------

    KMenu popmenu(this);
    popmenu.addTitle(contextMenuIcon(), contextMenuTitle());
    ContextMenuHelper cmhelper(&popmenu);

    AlbumPointer<Album> albumPointer(album);
    addCustomContextMenuActions(cmhelper, album);

    QAction* choice = cmhelper.exec(QCursor::pos());
    handleCustomContextMenuAction(choice, albumPointer);
}

void AbstractAlbumTreeView::setSelectOnContextMenu(bool select)
{
    d->selectOnContextMenu = select;
}

void AbstractAlbumTreeView::addCustomContextMenuActions(ContextMenuHelper& cmh, Album* album)
{
    Q_UNUSED(cmh);
    Q_UNUSED(album);
}

void AbstractAlbumTreeView::handleCustomContextMenuAction(QAction* action, AlbumPointer<Album> album)
{
    Q_UNUSED(action);
    Q_UNUSED(album);
}

void AbstractAlbumTreeView::albumSettingsChanged()
{
    d->delegate->updateHeight();
}

// --------------------------------------- //

AbstractCountingAlbumTreeView::AbstractCountingAlbumTreeView(QWidget* parent, Flags flags)
    : AbstractAlbumTreeView(parent, flags & ~CreateDefaultFilterModel)
{
    if (flags & CreateDefaultFilterModel)
    {
        setAlbumFilterModel(new AlbumFilterModel(this));
    }

    connect(this, SIGNAL(expanded(const QModelIndex&)),
            this, SLOT(slotExpanded(const QModelIndex&)));

    connect(this, SIGNAL(collapsed(const QModelIndex&)),
            this, SLOT(slotCollapsed(const QModelIndex&)));

    if (flags & ShowCountAccordingToSettings)
    {
        connect(AlbumSettings::instance(), SIGNAL(setupChanged()),
                this, SLOT(setShowCountFromSettings()));
    }
}

void AbstractCountingAlbumTreeView::setAlbumModel(AbstractCountingAlbumModel* model)
{
    AbstractAlbumTreeView::setAlbumModel(model);

    if (m_flags & ShowCountAccordingToSettings)
    {
        setShowCountFromSettings();
    }
}

void AbstractCountingAlbumTreeView::setAlbumFilterModel(AlbumFilterModel* filterModel)
{
    AbstractAlbumTreeView::setAlbumFilterModel(filterModel);

    // Initialize expanded/collapsed showCount state
    updateShowCountState(QModelIndex(), true);
}

void AbstractCountingAlbumTreeView::updateShowCountState(const QModelIndex& index, bool recurse)
{
    if (isExpanded(index))
    {
        slotExpanded(index);
    }
    else
    {
        slotCollapsed(index);
    }

    if (recurse)
    {
        int rows = m_albumFilterModel->rowCount(index);

        for (int i=0; i<rows; ++i)
        {
            updateShowCountState(m_albumFilterModel->index(i, 0, index), true);
        }
    }
}

void AbstractCountingAlbumTreeView::slotCollapsed(const QModelIndex& index)
{
    static_cast<AbstractCountingAlbumModel*>(m_albumModel)->includeChildrenCount(m_albumFilterModel->mapToSourceAlbumModel(index));
}

void AbstractCountingAlbumTreeView::slotExpanded(const QModelIndex& index)
{
    static_cast<AbstractCountingAlbumModel*>(m_albumModel)->excludeChildrenCount(m_albumFilterModel->mapToSourceAlbumModel(index));
}

void AbstractCountingAlbumTreeView::setShowCountFromSettings()
{
    static_cast<AbstractCountingAlbumModel*>(m_albumModel)->setShowCount(AlbumSettings::instance()->getShowFolderTreeViewItemsCount());
}

void AbstractCountingAlbumTreeView::rowsInserted(const QModelIndex& parent, int start, int end)
{
    AbstractAlbumTreeView::rowsInserted(parent, start, end);

    // initialize showCount state when items are added
    for (int i=start; i<=end; ++i)
    {
        updateShowCountState(m_albumFilterModel->index(i, 0, parent), false);
    }
}

// --------------------------------------- //

class AbstractCheckableAlbumTreeViewPriv
{
public:

    AbstractCheckableAlbumTreeViewPriv()
    {
    }

    static const QString configCheckedAlbumsEntry;
    static const QString configPartiallyCheckedAlbumsEntry;
    static const QString configRestoreCheckedEntry;

    QList<int>           checkedAlbumIds;
    QList<int>           partiallyCheckedAlbumIds;

};
const QString AbstractCheckableAlbumTreeViewPriv::configCheckedAlbumsEntry("Checked");
const QString AbstractCheckableAlbumTreeViewPriv::configPartiallyCheckedAlbumsEntry("PartiallyChecked");
const QString AbstractCheckableAlbumTreeViewPriv::configRestoreCheckedEntry("RestoreChecked");

// --------------------------------------------------------

AbstractCheckableAlbumTreeView::AbstractCheckableAlbumTreeView(QWidget* parent, Flags flags)
    : AbstractCountingAlbumTreeView(parent, flags & ~CreateDefaultFilterModel),
      d(new AbstractCheckableAlbumTreeViewPriv)
{
    m_checkOnMiddleClick = true;
    m_restoreCheckState  = false;

    if (flags & CreateDefaultFilterModel)
    {
        setAlbumFilterModel(new CheckableAlbumFilterModel(this));
    }
}

AbstractCheckableAlbumTreeView::~AbstractCheckableAlbumTreeView()
{
    delete d;
}

AbstractCheckableAlbumModel* AbstractCheckableAlbumTreeView::albumModel() const
{
    return dynamic_cast<AbstractCheckableAlbumModel*>(m_albumModel);
}

CheckableAlbumFilterModel* AbstractCheckableAlbumTreeView::albumFilterModel() const
{
    return dynamic_cast<CheckableAlbumFilterModel*> (m_albumFilterModel);
}

void AbstractCheckableAlbumTreeView::setCheckOnMiddleClick(bool doThat)
{
    m_checkOnMiddleClick = doThat;
}

void AbstractCheckableAlbumTreeView::middleButtonPressed(Album* a)
{
    AbstractCheckableAlbumModel* model = static_cast<AbstractCheckableAlbumModel*>(m_albumModel);

    if (!model)
    {
        return;
    }

    if (model->isCheckable())
    {
        if (model->isTristate())
        {
            switch (model->checkState(a))
            {
                case Qt::Unchecked:
                    model->setCheckState(a, Qt::PartiallyChecked);
                    break;
                case Qt::PartiallyChecked:
                    model->setCheckState(a, Qt::Checked);
                    break;
                case Qt::Checked:
                    model->setCheckState(a, Qt::Unchecked);
                    break;
            }
        }
        else
        {
            model->toggleChecked(a);
        }
    }
}

bool AbstractCheckableAlbumTreeView::isRestoreCheckState() const
{
    return m_restoreCheckState;
}

void AbstractCheckableAlbumTreeView::setRestoreCheckState(bool restore)
{
    m_restoreCheckState = restore;
}

void AbstractCheckableAlbumTreeView::doLoadState()
{
    AbstractCountingAlbumTreeView::doLoadState();

    KConfigGroup group = getConfigGroup();

    if (!m_restoreCheckState)
    {
        m_restoreCheckState = group.readEntry(entryName(d->configRestoreCheckedEntry), false);
    }

    if (!m_restoreCheckState || !checkableModel()->isCheckable())
    {
        return;
    }

    QStringList checkedAlbums = group.readEntry(entryName(
                                    d->configCheckedAlbumsEntry), QStringList());

    d->checkedAlbumIds.clear();
    foreach(const QString& albumId, checkedAlbums)
    {
        bool ok;
        int id = albumId.toInt(&ok);

        if (ok)
        {
            d->checkedAlbumIds << id;
        }
    }

    QStringList partiallyCheckedAlbums = group.readEntry(entryName(
            d->configPartiallyCheckedAlbumsEntry), QStringList());

    d->partiallyCheckedAlbumIds.clear();
    foreach(const QString& albumId, partiallyCheckedAlbums)
    {
        bool ok;
        int id = albumId.toInt(&ok);

        if (ok)
        {
            d->partiallyCheckedAlbumIds << id;
        }
    }

    // initially sync with the albums that are already in the model
    restoreCheckStateForHierarchy(QModelIndex());
}

void AbstractCheckableAlbumTreeView::rowsInserted(const QModelIndex& parent, int start, int end)
{
    AbstractCountingAlbumTreeView::rowsInserted(parent, start, end);

    if (!d->checkedAlbumIds.isEmpty())
    {
        for (int i = start; i <= end; ++i)
        {
            const QModelIndex child = checkableModel()->index(i, 0, parent);
            restoreCheckState(child);
        }
    }
}

void AbstractCheckableAlbumTreeView::restoreCheckStateForHierarchy(const QModelIndex& index)
{
    // recurse children
    for (int i = 0; i < checkableModel()->rowCount(index); ++i)
    {
        const QModelIndex child = checkableModel()->index(i, 0, index);
        restoreCheckState(child);
        restoreCheckStateForHierarchy(child);
    }
}

void AbstractCheckableAlbumTreeView::restoreCheckState(const QModelIndex& index)
{
    Album* album = checkableModel()->albumForIndex(index);

    if (!album)
    {
        return;
    }

    if (d->checkedAlbumIds.contains(album->id()))
    {
        checkableModel()->setCheckState(album, Qt::Checked);
        d->checkedAlbumIds.removeOne(album->id());
    }

    if (d->partiallyCheckedAlbumIds.contains(album->id()))
    {
        checkableModel()->setCheckState(album, Qt::PartiallyChecked);
        d->partiallyCheckedAlbumIds.removeOne(album->id());
    }
}

void AbstractCheckableAlbumTreeView::doSaveState()
{
    AbstractCountingAlbumTreeView::doSaveState();

    KConfigGroup group = getConfigGroup();

    group.writeEntry(entryName(d->configRestoreCheckedEntry), m_restoreCheckState);

    if (!m_restoreCheckState || !checkableModel()->isCheckable())
    {
        return;
    }

    QList<Album*> checkedAlbums = checkableModel()->checkedAlbums();
    QStringList checkedIds;
    foreach(Album* album, checkedAlbums)
    {
        checkedIds << QString::number(album->id());
    }

    group.writeEntry(entryName(d->configCheckedAlbumsEntry), checkedIds);

    if (!checkableModel()->isTristate())
    {
        return;
    }

    QList<Album*> partiallyCheckedAlbums = checkableModel()->partiallyCheckedAlbums();
    QStringList partiallyCheckedIds;
    foreach(Album* album, partiallyCheckedAlbums)
    {
        partiallyCheckedIds << QString::number(album->id());
    }

    group.writeEntry(entryName(d->configPartiallyCheckedAlbumsEntry), partiallyCheckedIds);

}

// --------------------------------------- //

AlbumTreeView::AlbumTreeView(QWidget* parent, Flags flags)
    : AbstractCheckableAlbumTreeView(parent, flags)
{
    setRootIsDecorated(false);
    setDragEnabled(true);
    setAcceptDrops(true);
    setDropIndicatorShown(false);
    setAutoExpandDelay(300);

    if (flags & CreateDefaultModel)
    {
        setAlbumModel(new AlbumModel(AlbumModel::IncludeRootAlbum, this));
    }
}

AlbumTreeView::~AlbumTreeView()
{
}

void AlbumTreeView::setAlbumModel(AlbumModel* model)
{
    // changing model is not implemented
    if (m_albumModel)
    {
        return;
    }

    AbstractCheckableAlbumTreeView::setAlbumModel(model);

    m_dragDropHandler = albumModel()->dragDropHandler();

    if (!m_dragDropHandler)
    {
        m_dragDropHandler = new AlbumDragDropHandler(albumModel());
        connect(m_dragDropHandler, SIGNAL(dioResult(KJob*)),
                this, SLOT(slotDIOResult(KJob*)));
        model->setDragDropHandler(m_dragDropHandler);
    }

}

void AlbumTreeView::setAlbumFilterModel(CheckableAlbumFilterModel* filterModel)
{
    AbstractCheckableAlbumTreeView::setAlbumFilterModel(filterModel);
}

AlbumModel* AlbumTreeView::albumModel() const
{
    return dynamic_cast<AlbumModel*>(m_albumModel);
}

PAlbum* AlbumTreeView::currentAlbum() const
{
    return dynamic_cast<PAlbum*> (m_albumFilterModel->albumForIndex(currentIndex()));
}

PAlbum* AlbumTreeView::albumForIndex(const QModelIndex& index) const
{
    return dynamic_cast<PAlbum*> (m_albumFilterModel->albumForIndex(index));
}

void AlbumTreeView::slotDIOResult(KJob* kjob)
{
    KIO::Job* job = static_cast<KIO::Job*>(kjob);

    if (job->error())
    {
        job->ui()->setWindow(this);
        job->ui()->showErrorMessage();
    }
}

void AlbumTreeView::setCurrentAlbum(PAlbum* album, bool selectInAlbumManager)
{
    AbstractCheckableAlbumTreeView::setCurrentAlbum(album, selectInAlbumManager);
}

void AlbumTreeView::setCurrentAlbum(int albumId, bool selectInAlbumManager)
{
    PAlbum* album = AlbumManager::instance()->findPAlbum(albumId);
    setCurrentAlbum(album, selectInAlbumManager);
}

// --------------------------------------- //

TagTreeView::TagTreeView(QWidget* parent, Flags flags)
    : AbstractCheckableAlbumTreeView(parent, flags), m_filteredModel(0)
{
    m_modificationHelper = new TagModificationHelper(this, this);
    setRootIsDecorated(true);
    setDragEnabled(true);
    setAcceptDrops(true);
    setDropIndicatorShown(false);
    setAutoExpandDelay(300);

    if (flags & CreateDefaultModel)
    {
        setAlbumModel(new TagModel(TagModel::IncludeRootAlbum, this));
    }

    if (flags & CreateDefaultFilterModel) // must set again!
    {
        setAlbumFilterModel(new TagPropertiesFilterModel(this), albumFilterModel());
    }
}

TagTreeView::~TagTreeView()
{
}

void TagTreeView::setAlbumFilterModel(TagPropertiesFilterModel* filteredModel, CheckableAlbumFilterModel* filterModel)
{
    m_filteredModel = filteredModel;
    AbstractCheckableAlbumTreeView::setAlbumFilterModel(filterModel);
    // hook in: source album model -> filtered model -> album filter model
    albumFilterModel()->setSourceFilterModel(m_filteredModel);
}

void TagTreeView::setAlbumModel(TagModel* model)
{
    // changing model is not implemented
    if (m_albumModel)
    {
        return;
    }

    AbstractCheckableAlbumTreeView::setAlbumModel(model);

    if (m_filteredModel)
    {
        m_filteredModel->setSourceAlbumModel(model);
    }

    m_dragDropHandler = albumModel()->dragDropHandler();

    if (!m_dragDropHandler)
    {
        m_dragDropHandler = new TagDragDropHandler(albumModel());
        albumModel()->setDragDropHandler(m_dragDropHandler);

        connect(albumModel()->dragDropHandler(), SIGNAL(assignTags(const QList<qlonglong>&, const QList<int>&)),
                MetadataManager::instance(), SLOT(assignTags(const QList<qlonglong>&, const QList<int>&)));
    }

    if (m_albumModel->rootAlbumBehavior() == AbstractAlbumModel::IncludeRootAlbum)
    {
        setRootIsDecorated(false);
    }

    if (m_albumFilterModel)
    {
        expand(m_albumFilterModel->rootAlbumIndex());
    }
}

TagModel* TagTreeView::albumModel() const
{
    return static_cast<TagModel*>(m_albumModel);
}

TagPropertiesFilterModel* TagTreeView::filteredModel() const
{
    return m_filteredModel;
}

TAlbum* TagTreeView::currentAlbum() const
{
    return dynamic_cast<TAlbum*> (m_albumFilterModel->albumForIndex(currentIndex()));
}

TAlbum* TagTreeView::albumForIndex(const QModelIndex& index) const
{
    return dynamic_cast<TAlbum*> (m_albumFilterModel->albumForIndex(index));
}

TagModificationHelper* TagTreeView::tagModificationHelper() const
{
    return m_modificationHelper;
}

void TagTreeView::setCurrentAlbum(TAlbum* album, bool selectInAlbumManager)
{
    AbstractCheckableAlbumTreeView::setCurrentAlbum(album, selectInAlbumManager);
}

void TagTreeView::setCurrentAlbum(int albumId, bool selectInAlbumManager)
{
    TAlbum* album = AlbumManager::instance()->findTAlbum(albumId);
    setCurrentAlbum(album, selectInAlbumManager);
}

// --------------------------------------- //

SearchTreeView::SearchTreeView(QWidget* parent, Flags flags)
    : AbstractCheckableAlbumTreeView(parent, flags)
{
    setRootIsDecorated(false);

    if (flags & CreateDefaultModel)
    {
        setAlbumModel(new SearchModel(this));
    }

    if (flags & CreateDefaultFilterModel) // must set again!
    {
        setAlbumFilterModel(new SearchFilterModel(this), albumFilterModel());
    }
}

SearchTreeView::~SearchTreeView()
{
}

void SearchTreeView::setAlbumModel(SearchModel* model)
{
    AbstractCheckableAlbumTreeView::setAlbumModel(model);

    if (m_filteredModel)
    {
        m_filteredModel->setSourceSearchModel(model);
    }
}

SearchModel* SearchTreeView::albumModel() const
{
    return static_cast<SearchModel*>(m_albumModel);
}

void SearchTreeView::setAlbumFilterModel(SearchFilterModel* filteredModel, CheckableAlbumFilterModel* filterModel)
{
    m_filteredModel = filteredModel;
    AbstractCheckableAlbumTreeView::setAlbumFilterModel(filterModel);
    // hook in: source album model -> filtered model -> album filter model
    albumFilterModel()->setSourceFilterModel(m_filteredModel);
}

SearchFilterModel* SearchTreeView::filteredModel() const
{
    return m_filteredModel;
}

SAlbum* SearchTreeView::currentAlbum() const
{
    return dynamic_cast<SAlbum*> (m_albumFilterModel->albumForIndex(currentIndex()));
}

void SearchTreeView::setCurrentAlbum(SAlbum* album, bool selectInAlbumManager)
{
    AbstractCheckableAlbumTreeView::setCurrentAlbum(album, selectInAlbumManager);
}

void SearchTreeView::setCurrentAlbum(int albumId, bool selectInAlbumManager)
{
    SAlbum* album = AlbumManager::instance()->findSAlbum(albumId);
    setCurrentAlbum(album, selectInAlbumManager);
}

// --------------------------------------- //

DateAlbumTreeView::DateAlbumTreeView(QWidget* parent, Flags flags)
    : AbstractCountingAlbumTreeView(parent, flags)
{
    // this view should always show the inclusive counts
    disconnect(this, SIGNAL(expanded(const QModelIndex&)),
               this, SLOT(slotExpanded(const QModelIndex&)));
    disconnect(this, SIGNAL(collapsed(const QModelIndex&)),
               this, SLOT(slotCollapsed(const QModelIndex&)));

    if (flags & CreateDefaultModel)
    {
        setAlbumModel(new DateAlbumModel(this));
    }
}

void DateAlbumTreeView::setAlbumModel(DateAlbumModel* model)
{
    AbstractCountingAlbumTreeView::setAlbumModel(model);
}

DateAlbumModel* DateAlbumTreeView::albumModel() const
{
    return static_cast<DateAlbumModel*>(m_albumModel);
}

void DateAlbumTreeView::setAlbumFilterModel(AlbumFilterModel* filterModel)
{
    AbstractCountingAlbumTreeView::setAlbumFilterModel(filterModel);
}

DAlbum* DateAlbumTreeView::currentAlbum() const
{
    return dynamic_cast<DAlbum*> (m_albumFilterModel->albumForIndex(currentIndex()));
}

DAlbum* DateAlbumTreeView::albumForIndex(const QModelIndex& index) const
{
    return dynamic_cast<DAlbum*> (m_albumFilterModel->albumForIndex(index));
}

void DateAlbumTreeView::setCurrentAlbum(DAlbum* album, bool selectInAlbumManager)
{
    AbstractCountingAlbumTreeView::setCurrentAlbum(album, selectInAlbumManager);
}

void DateAlbumTreeView::setCurrentAlbum(int albumId, bool selectInAlbumManager)
{
    DAlbum* album = AlbumManager::instance()->findDAlbum(albumId);
    setCurrentAlbum(album, selectInAlbumManager);
}

} // namespace Digikam

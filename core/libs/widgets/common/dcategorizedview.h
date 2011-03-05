/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2010-01-16
 * Description : Qt item view for images
 *
 * Copyright (C) 2009-2010 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef DCATEGORIZEDVIEW_H
#define DCATEGORIZEDVIEW_H

// Local includes

#include "digikam_export.h"
#include "digikamkcategorizedview.h"
#include "dragdropimplementations.h"

class QSortFilterProxyModel;

namespace Digikam
{

class DItemDelegate;
class DCategorizedViewPriv;
class AbstractItemDragDropHandler;
class ItemViewToolTip;

class DIGIKAM_EXPORT DCategorizedView : public DigikamKCategorizedView, public DragDropViewImplementation
{
    Q_OBJECT

public:

    DCategorizedView(QWidget* parent = 0);
    ~DCategorizedView();

    virtual QSortFilterProxyModel* filterModel() const = 0;

    DItemDelegate* delegate() const;

    int numberOfSelectedIndexes() const;

    /** Selects the index as current and scrolls to it */
    void toFirstIndex();
    void toLastIndex();
    void toNextIndex();
    void toPreviousIndex();
    void toIndex(const QModelIndex& index);
    void awayFromSelection();

    void invertSelection();

    void setToolTipEnabled(bool enabled);
    bool isToolTipEnabled() const;

    /** Sets the spacing. Does not use setSpacing()/spacing() from QListView */
    void setSpacing(int spacing);
    /** Set if the PointingHand Cursor should be shown over the activation area */
    void setUsePointingHandCursor(bool useCursor);
    /** Determine a step size for scrolling: The larger this number,
     *  the smaller and more precise is the scrolling. Default is 10. */
    void setScrollStepGranularity(int factor);

public Q_SLOTS:

    virtual void cut() { DragDropViewImplementation::cut(); }
    virtual void copy() { DragDropViewImplementation::copy(); }
    virtual void paste() { DragDropViewImplementation::paste(); }

Q_SIGNALS:

    /// Emitted when any selection change occurs. Any of the signals below will be emitted before.
    void selectionChanged();
    /// Emitted when the selection is completely cleared.
    void selectionCleared();

    void zoomOutStep();
    void zoomInStep();

    /** For overlays: Like the respective parent class signals, but with additional info.
     *  Do not change the mouse events.
     */
    void clicked(const QMouseEvent* e, const QModelIndex& index);
    void entered(const QMouseEvent* e, const QModelIndex& index);
    /**  Remember you may want to check if the event is accepted or ignored.
     *   This signal is emitted after being handled by this widget.
     *   You can accept it if ignored. */
    void keyPressed(QKeyEvent* e);

protected Q_SLOTS:

    virtual void slotThemeChanged();
    virtual void slotSetupChanged();

    void slotActivated(const QModelIndex& index);
    void slotClicked(const QModelIndex& index);
    void slotEntered(const QModelIndex& index);
    void layoutAboutToBeChanged();
    void layoutWasChanged();

protected:

    void encodeIsCutSelection(QMimeData* mime, bool isCutSelection);
    bool decodeIsCutSelection(const QMimeData* mimeData);

    void setToolTip(ItemViewToolTip* tip);
    void setItemDelegate(DItemDelegate* delegate);
    void updateDelegateSizes();
    void userInteraction();

    /// Reimplement these in a subclass
    virtual void showContextMenuOnIndex(QContextMenuEvent* event, const QModelIndex& index);
    virtual void showContextMenu(QContextMenuEvent* event);
    virtual void indexActivated(const QModelIndex& index);

    /** Provides default behavior, can reimplement in a subclass.
     *  Returns true if a tooltip was shown.
     */
    virtual bool showToolTip(QHelpEvent* he, const QModelIndex& index, QStyleOptionViewItem& option);

    /** Returns an index that is representative for the category at position pos */
    QModelIndex indexForCategoryAt(const QPoint& pos) const;

    DECLARE_VIEW_DRAG_DROP_METHODS(DigikamKCategorizedView)
    /// Note: pure virtual dragDropHandler() still open from DragDropViewImplementation
    virtual QModelIndex mapIndexForDragDrop(const QModelIndex& index) const;
    virtual QPixmap     pixmapForDrag(const QList<QModelIndex>& indexes) const;

    // reimplemented from parent class
    void contextMenuEvent(QContextMenuEvent* event);
    void keyPressEvent(QKeyEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    QModelIndex moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers);
    void resizeEvent(QResizeEvent* e);
    void reset();
    void rowsAboutToBeRemoved(const QModelIndex& parent, int start, int end);
    void rowsInserted(const QModelIndex& parent, int start, int end);
    void selectionChanged(const QItemSelection&, const QItemSelection&);
    bool viewportEvent(QEvent* event);
    void wheelEvent(QWheelEvent* event);

private Q_SLOTS:

    void slotGridSizeChanged(const QSize&);

private:

    void ensureSelectionAfterChanges();

private:

    DCategorizedViewPriv* const d;
};

} // namespace Digikam

#endif /* IMAGECATEGORIZEDVIEW_H */

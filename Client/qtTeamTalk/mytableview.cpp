/*
 * Copyright (C) 2023, Bjørn D. Rasmussen, BearWare.dk
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "mytableview.h"
#include "settings.h"
#include "utiltts.h"
#include <QAccessible>
#include <QAccessibleWidget>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QMap>
#include <QPersistentModelIndex>
#include <QShortcut>
#include <QShowEvent>
#include <QWindow>

extern NonDefaultSettings* ttSettings;

// ============================================================
// Accessibility fix for Qt 6.4.x bug:
// QAccessibleTable::focusChild() calls cellAt(-1,-1) when no
// row is selected, which crashes in Qt's AT-SPI2 bridge.
// We provide a complete replacement that guards against this.
// ============================================================

class MyTableCellAccessible : public QAccessibleInterface,
                               public QAccessibleTableCellInterface
{
    MyTableView* m_view;
    QPersistentModelIndex m_index;
public:
    MyTableCellAccessible(MyTableView* view, const QModelIndex& idx)
        : m_view(view), m_index(idx) {}

    bool isValid() const override { return m_index.isValid(); }
    QObject* object() const override { return nullptr; }
    QAccessible::Role role() const override { return QAccessible::Cell; }

    QAccessible::State state() const override {
        QAccessible::State s;
        if (!m_index.isValid()) return s;
        if (m_view->currentIndex() == m_index) s.focused = true;
        if (m_view->selectionModel()->isSelected(m_index)) s.selected = true;
        s.selectable = true;
        return s;
    }

    QRect rect() const override {
        if (!m_index.isValid()) return {};
        QRect r = m_view->visualRect(m_index);
        return QRect(m_view->viewport()->mapToGlobal(r.topLeft()), r.size());
    }

    void setText(QAccessible::Text, const QString&) override {}

    QString text(QAccessible::Text t) const override {
        if (!m_index.isValid()) return {};
        switch (t) {
        case QAccessible::Name:
        case QAccessible::Value:
            return m_index.data(Qt::DisplayRole).toString();
        case QAccessible::Description:
            return m_index.data(Qt::AccessibleDescriptionRole).toString();
        default:
            return {};
        }
    }

    int childCount() const override { return 0; }
    int indexOfChild(const QAccessibleInterface*) const override { return -1; }
    QAccessibleInterface* parent() const override {
        return QAccessible::queryAccessibleInterface(m_view);
    }
    QAccessibleInterface* child(int) const override { return nullptr; }
    QAccessibleInterface* childAt(int, int) const override { return nullptr; }

    QWindow* window() const override {
        return m_view ? m_view->window()->windowHandle() : nullptr;
    }

    void* interface_cast(QAccessible::InterfaceType t) override {
        if (t == QAccessible::TableCellInterface)
            return static_cast<QAccessibleTableCellInterface*>(this);
        return nullptr;
    }

    // QAccessibleTableCellInterface:
    int rowIndex() const override { return m_index.row(); }
    int columnIndex() const override { return m_index.column(); }
    int rowExtent() const override { return 1; }
    int columnExtent() const override { return 1; }
    bool isSelected() const override {
        return m_index.isValid() && m_view->selectionModel()->isSelected(m_index);
    }
    QAccessibleInterface* table() const override {
        return QAccessible::queryAccessibleInterface(m_view);
    }
    QList<QAccessibleInterface*> rowHeaderCells() const override { return {}; }
    QList<QAccessibleInterface*> columnHeaderCells() const override { return {}; }
};

class MyTableViewAccessible : public QAccessibleWidget, public QAccessibleTableInterface
{
    MyTableView* tableView() const { return static_cast<MyTableView*>(widget()); }
    // Store Qt-managed IDs, not raw pointers — Qt's registry owns the cell objects.
    // This mirrors how QAccessibleTable uses childToId so that deleteAccessibleInterface()
    // both removes the D-Bus object path and frees the memory in one step.
    mutable QMap<QPersistentModelIndex, QAccessible::Id> m_cellCache;
    mutable QAccessible::Id m_dummyCellId = 0;

    QAccessibleInterface* dummyCell() const {
        if (!m_dummyCellId) {
            auto* dummy = new MyTableCellAccessible(tableView(), QModelIndex());
            m_dummyCellId = QAccessible::uniqueId(dummy);
        }
        return QAccessible::accessibleInterface(m_dummyCellId);
    }

    QAccessibleInterface* getCell(int row, int col) const {
        auto* v = tableView();
        if (!v->model() || row < 0 || col < 0) return nullptr;
        if (row >= v->model()->rowCount() || col >= v->model()->columnCount()) return nullptr;
        QModelIndex idx = v->model()->index(row, col);
        if (!idx.isValid()) return nullptr;
        QPersistentModelIndex pidx(idx);
        auto it = m_cellCache.find(pidx);
        if (it != m_cellCache.end())
            return QAccessible::accessibleInterface(it.value());
        auto* cell = new MyTableCellAccessible(v, idx);
        QAccessible::Id id = QAccessible::uniqueId(cell);
        m_cellCache[pidx] = id;
        return cell;
    }

public:
    explicit MyTableViewAccessible(MyTableView* view)
        : QAccessibleWidget(view, QAccessible::Table) {}

    ~MyTableViewAccessible() {
        // deleteAccessibleInterface unregisters from the D-Bus registry AND frees memory.
        // Plain delete (qDeleteAll) would leave dangling pointers in Qt's AT-SPI bridge.
        for (QAccessible::Id id : std::as_const(m_cellCache))
            QAccessible::deleteAccessibleInterface(id);
        if (m_dummyCellId)
            QAccessible::deleteAccessibleInterface(m_dummyCellId);
    }

    void* interface_cast(QAccessible::InterfaceType t) override {
        if (t == QAccessible::TableInterface)
            return static_cast<QAccessibleTableInterface*>(this);
        return QAccessibleWidget::interface_cast(t);
    }

    QAccessibleInterface* focusChild() const override {
        auto* v = tableView();
        if (!v->model() || !v->selectionModel()) return nullptr;
        QModelIndex current = v->selectionModel()->currentIndex();
        if (!current.isValid()) return nullptr;
        return getCell(current.row(), current.column());
    }

    // Qt's QAbstractItemView::accessibleTable2Index uses:
    // entry = (row + hHeader) * (cols + vHeader) + col + vHeader
    // We must match this mapping in child() and childCount().
    bool hasHHeader() const {
        auto* v = tableView();
        return v->horizontalHeader() && !v->horizontalHeader()->isHidden();
    }
    bool hasVHeader() const {
        auto* v = tableView();
        return v->verticalHeader() && !v->verticalHeader()->isHidden();
    }

    int childCount() const override {
        auto* v = tableView();
        if (!v->model()) return 0;
        return (v->model()->rowCount() + (hasHHeader() ? 1 : 0))
               * (v->model()->columnCount() + (hasVHeader() ? 1 : 0));
    }

    QAccessibleInterface* child(int n) const override {
        auto* v = tableView();
        if (!v->model() || n < 0) return dummyCell();
        int hh = hasHHeader() ? 1 : 0;
        int vh = hasVHeader() ? 1 : 0;
        int totalCols = v->model()->columnCount() + vh;
        if (totalCols == 0) return dummyCell();
        int rowWithHeader = n / totalCols;
        int colWithHeader = n % totalCols;
        // Skip header rows/columns — return dummy for them
        if (hh && rowWithHeader == 0) return dummyCell();
        if (vh && colWithHeader == 0) return dummyCell();
        int row = rowWithHeader - hh;
        int col = colWithHeader - vh;
        auto* cell = getCell(row, col);
        return cell ? cell : dummyCell();
    }

    int indexOfChild(const QAccessibleInterface* ch) const override {
        if (m_dummyCellId && ch == QAccessible::accessibleInterface(m_dummyCellId)) return -1;
        auto* cellIface = static_cast<QAccessibleTableCellInterface*>(
            const_cast<QAccessibleInterface*>(ch)->interface_cast(QAccessible::TableCellInterface));
        if (!cellIface) return -1;
        auto* v = tableView();
        if (!v->model()) return -1;
        int hh = hasHHeader() ? 1 : 0;
        int vh = hasVHeader() ? 1 : 0;
        return (cellIface->rowIndex() + hh) * (v->model()->columnCount() + vh)
               + cellIface->columnIndex() + vh;
    }

    // QAccessibleTableInterface:
    QAccessibleInterface* caption() const override { return nullptr; }
    QAccessibleInterface* summary() const override { return nullptr; }

    QAccessibleInterface* cellAt(int row, int col) const override {
        if (row < 0 || col < 0) return dummyCell();
        auto* cell = getCell(row, col);
        return cell ? cell : dummyCell();
    }

    int columnCount() const override {
        auto* v = tableView();
        return v->model() ? v->model()->columnCount() : 0;
    }

    int rowCount() const override {
        auto* v = tableView();
        return v->model() ? v->model()->rowCount() : 0;
    }

    int selectedCellCount() const override {
        auto* v = tableView();
        if (!v->selectionModel()) return 0;
        return v->selectionModel()->selectedIndexes().size();
    }

    QList<QAccessibleInterface*> selectedCells() const override {
        QList<QAccessibleInterface*> result;
        auto* v = tableView();
        if (!v->model() || !v->selectionModel()) return result;
        for (const QModelIndex& idx : v->selectionModel()->selectedIndexes())
            if (auto* cell = getCell(idx.row(), idx.column()))
                result << cell;
        return result;
    }

    QString columnDescription(int col) const override {
        auto* v = tableView();
        if (!v->model()) return {};
        return v->model()->headerData(col, Qt::Horizontal).toString();
    }

    QString rowDescription(int row) const override {
        auto* v = tableView();
        if (!v->model()) return {};
        return v->model()->headerData(row, Qt::Vertical).toString();
    }

    int selectedColumnCount() const override { return 0; }
    int selectedRowCount() const override { return 0; }
    QList<int> selectedColumns() const override { return {}; }
    QList<int> selectedRows() const override { return {}; }
    bool isColumnSelected(int) const override { return false; }
    bool isRowSelected(int) const override { return false; }
    bool selectRow(int) override { return false; }
    bool selectColumn(int) override { return false; }
    bool unselectRow(int) override { return false; }
    bool unselectColumn(int) override { return false; }
    void modelChange(QAccessibleTableModelChangeEvent*) override {
        for (QAccessible::Id id : std::as_const(m_cellCache))
            QAccessible::deleteAccessibleInterface(id);
        m_cellCache.clear();
    }
};

static QAccessibleInterface* myTableViewFactory(const QString& classname, QObject* obj)
{
    if (classname == QLatin1String("MyTableView") && obj && obj->isWidgetType())
        return new MyTableViewAccessible(static_cast<MyTableView*>(obj));
    return nullptr;
}

// ============================================================

MyTableView::MyTableView(QWidget* parent/* = nullptr*/) : QTableView(parent)
{
    QAccessible::installFactory(myTableViewFactory);

    QShortcut* shortcutLeft = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Left), this);
    connect(shortcutLeft, &QShortcut::activated, this, &MyTableView::moveColumnLeft);
    QShortcut* shortcutRight = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Right), this);
    connect(shortcutRight, &QShortcut::activated, this, &MyTableView::moveColumnRight);
}

void MyTableView::keyPressEvent(QKeyEvent* e)
{
    if (e->key() == Qt::Key_Space)
    {
        QModelIndex currentIndex = this->currentIndex();
        if (currentIndex.isValid())
        {
            emit this->doubleClicked(currentIndex);
            QVariant value = currentIndex.data(Qt::CheckStateRole);
            if (value.isValid() && value.canConvert<Qt::CheckState>())
            {
                Qt::CheckState state = static_cast<Qt::CheckState>(value.toInt());
                state = (state == Qt::Checked) ? Qt::Unchecked : Qt::Checked;
                this->model()->setData(currentIndex, state, Qt::CheckStateRole);
                QAccessibleValueChangeEvent ev(this->viewport(), state);
                QAccessible::updateAccessibility(&ev);
            }
            this->setCurrentIndex(currentIndex);
        }
    }
    else if (e->key() == Qt::Key_Home && e->modifiers() & Qt::ControlModifier)
    {
        int currentColumn = this->currentIndex().column();
        QModelIndex firstRowIndex = this->model()->index(0, currentColumn);
        if (firstRowIndex.isValid())
        {
            this->setCurrentIndex(firstRowIndex);
        }
    }
    else if (e->key() == Qt::Key_End && e->modifiers() & Qt::ControlModifier)
    {
        int currentColumn = this->currentIndex().column();
        int lastRow = this->model()->rowCount() - 1;
        QModelIndex lastRowIndex = this->model()->index(lastRow, currentColumn);
        if (lastRowIndex.isValid())
        {
            this->setCurrentIndex(lastRowIndex);
        }
    }
#if defined(Q_OS_LINUX)
    else if (e->key() == Qt::Key_Enter || e->key() == Qt::Key_Return)
    {
        QModelIndex currentIndex = this->currentIndex();
        if (currentIndex.isValid())
            emit this->doubleClicked(currentIndex);
    }
#endif
    else
    {
        QTableView::keyPressEvent(e);
    }
}

void MyTableView::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    QTableView::currentChanged(current, previous);
#if defined(Q_OS_DARWIN)
    if (current.isValid() && ttSettings->value(SETTINGS_TTS_SPEAKLISTS, SETTINGS_TTS_SPEAKLISTS_DEFAULT).toBool() == true)
    {
        QString text = this->model()->data(current, Qt::AccessibleTextRole).toString();
        if (text.size())
            addTextToSpeechMessage(text);
    }
#endif
}

void MyTableView::showEvent(QShowEvent* e)
{
    QTableView::showEvent(e);
    ensureSelection();
    if (model())
        connect(model(), &QAbstractItemModel::rowsInserted, this, &MyTableView::ensureSelection, Qt::UniqueConnection);
}

void MyTableView::ensureSelection()
{
    if (model() && model()->rowCount() > 0 && !currentIndex().isValid())
        setCurrentIndex(model()->index(0, 0));
}

void MyTableView::moveColumnLeft()
{
    if (!this->horizontalHeader()->sectionsMovable())
        return;
    int col = this->currentIndex().column();
    int visualIndex = this->horizontalHeader()->visualIndex(col);
    if (visualIndex > 0)
    {
        this->horizontalHeader()->moveSection(visualIndex, visualIndex - 1);
        this->setCurrentIndex(this->model()->index(this->currentIndex().row(), this->horizontalHeader()->logicalIndex(visualIndex - 1)));
    }
}

void MyTableView::moveColumnRight()
{
    if (!this->horizontalHeader()->sectionsMovable())
        return;
    int col = this->currentIndex().column();
    int visualIndex = this->horizontalHeader()->visualIndex(col);
    if (visualIndex < this->model()->columnCount() - 1)
    {
        this->horizontalHeader()->moveSection(visualIndex, visualIndex + 1);
        this->setCurrentIndex(this->model()->index(this->currentIndex().row(), this->horizontalHeader()->logicalIndex(visualIndex + 1)));
    }
}

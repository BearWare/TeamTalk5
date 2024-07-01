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

#ifndef SHORTCUTSMODEL_H
#define SHORTCUTSMODEL_H

#include "utilhotkey.h"
#include <QAbstractTableModel>
#include <QVector>
#include <QTreeView>

class ShortcutsModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    ShortcutsModel(QObject* parent);
    QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const override;
    int columnCount ( const QModelIndex & parent = QModelIndex() ) const override;
    QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QModelIndex index ( int row, int column, const QModelIndex & parent = QModelIndex() ) const override;
    QModelIndex parent ( const QModelIndex & index ) const override;
    int rowCount ( const QModelIndex & parent = QModelIndex() ) const override;
    void setShortcuts(hotkeys_t active);
    const hotkeys_t& getShortcuts();

private:
    QVector<HotKeyID> m_shortcuts;
    hotkeys_t m_shortcutsselected;
};

#endif // SHORTCUTSMODEL_H

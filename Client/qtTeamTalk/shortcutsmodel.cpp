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

#include "shortcutsmodel.h"
#include "settings.h"

#include <QKeyEvent>
#include <QInputDialog>

extern QSettings* ttSettings;

enum
{
    COLUMN_NAME = 0,
    COLUMN_SHORTCUT = 1,
    COLUMN_COUNT,
};

ShortcutsModel::ShortcutsModel(QObject* parent)
    : QAbstractTableModel(parent)
{
    m_shortcuts.push_back(HOTKEY_PUSHTOTALK);
    m_shortcuts.push_back(HOTKEY_VOICEACTIVATION);
    m_shortcuts.push_back(HOTKEY_MICROPHONEGAIN_DEC);
    m_shortcuts.push_back(HOTKEY_MICROPHONEGAIN_INC);
    m_shortcuts.push_back(HOTKEY_DECVOLUME);
    m_shortcuts.push_back(HOTKEY_INCVOLUME);
    m_shortcuts.push_back(HOTKEY_MUTEALL);
    m_shortcuts.push_back(HOTKEY_VIDEOTX);
    m_shortcuts.push_back(HOTKEY_REINITSOUNDDEVS);
    m_shortcuts.push_back(HOTKEY_SHOWHIDE_WINDOW);
    m_shortcuts.push_back(HOTKEY_SPEAK_PING);
}

QVariant ShortcutsModel::headerData ( int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/ ) const
{
    switch(role)
    {
    case Qt::DisplayRole :
        if(orientation == Qt::Horizontal)
        {
            switch(section)
            {
            case COLUMN_NAME : return tr("Action");
            case COLUMN_SHORTCUT : return tr("Shortcut");
            }
        }
        break;
    case Qt::TextAlignmentRole :
        return Qt::AlignLeft;
    }
    return QVariant();
}

int ShortcutsModel::columnCount ( const QModelIndex & /*parent*/ /*= QModelIndex() */) const
{
    return COLUMN_COUNT;
}

QVariant ShortcutsModel::data ( const QModelIndex & index, int role /*= Qt::DisplayRole*/ ) const
{
    switch(role)
    {
    case Qt::DisplayRole :
        switch (index.column())
        {
        case COLUMN_NAME :
            return getHotKeyName(m_shortcuts[index.row()]);
        case COLUMN_SHORTCUT :
        {
            if (m_shortcutsselected.contains(m_shortcuts[index.row()]))
            {
                hotkey_t hk = m_shortcutsselected[m_shortcuts[index.row()]];
                return getHotKeyText(hk);
            }
            return tr("None");
        }
        }
        break;
    case Qt::AccessibleTextRole :
        switch (index.column())
        {
        case COLUMN_NAME :
            QString result = QString("%1: %2").arg(data(index, Qt::DisplayRole).toString()).arg(data(createIndex(index.row(), COLUMN_SHORTCUT), Qt::DisplayRole).toString());
            return result;
        }
        break;
    case Qt::CheckStateRole :
        switch (index.column())
        {
        case COLUMN_NAME :
            return m_shortcutsselected.contains(m_shortcuts[index.row()]) ? Qt::Checked : Qt::Unchecked;
        }
        break;
    }
    return QVariant();
}

Qt::ItemFlags ShortcutsModel::flags(const QModelIndex &index) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable |  Qt::ItemIsEditable;
}

QModelIndex ShortcutsModel::index ( int row, int column, const QModelIndex & /*parent*/ /*= QModelIndex()*/ ) const
{
    return createIndex(row, column, m_shortcuts[row]);
}

QModelIndex ShortcutsModel::parent ( const QModelIndex & /*index*/ ) const
{
    return QModelIndex();
}

int ShortcutsModel::rowCount ( const QModelIndex & /*parent*/ /*= QModelIndex()*/ ) const
{
    return int(m_shortcuts.size());
}

void ShortcutsModel::setShortcuts(hotkeys_t active)
{
    this->beginResetModel();
    m_shortcutsselected = active;
    this->endResetModel();
}

const hotkeys_t& ShortcutsModel::getShortcuts()
{
    return m_shortcutsselected;
}

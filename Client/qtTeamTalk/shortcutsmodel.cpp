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

#include <QSet>

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
}

QVariant ShortcutsModel::headerData(int section, Qt::Orientation orientation,
                                    int role) const
{
    switch (role)
    {
    case Qt::DisplayRole:
        if (orientation == Qt::Horizontal)
        {
            switch (section)
            {
            case COLUMN_NAME: return tr("Action");
            case COLUMN_SHORTCUT: return tr("Shortcut");
            }
        }
        break;
    case Qt::TextAlignmentRole:
        return Qt::AlignLeft;
    }
    return QVariant();
}

int ShortcutsModel::columnCount(const QModelIndex& /*parent*/) const
{
    return COLUMN_COUNT;
}

QVariant ShortcutsModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= rowCount())
        return QVariant();

    const bool actionItem = isActionShortcut(index);

    switch (role)
    {
    case Qt::DisplayRole:
        switch (index.column())
        {
        case COLUMN_NAME:
            return actionItem
                ? actionDisplayName(index)
                : getHotKeyName(m_shortcuts[index.row()]);

        case COLUMN_SHORTCUT:
            if (actionItem)
            {
                const QKeySequence shortcut = actionShortcut(index);
                return shortcut.isEmpty()
                    ? tr("None")
                    : shortcut.toString(QKeySequence::NativeText);
            }

            if (m_shortcutsselected.contains(m_shortcuts[index.row()]))
                return getHotKeyText(m_shortcutsselected[m_shortcuts[index.row()]]);
            return tr("None");
        }
        break;

    case Qt::AccessibleTextRole:
        if (index.column() == COLUMN_NAME)
        {
            return QString("%1: %2")
                .arg(data(index, Qt::DisplayRole).toString())
                .arg(data(createIndex(index.row(), COLUMN_SHORTCUT),
                          Qt::DisplayRole).toString());
        }
        break;

    case Qt::CheckStateRole:
        if (index.column() == COLUMN_NAME)
        {
            if (actionItem)
                return actionShortcut(index).isEmpty()
                    ? Qt::Unchecked
                    : Qt::Checked;
            return m_shortcutsselected.contains(m_shortcuts[index.row()])
                ? Qt::Checked
                : Qt::Unchecked;
        }
        break;
    }

    return QVariant();
}

Qt::ItemFlags ShortcutsModel::flags(const QModelIndex& /*index*/) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable |
           Qt::ItemIsUserCheckable | Qt::ItemIsEditable;
}

QModelIndex ShortcutsModel::index(int row, int column,
                                  const QModelIndex& /*parent*/) const
{
    if (row < 0 || row >= rowCount() ||
        column < 0 || column >= columnCount())
        return QModelIndex();
    return createIndex(row, column);
}

QModelIndex ShortcutsModel::parent(const QModelIndex& /*index*/) const
{
    return QModelIndex();
}

int ShortcutsModel::rowCount(const QModelIndex& /*parent*/) const
{
    return int(m_shortcuts.size() + m_actions.size());
}

void ShortcutsModel::setShortcuts(hotkeys_t active)
{
    beginResetModel();
    m_shortcutsselected = active;
    endResetModel();
}

const hotkeys_t& ShortcutsModel::getShortcuts()
{
    return m_shortcutsselected;
}

void ShortcutsModel::setActions(const QList<QAction*>& actions)
{
    beginResetModel();
    m_actions.clear();

    QSet<QString> names;
    for (QAction* action : actions)
    {
        if (!action || action->isSeparator() || action->objectName().isEmpty() ||
            names.contains(action->objectName()))
            continue;

        ActionEntry entry;
        entry.action = action;
        entry.name = action->objectName();
        entry.displayName =
            action->property("teamtalkShortcutText").toString();
        if (entry.displayName.isEmpty())
            entry.displayName = action->text();
        entry.displayName.remove('&');
        entry.displayName = entry.displayName.trimmed();
        if (entry.displayName.isEmpty() ||
            entry.displayName == QStringLiteral("-"))
            continue;

        const QVariant defaultShortcut =
            action->property("teamtalkDefaultShortcut");
        entry.defaultShortcut = defaultShortcut.isValid()
            ? defaultShortcut.value<QKeySequence>()
            : action->shortcut();

        m_actions.push_back(entry);
        names.insert(entry.name);
    }

    endResetModel();
}

void ShortcutsModel::setActionShortcuts(const actionshortcuts_t& shortcuts)
{
    beginResetModel();
    m_actionshortcuts = shortcuts;
    endResetModel();
}

const actionshortcuts_t& ShortcutsModel::getActionShortcuts() const
{
    return m_actionshortcuts;
}

bool ShortcutsModel::isActionShortcut(const QModelIndex& index) const
{
    return index.isValid() && index.row() >= m_shortcuts.size() &&
           index.row() < rowCount();
}

HotKeyID ShortcutsModel::hotKeyId(const QModelIndex& index) const
{
    if (!index.isValid() || isActionShortcut(index))
        return HOTKEY_NONE;
    return m_shortcuts[index.row()];
}

QString ShortcutsModel::actionName(const QModelIndex& index) const
{
    if (!isActionShortcut(index))
        return QString();
    return m_actions[index.row() - m_shortcuts.size()].name;
}

QString ShortcutsModel::actionDisplayName(const QModelIndex& index) const
{
    if (!isActionShortcut(index))
        return QString();
    return m_actions[index.row() - m_shortcuts.size()].displayName;
}

QKeySequence ShortcutsModel::actionShortcut(const QModelIndex& index) const
{
    if (!isActionShortcut(index))
        return QKeySequence();

    const ActionEntry& entry = m_actions[index.row() - m_shortcuts.size()];
    if (m_actionshortcuts.contains(entry.name))
        return m_actionshortcuts.value(entry.name);
    return entry.defaultShortcut;
}

void ShortcutsModel::setActionShortcut(const QString& actionName,
                                       const QKeySequence& shortcut)
{
    m_actionshortcuts.insert(actionName, shortcut);

    const int row = actionRow(actionName);
    if (row >= 0)
        emit dataChanged(index(row, COLUMN_NAME),
                         index(row, COLUMN_SHORTCUT));
}

int ShortcutsModel::actionRow(const QString& actionName) const
{
    for (int i = 0; i < m_actions.size(); ++i)
    {
        if (m_actions[i].name == actionName)
            return m_shortcuts.size() + i;
    }
    return -1;
}

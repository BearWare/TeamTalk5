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

#include "profilesmodel.h"

ProfilesModel::ProfilesModel(QObject* parent, get_logical_index_t getindex)
: QAbstractTableModel(parent)
, m_logical_column(getindex)
{
}

QVariant ProfilesModel::headerData(int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole */) const
{
    switch(role)
    {
    case Qt::DisplayRole :
        if(orientation == Qt::Horizontal)
            switch(section)
        {
            case COLUMN_INDEX_PROFILE_NAME: return tr("Profile Name");
        }
    }
    return QVariant();
}

int ProfilesModel::columnCount(const QModelIndex & /*parent = QModelIndex() */) const
{
    return COLUMN_COUNT_PROFILES;
}

QVariant ProfilesModel::data(const QModelIndex & index, int role /*= Qt::DisplayRole */) const
{
    Q_ASSERT(index.isValid());
    switch(role)
    {
    case Qt::DisplayRole :
        Q_ASSERT(index.row() < m_profiles.size());
        switch(index.column())
        {
        case COLUMN_INDEX_PROFILE_NAME :
            return m_profiles[index.row()].name;
        }
        break;
    case Qt::AccessibleTextRole:
        if (index.column() == m_logical_column(0))
        {
            QString accessibleText;
            int columnCount = this->columnCount(index);
            for (int i = 0; i < columnCount; ++i)
            {
                int logicalIndex = m_logical_column(i);
                accessibleText += QString("%1: %2, ")
                                      .arg(headerData(logicalIndex, Qt::Horizontal, Qt::DisplayRole).toString())
                                      .arg(data(createIndex(index.row(), logicalIndex, index.internalId()), Qt::DisplayRole).toString());
            }
            return accessibleText;
        }
        break;
    case Qt::UserRole :
        return m_profiles[index.row()].name;
    }
    return QVariant();
}

QModelIndex ProfilesModel::index(int row, int column, const QModelIndex & parent /*= QModelIndex() */) const
{
    if(!parent.isValid() && row<m_profiles.size())
        return createIndex(row, column);
    return QModelIndex();
}

QModelIndex ProfilesModel::parent(const QModelIndex &/* index */) const
{
    return QModelIndex();
}

int ProfilesModel::rowCount(const QModelIndex & /*parent = QModelIndex() */) const
{
    return m_profiles.size();
}

void ProfilesModel::addProfile(const ProfileEntry& profile)
{
    this->beginResetModel();
    m_profiles.push_back(profile);
    this->endResetModel();
}

void ProfilesModel::delProfile(int index)
{
    if(index < 0 || index >= m_profiles.size())
        return;
    this->beginResetModel();
    m_profiles.erase(m_profiles.begin()+index);
    this->endResetModel();
}

void ProfilesModel::renameProfile(int index, const QString& newname)
{
    if(index < 0 || index >= m_profiles.size())
        return;
    this->beginResetModel();
    m_profiles[index].name = newname;
    this->endResetModel();
}

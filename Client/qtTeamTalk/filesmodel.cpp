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

#include "filesmodel.h"

extern TTInstance* ttInst;

FilesModel::FilesModel(QObject* parent)
: QAbstractTableModel(parent)
, m_channelid(0)
{
}

QVariant FilesModel::headerData ( int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole */) const
{
    switch(role)
    {
    case Qt::DisplayRole :
        if(orientation == Qt::Horizontal)
        {
            switch(section)
            {
            case COLUMN_INDEX_NAME: return tr("Name");
            case COLUMN_INDEX_SIZE: return tr("Size");
            case COLUMN_INDEX_OWNER: return tr("Owner");
            case COLUMN_INDEX_UPLOADED : return tr("Date");
            }
        }
        break;
    case Qt::TextAlignmentRole :
        if(orientation == Qt::Horizontal)
        {
            switch(section)
            {
            case COLUMN_INDEX_NAME:
            case COLUMN_INDEX_OWNER:
                return Qt::AlignLeft;
            default:
                return Qt::AlignRight;
            }
        }
    }
    return QVariant();
}

int FilesModel::columnCount ( const QModelIndex & /*parent = QModelIndex()*/ ) const
{
    return COLUMN_COUNT_FILESMODEL;
}

QVariant FilesModel::data ( const QModelIndex & index, int role /*= Qt::DisplayRole*/ ) const
{
    Q_ASSERT(index.isValid());
    switch(role)
    {
    case Qt::DisplayRole :
        Q_ASSERT(index.row() < m_files.size());
        switch(index.column())
        {
        case COLUMN_INDEX_NAME :
            return _Q(m_files[index.row()].szFileName);
        case COLUMN_INDEX_SIZE :
            return getFormattedSize(m_files[index.row()].nFileSize);
        case COLUMN_INDEX_OWNER :
            return _Q(m_files[index.row()].szUsername);
        case COLUMN_INDEX_UPLOADED :
            return getFormattedDateTime(_Q(m_files[index.row()].szUploadTime), "yyyy/MM/dd hh:mm");
        }
        break;
    case Qt::UserRole :
        switch(index.column())
        {
        case COLUMN_INDEX_UPLOADED :
            return _Q(m_files[index.row()].szUploadTime);
            break;
        case COLUMN_INDEX_SIZE :
            return m_files[index.row()].nFileSize;
            break;
        default :
            return data(index, Qt::DisplayRole);
            break;
        }
        break;
    case Qt::AccessibleTextRole :
    {
        if (index.column() == COLUMN_INDEX_NAME)
        {
            return QString("%1: %2, %3: %4, %5: %6, %7: %8").arg(headerData(COLUMN_INDEX_NAME, Qt::Horizontal, Qt::DisplayRole).toString()).arg(data(createIndex(index.row(), COLUMN_INDEX_NAME, index.internalId()), Qt::DisplayRole).toString()).arg(headerData(COLUMN_INDEX_SIZE, Qt::Horizontal, Qt::DisplayRole).toString()).arg(data(createIndex(index.row(), COLUMN_INDEX_SIZE, index.internalId()), Qt::DisplayRole).toString()).arg(headerData(COLUMN_INDEX_OWNER, Qt::Horizontal, Qt::DisplayRole).toString()).arg(data(createIndex(index.row(), COLUMN_INDEX_OWNER, index.internalId()), Qt::DisplayRole).toString()).arg(headerData(COLUMN_INDEX_UPLOADED, Qt::Horizontal, Qt::DisplayRole).toString()).arg(data(createIndex(index.row(), COLUMN_INDEX_UPLOADED, index.internalId()), Qt::DisplayRole).toString());
        }
    }
    break;
    }
    return QVariant();
}

QModelIndex FilesModel::index ( int row, int column, const QModelIndex & parent /*= QModelIndex()*/ ) const
{
   if(!parent.isValid() && row<m_files.size())
       return createIndex(row, column, (int)m_files[row].nFileID);
    return QModelIndex();
}

QModelIndex FilesModel::parent ( const QModelIndex & /*index */) const
{
    return QModelIndex();
}

int FilesModel::rowCount ( const QModelIndex & /*parent = QModelIndex()*/ ) const
{
    return m_files.size();
}

void FilesModel::slotChannelUpdated(int channelid)
{
    this->beginResetModel();

    int count = 0;
    TT_GetChannelFiles(ttInst, channelid, nullptr, &count);
    m_files.resize(count);
    if(count)
        TT_GetChannelFiles(ttInst, channelid, &m_files[0], &count);

    m_channelid = channelid;
    
    this->endResetModel();
}


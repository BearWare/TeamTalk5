/*
 * Copyright (c) 2005-2016, BearWare.dk
 * 
 * Contact Information:
 *
 * Bjoern D. Rasmussen
 * Kirketoften 5
 * DK-8260 Viby J
 * Denmark
 * Email: contact@bearware.dk
 * Phone: +45 20 20 54 59
 * Web: http://www.bearware.dk
 *
 * This source code is part of the TeamTalk 5 SDK owned by
 * BearWare.dk. All copyright statements may not be removed 
 * or altered from any source distribution. If you use this
 * software in a product, an acknowledgment in the product 
 * documentation is required.
 *
 */

#include "filesmodel.h"

extern TTInstance* ttInst;

FilesModel::FilesModel(QObject* parent)
: QAbstractItemModel(parent)
, m_channelid(0)
{
}

QVariant FilesModel::headerData ( int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole */) const
{
    switch(role)
    {
    case Qt::DisplayRole :
        if(orientation == Qt::Horizontal)
            switch(section)
        {
            case COLUMN_INDEX_NAME: return tr("Name");
            case COLUMN_INDEX_SIZE: return tr("Size");
            case COLUMN_INDEX_OWNER: return tr("Owner");
        }
        break;
    case Qt::TextAlignmentRole :
        if(orientation == Qt::Horizontal)
            switch(section)
        {
            case COLUMN_INDEX_NAME:
            case COLUMN_INDEX_OWNER:
                return Qt::AlignLeft;
            default:
                return Qt::AlignRight;
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
            {
                QString result;
                if(m_files[index.row()].nFileSize>=1024*1024)
                    result = QString("%1 M").arg(m_files[index.row()].nFileSize/(1024*1024));
                else if(m_files[index.row()].nFileSize>=1024)
                    result = QString("%1 K").arg(m_files[index.row()].nFileSize/1024);
                else
                    result = QString("%1").arg(m_files[index.row()].nFileSize);
                return result;
            }
        case COLUMN_INDEX_OWNER :
            return _Q(m_files[index.row()].szUsername);
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
    TT_GetChannelFiles(ttInst, channelid, NULL, &count);
    m_files.resize(count);
    if(count)
        TT_GetChannelFiles(ttInst, channelid, &m_files[0], &count);

    m_channelid = channelid;
    
    this->endResetModel();
}


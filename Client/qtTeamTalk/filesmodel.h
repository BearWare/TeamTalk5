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

#ifndef FILESMODEL_H
#define FILESMODEL_H

#include "common.h"
#include "utilui.h"

#include <QAbstractTableModel>
#include <QVector>

enum
{
    COLUMN_INDEX_NAME,
    COLUMN_INDEX_SIZE,
    COLUMN_INDEX_OWNER,
    COLUMN_INDEX_UPLOADED,
    COLUMN_COUNT_FILESMODEL
};

class FilesModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    FilesModel(QObject* parent);
    QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
    int columnCount ( const QModelIndex & parent = QModelIndex() ) const;
    QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
    QModelIndex index ( int row, int column, const QModelIndex & parent = QModelIndex() ) const;
    QModelIndex parent ( const QModelIndex & index ) const;
    int rowCount ( const QModelIndex & parent = QModelIndex() ) const;

    int getChannelID() const { return m_channelid; }
public:
    void slotChannelUpdated(int channelid);

private:
   QVector<RemoteFile> m_files;
   int m_channelid;
};

#endif

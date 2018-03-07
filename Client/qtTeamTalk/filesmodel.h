/*
 * Copyright (c) 2005-2018, BearWare.dk
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
 * This source code is part of the TeamTalk SDK owned by
 * BearWare.dk. Use of this file, or its compiled unit, requires a
 * TeamTalk SDK License Key issued by BearWare.dk.
 *
 * The TeamTalk SDK License Agreement along with its Terms and
 * Conditions are outlined in the file License.txt included with the
 * TeamTalk SDK distribution.
 *
 */

#ifndef FILESMODEL_H
#define FILESMODEL_H

#include <QAbstractItemModel>
#include <QVector>

#include "common.h"

enum
{
    COLUMN_INDEX_NAME,
    COLUMN_INDEX_SIZE,
    COLUMN_INDEX_OWNER,
    COLUMN_COUNT_FILESMODEL
};

class FilesModel : public QAbstractItemModel
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
public slots:
    void slotChannelUpdated(int channelid);

private:
   QVector<RemoteFile> m_files;
   int m_channelid;
};

#endif

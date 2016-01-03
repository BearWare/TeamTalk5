/*
 * Copyright (c) 2005-2016, BearWare.dk
 * 
 * Contact Information:
 *
 * Bjoern D. Rasmussen
 * Skanderborgvej 40 4-2
 * DK-8000 Aarhus C
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

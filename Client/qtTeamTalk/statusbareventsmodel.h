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

#ifndef STATUSBAREVENTSMODEL_H
#define STATUSBAREVENTSMODEL_H

#include "utilui.h"

#include <QAbstractItemModel>
#include <QVector>
#include <QTreeView>

class StatusBarEventsModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    StatusBarEventsModel(QObject* parent);
    QVariant headerData ( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;
    int columnCount ( const QModelIndex & parent = QModelIndex() ) const;
    QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;
    QModelIndex index ( int row, int column, const QModelIndex & parent = QModelIndex() ) const;
    QModelIndex parent ( const QModelIndex & index ) const;
    int rowCount ( const QModelIndex & parent = QModelIndex() ) const;

    void setStatusBarEvents(StatusBarEvents statusbaractive);
    StatusBarEvents getStatusBarEvents();
private:
    QVector<StatusBarEvent> m_statusbarevents;
    StatusBarEvents m_statusbarselected = STATUSBAR_NONE;
};

#endif // STATUSBAREVENTSMODEL_H

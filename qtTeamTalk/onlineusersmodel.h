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

#ifndef ONLINEUSERSMODEL_H
#define ONLINEUSERSMODEL_H

#include <QAbstractItemModel>
#include <QVector>

#include "common.h"

enum
{
    COLUMN_USERID,
    COLUMN_NICKNAME,
    COLUMN_STATUSMSG,
    COLUMN_USERNAME,
    COLUMN_CHANNEL,
    COLUMN_IPADDRESS,
    COLUMN_VERSION,
    _COLUMN_LAST_COUNT
};

class OnlineUsersModel : public QAbstractItemModel
{
    Q_OBJECT
	
public:
	OnlineUsersModel(QObject* parent);
	void resetUsers();

    void addUser(int userid);
    void updateUser(int userid);
    void removeUser(int userid);

    QModelIndex userRow(int userid);

    int columnCount(const QModelIndex & parent = QModelIndex() ) const;
    int rowCount(const QModelIndex & parent = QModelIndex() ) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    QVariant data(const QModelIndex &index, int role) const;
    QModelIndex index(int row, int column, const QModelIndex & parent = QModelIndex() ) const;
    QModelIndex parent(const QModelIndex& /*index */) const { return QModelIndex(); }
private:
    typedef QMap<int, User> user_cache_t;
    user_cache_t m_users;
};

#endif

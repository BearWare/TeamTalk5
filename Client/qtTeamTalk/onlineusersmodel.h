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

#ifndef ONLINEUSERSMODEL_H
#define ONLINEUSERSMODEL_H

#include <QAbstractTableModel>
#include <QVector>

#include "utilui.h"

enum
{
    COLUMN_NICKNAME,
    COLUMN_STATUSMSG,
    COLUMN_USERNAME,
    COLUMN_CHANNEL,
    COLUMN_IPADDRESS,
    COLUMN_VERSION,
    COLUMN_USERID,
    COLUMN_LAST_COUNT
};

class OnlineUsersModel : public QAbstractTableModel
{
    Q_OBJECT
	
public:
    OnlineUsersModel(QObject* parent, get_logical_index_t getindex);
    void resetUsers();

    void addUser(const User& user);
    void updateUser(const User& user);
    void removeUser(const User& user, bool keep);
    void removeDisconnected();
    User getUser(int userid) const;

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
    get_logical_index_t m_logical_column;
};

#endif

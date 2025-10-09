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

#include "onlineusersmodel.h"

extern TTInstance* ttInst;

#define DISCONNECTED_USERID -1

OnlineUsersModel::OnlineUsersModel(QObject* parent, get_logical_index_t getindex)
: QAbstractTableModel(parent)
    , m_logical_column(getindex)
{
}

void OnlineUsersModel::resetUsers()
{
    this->beginResetModel();

    m_users.clear();
    int n_users = 0;
    TT_GetServerUsers(ttInst, nullptr, &n_users);
    if(n_users)
    {
        QVector<User> users;
        users.resize(n_users);
        TT_GetServerUsers(ttInst, &users[0], &n_users);

        for(int i=0;i<users.size();i++)
        {
            m_users.insert(users[i].nUserID, users[i]);
        }
    }

    this->endResetModel();
}

void OnlineUsersModel::addUser(const User& user)
{
    this->beginResetModel();
    m_users.insert(user.nUserID, user);
    this->endResetModel();
}

void OnlineUsersModel::updateUser(const User& user)
{
    this->beginResetModel();
    m_users.insert(user.nUserID, user);
    this->endResetModel();
}

void OnlineUsersModel::removeUser(const User& user, bool keep)
{
    this->beginResetModel();
    if (keep)
        m_users[user.nUserID].nUserID = DISCONNECTED_USERID;
    else
        m_users.remove(user.nUserID);

    this->endResetModel();
}

void OnlineUsersModel::removeDisconnected()
{
    this->beginResetModel();
    for (auto ii = m_users.begin(); ii != m_users.end();)
    {
        if (ii->nUserID == DISCONNECTED_USERID)
            ii = m_users.erase(ii);
        else ++ii;
    }
    this->endResetModel();
}

User OnlineUsersModel::getUser(int userid) const
{
    User user = {};
    auto i = m_users.find(userid);
    if (i != m_users.end())
    {
        user = i.value();
        user.nUserID = userid; // user.nUserID == DISCONNECTED_USERID
    }
    return user;
}

QModelIndex OnlineUsersModel::userRow(int userid)
{
    int i=0;
    user_cache_t::const_iterator ii;
    for(ii=m_users.begin();ii!=m_users.end();ii++,i++)
        if(ii.key() == userid)
            return createIndex(i, 0, userid);
    return QModelIndex();
}

int OnlineUsersModel::columnCount(const QModelIndex& /*parent = QModelIndex()*/) const
{
    return COLUMN_LAST_COUNT;
}

int OnlineUsersModel::rowCount(const QModelIndex& /*parent = QModelIndex()*/) const
{
    return m_users.size();
}

QVariant OnlineUsersModel::headerData(int section, Qt::Orientation orientation,
                                      int role) const
{
    switch(role)
    {
    case Qt::DisplayRole :
        if (orientation == Qt::Horizontal)
        {
            switch (section)
            {
            case COLUMN_NICKNAME:
                return tr("Nickname");
            case COLUMN_STATUSMSG:
                return tr("Status message");
            case COLUMN_USERNAME:
                return tr("Username");
            case COLUMN_CHANNEL:
                return tr("Channel");
            case COLUMN_IPADDRESS:
                return tr("IP-address");
            case COLUMN_VERSION:
                return tr("Version");
            case COLUMN_USERID:
                return tr("ID");
            }
        }
        break;
    case Qt::TextAlignmentRole :
        break;
    }
    return QVariant();
}

QVariant OnlineUsersModel::data(const QModelIndex& index, int role) const
{
    user_cache_t::const_iterator ii = m_users.find(index.internalId());
    Q_ASSERT(ii != m_users.end());
    if(ii == m_users.end())
        return QVariant();

    const User& user = ii.value();
    TTCHAR channel[TT_STRLEN] = {};

    switch(role)
    {
    case Qt::DisplayRole :
        switch(index.column())
        {
        case COLUMN_NICKNAME :
            return getDisplayName(user);
        case COLUMN_STATUSMSG :
            return _Q(user.szStatusMsg);
        case COLUMN_USERNAME :
            return _Q(user.szUsername);
        case COLUMN_CHANNEL :
            TT_GetChannelPath(ttInst, user.nChannelID, channel);
            return _Q(channel);
        case COLUMN_IPADDRESS :
            return _Q(user.szIPAddress);
        case COLUMN_VERSION :
            return getVersion(user);
        case COLUMN_USERID :
            return user.nUserID;
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
    }
    return QVariant();
}

QModelIndex OnlineUsersModel::index(int row, int column, const QModelIndex&/*parent= QModelIndex()*/ ) const
{
    if(m_users.empty())
        return QModelIndex();
    
    auto i = m_users.begin();
    int x = row;
    while (x--) i++;
    return createIndex(row, column, i.key());
}


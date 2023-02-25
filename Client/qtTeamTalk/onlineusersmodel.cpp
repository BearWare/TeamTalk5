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

#include "onlineusersmodel.h"

extern TTInstance* ttInst;

#define DISCONNECTED_USERID -1

OnlineUsersModel::OnlineUsersModel(QObject* parent)
: QAbstractItemModel(parent)
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

void OnlineUsersModel::addUser(int userid)
{
    this->beginResetModel();

    User user;
    if(TT_GetUser(ttInst, userid, &user))
    {
        m_users.insert(user.nUserID, user);
    }

    this->endResetModel();
}

void OnlineUsersModel::updateUser(int userid)
{
    User user;
    if(TT_GetUser(ttInst, userid, &user))
        m_users.insert(user.nUserID, user);
}

void OnlineUsersModel::removeUser(int userid, bool keep)
{
    this->beginResetModel();
    if (keep)
        m_users[userid].nUserID = DISCONNECTED_USERID;
    else
        m_users.remove(userid);

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
    return _COLUMN_LAST_COUNT;
}

int OnlineUsersModel::rowCount(const QModelIndex& /*parent = QModelIndex()*/) const
{
    return m_users.size();
}

QVariant OnlineUsersModel::headerData(int section, Qt::Orientation /*orientation*/,
                                      int role) const
{
    switch(role)
    {
    case Qt::DisplayRole :
        switch(section)
        {
        case COLUMN_NICKNAME :
            return tr("Nickname");
        case COLUMN_STATUSMSG :
            return tr("Status message");
        case COLUMN_USERNAME :
            return tr("Username");
        case COLUMN_CHANNEL :
            return tr("Channel");
        case COLUMN_IPADDRESS :
            return tr("IP-address");
        case COLUMN_VERSION :
            return tr("Version");
        case COLUMN_USERID :
            return tr("ID");
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
        case Qt::AccessibleTextRole :
        {
            TT_GetChannelPath(ttInst, user.nChannelID, channel);
            return QString(tr("Nickname: %2, Status message: %3, Username: %4, Channel: %5, IP address: %6, Version: %7, ID: %1").arg(user.nUserID).arg(getDisplayName(user)).arg(_Q(user.szStatusMsg)).arg(_Q(user.szUsername)).arg(_Q(channel)).arg(_Q(user.szIPAddress)).arg(getVersion(user)));
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


/*
 * Copyright (c) 2005-2014, BearWare.dk
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

#include "onlineusersmodel.h"

extern TTInstance* ttInst;

OnlineUsersModel::OnlineUsersModel(QObject* parent)
: QAbstractItemModel(parent)
{
}

void OnlineUsersModel::resetUsers()
{
    this->beginResetModel();

    m_users.clear();
    int n_users = 0;
    TT_GetServerUsers(ttInst, NULL, &n_users);
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

void OnlineUsersModel::removeUser(int userid)
{
    this->beginResetModel();

    m_users.remove(userid);

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
        case COLUMN_USERID :
            return tr("ID");
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
    TTCHAR channel[TT_STRLEN] = {0};

    switch(role)
    {
    case Qt::DisplayRole :
        switch(index.column())
        {
        case COLUMN_USERID :
            return user.nUserID;
        case COLUMN_NICKNAME :
            return _Q(user.szNickname);
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
        }
        break;
    }
    return QVariant();
}

QModelIndex OnlineUsersModel::index(int row, int column, const QModelIndex&/*parent= QModelIndex()*/ ) const
{
    if(m_users.empty())
        return QModelIndex();
    
    return createIndex(row, column, (m_users.begin()+row)->nUserID);
}


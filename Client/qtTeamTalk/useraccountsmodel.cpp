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

#include "useraccountsmodel.h"

UserAccountsModel::UserAccountsModel(QObject* parent)
: QAbstractItemModel(parent)
{
}

QVariant UserAccountsModel::headerData(int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole */) const
{
    switch(role)
    {
    case Qt::DisplayRole :
        if(orientation == Qt::Horizontal)
            switch(section)
        {
            case COLUMN_INDEX_USERNAME: return tr("Username");
            case COLUMN_INDEX_PASSWORD: return tr("Password");
            case COLUMN_INDEX_USERTYPE: return tr("User Type");
            case COLUMN_INDEX_NOTE: return tr("Note");
            case COLUMN_INDEX_CHANNEL: return tr("Channel");
            case COLUMN_INDEX_MODIFIED : return tr("Modified");
        }
    }
    return QVariant();
}

int UserAccountsModel::columnCount(const QModelIndex & parent /*= QModelIndex() */) const
{
    if(!parent.isValid())
        return COLUMN_COUNT_USERACCOUNTS;
    return 0;
}

QVariant UserAccountsModel::data(const QModelIndex & index, int role /*= Qt::DisplayRole */) const
{
    Q_ASSERT(index.isValid());
    switch(role)
    {
    case Qt::DisplayRole :
        Q_ASSERT(index.row() < m_users.size());
        switch(index.column())
        {
        case COLUMN_INDEX_USERNAME :
            return _Q(m_users[index.row()].szUsername);
        case COLUMN_INDEX_PASSWORD :
            return _Q(m_users[index.row()].szPassword);
        case COLUMN_INDEX_USERTYPE :
            if(m_users[index.row()].uUserType & USERTYPE_ADMIN)
                return tr("Administrator");
            else if(m_users[index.row()].uUserType & USERTYPE_DEFAULT)
                return tr("Default User");
            else if(m_users[index.row()].uUserType == USERTYPE_NONE)
                return tr("Disabled");
            else
                return tr("Unknown");
        case COLUMN_INDEX_NOTE :
            return _Q(m_users[index.row()].szNote);
        case COLUMN_INDEX_CHANNEL :
            return _Q(m_users[index.row()].szInitChannel);
        case COLUMN_INDEX_MODIFIED :
            return _Q(m_users[index.row()].szLastModified);
        }
        break;
    case Qt::AccessibleTextRole :
    {
        QString result;
        if(m_users[index.row()].uUserType & USERTYPE_ADMIN)
            result = tr("Administrator");
        else if(m_users[index.row()].uUserType & USERTYPE_DEFAULT)
            result = tr("Default User");
        else if(m_users[index.row()].uUserType == USERTYPE_NONE)
            result = tr("Disabled");
        else
            result = tr("Unknown");
        return QString(tr("Username: %1, Password: %2, Type: %3, Note: %4, Initial channel: %5, Modified: %6").arg(_Q(m_users[index.row()].szUsername)).arg(_Q(m_users[index.row()].szPassword)).arg(result).arg(_Q(m_users[index.row()].szNote)).arg(_Q(m_users[index.row()].szInitChannel)).arg(_Q(m_users[index.row()].szLastModified)));
    }
    break;
    }
    return QVariant();
}

QModelIndex UserAccountsModel::index(int row, int column, const QModelIndex & parent /*= QModelIndex() */) const
{
    if(!parent.isValid() && row<m_users.size())
        return createIndex(row, column);
    return QModelIndex();
}

QModelIndex UserAccountsModel::parent(const QModelIndex &/* index */) const
{
    return QModelIndex();
}

int UserAccountsModel::rowCount(const QModelIndex & /*parent = QModelIndex() */) const
{
    return m_users.size();
}

void UserAccountsModel::addRegUser(const UserAccount& user, bool do_reset)
{
    for(int i=0;i<m_users.size();i++)
    {
        if(_Q(m_users[i].szUsername) == _Q(user.szUsername))
        {
            m_users.erase(m_users.begin()+i);
            break;
        }
    }
    m_users.push_back(user);
    if(do_reset)
    {
        this->beginResetModel();
        this->endResetModel();
    }
}

void UserAccountsModel::delRegUser(int index)
{
    if(m_users.size())
        m_users.erase(m_users.begin()+index);
    this->beginResetModel();
    this->endResetModel();
}

void UserAccountsModel::delRegUser(const QString& username)
{
    this->beginResetModel();

    for(int i=0;i<m_users.size();i++)
    {
        if(_Q(m_users[i].szUsername) == username)
        {
            m_users.erase(m_users.begin()+i);
            break;
        }
    }
    this->endResetModel();
}

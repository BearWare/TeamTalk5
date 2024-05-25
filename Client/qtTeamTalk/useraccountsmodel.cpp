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

#include "useraccountsmodel.h"
#include "utilui.h"

UserAccountsModel::UserAccountsModel(QObject* parent)
: QAbstractTableModel(parent)
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
            return getFormattedDateTime(_Q(m_users[index.row()].szLastModified), "yyyy/MM/dd hh:mm");
        }
        break;
    case Qt::UserRole :
        switch(index.column())
        {
        case COLUMN_INDEX_MODIFIED :
            return _Q(m_users[index.row()].szLastModified);
            break;
        default :
            return data(index, Qt::DisplayRole);
            break;
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

UserRightsModel::UserRightsModel(QObject* parent) : QAbstractItemModel(parent)
{
}

void UserRightsModel::insertUserRights()
{
    m_userrights.clear();
    m_userrights.push_back(USERRIGHT_MULTI_LOGIN);
    m_userrights.push_back(USERRIGHT_LOCKED_NICKNAME);
    m_userrights.push_back(USERRIGHT_VIEW_ALL_USERS);
    m_userrights.push_back(USERRIGHT_VIEW_HIDDEN_CHANNELS);
    m_userrights.push_back(USERRIGHT_MODIFY_CHANNELS);
    m_userrights.push_back(USERRIGHT_CREATE_TEMPORARY_CHANNEL);
    m_userrights.push_back(USERRIGHT_TEXTMESSAGE_BROADCAST);
    m_userrights.push_back(USERRIGHT_KICK_USERS);
    m_userrights.push_back(USERRIGHT_BAN_USERS);
    m_userrights.push_back(USERRIGHT_MOVE_USERS);
    m_userrights.push_back(USERRIGHT_OPERATOR_ENABLE);
    m_userrights.push_back(USERRIGHT_UPLOAD_FILES);
    m_userrights.push_back(USERRIGHT_DOWNLOAD_FILES);
    m_userrights.push_back(USERRIGHT_RECORD_VOICE);
    m_userrights.push_back(USERRIGHT_UPDATE_SERVERPROPERTIES);
    m_userrights.push_back(USERRIGHT_TRANSMIT_VOICE);
    m_userrights.push_back(USERRIGHT_TRANSMIT_VIDEOCAPTURE);
    m_userrights.push_back(USERRIGHT_TRANSMIT_DESKTOP);
    m_userrights.push_back(USERRIGHT_TRANSMIT_DESKTOPINPUT);
    m_userrights.push_back(USERRIGHT_TRANSMIT_MEDIAFILE_AUDIO);
    m_userrights.push_back(USERRIGHT_TRANSMIT_MEDIAFILE_VIDEO);
    m_userrights.push_back(USERRIGHT_TEXTMESSAGE_USER);
    m_userrights.push_back(USERRIGHT_TEXTMESSAGE_CHANNEL);
    //m_userrights.push_back(USERRIGHT_LOCKED_STATUS);
}

int UserRightsModel::columnCount(const QModelIndex & /*parent = QModelIndex()*/) const
{
    return 1;
}

QVariant UserRightsModel::data(const QModelIndex & index, int role /*= Qt::DisplayRole*/) const
{
    Q_ASSERT(index.isValid());
    switch(role)
    {
    case Qt::DisplayRole :
        switch (m_userrights[index.row()])
        {
        case USERRIGHT_NONE :
            Q_ASSERT(m_userrights[index.row()] != USERRIGHT_NONE);
            break;
        case USERRIGHT_MULTI_LOGIN :
            return tr("User can log in multiple times");
        case USERRIGHT_VIEW_ALL_USERS :
            return tr("User can see users in all channels");
        case USERRIGHT_VIEW_HIDDEN_CHANNELS :
            return tr("User can see hidden channels");
        case USERRIGHT_MODIFY_CHANNELS :
            return tr("User can create/modify all channels");
        case USERRIGHT_CREATE_TEMPORARY_CHANNEL :
            return tr("User can create temporary channels");
        case USERRIGHT_TEXTMESSAGE_BROADCAST :
            return tr("User can broadcast text messages");
        case USERRIGHT_KICK_USERS :
            return tr("User can kick users off the server");
        case USERRIGHT_BAN_USERS :
            return tr("User can ban users from server");
        case USERRIGHT_MOVE_USERS :
            return tr("User can move users between channels");
        case USERRIGHT_OPERATOR_ENABLE :
            return tr("User can make other users channel operator");
        case USERRIGHT_UPLOAD_FILES :
            return tr("User can upload files");
        case USERRIGHT_DOWNLOAD_FILES :
            return tr("User can download files");
        case USERRIGHT_RECORD_VOICE :
            return tr("User can record voice in all channels");
        case USERRIGHT_UPDATE_SERVERPROPERTIES :
            return tr("User can update server properties");
        case USERRIGHT_TRANSMIT_VOICE :
            return tr("User can transmit voice data (microphone)");
        case USERRIGHT_TRANSMIT_VIDEOCAPTURE :
            return tr("User can transmit video data (webcam)");
        case USERRIGHT_TRANSMIT_DESKTOP :
            return tr("User can transmit desktop sessions (shared desktop)");
        case USERRIGHT_TRANSMIT_DESKTOPINPUT :
            return tr("User can get remote access to desktop sessions");
        case USERRIGHT_TRANSMIT_MEDIAFILE_AUDIO :
            return tr("User can transmit audio files (wav, mp3 files)");
        case USERRIGHT_TRANSMIT_MEDIAFILE_VIDEO :
            return tr("User can transmit video files (avi, mp4 files)");
        case USERRIGHT_TRANSMIT_MEDIAFILE :
            // OR'ed value
            Q_ASSERT(m_userrights[index.row()] != USERRIGHT_TRANSMIT_MEDIAFILE);
            break;
        case USERRIGHT_TEXTMESSAGE_USER :
            return tr("User can send private text messages");
        case USERRIGHT_TEXTMESSAGE_CHANNEL :
            return tr("User can send channel text messages");
        case USERRIGHT_LOCKED_NICKNAME :
            return tr("User can change nickname"); // inverted text explanation due to compatibility
        case USERRIGHT_LOCKED_STATUS :
            return tr("User can change status mode"); // inverted text explanation due to compatibility
        }

        break;
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    case Qt::AccessibleTextRole :
        switch (m_userrights[index.row()])
        {
        case USERRIGHT_LOCKED_NICKNAME :
        case USERRIGHT_LOCKED_STATUS :
            return QString("%1: %2").arg(data(index, Qt::DisplayRole).toString()).arg((m_activeUserRights & m_userrights[index.row()])? tr("Disabled") : tr("Enabled"));
        default :
            return QString("%1: %2").arg(data(index, Qt::DisplayRole).toString()).arg((m_activeUserRights & m_userrights[index.row()])? tr("Enabled") : tr("Disabled"));
        }
#endif
    case Qt::CheckStateRole :
        switch (m_userrights[index.row()])
        {
        case USERRIGHT_LOCKED_NICKNAME :
        case USERRIGHT_LOCKED_STATUS :
            return (m_userrights[index.row()] & getUserRights()) ? Qt::Unchecked : Qt::Checked;
        default :
            return (m_userrights[index.row()] & getUserRights()) ? Qt::Checked : Qt::Unchecked;
        }
    }
    return QVariant();
}

Qt::ItemFlags UserRightsModel::flags(const QModelIndex &index) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable |  Qt::ItemIsEditable;
}

QModelIndex UserRightsModel::index(int row, int column, const QModelIndex & /*parent = QModelIndex()*/) const
{
    return createIndex(row, column, m_userrights[row]);
}

QModelIndex UserRightsModel::parent(const QModelIndex & /*index*/) const
{
    return QModelIndex();
}

int UserRightsModel::rowCount(const QModelIndex & /*parent = QModelIndex()*/) const
{
    return m_userrights.size();
}

void UserRightsModel::setUserRights(UserTypes usertypes, UserRights rights)
{
    this->beginResetModel();
    if (usertypes & USERTYPE_ADMIN)
        m_userrights.clear();
    else
        insertUserRights();

    m_activeUserRights = rights;
    this->endResetModel();
}

UserRights UserRightsModel::getUserRights() const
{
    return m_activeUserRights;
}

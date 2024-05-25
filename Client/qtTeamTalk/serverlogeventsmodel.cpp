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

#include "serverlogeventsmodel.h"

enum
{
    COLUMN_NAME = 0,
    COLUMN_COUNT = 1,
};

ServerLogEventsModel::ServerLogEventsModel(QObject* parent)
    : QAbstractTableModel(parent)
{
    m_serverlogevents.push_back(SERVERLOGEVENT_USER_CONNECTED);
    m_serverlogevents.push_back(SERVERLOGEVENT_USER_DISCONNECTED);
    m_serverlogevents.push_back(SERVERLOGEVENT_USER_LOGGEDIN);
    m_serverlogevents.push_back(SERVERLOGEVENT_USER_LOGGEDOUT);
    m_serverlogevents.push_back(SERVERLOGEVENT_USER_LOGINFAILED);
    m_serverlogevents.push_back(SERVERLOGEVENT_USER_TIMEDOUT);
    m_serverlogevents.push_back(SERVERLOGEVENT_USER_KICKED);
    m_serverlogevents.push_back(SERVERLOGEVENT_USER_BANNED);
    m_serverlogevents.push_back(SERVERLOGEVENT_USER_UNBANNED);
    m_serverlogevents.push_back(SERVERLOGEVENT_USER_UPDATED);
    m_serverlogevents.push_back(SERVERLOGEVENT_USER_JOINEDCHANNEL);
    m_serverlogevents.push_back(SERVERLOGEVENT_USER_LEFTCHANNEL);
    m_serverlogevents.push_back(SERVERLOGEVENT_USER_MOVED);
    m_serverlogevents.push_back(SERVERLOGEVENT_USER_TEXTMESSAGE_PRIVATE);
    m_serverlogevents.push_back(SERVERLOGEVENT_USER_TEXTMESSAGE_CUSTOM);
    m_serverlogevents.push_back(SERVERLOGEVENT_USER_TEXTMESSAGE_CHANNEL);
    m_serverlogevents.push_back(SERVERLOGEVENT_USER_TEXTMESSAGE_BROADCAST);
    m_serverlogevents.push_back(SERVERLOGEVENT_CHANNEL_CREATED);
    m_serverlogevents.push_back(SERVERLOGEVENT_CHANNEL_UPDATED);
    m_serverlogevents.push_back(SERVERLOGEVENT_CHANNEL_REMOVED);
    m_serverlogevents.push_back(SERVERLOGEVENT_FILE_UPLOADED);
    m_serverlogevents.push_back(SERVERLOGEVENT_FILE_DOWNLOADED);
    m_serverlogevents.push_back(SERVERLOGEVENT_FILE_DELETED);
    m_serverlogevents.push_back(SERVERLOGEVENT_SERVER_UPDATED);
    m_serverlogevents.push_back(SERVERLOGEVENT_SERVER_SAVECONFIG);
}

QVariant ServerLogEventsModel::headerData (int section, Qt::Orientation orientation, int role/* = Qt::DisplayRole*/) const
{
    switch(role)
    {
    case Qt::DisplayRole :
        if(orientation == Qt::Horizontal)
        {
            switch(section)
            {
            case COLUMN_NAME: return tr("Event");
            }
        }
        break;
    case Qt::TextAlignmentRole :
        return Qt::AlignLeft;
    }
    return QVariant();
}

int ServerLogEventsModel::columnCount ( const QModelIndex & parent /*= QModelIndex()*/) const
{
    return COLUMN_COUNT;
}

QVariant ServerLogEventsModel::data(const QModelIndex& index, int role /*= Qt::DisplayRole*/) const
{
    switch(role)
    {
    case Qt::DisplayRole :
        Q_ASSERT(index.column() == COLUMN_NAME);
        switch(m_serverlogevents[index.row()])
        {
        case SERVERLOGEVENT_USER_CONNECTED :
            return tr("User connected");
        case SERVERLOGEVENT_USER_DISCONNECTED :
            return tr("User disconnected");
        case SERVERLOGEVENT_USER_LOGGEDIN :
            return tr("User logged in");
        case SERVERLOGEVENT_USER_LOGGEDOUT :
            return tr("User logged out");
        case SERVERLOGEVENT_USER_LOGINFAILED :
            return tr("User login failed");
        case SERVERLOGEVENT_USER_TIMEDOUT :
            return tr("User connection timed out");
        case SERVERLOGEVENT_USER_KICKED :
            return tr("User kicked");
        case SERVERLOGEVENT_USER_BANNED :
            return tr("User banned");
        case SERVERLOGEVENT_USER_UNBANNED :
            return tr("User ban removed");
        case SERVERLOGEVENT_USER_UPDATED :
            return tr("User status updated");
        case SERVERLOGEVENT_USER_JOINEDCHANNEL :
            return tr("User joined channel");
        case SERVERLOGEVENT_USER_LEFTCHANNEL :
            return tr("User left channel");
        case SERVERLOGEVENT_USER_MOVED :
            return tr("User moved to other channel");
        case SERVERLOGEVENT_USER_TEXTMESSAGE_PRIVATE :
            return tr("User sent private text message");
        case SERVERLOGEVENT_USER_TEXTMESSAGE_CUSTOM :
            return tr("User sent custom text message");
        case SERVERLOGEVENT_USER_TEXTMESSAGE_CHANNEL :
            return tr("User sent channel text message");
        case SERVERLOGEVENT_USER_TEXTMESSAGE_BROADCAST :
            return tr("User sent broadcast text message");
        case SERVERLOGEVENT_CHANNEL_CREATED :
            return tr("Channel created");
        case SERVERLOGEVENT_CHANNEL_UPDATED :
            return tr("Channel updated");
        case SERVERLOGEVENT_CHANNEL_REMOVED :
            return tr("Channel removed");
        case SERVERLOGEVENT_FILE_UPLOADED :
            return tr("File uploaded");
        case SERVERLOGEVENT_FILE_DOWNLOADED :
            return tr("File downloaded");
        case SERVERLOGEVENT_FILE_DELETED :
            return tr("File deleted");
        case SERVERLOGEVENT_SERVER_UPDATED :
            return tr("Server updated");
        case SERVERLOGEVENT_SERVER_SAVECONFIG :
            return tr("Server configuration saved");
        case SERVERLOGEVENT_NONE :
            break;
        }
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    case Qt::AccessibleTextRole :
        return QString("%1: %2").arg(data(index, Qt::DisplayRole).toString()).arg((m_serverlogselected & m_serverlogevents[index.row()])? tr("Enabled") : tr("Disabled"));
#else
    case Qt::CheckStateRole :
        return (m_serverlogselected & m_serverlogevents[index.row()])? Qt::Checked : Qt::Unchecked;
#endif
    }
    return QVariant();
}

Qt::ItemFlags ServerLogEventsModel::flags(const QModelIndex &index) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable |  Qt::ItemIsEditable;
}

QModelIndex ServerLogEventsModel::index(int row, int column, const QModelIndex& parent /*= QModelIndex()*/) const
{
    return createIndex(row, column, m_serverlogevents[row]);
}

QModelIndex ServerLogEventsModel::parent(const QModelIndex& index) const
{
    return QModelIndex();
}

int ServerLogEventsModel::rowCount(const QModelIndex& parent /*= QModelIndex()*/) const
{
    return int(m_serverlogevents.size());
}

void ServerLogEventsModel::setServerLogEvents(ServerLogEvents serverlogactive)
{
    this->beginResetModel();
    m_serverlogselected = serverlogactive;
    this->endResetModel();
}

ServerLogEvents ServerLogEventsModel::getServerLogEvents()
{
    return m_serverlogselected;
}

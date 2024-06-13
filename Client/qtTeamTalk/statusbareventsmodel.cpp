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

#include "statusbareventsmodel.h"

#include <QKeyEvent>

enum
{
    COLUMN_NAME = 0,
    COLUMN_MESSAGE = 1,
    COLUMN_COUNT,
};

StatusBarEventsModel::StatusBarEventsModel(QObject* parent)
    : QAbstractTableModel(parent)
{
    m_statusbarevents.push_back(STATUSBAR_USER_LOGGEDIN);
    m_statusbarevents.push_back(STATUSBAR_USER_LOGGEDOUT);
    m_statusbarevents.push_back(STATUSBAR_USER_JOINED);
    m_statusbarevents.push_back(STATUSBAR_USER_LEFT);
    m_statusbarevents.push_back(STATUSBAR_USER_JOINED_SAME);
    m_statusbarevents.push_back(STATUSBAR_USER_LEFT_SAME);

    m_statusbarevents.push_back(STATUSBAR_SUBSCRIPTIONS_TEXTMSG_PRIVATE);
    m_statusbarevents.push_back(STATUSBAR_SUBSCRIPTIONS_TEXTMSG_CHANNEL);
    m_statusbarevents.push_back(STATUSBAR_SUBSCRIPTIONS_TEXTMSG_BROADCAST);
    m_statusbarevents.push_back(STATUSBAR_SUBSCRIPTIONS_VOICE);
    m_statusbarevents.push_back(STATUSBAR_SUBSCRIPTIONS_VIDEO);
    m_statusbarevents.push_back(STATUSBAR_SUBSCRIPTIONS_DESKTOP);
    m_statusbarevents.push_back(STATUSBAR_SUBSCRIPTIONS_DESKTOPINPUT);
    m_statusbarevents.push_back(STATUSBAR_SUBSCRIPTIONS_MEDIAFILE);

    m_statusbarevents.push_back(STATUSBAR_SUBSCRIPTIONS_INTERCEPT_TEXTMSG_PRIVATE);
    m_statusbarevents.push_back(STATUSBAR_SUBSCRIPTIONS_INTERCEPT_TEXTMSG_CHANNEL);
    m_statusbarevents.push_back(STATUSBAR_SUBSCRIPTIONS_INTERCEPT_VOICE);
    m_statusbarevents.push_back(STATUSBAR_SUBSCRIPTIONS_INTERCEPT_VIDEO);
    m_statusbarevents.push_back(STATUSBAR_SUBSCRIPTIONS_INTERCEPT_DESKTOP);
    m_statusbarevents.push_back(STATUSBAR_SUBSCRIPTIONS_INTERCEPT_MEDIAFILE);

    m_statusbarevents.push_back(STATUSBAR_CLASSROOM_CHANMSG_TX);
    m_statusbarevents.push_back(STATUSBAR_CLASSROOM_VOICE_TX);
    m_statusbarevents.push_back(STATUSBAR_CLASSROOM_VIDEO_TX);
    m_statusbarevents.push_back(STATUSBAR_CLASSROOM_DESKTOP_TX);
    m_statusbarevents.push_back(STATUSBAR_CLASSROOM_MEDIAFILE_TX);

    m_statusbarevents.push_back(STATUSBAR_FILE_ADD);
    m_statusbarevents.push_back(STATUSBAR_FILE_REMOVE);

    m_statusbarevents.push_back(STATUSBAR_SAVE_SERVER_CONFIG);

    m_statusbarevents.push_back(STATUSBAR_START_RECORD);

    m_statusbarevents.push_back(STATUSBAR_TRANSMISSION_BLOCKED);
}

QVariant StatusBarEventsModel::headerData ( int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/ ) const
{
    switch(role)
    {
    case Qt::DisplayRole :
        if(orientation == Qt::Horizontal)
        {
            switch(section)
            {
            case COLUMN_NAME: return tr("Event");
            case COLUMN_MESSAGE : return tr("Message");
            }
        }
        break;
    case Qt::TextAlignmentRole :
        return Qt::AlignLeft;
    }
    return QVariant();
}

int StatusBarEventsModel::columnCount ( const QModelIndex & /*parent*/ /*= QModelIndex() */) const
{
    return COLUMN_COUNT;
}

QVariant StatusBarEventsModel::data ( const QModelIndex & index, int role /*= Qt::DisplayRole*/ ) const
{
    switch(role)
    {
    case Qt::DisplayRole :
        switch(index.column())
        {
        case COLUMN_NAME :
            switch(m_statusbarevents[index.row()])
            {
            case STATUSBAR_USER_LOGGEDIN :
                return tr("User logged in");
            case STATUSBAR_USER_LOGGEDOUT :
                return tr("User logged out");
            case STATUSBAR_USER_JOINED :
                return tr("User joined channel");
            case STATUSBAR_USER_LEFT :
                return tr("User left channel");
            case STATUSBAR_USER_JOINED_SAME :
                return tr("User joined current channel");
            case STATUSBAR_USER_LEFT_SAME :
                return tr("User left current channel");
            case STATUSBAR_SUBSCRIPTIONS_TEXTMSG_PRIVATE :
                return tr("Subscription private text message changed");
            case STATUSBAR_SUBSCRIPTIONS_TEXTMSG_CHANNEL :
                return tr("Subscription channel text message changed");
            case STATUSBAR_SUBSCRIPTIONS_TEXTMSG_BROADCAST :
                return tr("Subscription broadcast text message changed");
            case STATUSBAR_SUBSCRIPTIONS_VOICE :
                return tr("Subscription voice stream changed");
            case STATUSBAR_SUBSCRIPTIONS_VIDEO :
                return tr("Subscription webcam stream changed");
            case STATUSBAR_SUBSCRIPTIONS_DESKTOP :
                return tr("Subscription shared desktop stream changed");
            case STATUSBAR_SUBSCRIPTIONS_DESKTOPINPUT :
                return tr("Subscription desktop access changed");
            case STATUSBAR_SUBSCRIPTIONS_MEDIAFILE :
                return tr("Subscription media file stream changed");
            case STATUSBAR_SUBSCRIPTIONS_INTERCEPT_TEXTMSG_PRIVATE :
                return tr("Subscription intercept private text message changed");
            case STATUSBAR_SUBSCRIPTIONS_INTERCEPT_TEXTMSG_CHANNEL :
                return tr("Subscription intercept channel text message changed");
            case STATUSBAR_SUBSCRIPTIONS_INTERCEPT_VOICE :
                return tr("Subscription intercept voice stream changed");
            case STATUSBAR_SUBSCRIPTIONS_INTERCEPT_VIDEO :
                return tr("Subscription intercept webcam stream changed");
            case STATUSBAR_SUBSCRIPTIONS_INTERCEPT_DESKTOP :
                return tr("Subscription intercept desktop stream changed");
            case STATUSBAR_SUBSCRIPTIONS_INTERCEPT_MEDIAFILE :
                return tr("Subscription intercept media file stream changed");
            case STATUSBAR_CLASSROOM_CHANMSG_TX :
                return tr("Classroom allow channel messages transmission changed");
            case STATUSBAR_CLASSROOM_VOICE_TX :
                return tr("Classroom allow voice transmission changed");
            case STATUSBAR_CLASSROOM_VIDEO_TX :
                return tr("Classroom allow webcam transmission changed");
            case STATUSBAR_CLASSROOM_DESKTOP_TX :
                return tr("Classroom allow desktop transmission changed");
            case STATUSBAR_CLASSROOM_MEDIAFILE_TX :
                return tr("Classroom allow media file transmission changed");
            case STATUSBAR_FILE_ADD :
                return tr("File added");
            case STATUSBAR_FILE_REMOVE :
                return tr("File removed");
            case STATUSBAR_SAVE_SERVER_CONFIG :
                return tr("Server configuration saved");
            case STATUSBAR_START_RECORD :
                return tr("Recording started");
            case STATUSBAR_TRANSMISSION_BLOCKED :
                return tr("Transmission blocked by channel operator");
            case STATUSBAR_NONE :
            case STATUSBAR_NEXT_UNUSED :
            case STATUSBAR_BYPASS :
                break;
            }
        case COLUMN_MESSAGE :
        {
            auto eventMap = UtilUI::eventToSettingMap();
            if (eventMap.contains(m_statusbarevents[index.row()]))
            {
                QString paramKey = eventMap[m_statusbarevents[index.row()]].settingKey;
                return UtilUI::getRawStatusBarMessage(paramKey);
            }
            return QVariant();
        }
        }
        break;
    case Qt::AccessibleTextRole :
        switch (index.column())
        {
        case COLUMN_NAME :
            QString result = data(index, Qt::DisplayRole).toString();
            QString msg = data(createIndex(index.row(), COLUMN_MESSAGE), Qt::DisplayRole).toString();
            if (msg.size() > 0)
                result += " - " + msg;
#if QT_VERSION < QT_VERSION_CHECK(6,0,0) || defined(Q_OS_MAC)
            QString state = (m_statusbarselected & m_statusbarevents[index.row()])? tr("Enabled") : tr("Disabled");
            result += ": " + state;
#endif
            return result;
        }
        break;
    case Qt::CheckStateRole :
        switch (index.column())
        {
        case COLUMN_NAME :
            return (m_statusbarselected & m_statusbarevents[index.row()])? Qt::Checked : Qt::Unchecked;
        }
        break;
    }
    return QVariant();
}

Qt::ItemFlags StatusBarEventsModel::flags(const QModelIndex &index) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable |  Qt::ItemIsEditable;
}

QModelIndex StatusBarEventsModel::index ( int row, int column, const QModelIndex & /*parent*/ /*= QModelIndex()*/ ) const
{
    return createIndex(row, column, m_statusbarevents[row]);
}

QModelIndex StatusBarEventsModel::parent ( const QModelIndex & /*index*/ ) const
{
    return QModelIndex();
}

int StatusBarEventsModel::rowCount ( const QModelIndex & /*parent*/ /*= QModelIndex()*/ ) const
{
    return int(m_statusbarevents.size());
}

void StatusBarEventsModel::setStatusBarEvents(StatusBarEvents statusbaractive)
{
    this->beginResetModel();
    m_statusbarselected = statusbaractive;
    this->endResetModel();
}

StatusBarEvents StatusBarEventsModel::getStatusBarEvents()
{
    return m_statusbarselected;
}

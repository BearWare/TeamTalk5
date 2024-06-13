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

#include "ttseventsmodel.h"
#include "settings.h"

#include <QKeyEvent>
#include <QInputDialog>

extern QSettings* ttSettings;

enum
{
    COLUMN_NAME = 0,
    COLUMN_MESSAGE = 1,
    COLUMN_COUNT,
};

TTSEventsModel::TTSEventsModel(QObject* parent)
    : QAbstractTableModel(parent)
{
    m_ttsevents.push_back(TTS_USER_LOGGEDIN);
    m_ttsevents.push_back(TTS_USER_LOGGEDOUT);
    m_ttsevents.push_back(TTS_USER_JOINED);
    m_ttsevents.push_back(TTS_USER_LEFT);
    m_ttsevents.push_back(TTS_USER_JOINED_SAME);
    m_ttsevents.push_back(TTS_USER_LEFT_SAME);
    m_ttsevents.push_back(TTS_USER_TEXTMSG_PRIVATE);
    m_ttsevents.push_back(TTS_USER_TEXTMSG_PRIVATE_SEND);
    m_ttsevents.push_back(TTS_USER_TEXTMSG_PRIVATE_TYPING);
    m_ttsevents.push_back(TTS_USER_TEXTMSG_PRIVATE_TYPING_GLOBAL);

    m_ttsevents.push_back(TTS_USER_TEXTMSG_CHANNEL);
    m_ttsevents.push_back(TTS_USER_TEXTMSG_CHANNEL_SEND);
    m_ttsevents.push_back(TTS_USER_TEXTMSG_BROADCAST);
    m_ttsevents.push_back(TTS_USER_TEXTMSG_BROADCAST_SEND);
    m_ttsevents.push_back(TTS_USER_QUESTIONMODE);

    m_ttsevents.push_back(TTS_SUBSCRIPTIONS_TEXTMSG_PRIVATE);
    m_ttsevents.push_back(TTS_SUBSCRIPTIONS_TEXTMSG_CHANNEL);
    m_ttsevents.push_back(TTS_SUBSCRIPTIONS_TEXTMSG_BROADCAST);
    m_ttsevents.push_back(TTS_SUBSCRIPTIONS_VOICE);
    m_ttsevents.push_back(TTS_SUBSCRIPTIONS_VIDEO);
    m_ttsevents.push_back(TTS_SUBSCRIPTIONS_DESKTOP);
    m_ttsevents.push_back(TTS_SUBSCRIPTIONS_DESKTOPINPUT);
    m_ttsevents.push_back(TTS_SUBSCRIPTIONS_MEDIAFILE);

    m_ttsevents.push_back(TTS_SUBSCRIPTIONS_INTERCEPT_TEXTMSG_PRIVATE);
    m_ttsevents.push_back(TTS_SUBSCRIPTIONS_INTERCEPT_TEXTMSG_CHANNEL);
    m_ttsevents.push_back(TTS_SUBSCRIPTIONS_INTERCEPT_VOICE);
    m_ttsevents.push_back(TTS_SUBSCRIPTIONS_INTERCEPT_VIDEO);
    m_ttsevents.push_back(TTS_SUBSCRIPTIONS_INTERCEPT_DESKTOP);
    m_ttsevents.push_back(TTS_SUBSCRIPTIONS_INTERCEPT_MEDIAFILE);

    m_ttsevents.push_back(TTS_CLASSROOM_CHANMSG_TX);
    m_ttsevents.push_back(TTS_CLASSROOM_VOICE_TX);
    m_ttsevents.push_back(TTS_CLASSROOM_VIDEO_TX);
    m_ttsevents.push_back(TTS_CLASSROOM_DESKTOP_TX);
    m_ttsevents.push_back(TTS_CLASSROOM_MEDIAFILE_TX);

    m_ttsevents.push_back(TTS_FILE_ADD);
    m_ttsevents.push_back(TTS_FILE_REMOVE);

    m_ttsevents.push_back(TTS_MENU_ACTIONS);
    m_ttsevents.push_back(TTS_TOGGLE_VOICETRANSMISSION);
    m_ttsevents.push_back(TTS_TOGGLE_VIDEOTRANSMISSION);
    m_ttsevents.push_back(TTS_TOGGLE_DESKTOPTRANSMISSION);

    m_ttsevents.push_back(TTS_SERVER_CONNECTIVITY);

}

QVariant TTSEventsModel::headerData ( int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/ ) const
{
    switch(role)
    {
    case Qt::DisplayRole :
        if(orientation == Qt::Horizontal)
        {
            switch(section)
            {
            case COLUMN_NAME : return tr("Event");
            case COLUMN_MESSAGE : return tr("Message");
            }
        }
        break;
    case Qt::TextAlignmentRole :
        return Qt::AlignLeft;
    }
    return QVariant();
}

int TTSEventsModel::columnCount ( const QModelIndex & /*parent*/ /*= QModelIndex() */) const
{
    return COLUMN_COUNT;
}

QVariant TTSEventsModel::data ( const QModelIndex & index, int role /*= Qt::DisplayRole*/ ) const
{
    switch(role)
    {
    case Qt::DisplayRole :
        switch (index.column())
        {
        case COLUMN_NAME :
            switch(m_ttsevents[index.row()])
            {
            case TTS_USER_LOGGEDIN :
                return tr("User logged in");
            case TTS_USER_LOGGEDOUT :
                return tr("User logged out");
            case TTS_USER_JOINED :
                return tr("User joined channel");
            case TTS_USER_LEFT :
                return tr("User left channel");
            case TTS_USER_JOINED_SAME :
                return tr("User joined current channel");
            case TTS_USER_LEFT_SAME :
                return tr("User left current channel");
            case TTS_USER_TEXTMSG_PRIVATE :
                return tr("Private message received");
            case TTS_USER_TEXTMSG_PRIVATE_SEND :
                return tr("Private message sent");
            case TTS_USER_TEXTMSG_PRIVATE_TYPING :
                return tr("User is typing a private message in focused window");
            case TTS_USER_TEXTMSG_PRIVATE_TYPING_GLOBAL :
                return tr("User is typing a private message");
            case TTS_USER_TEXTMSG_CHANNEL :
                return tr("Channel message received");
            case TTS_USER_TEXTMSG_CHANNEL_SEND :
                return tr("Channel message sent");
            case TTS_USER_TEXTMSG_BROADCAST :
                return tr("Broadcast message received");
            case TTS_USER_TEXTMSG_BROADCAST_SEND :
                return tr("Broadcast message sent");
            case TTS_USER_QUESTIONMODE :
                return tr("User enabled question mode");
            case TTS_SUBSCRIPTIONS_TEXTMSG_PRIVATE :
                return tr("Subscription private text message changed");
            case TTS_SUBSCRIPTIONS_TEXTMSG_CHANNEL :
                return tr("Subscription channel text message changed");
            case TTS_SUBSCRIPTIONS_TEXTMSG_BROADCAST :
                return tr("Subscription broadcast text message changed");
            case TTS_SUBSCRIPTIONS_VOICE :
                return tr("Subscription voice stream changed");
            case TTS_SUBSCRIPTIONS_VIDEO :
                return tr("Subscription webcam stream changed");
            case TTS_SUBSCRIPTIONS_DESKTOP :
                return tr("Subscription shared desktop stream changed");
            case TTS_SUBSCRIPTIONS_DESKTOPINPUT :
                return tr("Subscription desktop access changed");
            case TTS_SUBSCRIPTIONS_MEDIAFILE :
                return tr("Subscription media file stream changed");
            case TTS_SUBSCRIPTIONS_INTERCEPT_TEXTMSG_PRIVATE :
                return tr("Subscription intercept private text message changed");
            case TTS_SUBSCRIPTIONS_INTERCEPT_TEXTMSG_CHANNEL :
                return tr("Subscription intercept channel text message changed");
            case TTS_SUBSCRIPTIONS_INTERCEPT_VOICE :
                return tr("Subscription intercept voice stream changed");
            case TTS_SUBSCRIPTIONS_INTERCEPT_VIDEO :
                return tr("Subscription intercept webcam stream changed");
            case TTS_SUBSCRIPTIONS_INTERCEPT_DESKTOP :
                return tr("Subscription intercept desktop stream changed");
            case TTS_SUBSCRIPTIONS_INTERCEPT_MEDIAFILE :
                return tr("Subscription intercept media file stream changed");
            case TTS_CLASSROOM_CHANMSG_TX :
                return tr("Classroom allow channel messages transmission changed");
            case TTS_CLASSROOM_VOICE_TX :
                return tr("Classroom allow voice transmission changed");
            case TTS_CLASSROOM_VIDEO_TX :
                return tr("Classroom allow webcam transmission changed");
            case TTS_CLASSROOM_DESKTOP_TX :
                return tr("Classroom allow desktop transmission changed");
            case TTS_CLASSROOM_MEDIAFILE_TX :
                return tr("Classroom allow media file transmission changed");
            case TTS_FILE_ADD :
                return tr("File added");
            case TTS_FILE_REMOVE :
                return tr("File removed");
            case TTS_MENU_ACTIONS :
                return tr("Menu actions");
            case TTS_TOGGLE_VOICETRANSMISSION :
                return tr("Voice transmission mode toggled");
            case TTS_TOGGLE_VIDEOTRANSMISSION :
                return tr("Video transmission toggled");
            case TTS_TOGGLE_DESKTOPTRANSMISSION :
                return tr("Desktop sharing toggled");
            case TTS_SERVER_CONNECTIVITY :
                return tr("Server connectivity");
            case TTS_NEXT_UNUSED :
            case TTS_NONE :
                break;
            }
        case COLUMN_MESSAGE :
        {
            auto eventMap = UtilTTS::eventToSettingMap();
            if (eventMap.contains(m_ttsevents[index.row()]))
            {
                QString paramKey = eventMap[m_ttsevents[index.row()]].settingKey;
                return UtilTTS::getRawTTSMessage(paramKey);
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
            QString state = (m_ttsselected & m_ttsevents[index.row()])? tr("Enabled") : tr("Disabled");
            result += ": " + state;
#endif
            return result;
        }
        break;
    case Qt::CheckStateRole :
        switch (index.column())
        {
        case COLUMN_NAME :
            return (m_ttsselected & m_ttsevents[index.row()])? Qt::Checked : Qt::Unchecked;
        }
        break;
    }
    return QVariant();
}

Qt::ItemFlags TTSEventsModel::flags(const QModelIndex &index) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable |  Qt::ItemIsEditable;
}

QModelIndex TTSEventsModel::index ( int row, int column, const QModelIndex & /*parent*/ /*= QModelIndex()*/ ) const
{
    return createIndex(row, column, m_ttsevents[row]);
}

QModelIndex TTSEventsModel::parent ( const QModelIndex & /*index*/ ) const
{
    return QModelIndex();
}

int TTSEventsModel::rowCount ( const QModelIndex & /*parent*/ /*= QModelIndex()*/ ) const
{
    return int(m_ttsevents.size());
}

void TTSEventsModel::setTTSEvents(TTSEvents ttsactive)
{
    this->beginResetModel();
    m_ttsselected = ttsactive;
    this->endResetModel();
}

TTSEvents TTSEventsModel::getTTSEvents()
{
    return m_ttsselected;
}

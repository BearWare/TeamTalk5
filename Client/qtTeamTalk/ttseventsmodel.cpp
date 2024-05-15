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
    : QAbstractItemModel(parent)
{
    m_ttsevents.push_back(TTS_USER_LOGGEDIN);
    m_ttsevents.push_back(TTS_USER_LOGGEDOUT);
    m_ttsevents.push_back(TTS_USER_JOINED);
    m_ttsevents.push_back(TTS_USER_LEFT);
    m_ttsevents.push_back(TTS_USER_JOINED_SAME);
    m_ttsevents.push_back(TTS_USER_LEFT_SAME);
    m_ttsevents.push_back(TTS_USER_QUESTIONMODE);
    m_ttsevents.push_back(TTS_USER_TEXTMSG_PRIVATE);
    m_ttsevents.push_back(TTS_USER_TEXTMSG_PRIVATE_SEND);
    m_ttsevents.push_back(TTS_USER_TEXTMSG_PRIVATE_TYPING);
    m_ttsevents.push_back(TTS_USER_TEXTMSG_PRIVATE_TYPING_GLOBAL);

    m_ttsevents.push_back(TTS_USER_TEXTMSG_CHANNEL);
    m_ttsevents.push_back(TTS_USER_TEXTMSG_CHANNEL_SEND);
    m_ttsevents.push_back(TTS_USER_TEXTMSG_BROADCAST);
    m_ttsevents.push_back(TTS_USER_TEXTMSG_BROADCAST_SEND);

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
    QString displayName;
    switch(role)
    {
    case Qt::DisplayRole :
        switch (index.column())
        {
        case COLUMN_NAME :
            switch(m_ttsevents[index.row()])
            {
            case TTS_USER_LOGGEDIN :
                displayName = tr("User logged in");
                break;
            case TTS_USER_LOGGEDOUT :
                displayName = tr("User logged out");
                break;
            case TTS_USER_JOINED :
                displayName = tr("User joined channel");
                break;
            case TTS_USER_LEFT :
                displayName = tr("User left channel");
                break;
            case TTS_USER_JOINED_SAME :
                displayName = tr("User join current channel");
                break;
            case TTS_USER_LEFT_SAME :
                displayName = tr("User left current channel");
                break;
            case TTS_USER_TEXTMSG_PRIVATE :
                displayName = tr("Received private message");
                break;
            case TTS_USER_TEXTMSG_PRIVATE_SEND :
                displayName = tr("Sent private message");
                break;
            case TTS_USER_TEXTMSG_PRIVATE_TYPING :
                displayName = tr("User is typing a private text message in focused window");
                break;
            case TTS_USER_TEXTMSG_PRIVATE_TYPING_GLOBAL :
                displayName = tr("User is typing a private text message");
                break;
            case TTS_USER_QUESTIONMODE :
                displayName = tr("User enabled question mode");
                break;
            case TTS_USER_TEXTMSG_CHANNEL :
                displayName = tr("Received channel message");
                break;
            case TTS_USER_TEXTMSG_CHANNEL_SEND :
                displayName = tr("Sent channel message");
                break;
            case TTS_USER_TEXTMSG_BROADCAST :
                displayName = tr("Received broadcast message");
                break;
            case TTS_USER_TEXTMSG_BROADCAST_SEND :
                displayName = tr("Sent broadcast message");
                break;
            case TTS_SUBSCRIPTIONS_TEXTMSG_PRIVATE :
                displayName = tr("Subscription private text message changed");
                break;
            case TTS_SUBSCRIPTIONS_TEXTMSG_CHANNEL :
                displayName = tr("Subscription channel text message changed");
                break;
            case TTS_SUBSCRIPTIONS_TEXTMSG_BROADCAST :
                displayName = tr("Subscription broadcast text message changed");
                break;
            case TTS_SUBSCRIPTIONS_VOICE :
                displayName = tr("Subscription voice stream changed");
                break;
            case TTS_SUBSCRIPTIONS_VIDEO :
                displayName = tr("Subscription webcam stream changed");
                break;
            case TTS_SUBSCRIPTIONS_DESKTOP :
                displayName = tr("Subscription shared desktop stream changed");
                break;
            case TTS_SUBSCRIPTIONS_DESKTOPINPUT :
                displayName = tr("Subscription desktop access changed");
                break;
            case TTS_SUBSCRIPTIONS_MEDIAFILE :
                displayName = tr("Subscription media file stream changed");
                break;
            case TTS_SUBSCRIPTIONS_INTERCEPT_TEXTMSG_PRIVATE :
                displayName = tr("Subscription intercept private text message changed");
                break;
            case TTS_SUBSCRIPTIONS_INTERCEPT_TEXTMSG_CHANNEL :
                displayName = tr("Subscription intercept channel text message changed");
                break;
            case TTS_SUBSCRIPTIONS_INTERCEPT_VOICE :
                displayName = tr("Subscription intercept voice stream changed");
                break;
            case TTS_SUBSCRIPTIONS_INTERCEPT_VIDEO :
                displayName = tr("Subscription intercept webcam stream changed");
                break;
            case TTS_SUBSCRIPTIONS_INTERCEPT_DESKTOP :
                displayName = tr("Subscription intercept desktop stream changed");
                break;
            case TTS_SUBSCRIPTIONS_INTERCEPT_MEDIAFILE :
                displayName = tr("Subscription intercept media file stream changed");
                break;
            case TTS_CLASSROOM_CHANMSG_TX :
                displayName = tr("Classroom allow channel messages transmission changed");
                break;
            case TTS_CLASSROOM_VOICE_TX :
                displayName = tr("Classroom allow voice transmission changed");
                break;
            case TTS_CLASSROOM_VIDEO_TX :
                displayName = tr("Classroom allow webcam transmission changed");
                break;
            case TTS_CLASSROOM_DESKTOP_TX :
                displayName = tr("Classroom allow desktop transmission changed");
                break;
            case TTS_CLASSROOM_MEDIAFILE_TX :
                displayName = tr("Classroom allow media file transmission changed");
                break;
            case TTS_FILE_ADD :
                displayName = tr("File added");
                break;
            case TTS_FILE_REMOVE :
                displayName = tr("File removed");
                break;
            case TTS_MENU_ACTIONS :
                displayName = tr("Menu actions");
                break;
            case TTS_TOGGLE_VOICETRANSMISSION :
                displayName = tr("Voice transmission mode toggled");
                break;
            case TTS_TOGGLE_VIDEOTRANSMISSION :
                displayName = tr("Video transmission toggled");
                break;
            case TTS_TOGGLE_DESKTOPTRANSMISSION :
                displayName = tr("Desktop sharing toggled");
                break;
            case TTS_SERVER_CONNECTIVITY :
                displayName = tr("Server connectivity");
                break;
            case TTS_NEXT_UNUSED :
            case TTS_NONE :
                break;
            }
            if (UtilTTS::eventToSettingMap().contains(m_ttsevents[index.row()]))
            {
                displayName += tr(" (Customizable)");
            }

            return displayName;
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
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
            result += ": " + (m_ttsselected & m_ttsevents[index.row()])? tr("Enabled") : tr("Disabled"));
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

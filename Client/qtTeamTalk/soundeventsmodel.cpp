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

#include "soundeventsmodel.h"
#include "settings.h"

#include <QKeyEvent>
#include <QInputDialog>

extern QSettings* ttSettings;

enum
{
    COLUMN_NAME = 0,
    COLUMN_FILENAME = 1,
    COLUMN_COUNT,
};

SoundEventsModel::SoundEventsModel(QObject* parent)
    : QAbstractTableModel(parent)
{
    m_soundevents.push_back(SOUNDEVENT_USERLOGGEDIN);
    m_soundevents.push_back(SOUNDEVENT_USERLOGGEDOUT);
    m_soundevents.push_back(SOUNDEVENT_NEWUSER);
    m_soundevents.push_back(SOUNDEVENT_REMOVEUSER);
    m_soundevents.push_back(SOUNDEVENT_SERVERLOST);
    m_soundevents.push_back(SOUNDEVENT_USERMSG);
    m_soundevents.push_back(SOUNDEVENT_USERMSGSENT);
    m_soundevents.push_back(SOUNDEVENT_TYPING);
    m_soundevents.push_back(SOUNDEVENT_CHANNELMSG);
    m_soundevents.push_back(SOUNDEVENT_CHANNELMSGSENT);
    m_soundevents.push_back(SOUNDEVENT_BROADCASTMSG);
    m_soundevents.push_back(SOUNDEVENT_HOTKEY);
    m_soundevents.push_back(SOUNDEVENT_SILENCE);
    m_soundevents.push_back(SOUNDEVENT_NEWVIDEO);
    m_soundevents.push_back(SOUNDEVENT_NEWDESKTOP);
    m_soundevents.push_back(SOUNDEVENT_DESKTOPACCESS);
    m_soundevents.push_back(SOUNDEVENT_FILESUPD);
    m_soundevents.push_back(SOUNDEVENT_FILETXDONE);
    m_soundevents.push_back(SOUNDEVENT_QUESTIONMODE);
    m_soundevents.push_back(SOUNDEVENT_VOICEACTON);
    m_soundevents.push_back(SOUNDEVENT_VOICEACTOFF);
    m_soundevents.push_back(SOUNDEVENT_VOICEACTMEON);
    m_soundevents.push_back(SOUNDEVENT_VOICEACTMEOFF);
    m_soundevents.push_back(SOUNDEVENT_VOICEACTTRIG);
    m_soundevents.push_back(SOUNDEVENT_VOICEACTSTOP);
    m_soundevents.push_back(SOUNDEVENT_MUTEALLON);
    m_soundevents.push_back(SOUNDEVENT_MUTEALLOFF);
    m_soundevents.push_back(SOUNDEVENT_TRANSMITQUEUE_HEAD);
    m_soundevents.push_back(SOUNDEVENT_TRANSMITQUEUE_STOP);
    m_soundevents.push_back(SOUNDEVENT_INTERCEPT);
    m_soundevents.push_back(SOUNDEVENT_INTERCEPTEND);
}

QVariant SoundEventsModel::headerData ( int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/ ) const
{
    switch(role)
    {
    case Qt::DisplayRole :
        if(orientation == Qt::Horizontal)
        {
            switch(section)
            {
            case COLUMN_NAME : return tr("Event");
            case COLUMN_FILENAME : return tr("File");
            }
        }
        break;
    case Qt::TextAlignmentRole :
        return Qt::AlignLeft;
    }
    return QVariant();
}

int SoundEventsModel::columnCount ( const QModelIndex & /*parent*/ /*= QModelIndex() */) const
{
    return COLUMN_COUNT;
}

QVariant SoundEventsModel::data ( const QModelIndex & index, int role /*= Qt::DisplayRole*/ ) const
{
    switch(role)
    {
    case Qt::DisplayRole :
        switch (index.column())
        {
        case COLUMN_NAME :
            switch(m_soundevents[index.row()])
            {
            case SOUNDEVENT_USERLOGGEDIN :
                return tr("User logged in");
            case SOUNDEVENT_USERLOGGEDOUT :
                return tr("User logged out");
            case SOUNDEVENT_NEWUSER :
                return tr("User joined channel");
            case SOUNDEVENT_REMOVEUSER :
                return tr("User left channel");
            case SOUNDEVENT_SERVERLOST :
                return tr("Connection to server lost");
            case SOUNDEVENT_USERMSG :
                return tr("Private message received");
            case SOUNDEVENT_USERMSGSENT :
                return tr("Private message sent");
            case SOUNDEVENT_TYPING :
                return tr("User is typing a private message in focused window");
            case SOUNDEVENT_CHANNELMSG :
                return tr("Channel message received");
            case SOUNDEVENT_CHANNELMSGSENT :
                return tr("Channel message sent");
            case SOUNDEVENT_BROADCASTMSG :
                return tr("Broadcast message received");
            case SOUNDEVENT_HOTKEY :
                return tr("Hotkey pressed");
            case SOUNDEVENT_SILENCE :
                return tr("Channel silent");
            case SOUNDEVENT_NEWVIDEO :
                return tr("New video session");
            case SOUNDEVENT_NEWDESKTOP :
                return tr("New desktop session");
            case SOUNDEVENT_DESKTOPACCESS :
                return tr("Desktop access request");
            case SOUNDEVENT_FILESUPD :
                return tr("Files updated");
            case SOUNDEVENT_FILETXDONE :
                return tr("File transfer completed");
            case SOUNDEVENT_QUESTIONMODE :
                return tr("User enabled question mode");
            case SOUNDEVENT_VOICEACTON :
                return tr("Voice activation enabled");
            case SOUNDEVENT_VOICEACTOFF :
                return tr("Voice activation disabled");
            case SOUNDEVENT_VOICEACTMEON :
                return tr("Voice activation enabled via \"Me\" menu");
            case SOUNDEVENT_VOICEACTMEOFF :
                return tr("Voice activation disabled via \"Me\" menu");
            case SOUNDEVENT_VOICEACTTRIG :
                return tr("Voice activation triggered");
            case SOUNDEVENT_VOICEACTSTOP :
                return tr("Voice activation stopped");
            case SOUNDEVENT_MUTEALLON :
                return tr("Mute master volume");
            case SOUNDEVENT_MUTEALLOFF :
                return tr("Unmute master volume");
            case SOUNDEVENT_TRANSMITQUEUE_HEAD :
                return tr("Transmit ready in \"No interruption\" channel");
            case SOUNDEVENT_TRANSMITQUEUE_STOP :
                return tr("Transmit stopped in \"No interruption\" channel");
            case SOUNDEVENT_INTERCEPT :
                return tr("Interception by another user");
            case SOUNDEVENT_INTERCEPTEND :
                return tr("End of interception by another user");
            case SOUNDEVENT_NEXT_UNUSED :
            case SOUNDEVENT_NONE :
                break;
            }
        case COLUMN_FILENAME :
        {
            auto eventMap = UtilSound::eventToSettingMap();
            if (eventMap.contains(m_soundevents[index.row()]))
            {
                QString paramKey = eventMap[m_soundevents[index.row()]].settingKey;
                return UtilSound::getFile(paramKey);
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
            QString file = data(createIndex(index.row(), COLUMN_FILENAME), Qt::DisplayRole).toString();
            if (file.size() > 0)
                result += " - " + file;
#if QT_VERSION < QT_VERSION_CHECK(6,0,0) || defined(Q_OS_MAC)
            QString state = (m_soundselected & m_soundevents[index.row()])? tr("Enabled") : tr("Disabled");
            result += ": " + state;
#endif
            return result;
        }
        break;
    case Qt::CheckStateRole :
        switch (index.column())
        {
        case COLUMN_NAME :
            return (m_soundselected & m_soundevents[index.row()])? Qt::Checked : Qt::Unchecked;
        }
        break;
    }
    return QVariant();
}

Qt::ItemFlags SoundEventsModel::flags(const QModelIndex &index) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable |  Qt::ItemIsEditable;
}

QModelIndex SoundEventsModel::index ( int row, int column, const QModelIndex & /*parent*/ /*= QModelIndex()*/ ) const
{
    return createIndex(row, column, m_soundevents[row]);
}

QModelIndex SoundEventsModel::parent ( const QModelIndex & /*index*/ ) const
{
    return QModelIndex();
}

int SoundEventsModel::rowCount ( const QModelIndex & /*parent*/ /*= QModelIndex()*/ ) const
{
    return int(m_soundevents.size());
}

void SoundEventsModel::setSoundEvents(SoundEvents soundactive)
{
    this->beginResetModel();
    m_soundselected = soundactive;
    this->endResetModel();
}

SoundEvents SoundEventsModel::getSoundEvents()
{
    return m_soundselected;
}

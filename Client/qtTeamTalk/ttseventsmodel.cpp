/*
 * Copyright (c) 2005-2016, BearWare.dk
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
 * This source code is part of the TeamTalk 5 SDK owned by
 * BearWare.dk. All copyright statements may not be removed
 * or altered from any source distribution. If you use this
 * software in a product, an acknowledgment in the product
 * documentation is required.
 *
 */

#include "ttseventsmodel.h"

#include <QKeyEvent>

enum
{
    COLUMN_NAME = 0,
    COLUMN_CHECK = 1,
    COLUMN_COUNT = 2,
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
    m_ttsevents.push_back(TTS_USER_TEXTMSG_PRIVATE);
    m_ttsevents.push_back(TTS_USER_TEXTMSG_CHANNEL);
    m_ttsevents.push_back(TTS_USER_TEXTMSG_BROADCAST);

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
            case COLUMN_NAME: return tr("Event");
            case COLUMN_CHECK: return tr("Enabled");
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
        if (index.column() == COLUMN_CHECK)
            return (m_ttsselected & m_ttsevents[index.row()])? tr("Enabled") : tr("Disabled");
        Q_ASSERT(index.column() == COLUMN_NAME);
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
            return tr("User join current channel");
        case TTS_USER_LEFT_SAME :
            return tr("User left current channel");
        case TTS_USER_TEXTMSG_PRIVATE :
            return tr("Private message received/sent");
        case TTS_USER_TEXTMSG_CHANNEL :
            return tr("Channel message received/sent");
        case TTS_USER_TEXTMSG_BROADCAST :
            return tr("Received broadcast text message");
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
        case TTS_NONE :
            break;
        }
    case Qt::AccessibleDescriptionRole :
        return (m_ttsselected & m_ttsevents[index.row()])? tr("Enabled") : tr("Disabled");
    }
    return QVariant();
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

TTSEventsTreeView::TTSEventsTreeView(QWidget* parent)
    : QTreeView(parent)
{
}

void TTSEventsTreeView::keyPressEvent(QKeyEvent* e)
{
    if (e->key() == Qt::Key_Space)
    {
        auto i = currentIndex();
        emit doubleClicked(i);
        setCurrentIndex(i);
    }

    QTreeView::keyPressEvent(e);
}

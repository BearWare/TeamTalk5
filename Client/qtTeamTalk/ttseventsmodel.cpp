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

enum
{
    COLUMN_NAME = 0,
    COLUMN_CHECK = 1,
    COLUMN_COUNT,
};

TTSEventsModel::TTSEventsModel(QObject* parent)
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
    m_ttsevents.push_back(TTS_SUBSCRIPTIONS_INTERCEPT_DESKTOPINPUT);
    m_ttsevents.push_back(TTS_SUBSCRIPTIONS_INTERCEPT_MEDIAFILE);

    m_ttsevents.push_back(TTS_CLASSROOM_VOICE_TX);
    m_ttsevents.push_back(TTS_CLASSROOM_VIDEO_TX);
    m_ttsevents.push_back(TTS_CLASSROOM_DESKTOP_TX);
    m_ttsevents.push_back(TTS_CLASSROOM_MEDIAFILE_TX);
}

QVariant TTSEventsModel::headerData ( int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/ ) const
{
    return QVariant();
}

int TTSEventsModel::columnCount ( const QModelIndex & parent /*= QModelIndex() */) const
{
    return COLUMN_COUNT;
}

QVariant TTSEventsModel::data ( const QModelIndex & index, int role /*= Qt::DisplayRole*/ ) const
{
    switch(role)
    {
    case Qt::DisplayRole :
        switch(m_ttsevents[index.row()])
        {
        case TTS_USER_LOGGEDIN :
            if (index.column() == COLUMN_NAME)
                return tr("User logged in");

        case TTS_USER_LOGGEDOUT :
            if (index.column() == COLUMN_NAME)
            return tr("User logged out");
        case TTS_USER_JOINED :
            if (index.column() == COLUMN_NAME)
            return tr("User join channel");
        case TTS_USER_LEFT :
            if (index.column() == COLUMN_NAME)
            return tr("User left channel");
        case TTS_USER_JOINED_SAME :
        case TTS_USER_LEFT_SAME :
        case TTS_USER_TEXTMSG_PRIVATE :
        case TTS_USER_TEXTMSG_CHANNEL :
        case TTS_USER_TEXTMSG_BROADCAST :

        case TTS_SUBSCRIPTIONS_TEXTMSG_PRIVATE :
        case TTS_SUBSCRIPTIONS_TEXTMSG_CHANNEL :
        case TTS_SUBSCRIPTIONS_TEXTMSG_BROADCAST :
        case TTS_SUBSCRIPTIONS_VOICE :
        case TTS_SUBSCRIPTIONS_VIDEO :
        case TTS_SUBSCRIPTIONS_DESKTOP :
        case TTS_SUBSCRIPTIONS_DESKTOPINPUT :
        case TTS_SUBSCRIPTIONS_MEDIAFILE :

        case TTS_SUBSCRIPTIONS_INTERCEPT_TEXTMSG_PRIVATE :
        case TTS_SUBSCRIPTIONS_INTERCEPT_TEXTMSG_CHANNEL :
        case TTS_SUBSCRIPTIONS_INTERCEPT_VOICE :
        case TTS_SUBSCRIPTIONS_INTERCEPT_VIDEO :
        case TTS_SUBSCRIPTIONS_INTERCEPT_DESKTOP :
        case TTS_SUBSCRIPTIONS_INTERCEPT_DESKTOPINPUT :
        case TTS_SUBSCRIPTIONS_INTERCEPT_MEDIAFILE :

        case TTS_CLASSROOM_VOICE_TX :
        case TTS_CLASSROOM_VIDEO_TX :
        case TTS_CLASSROOM_DESKTOP_TX :
        case TTS_CLASSROOM_MEDIAFILE_TX :
            if (index.column() == COLUMN_NAME)
                return ("TODO");
        }
    }
    return QVariant();
}

QModelIndex TTSEventsModel::index ( int row, int column, const QModelIndex & parent /*= QModelIndex()*/ ) const
{
    return createIndex(row, column, m_ttsevents[row]);
}

QModelIndex TTSEventsModel::parent ( const QModelIndex & index ) const
{
    return QModelIndex();
}

int TTSEventsModel::rowCount ( const QModelIndex & parent /*= QModelIndex()*/ ) const
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

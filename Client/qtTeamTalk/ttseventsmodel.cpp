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
    ROOT_USER_INDEX             = 0,
    ROOT_SUBSCRIPTIONS_INDEX    = 1,
    ROOT_CLASSROOM_INDEX        = 2,

    ROOT_COUNT                  = 3
};

TTSEventsModel::TTSEventsModel(QObject* parent)
{
    m_userevents.push_back(TTS_USER_LOGGEDIN);
    m_userevents.push_back(TTS_USER_LOGGEDOUT);
    m_userevents.push_back(TTS_USER_JOINED);
    m_userevents.push_back(TTS_USER_LEFT);
    m_userevents.push_back(TTS_USER_JOINED_SAME);
    m_userevents.push_back(TTS_USER_LEFT_SAME);
    m_userevents.push_back(TTS_USER_TEXTMSG_PRIVATE);
    m_userevents.push_back(TTS_USER_TEXTMSG_CHANNEL);
    m_userevents.push_back(TTS_USER_TEXTMSG_BROADCAST);

    m_subevents.push_back(TTS_SUBSCRIPTIONS_TEXTMSG_PRIVATE);
    m_subevents.push_back(TTS_SUBSCRIPTIONS_TEXTMSG_CHANNEL);
    m_subevents.push_back(TTS_SUBSCRIPTIONS_TEXTMSG_BROADCAST);
    m_subevents.push_back(TTS_SUBSCRIPTIONS_VOICE);
    m_subevents.push_back(TTS_SUBSCRIPTIONS_VIDEO);
    m_subevents.push_back(TTS_SUBSCRIPTIONS_DESKTOP);
    m_subevents.push_back(TTS_SUBSCRIPTIONS_DESKTOPINPUT);
    m_subevents.push_back(TTS_SUBSCRIPTIONS_MEDIAFILE);

    m_subevents.push_back(TTS_SUBSCRIPTIONS_INTERCEPT_TEXTMSG_PRIVATE);
    m_subevents.push_back(TTS_SUBSCRIPTIONS_INTERCEPT_TEXTMSG_CHANNEL);
    m_subevents.push_back(TTS_SUBSCRIPTIONS_INTERCEPT_VOICE);
    m_subevents.push_back(TTS_SUBSCRIPTIONS_INTERCEPT_VIDEO);
    m_subevents.push_back(TTS_SUBSCRIPTIONS_INTERCEPT_DESKTOP);
    m_subevents.push_back(TTS_SUBSCRIPTIONS_INTERCEPT_DESKTOPINPUT);
    m_subevents.push_back(TTS_SUBSCRIPTIONS_INTERCEPT_MEDIAFILE);

    m_clsevents.push_back(TTS_CLASSROOM_VOICE_TX);
    m_clsevents.push_back(TTS_CLASSROOM_VIDEO_TX);
    m_clsevents.push_back(TTS_CLASSROOM_DESKTOP_TX);
    m_clsevents.push_back(TTS_CLASSROOM_MEDIAFILE_TX);
}

QVariant TTSEventsModel::headerData ( int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/ ) const
{
    return QVariant();
}

int TTSEventsModel::columnCount ( const QModelIndex & parent /*= QModelIndex() */) const
{
    return 1;
}

QVariant TTSEventsModel::data ( const QModelIndex & index, int role /*= Qt::DisplayRole*/ ) const
{
    switch(role)
    {
    case Qt::DisplayRole :

        switch(index.internalId())
        {
        case TTS_USER_ALL :
            return tr("User events");
        case TTS_SUBSCRIPTIONS_ALL :
            return tr("Subscription changes");
        case TTS_CLASSROOM_ALL :
            return tr("Classroom changes");
        case TTS_USER_JOINED :
            return tr("User joined channel");
        case TTS_USER_JOINED_SAME :
            return tr("User joined current channel");
        }

        return tr("Hest");
    }
    return QVariant();
}

QModelIndex TTSEventsModel::index ( int row, int column, const QModelIndex & parent /*= QModelIndex()*/ ) const
{
    if(!parent.isValid())
    {
        switch(row)
        {
        case ROOT_USER_INDEX :
            return createIndex(row, column, TTS_USER_ALL);
        case ROOT_SUBSCRIPTIONS_INDEX :
            return createIndex(row, column, TTS_SUBSCRIPTIONS_ALL);
        case ROOT_CLASSROOM_INDEX :
            return createIndex(row, column, TTS_CLASSROOM_ALL);
        }
    }

    switch(parent.internalId())
    {
    case TTS_USER_ALL :
        return createIndex(row, column, m_userevents[row]);
    case TTS_SUBSCRIPTIONS_ALL :
        return createIndex(row, column, m_subevents[row]);
    case TTS_CLASSROOM_ALL :
        return createIndex(row, column, m_clsevents[row]);
    }

    return QModelIndex();
}

QModelIndex TTSEventsModel::parent ( const QModelIndex & index ) const
{
    if(index.internalId() & TTS_USER_ALL)
        return createIndex(0, 0, TTS_USER_ALL);
    if(index.internalId() & TTS_SUBSCRIPTIONS_ALL)
        return createIndex(1, 0, TTS_SUBSCRIPTIONS_ALL);
    if(index.internalId() & TTS_CLASSROOM_ALL)
        return createIndex(2, 0, TTS_CLASSROOM_ALL);
    return QModelIndex();
}

int TTSEventsModel::rowCount ( const QModelIndex & parent /*= QModelIndex()*/ ) const
{
    if(!parent.isValid())
        return ROOT_COUNT;

    switch(parent.internalId())
    {
    case TTS_USER_ALL :
        return m_userevents.size();
    case TTS_SUBSCRIPTIONS_ALL :
        return m_subevents.size();
    case TTS_CLASSROOM_ALL :
        return m_clsevents.size();
    }
    return 0;
}

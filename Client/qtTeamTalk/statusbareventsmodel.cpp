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

#include "statusbareventsmodel.h"

#include <QKeyEvent>

enum
{
    COLUMN_NAME = 0,
    COLUMN_CHECK = 1,
    COLUMN_COUNT = 2,
};

StatusBarEventsModel::StatusBarEventsModel(QObject* parent)
    : QAbstractItemModel(parent)
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
            case COLUMN_CHECK: return tr("Enabled");
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
        if (index.column() == COLUMN_CHECK)
            return (m_statusbarselected & m_statusbarevents[index.row()])? tr("Enabled") : tr("Disabled");
        Q_ASSERT(index.column() == COLUMN_NAME);
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
            return tr("User join current channel");
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
            return tr("Save server configuration");
        case STATUSBAR_START_RECORD :
            return tr("Start a recording");
        case STATUSBAR_TRANSMISSION_BLOCKED :
            return tr("Transmission blocked by channel operator");
        case STATUSBAR_NONE :
        case STATUSBAR_NEXT_UNUSED :
        case STATUSBAR_BYPASS :
            break;
        }
    case Qt::AccessibleTextRole :
        return QString("%1: %2").arg(data(index, Qt::DisplayRole).toString()).arg((m_statusbarselected & m_statusbarevents[index.row()])? tr("Enabled") : tr("Disabled"));
    }
    return QVariant();
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

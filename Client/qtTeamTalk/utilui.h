/*
 * Copyright (c) 2005-2018, BearWare.dk
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
 * This source code is part of the TeamTalk SDK owned by
 * BearWare.dk. Use of this file, or its compiled unit, requires a
 * TeamTalk SDK License Key issued by BearWare.dk.
 *
 * The TeamTalk SDK License Agreement along with its Terms and
 * Conditions are outlined in the file License.txt included with the
 * TeamTalk SDK distribution.
 *
 */

#ifndef UTILUI_H
#define UTILUI_H

#include "common.h"

#include <QSet>
#include <QPainter>
#include <QLayout>
#include <QComboBox>
#include <QDialog>
#include <QAbstractItemView>

enum DoubleClickChannelAction
{
    ACTION_NOTHING          = 0x0,
    ACTION_JOIN             = 0x1,
    ACTION_LEAVE            = 0x2,
    ACTION_JOINLEAVE        = (ACTION_JOIN | ACTION_LEAVE),
};

enum StatusBarEvent : qulonglong
{
    STATUSBAR_NONE                                        = 0x0,
    STATUSBAR_USER_LOGGEDIN                               = qulonglong(1) << 0,
    STATUSBAR_USER_LOGGEDOUT                              = qulonglong(1) << 1,
    STATUSBAR_USER_JOINED                                 = qulonglong(1) << 2,
    STATUSBAR_USER_LEFT                                   = qulonglong(1) << 3,
    STATUSBAR_USER_JOINED_SAME                            = qulonglong(1) << 4,
    STATUSBAR_USER_LEFT_SAME                              = qulonglong(1) << 5,

    STATUSBAR_SUBSCRIPTIONS_TEXTMSG_PRIVATE               = qulonglong(1) << 6,
    STATUSBAR_SUBSCRIPTIONS_TEXTMSG_CHANNEL               = qulonglong(1) << 7,
    STATUSBAR_SUBSCRIPTIONS_TEXTMSG_BROADCAST             = qulonglong(1) << 8,
    STATUSBAR_SUBSCRIPTIONS_VOICE                         = qulonglong(1) << 9,
    STATUSBAR_SUBSCRIPTIONS_VIDEO                         = qulonglong(1) << 10,
    STATUSBAR_SUBSCRIPTIONS_DESKTOP                       = qulonglong(1) << 11,
    STATUSBAR_SUBSCRIPTIONS_DESKTOPINPUT                  = qulonglong(1) << 12,
    STATUSBAR_SUBSCRIPTIONS_MEDIAFILE                     = qulonglong(1) << 13,

    STATUSBAR_SUBSCRIPTIONS_INTERCEPT_TEXTMSG_PRIVATE     = qulonglong(1) << 14,
    STATUSBAR_SUBSCRIPTIONS_INTERCEPT_TEXTMSG_CHANNEL     = qulonglong(1) << 15,
    STATUSBAR_SUBSCRIPTIONS_INTERCEPT_VOICE               = qulonglong(1) << 16,
    STATUSBAR_SUBSCRIPTIONS_INTERCEPT_VIDEO               = qulonglong(1) << 17,
    STATUSBAR_SUBSCRIPTIONS_INTERCEPT_DESKTOP             = qulonglong(1) << 18,
    STATUSBAR_SUBSCRIPTIONS_INTERCEPT_MEDIAFILE           = qulonglong(1) << 19,

    STATUSBAR_CLASSROOM_CHANMSG_TX                        = qulonglong(1) << 20,
    STATUSBAR_CLASSROOM_VOICE_TX                          = qulonglong(1) << 21,
    STATUSBAR_CLASSROOM_VIDEO_TX                          = qulonglong(1) << 22,
    STATUSBAR_CLASSROOM_DESKTOP_TX                        = qulonglong(1) << 23,
    STATUSBAR_CLASSROOM_MEDIAFILE_TX                      = qulonglong(1) << 24,

    STATUSBAR_FILE_ADD                                    = qulonglong(1) << 25,
    STATUSBAR_FILE_REMOVE                                 = qulonglong(1) << 26,

    STATUSBAR_SAVE_SERVER_CONFIG                          = qulonglong(1) << 27,

    STATUSBAR_START_RECORD                                = qulonglong(1) << 28,

    STATUSBAR_TRANSMISSION_BLOCKED                        = qulonglong(1) << 29,

    STATUSBAR_NEXT_UNUSED                                 = qulonglong(1) << 30,

    STATUSBAR_BYPASS                                      = qulonglong(~0),
};

typedef qulonglong StatusBarEvents;

enum ChannelSort
{
    CHANNELSORT_ASCENDING  = 0x1,
    CHANNELSORT_POPULARITY = 0x2
};

enum VideoText
{
    VIDEOTEXT_NONE                   = 0x0000,
    VIDTEXT_POSITION_MASK            = 0x000F,
    VIDTEXT_POSITION_TOPLEFT         = 0x0001,
    VIDTEXT_POSITION_TOPRIGHT        = 0x0002,
    VIDTEXT_POSITION_BOTTOMLEFT      = 0x0003,
    VIDTEXT_POSITION_BOTTOMRIGHT     = 0x0004,

    VIDTEXT_SHOW_MASK                = 0x00F0,
    VIDTEXT_SHOW_NICKNAME            = 0x0010,
    VIDTEXT_SHOW_USERNAME            = 0x0020,
    VIDTEXT_SHOW_STATUSTEXT          = 0x0040,
};

void setVideoTextBox(const QRect& rect, const QColor& bgcolor,
                     const QColor& fgcolor, const QString& text,
                     quint32 text_pos, int w_percent, int h_percent,
                     QPainter& painter);

#if defined(Q_OS_DARWIN)
void setMacResizeMargins(QDialog* dlg, QLayout* layout);
#endif /* Q_OS_DARWIN */

void setCurrentItemData(QComboBox* cbox, const QVariant& itemdata);
QVariant getCurrentItemData(QComboBox* cbox, const QVariant& not_found = QVariant());

QString getBearWareWebLogin(QWidget* parent);

textmessages_t sendTextMessage(const TextMessage& msg, const QString& content);

class RestoreIndex
{
    Q_DISABLE_COPY(RestoreIndex)
    int m_row, m_column;
    QModelIndex m_parent;
    QAbstractItemView* m_view;
public:
    RestoreIndex(QAbstractItemView* view);
    ~RestoreIndex();
};

void saveWindowPosition(const QString& setting, QWidget* widget);
bool restoreWindowPosition(const QString& setting, QWidget* widget);

#endif

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

#ifndef UTILTTS_H
#define UTILTTS_H

#include <QString>

#if defined(ENABLE_TOLK)
#include <Tolk.h>
#endif

enum TextToSpeechEvent : qulonglong
{
    TTS_NONE = 0x0,
    TTS_USER_LOGGEDIN = qulonglong(1) << 0,
    TTS_USER_LOGGEDOUT = qulonglong(1) << 1,
    TTS_USER_JOINED = qulonglong(1) << 2,
    TTS_USER_LEFT = qulonglong(1) << 3,
    TTS_USER_JOINED_SAME = qulonglong(1) << 4,
    TTS_USER_LEFT_SAME = qulonglong(1) << 5,
    TTS_USER_TEXTMSG_PRIVATE = qulonglong(1) << 6,
    TTS_USER_TEXTMSG_PRIVATE_SEND = qulonglong(1) << 7,
    TTS_USER_TEXTMSG_PRIVATE_TYPING = qulonglong(1) << 35,
    TTS_USER_TEXTMSG_PRIVATE_TYPING_GLOBAL = qulonglong(1) << 36,
    TTS_USER_QUESTIONMODE = qulonglong(1) << 37,

    TTS_USER_TEXTMSG_CHANNEL = qulonglong(1) << 8,
    TTS_USER_TEXTMSG_CHANNEL_SEND = qulonglong(1) << 9,
    TTS_USER_TEXTMSG_BROADCAST = qulonglong(1) << 10,
    TTS_USER_TEXTMSG_BROADCAST_SEND = qulonglong(1) << 11,

    TTS_SUBSCRIPTIONS_TEXTMSG_PRIVATE = qulonglong(1) << 12,
    TTS_SUBSCRIPTIONS_TEXTMSG_CHANNEL = qulonglong(1) << 13,
    TTS_SUBSCRIPTIONS_TEXTMSG_BROADCAST = qulonglong(1) << 14,
    TTS_SUBSCRIPTIONS_VOICE = qulonglong(1) << 15,
    TTS_SUBSCRIPTIONS_VIDEO = qulonglong(1) << 16,
    TTS_SUBSCRIPTIONS_DESKTOP = qulonglong(1) << 17,
    TTS_SUBSCRIPTIONS_DESKTOPINPUT = qulonglong(1) << 18,
    TTS_SUBSCRIPTIONS_MEDIAFILE = qulonglong(1) << 19,

    TTS_SUBSCRIPTIONS_INTERCEPT_TEXTMSG_PRIVATE = qulonglong(1) << 20,
    TTS_SUBSCRIPTIONS_INTERCEPT_TEXTMSG_CHANNEL = qulonglong(1) << 21,
    TTS_SUBSCRIPTIONS_INTERCEPT_VOICE = qulonglong(1) << 22,
    TTS_SUBSCRIPTIONS_INTERCEPT_VIDEO = qulonglong(1) << 23,
    TTS_SUBSCRIPTIONS_INTERCEPT_DESKTOP = qulonglong(1) << 24,
    TTS_SUBSCRIPTIONS_INTERCEPT_MEDIAFILE = qulonglong(1) << 25,

    TTS_CLASSROOM_CHANMSG_TX = qulonglong(1) << 26,
    TTS_CLASSROOM_VOICE_TX = qulonglong(1) << 27,
    TTS_CLASSROOM_VIDEO_TX = qulonglong(1) << 28,
    TTS_CLASSROOM_DESKTOP_TX = qulonglong(1) << 29,
    TTS_CLASSROOM_MEDIAFILE_TX = qulonglong(1) << 30,

    TTS_FILE_ADD = qulonglong(1) << 31,
    TTS_FILE_REMOVE = qulonglong(1) << 32,

    TTS_MENU_ACTIONS = qulonglong(1) << 33,

    TTS_SERVER_CONNECTIVITY = qulonglong(1) << 34,

    // next free value
    TTS_NEXT_UNUSED = qulonglong(1) << 38,
};

typedef qulonglong TTSEvents;

enum TextToSpeechEngine
{
    TTSENGINE_NONE = 0,
    TTSENGINE_QT = 1,
    TTSENGINE_TOLK = 2,
    TTSENGINE_NOTIFY = 3,
};

#define TTSENGINE_NOTIFY_PATH "/usr/bin/notify-send"

void addTextToSpeechMessage(TextToSpeechEvent event, const QString& msg);
void addTextToSpeechMessage(const QString& msg);

#endif

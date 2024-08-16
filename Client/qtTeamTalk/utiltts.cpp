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

#include "utiltts.h"
#include "settings.h"
#include "common.h"
#include "appinfo.h"

#include <QProcess>

#if defined(QT_TEXTTOSPEECH_LIB)
#include <QTextToSpeech>
#endif
#if QT_VERSION >= QT_VERSION_CHECK(6,8,0)
#include <QAccessible>
#include <QAccessibleAnnouncementEvent>
#endif

#if defined(QT_TEXTTOSPEECH_LIB)
extern QTextToSpeech* ttSpeech;
#endif
#if QT_VERSION >= QT_VERSION_CHECK(6,8,0)
extern QObject* announcerObject;
#endif

extern QSettings* ttSettings;

QHash<TTSEvents, TTSEventInfo> UtilTTS::eventToSettingMap()
{
    static QHash<TTSEvents, TTSEventInfo> map = {
        { TTS_USER_LOGGEDIN, {SETTINGS_TTSMSG_USER_LOGGEDIN, {{"{user}", tr("User's nickname who logged in")}, {"{username}", tr("User's username who logged in")}, {"{server}", tr("Server's name from which event was emited")}}, "" } },
        { TTS_USER_LOGGEDOUT, {SETTINGS_TTSMSG_USER_LOGGEDOUT, {{"{user}", tr("User's nickname who logged out")}, {"{username}", tr("User's username who logged out")}, {"{server}", tr("Server's name from which event was emited")}}, "" } },
        { TTS_USER_JOINED, {SETTINGS_TTSMSG_USER_JOINED, {{"{user}", tr("User's nickname who joined channel")}, {"{username}", tr("User's username who joined channel")}, {"{channel}", tr("Channel's name joined by user")}}, "" } },
        { TTS_USER_LEFT, {SETTINGS_TTSMSG_USER_LEFT, {{"{user}", tr("User's nickname who left channel")}, {"{username}", tr("User's username who left channel")}, {"{channel}", tr("Channel's name left by user")}}, "" } },
        { TTS_USER_JOINED_SAME, {SETTINGS_TTSMSG_USER_JOINED_SAME, {{"{user}", tr("User's nickname who joined channel")}, {"{username}", tr("User's username who joined channel")}}, "" } },
        { TTS_USER_LEFT_SAME, {SETTINGS_TTSMSG_USER_LEFT_SAME, {{"{user}", tr("User's nickname who left channel")}, {"{username}", tr("User's username who left channel")}}, "" } },
        { TTS_USER_TEXTMSG_PRIVATE, {SETTINGS_TTSMSG_PRIVATEMSG, {{"{user}", tr("User's nickname who sent message")}, {"{username}", tr("User's username who sent message")}, {"{message}", tr("Message content")}, {"{server}", tr("Server's name from which event was emited")}}, "" } },
        { TTS_USER_TEXTMSG_PRIVATE_SEND, {SETTINGS_TTSMSG_PRIVATEMSGSEND, {{"{message}", tr("Message content")}}, "" } },
        { TTS_USER_TEXTMSG_PRIVATE_TYPING, {SETTINGS_TTSMSG_TYPING, {{"{user}", tr("User's nickname who is typing")}, {"{username}", tr("User's username who is typing")}}, tr("User typing") } },
        { TTS_USER_TEXTMSG_PRIVATE_TYPING_GLOBAL, {SETTINGS_TTSMSG_TYPING, {{"{user}", tr("User's nickname who is typing")}, {"{username}", tr("User's username who is typing")}}, tr("User typing") } },
        { TTS_USER_QUESTIONMODE, {SETTINGS_TTSMSG_QUESTIONMODE, {{"{user}", tr("User's nickname who set question mode")}, {"{username}", tr("User's username who set question mode")}, {"{server}", tr("Server's name from which event was emited")}}, "" } },
        { TTS_USER_TEXTMSG_CHANNEL, {SETTINGS_TTSMSG_CHANNELMSG, {{"{user}", tr("User's nickname who sent message")}, {"{username}", tr("User's username who sent message")}, {"{message}", tr("Message content")}, {"{server}", tr("Server's name from which event was emited")}}, "" } },
        { TTS_USER_TEXTMSG_CHANNEL_SEND, {SETTINGS_TTSMSG_CHANNELMSGSEND, {{"{message}", tr("Message content")}}, "" } },
        { TTS_USER_TEXTMSG_BROADCAST, {SETTINGS_TTSMSG_BROADCASTMSG, {{"{user}", tr("User's nickname who sent message")}, {"{username}", tr("User's username who sent message")}, {"{message}", tr("Message content")}, {"{server}", tr("Server's name from which event was emited")}}, "" } },
        { TTS_USER_TEXTMSG_BROADCAST_SEND, {SETTINGS_TTSMSG_BROADCASTMSGSEND, {{"{message}", tr("Message content")}}, "" } },
        { TTS_SUBSCRIPTIONS_TEXTMSG_PRIVATE, {SETTINGS_TTSMSG_SUBCHANGE, {{"{user}", tr("User concerns by change")}, {"{username}", tr("User's username concerns by change")}, {"{type}", tr("Subscription type")}, {"{state}", tr("Subscription state")}}, tr("Subscription change") } },
        { TTS_SUBSCRIPTIONS_TEXTMSG_CHANNEL, {SETTINGS_TTSMSG_SUBCHANGE, {{"{user}", tr("User concerns by change")}, {"{username}", tr("User's username concerns by change")}, {"{type}", tr("Subscription type")}, {"{state}", tr("Subscription state")}}, tr("Subscription change") } },
        { TTS_SUBSCRIPTIONS_TEXTMSG_BROADCAST, {SETTINGS_TTSMSG_SUBCHANGE, {{"{user}", tr("User concerns by change")}, {"{username}", tr("User's username concerns by change")}, {"{type}", tr("Subscription type")}, {"{state}", tr("Subscription state")}}, tr("Subscription change") } },
        { TTS_SUBSCRIPTIONS_VOICE, {SETTINGS_TTSMSG_SUBCHANGE, {{"{user}", tr("User concerns by change")}, {"{username}", tr("User's username concerns by change")}, {"{type}", tr("Subscription type")}, {"{state}", tr("Subscription state")}}, tr("Subscription change") } },
        { TTS_SUBSCRIPTIONS_VIDEO, {SETTINGS_TTSMSG_SUBCHANGE, {{"{user}", tr("User concerns by change")}, {"{username}", tr("User's username concerns by change")}, {"{type}", tr("Subscription type")}, {"{state}", tr("Subscription state")}}, tr("Subscription change") } },
        { TTS_SUBSCRIPTIONS_DESKTOP, {SETTINGS_TTSMSG_SUBCHANGE, {{"{user}", tr("User concerns by change")}, {"{username}", tr("User's username concerns by change")}, {"{type}", tr("Subscription type")}, {"{state}", tr("Subscription state")}}, tr("Subscription change") } },
        { TTS_SUBSCRIPTIONS_DESKTOPINPUT, {SETTINGS_TTSMSG_SUBCHANGE, {{"{user}", tr("User concerns by change")}, {"{username}", tr("User's username concerns by change")}, {"{type}", tr("Subscription type")}, {"{state}", tr("Subscription state")}}, tr("Subscription change") } },
        { TTS_SUBSCRIPTIONS_MEDIAFILE, {SETTINGS_TTSMSG_SUBCHANGE, {{"{user}", tr("User concerns by change")}, {"{username}", tr("User's username concerns by change")}, {"{type}", tr("Subscription type")}, {"{state}", tr("Subscription state")}}, tr("Subscription change") } },
        { TTS_SUBSCRIPTIONS_INTERCEPT_TEXTMSG_PRIVATE, {SETTINGS_TTSMSG_SUBCHANGE, {{"{user}", tr("User concerns by change")}, {"{username}", tr("User's username concerns by change")}, {"{type}", tr("Subscription type")}, {"{state}", tr("Subscription state")}}, tr("Subscription change") } },
        { TTS_SUBSCRIPTIONS_INTERCEPT_TEXTMSG_CHANNEL, {SETTINGS_TTSMSG_SUBCHANGE, {{"{user}", tr("User concerns by change")}, {"{username}", tr("User's username concerns by change")}, {"{type}", tr("Subscription type")}, {"{state}", tr("Subscription state")}}, tr("Subscription change") } },
        { TTS_SUBSCRIPTIONS_INTERCEPT_VOICE, {SETTINGS_TTSMSG_SUBCHANGE, {{"{user}", tr("User concerns by change")}, {"{username}", tr("User's username concerns by change")}, {"{type}", tr("Subscription type")}, {"{state}", tr("Subscription state")}}, tr("Subscription change") } },
        { TTS_SUBSCRIPTIONS_INTERCEPT_VIDEO, {SETTINGS_TTSMSG_SUBCHANGE, {{"{user}", tr("User concerns by change")}, {"{username}", tr("User's username concerns by change")}, {"{type}", tr("Subscription type")}, {"{state}", tr("Subscription state")}}, tr("Subscription change") } },
        { TTS_SUBSCRIPTIONS_INTERCEPT_DESKTOP, {SETTINGS_TTSMSG_SUBCHANGE, {{"{user}", tr("User concerns by change")}, {"{username}", tr("User's username concerns by change")}, {"{type}", tr("Subscription type")}, {"{state}", tr("Subscription state")}}, tr("Subscription change") } },
        { TTS_SUBSCRIPTIONS_INTERCEPT_MEDIAFILE, {SETTINGS_TTSMSG_SUBCHANGE, {{"{user}", tr("User concerns by change")}, {"{username}", tr("User's username concerns by change")}, {"{type}", tr("Subscription type")}, {"{state}", tr("Subscription state")}}, tr("Subscription change") } },
        { TTS_CLASSROOM_CHANMSG_TX, {SETTINGS_TTSMSG_CLASSROOM, {{"{type}", tr("Transmission type")}, {"{state}", tr("Transmission state")}, {"{user}", tr("User concerns by change")}}, tr("Classroom transmission authorization change") } },
        { TTS_CLASSROOM_VOICE_TX, {SETTINGS_TTSMSG_CLASSROOM, {{"{type}", tr("Transmission type")}, {"{state}", tr("Transmission state")}, {"{user}", tr("User concerns by change")}}, tr("Classroom transmission authorization change") } },
        { TTS_CLASSROOM_VIDEO_TX, {SETTINGS_TTSMSG_CLASSROOM, {{"{type}", tr("Transmission type")}, {"{state}", tr("Transmission state")}, {"{user}", tr("User concerns by change")}}, tr("Classroom transmission authorization change") } },
        { TTS_CLASSROOM_DESKTOP_TX, {SETTINGS_TTSMSG_CLASSROOM, {{"{type}", tr("Transmission type")}, {"{state}", tr("Transmission state")}, {"{user}", tr("User concerns by change")}}, tr("Classroom transmission authorization change") } },
        { TTS_CLASSROOM_MEDIAFILE_TX, {SETTINGS_TTSMSG_CLASSROOM, {{"{type}", tr("Transmission type")}, {"{state}", tr("Transmission state")}, {"{user}", tr("User concerns by change")}}, tr("Classroom transmission authorization change") } },
        { TTS_FILE_ADD, {SETTINGS_TTSMSG_FILE_ADDED, {{"{filename}", tr("File name")}, {"{user}", tr("User's nickname who added the file")}, {"{username}", tr("User's username who added the file")}, {"{filesize}", tr("File size")}}, "" } },
        { TTS_FILE_REMOVE, {SETTINGS_TTSMSG_FILE_REMOVED, {{"{file}", tr("File name")}, {"{user}", tr("User's nickname who removed the file")}, {"{username}", tr("User's username who removed the file")}}, "" } },
    };
    return map;
}

void addTextToSpeechMessage(const QString& msg)
{
    switch (ttSettings->value(SETTINGS_TTS_ENGINE, SETTINGS_TTS_ENGINE_DEFAULT).toUInt())
    {
    case TTSENGINE_QT:
#if QT_TEXTTOSPEECH_LIB
        Q_ASSERT(ttSpeech);
        ttSpeech->say(msg);
#endif
        break;
    case TTSENGINE_TOLK:
#if defined(ENABLE_TOLK)
        Tolk_PreferSAPI(ttSettings->value(SETTINGS_TTS_SAPI, SETTINGS_TTS_SAPI_DEFAULT).toBool());
        Tolk_TrySAPI(ttSettings->value(SETTINGS_TTS_TRY_SAPI, SETTINGS_TTS_TRY_SAPI_DEFAULT).toBool());
        switch (ttSettings->value(SETTINGS_TTS_OUTPUT_MODE, SETTINGS_TTS_OUTPUT_MODE_DEFAULT).toInt())
        {
            case TTS_OUTPUTMODE_BRAILLE:
                Tolk_Braille(_W(msg));
                break;
            case TTS_OUTPUTMODE_SPEECH:
                Tolk_Speak(_W(msg));
                break;
            case TTS_OUTPUTMODE_SPEECHBRAILLE:
                Tolk_Output(_W(msg));
                break;
        }
#endif
        break;
    case TTSENGINE_NOTIFY:
    {
#if defined(Q_OS_LINUX)
        int timestamp = ttSettings->value(SETTINGS_TTS_TIMESTAMP, SETTINGS_TTS_TIMESTAMP_DEFAULT).toUInt();
        QString noquote = msg;
        noquote.replace('"', ' ');
        QProcess ps;
        ps.startDetached(QString("%1 -t %2 -a \"%3\" -u low \"%4: %5\"")
            .arg(TTSENGINE_NOTIFY_PATH)
            .arg(timestamp)
            .arg(APPNAME_SHORT)
            .arg(APPNAME_SHORT)
            .arg(noquote));
#endif
        break;
    }
    case TTSENGINE_QTANNOUNCEMENT:
    {
#if QT_VERSION >= QT_VERSION_CHECK(6,8,0)
        QAccessibleAnnouncementEvent announcementEvent(announcerObject, msg);
        if (ttSettings->value(SETTINGS_TTS_ASSERTIVE, SETTINGS_TTS_ASSERTIVE_DEFAULT).toBool() == true)
            announcementEvent.setPoliteness(QAccessible::AnnouncementPoliteness::Assertive);
        QAccessible::updateAccessibility(&announcementEvent);
#endif
        break;
    }
    case TTSENGINE_APPLESCRIPT:
#if defined(Q_OS_MAC)
        QString escapedMsg = msg;
        escapedMsg.replace("\"", "\\\"");
        QString appleScript = QString(R"(
            tell application "VoiceOver"
                output "%1"
            end tell
        )").arg(escapedMsg);
        QStringList arguments;
        arguments << "-e" << appleScript;
        QProcess::startDetached("osascript", arguments);
#endif
        break;
    }
}

void addTextToSpeechMessage(TextToSpeechEvent event, const QString& msg)
{
    if ((ttSettings->value(SETTINGS_TTS_ACTIVEEVENTS, SETTINGS_TTS_ACTIVEEVENTS_DEFAULT).toULongLong() & event) && ttSettings->value(SETTINGS_TTS_ENABLE, SETTINGS_TTS_ENABLE_DEFAULT).toBool() == true)
    {
        addTextToSpeechMessage(msg);
    }
}

QString UtilTTS::getDefaultValue(const QString& paramKey)
{
    if (paramKey == SETTINGS_TTSMSG_USER_LOGGEDIN)
        return QCoreApplication::translate("UtilTTS", SETTINGS_TTSMSG_USER_LOGGEDIN_DEFAULT);
    if (paramKey == SETTINGS_TTSMSG_USER_LOGGEDOUT)
        return QCoreApplication::translate("UtilTTS", SETTINGS_TTSMSG_USER_LOGGEDOUT_DEFAULT);
    if (paramKey == SETTINGS_TTSMSG_USER_JOINED)
        return QCoreApplication::translate("UtilTTS", SETTINGS_TTSMSG_USER_JOINED_DEFAULT);
    if (paramKey == SETTINGS_TTSMSG_USER_LEFT)
        return QCoreApplication::translate("UtilTTS", SETTINGS_TTSMSG_USER_LEFT_DEFAULT);
    if (paramKey == SETTINGS_TTSMSG_USER_JOINED_SAME)
        return QCoreApplication::translate("UtilTTS", SETTINGS_TTSMSG_USER_JOINED_SAME_DEFAULT);
    if (paramKey == SETTINGS_TTSMSG_USER_LEFT_SAME)
        return QCoreApplication::translate("UtilTTS", SETTINGS_TTSMSG_USER_LEFT_SAME_DEFAULT);
    if (paramKey == SETTINGS_TTSMSG_PRIVATEMSG)
        return QCoreApplication::translate("UtilTTS", SETTINGS_TTSMSG_PRIVATEMSG_DEFAULT);
    if (paramKey == SETTINGS_TTSMSG_PRIVATEMSGSEND)
        return QCoreApplication::translate("UtilTTS", SETTINGS_TTSMSG_PRIVATEMSGSEND_DEFAULT);
    if (paramKey == SETTINGS_TTSMSG_TYPING)
        return QCoreApplication::translate("UtilTTS", SETTINGS_TTSMSG_TYPING_DEFAULT);
    if (paramKey == SETTINGS_TTSMSG_QUESTIONMODE)
        return QCoreApplication::translate("UtilTTS", SETTINGS_TTSMSG_QUESTIONMODE_DEFAULT);
    if (paramKey == SETTINGS_TTSMSG_CHANNELMSG)
        return QCoreApplication::translate("UtilTTS", SETTINGS_TTSMSG_CHANNELMSG_DEFAULT);
    if (paramKey == SETTINGS_TTSMSG_CHANNELMSGSEND)
        return QCoreApplication::translate("UtilTTS", SETTINGS_TTSMSG_CHANNELMSGSEND_DEFAULT);
    if (paramKey == SETTINGS_TTSMSG_BROADCASTMSG)
        return QCoreApplication::translate("UtilTTS", SETTINGS_TTSMSG_BROADCASTMSG_DEFAULT);
    if (paramKey == SETTINGS_TTSMSG_BROADCASTMSGSEND)
        return QCoreApplication::translate("UtilTTS", SETTINGS_TTSMSG_BROADCASTMSGSEND_DEFAULT);
    if (paramKey == SETTINGS_TTSMSG_SUBCHANGE)
        return QCoreApplication::translate("UtilTTS", SETTINGS_TTSMSG_SUBCHANGE_DEFAULT);
    if (paramKey == SETTINGS_TTSMSG_CLASSROOM)
        return QCoreApplication::translate("UtilTTS", SETTINGS_TTSMSG_CLASSROOM_DEFAULT);
    if (paramKey == SETTINGS_TTSMSG_FILE_ADDED)
        return QCoreApplication::translate("UtilTTS", SETTINGS_TTSMSG_FILE_ADDED_DEFAULT);
    if (paramKey == SETTINGS_TTSMSG_FILE_REMOVED)
        return QCoreApplication::translate("UtilTTS", SETTINGS_TTSMSG_FILE_REMOVED_DEFAULT);
    return QString();
}

QString UtilTTS::getTTSMessage(const QString& paramKey, const QHash<QString, QString>& variables)
{
    QString messageTemplate = ttSettings->value(paramKey, getDefaultValue(paramKey)).toString();

    for (auto it = variables.constBegin(); it != variables.constEnd(); ++it)
    {
        messageTemplate.replace(it.key(), it.value());
    }

    return messageTemplate;
}

QString UtilTTS::getRawTTSMessage(const QString& paramKey)
{
    return ttSettings->value(paramKey, getDefaultValue(paramKey)).toString();
}

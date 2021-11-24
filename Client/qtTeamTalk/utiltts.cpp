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

#include "utiltts.h"
#include "settings.h"
#include "common.h"
#include "appinfo.h"

#include <QProcess>

#if defined(QT_TEXTTOSPEECH_LIB)
#include <QTextToSpeech>
#endif

#if defined(QT_TEXTTOSPEECH_LIB)
extern QTextToSpeech* ttSpeech;
#endif

extern QSettings* ttSettings;

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
        Tolk_Output(_W(msg));
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
    }
}

void addTextToSpeechMessage(TextToSpeechEvent event, const QString& msg)
{
    if ((ttSettings->value(SETTINGS_TTS_ACTIVEEVENTS, SETTINGS_TTS_ACTIVEEVENTS_DEFAULT).toULongLong() & event) && ttSettings->value(SETTINGS_TTS_ENABLE, SETTINGS_TTS_ENABLE_DEFAULT).toBool() == true)
    {
        addTextToSpeechMessage(msg);
    }
}

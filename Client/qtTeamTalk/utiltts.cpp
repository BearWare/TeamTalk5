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
    }
}

void addTextToSpeechMessage(TextToSpeechEvent event, const QString& msg)
{
    if ((ttSettings->value(SETTINGS_TTS_ACTIVEEVENTS, SETTINGS_TTS_ACTIVEEVENTS_DEFAULT).toULongLong() & event) && ttSettings->value(SETTINGS_TTS_ENABLE, SETTINGS_TTS_ENABLE_DEFAULT).toBool() == true)
    {
        addTextToSpeechMessage(msg);
    }
}

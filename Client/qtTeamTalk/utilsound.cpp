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

#include "utilsound.h"
#include "settings.h"

#include <math.h>

#include <QSettings>
#include <QDebug>

#if defined(QT_MULTIMEDIA_LIB)
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
#include <QSound>
#else
#include <QSoundEffect>
#endif /* QT_VERSION_CHECK */
#endif /* QT_MULTIMEDIA_LIB */

extern QSettings* ttSettings;
extern TTInstance* ttInst;
extern PlaySoundEvent* playsoundevent;

QVector<SoundDevice> getSoundDevices()
{
    //try getting all sound devices at once
    int count = 25;
    QVector<SoundDevice> result(count);
    TT_GetSoundDevices(&result[0], &count);
    if (result.size() == count)
    {
        //query again since we didn't have enough room
        TT_GetSoundDevices(nullptr, &count);
        result.resize(count);
        TT_GetSoundDevices(&result[0], &count);
    }
    else
        result.resize(count);
    return result;
}

bool getSoundDevice(int deviceid, const QVector<SoundDevice>& devs, SoundDevice& dev)
{
    for (auto d : devs)
    {
        if (d.nDeviceID == deviceid)
        {
            dev = d;
            return true;
        }
    }
    return false;
}

bool getSoundDevice(const QString& devid, bool input, const QVector<SoundDevice>& devs, SoundDevice& dev)
{
    if (devid.isEmpty())
        return false;

    for (auto d : devs)
    {
        if ( _Q(d.szDeviceID) == devid && ((input && d.nMaxInputChannels > 0) || (!input && d.nMaxOutputChannels > 0)))
        {
            dev = d;
            return true;
        }
    }
    return false;
}

int getSoundDuplexSampleRate(const SoundDevice& indev, const SoundDevice& outdev)
{
    auto isend = indev.inputSampleRates + sizeof(indev.inputSampleRates);
    auto isr = std::find_if(indev.inputSampleRates, isend,
        [outdev](int sr) { return sr == outdev.nDefaultSampleRate; });
    bool duplexmode = (indev.uSoundDeviceFeatures & SOUNDDEVICEFEATURE_DUPLEXMODE) &&
        (outdev.uSoundDeviceFeatures & SOUNDDEVICEFEATURE_DUPLEXMODE);

    return (duplexmode && isr != isend) ? outdev.nDefaultSampleRate : 0;
}

bool isSoundDeviceEchoCapable(const SoundDevice& indev, const SoundDevice& outdev)
{
    return getSoundDuplexSampleRate(indev, outdev) > 0 || (indev.uSoundDeviceFeatures & SOUNDDEVICEFEATURE_AEC);
}

int getDefaultSndCommunicationInputDevice()
{
    for (const auto& d : getSoundDevices())
    {
        if (d.nMaxInputChannels > 0 && (d.uSoundDeviceFeatures & SOUNDDEVICEFEATURE_DEFAULTCOMDEVICE))
            return d.nDeviceID;
    }
    return SOUNDDEVICEID_DEFAULT;
}

int getDefaultSndCommunicationOutputDevice()
{
    for (const auto& d : getSoundDevices())
    {
        if (d.nMaxOutputChannels > 0 && (d.uSoundDeviceFeatures & SOUNDDEVICEFEATURE_DEFAULTCOMDEVICE))
            return d.nDeviceID;
    }
    return SOUNDDEVICEID_DEFAULT;
}

int getDefaultSndInputDevice()
{
    int inputid = getDefaultSndCommunicationInputDevice();
    if (inputid != SOUNDDEVICEID_DEFAULT)
        return inputid;

    SoundSystem sndsys = SoundSystem(ttSettings->value(SETTINGS_SOUND_SOUNDSYSTEM, SOUNDSYSTEM_NONE).toInt());
    inputid = ttSettings->value(SETTINGS_SOUND_INPUTDEVICE, TT_SOUNDDEVICE_ID_TEAMTALK_VIRTUAL).toInt();
    if (sndsys != SOUNDSYSTEM_NONE)
        TT_GetDefaultSoundDevicesEx(sndsys, &inputid, nullptr);
    else
        TT_GetDefaultSoundDevices(&inputid, nullptr);
    return inputid;
}

int getDefaultSndOutputDevice()
{
    int outputid = getDefaultSndCommunicationOutputDevice();
    if (outputid != SOUNDDEVICEID_DEFAULT)
        return outputid;

    SoundSystem sndsys = (SoundSystem)ttSettings->value(SETTINGS_SOUND_SOUNDSYSTEM, SOUNDSYSTEM_NONE).toInt();
    outputid = ttSettings->value(SETTINGS_SOUND_OUTPUTDEVICE, TT_SOUNDDEVICE_ID_TEAMTALK_VIRTUAL).toInt();
    if (sndsys != SOUNDSYSTEM_NONE)
        TT_GetDefaultSoundDevicesEx(sndsys, nullptr, &outputid);
    else
        TT_GetDefaultSoundDevices(nullptr, &outputid);
    return outputid;
}

int getSoundInputFromUID(int inputid, const QString& uid)
{
    if (uid.isEmpty())
        return inputid;

    QVector<SoundDevice> inputdev = getSoundDevices();

    SoundDevice dev;
    if (getSoundDevice(uid, true, inputdev, dev))
        inputid = dev.nDeviceID;

    return inputid;
}

int getSoundOutputFromUID(int outputid, const QString& uid)
{
    if (uid.isEmpty())
        return outputid;

    QVector<SoundDevice> outputdev = getSoundDevices();

    SoundDevice dev;
    if (getSoundDevice(uid, false, outputdev, dev))
        outputid = dev.nDeviceID;
    return outputid;
}

int getSelectedSndInputDevice()
{
    int inputid = ttSettings->value(SETTINGS_SOUND_INPUTDEVICE, SOUNDDEVICEID_DEFAULT).toInt();
    qDebug() << "Input device in settings #" << inputid;
    if (inputid == SOUNDDEVICEID_DEFAULT)
        inputid = getDefaultSndInputDevice();
    else
    {
        //check if device-id has changed since last
        QString uid = ttSettings->value(SETTINGS_SOUND_INPUTDEVICE_UID, "").toString();
        if (uid.size())
            inputid = getSoundInputFromUID(inputid, uid);
    }
    qDebug() << "Returning input device #" << inputid;
    return inputid;
}

int getSelectedSndOutputDevice()
{
    int outputid = ttSettings->value(SETTINGS_SOUND_OUTPUTDEVICE, SOUNDDEVICEID_DEFAULT).toInt();
    qDebug() << "Output device in settings #" << outputid;
    if (outputid == SOUNDDEVICEID_DEFAULT)
        outputid = getDefaultSndOutputDevice();
    else
    {
        //check if device-id has changed since last
        QString uid = ttSettings->value(SETTINGS_SOUND_OUTPUTDEVICE_UID, "").toString();
        if (uid.size())
            outputid = getSoundOutputFromUID(outputid, uid);
    }
    qDebug() << "Returning output device #" << outputid;
    return outputid;
}

QStringList initSelectedSoundDevices(SoundDevice& indev, SoundDevice& outdev)
{
    QStringList result;

    TT_CloseSoundInputDevice(ttInst);
    TT_CloseSoundOutputDevice(ttInst);
    TT_CloseSoundDuplexDevices(ttInst);

    //Restart sound system so we have the latest sound devices
    TT_RestartSoundSystem();

    int inputid = getSelectedSndInputDevice();
    int outputid = getSelectedSndOutputDevice();

    QVector<SoundDevice> devs = getSoundDevices();
    getSoundDevice(inputid, devs, indev);
    getSoundDevice(outputid, devs, outdev);

    SoundDeviceEffects effects = {};
    bool echocancel = ttSettings->value(SETTINGS_SOUND_ECHOCANCEL, SETTINGS_SOUND_ECHOCANCEL_DEFAULT).toBool();

    int samplerate = getSoundDuplexSampleRate(indev, outdev);
    bool duplex = samplerate > 0 && echocancel;

    // prefer WebRTC to echo cancel if duplex is available
    if (echocancel && !duplex)
    {
        // toggle sound device effect if it's supported by input device
        effects.bEnableEchoCancellation = (indev.uSoundDeviceFeatures & SOUNDDEVICEFEATURE_AEC) && echocancel;

        // WASAPI must know input and output device to echo cancel
        duplex = outdev.nSoundSystem == SOUNDSYSTEM_WASAPI;
    }

    TT_SetSoundDeviceEffects(ttInst, &effects);

    if (duplex)
    {
        if (!TT_InitSoundDuplexDevices(ttInst, inputid, outputid))
        {
            result.append(QObject::tr("Failed to initialize sound duplex mode"));
            indev = {}, outdev = {};
        }
    }
    else
    {
        if (!TT_InitSoundInputDevice(ttInst, inputid))
        {
            result.append(QObject::tr("Failed to initialize sound input device"));
            indev = {};
        }
        if (!TT_InitSoundOutputDevice(ttInst, outputid))
        {
            result.append(QObject::tr("Failed to initialize sound output device"));
            outdev = {};
        }
    }
    return result;
}

QStringList initDefaultSoundDevices(SoundDevice& indev, SoundDevice& outdev)
{
    QStringList result;

    TT_CloseSoundInputDevice(ttInst);
    TT_CloseSoundOutputDevice(ttInst);
    TT_CloseSoundDuplexDevices(ttInst);

    result.append(QObject::tr("Switching to default sound devices"));

    //Restart sound system so we have the latest sound devices
    TT_RestartSoundSystem();

    int inputid, outputid;
    if (!TT_GetDefaultSoundDevices(&inputid, &outputid))
    {
        result.append(QObject::tr("Unable to get default sound devices"));
    }
    else
    {
        QVector<SoundDevice> devs = getSoundDevices();
        getSoundDevice(inputid, devs, indev);
        getSoundDevice(outputid, devs, outdev);

        // reset sound device effects
        SoundDeviceEffects effects = {};
        TT_SetSoundDeviceEffects(ttInst, &effects);

        bool duplex = getSoundDuplexSampleRate(indev, outdev) > 0;

        if (duplex)
        {
            if (!TT_InitSoundDuplexDevices(ttInst, inputid, outputid))
            {
                result.append(QObject::tr("Failed to initialize sound duplex mode"));
                indev = {}, outdev = {};
            }
        }
        else
        {
            if (!TT_InitSoundInputDevice(ttInst, inputid))
            {
                result.append(QObject::tr("Failed to initialize default sound input device"));
                indev = {};
            }
            if (!TT_InitSoundOutputDevice(ttInst, outputid))
            {
                result.append(QObject::tr("Failed to initialize default sound output device"));
                outdev = {};
            }
        }
    }
    return result;
}

void setVolume(int userid, int vol_diff, StreamType stream_type)
{
    User user;
    if (TT_GetUser(ttInst, userid, &user))
    {
        int vol = 0;
        switch (stream_type)
        {
        case STREAMTYPE_VOICE:
            vol = refVolumeToPercent(user.nVolumeVoice);
            vol = refVolume(vol + vol_diff);
            break;
        case STREAMTYPE_MEDIAFILE_AUDIO:
            vol = refVolumeToPercent(user.nVolumeMediaFile);
            vol = refVolume(vol + vol_diff);
            break;
        default:
            Q_ASSERT(0);
        }

        vol = qMin(vol, (int)SOUND_VOLUME_MAX);
        vol = qMax(vol, (int)SOUND_VOLUME_MIN);
        TT_SetUserVolume(ttInst, userid, stream_type, vol);
    }
}

void incVolume(int userid, StreamType stream_type)
{
    setVolume(userid, 1, stream_type);
}

void decVolume(int userid, StreamType stream_type)
{
    setVolume(userid, -1, stream_type);
}

int refVolume(double percent)
{
    //82.832*EXP(0.0508*x) - 50 
    if (percent == 0)
        return 0;
    double d = 82.832 * exp(0.0508 * percent) - 50;
    return d;
}

int refVolumeToPercent(int volume)
{
    if (volume == 0)
        return 0;

    double d = (volume + 50) / 82.832;
    d = log(d) / 0.0508;
    return d + .5;
}

int refGain(double percent)
{
    if (percent == 0)
        return 0;

    return  82.832 * exp(0.0508 * percent) - 50;
}

QString getSoundEventFilename(SoundEvent event)
{
    QString filename;
    if (ttSettings->value(SETTINGS_SOUNDEVENT_ENABLE, SETTINGS_SOUNDEVENT_ENABLE_DEFAULT).toBool())
    {
        switch (event)
        {
        case SOUNDEVENT_NEWUSER:
            filename = ttSettings->value(SETTINGS_SOUNDEVENT_NEWUSER, SETTINGS_SOUNDEVENT_NEWUSER_DEFAULT).toString();
            break;
        case SOUNDEVENT_REMOVEUSER:
            filename = ttSettings->value(SETTINGS_SOUNDEVENT_REMOVEUSER, SETTINGS_SOUNDEVENT_REMOVEUSER_DEFAULT).toString();
            break;
        case SOUNDEVENT_SERVERLOST:
            filename = ttSettings->value(SETTINGS_SOUNDEVENT_SERVERLOST, SETTINGS_SOUNDEVENT_SERVERLOST_DEFAULT).toString();
            break;
        case SOUNDEVENT_USERMSG:
            filename = ttSettings->value(SETTINGS_SOUNDEVENT_USERMSG, SETTINGS_SOUNDEVENT_USERMSG_DEFAULT).toString();
            break;
        case SOUNDEVENT_USERMSGSENT:
            filename = ttSettings->value(SETTINGS_SOUNDEVENT_USERMSGSENT, SETTINGS_SOUNDEVENT_USERMSGSENT_DEFAULT).toString();
            break;
        case SOUNDEVENT_CHANNELMSG:
            filename = ttSettings->value(SETTINGS_SOUNDEVENT_CHANNELMSG, SETTINGS_SOUNDEVENT_CHANNELMSG_DEFAULT).toString();
            break;
        case SOUNDEVENT_CHANNELMSGSENT:
            filename = ttSettings->value(SETTINGS_SOUNDEVENT_CHANNELMSGSENT, SETTINGS_SOUNDEVENT_CHANNELMSGSENT_DEFAULT).toString();
            break;
        case SOUNDEVENT_BROADCASTMSG:
            filename = ttSettings->value(SETTINGS_SOUNDEVENT_BROADCASTMSG, SETTINGS_SOUNDEVENT_BROADCASTMSG_DEFAULT).toString();
            break;
        case SOUNDEVENT_HOTKEY:
            filename = ttSettings->value(SETTINGS_SOUNDEVENT_HOTKEY, SETTINGS_SOUNDEVENT_HOTKEY_DEFAULT).toString();
            break;
        case SOUNDEVENT_SILENCE:
            filename = ttSettings->value(SETTINGS_SOUNDEVENT_SILENCE).toString();
            break;
        case SOUNDEVENT_NEWVIDEO:
            filename = ttSettings->value(SETTINGS_SOUNDEVENT_NEWVIDEO, SETTINGS_SOUNDEVENT_NEWVIDEO_DEFAULT).toString();
            break;
        case SOUNDEVENT_NEWDESKTOP:
            filename = ttSettings->value(SETTINGS_SOUNDEVENT_NEWDESKTOP, SETTINGS_SOUNDEVENT_NEWDESKTOP_DEFAULT).toString();
            break;
        case SOUNDEVENT_FILESUPD:
            filename = ttSettings->value(SETTINGS_SOUNDEVENT_FILESUPD, SETTINGS_SOUNDEVENT_FILESUPD_DEFAULT).toString();
            break;
        case SOUNDEVENT_FILETXDONE:
            filename = ttSettings->value(SETTINGS_SOUNDEVENT_FILETXDONE, SETTINGS_SOUNDEVENT_FILETXDONE_DEFAULT).toString();
            break;
        case SOUNDEVENT_QUESTIONMODE:
            filename = ttSettings->value(SETTINGS_SOUNDEVENT_QUESTIONMODE, SETTINGS_SOUNDEVENT_QUESTIONMODE_DEFAULT).toString();
            break;
        case SOUNDEVENT_DESKTOPACCESS:
            filename = ttSettings->value(SETTINGS_SOUNDEVENT_DESKTOPACCESS, SETTINGS_SOUNDEVENT_DESKTOPACCESS_DEFAULT).toString();
            break;
        case SOUNDEVENT_USERLOGGEDIN:
            filename = ttSettings->value(SETTINGS_SOUNDEVENT_USERLOGGEDIN, SETTINGS_SOUNDEVENT_USERLOGGEDIN_DEFAULT).toString();
            break;
        case SOUNDEVENT_USERLOGGEDOUT:
            filename = ttSettings->value(SETTINGS_SOUNDEVENT_USERLOGGEDOUT, SETTINGS_SOUNDEVENT_USERLOGGEDOUT_DEFAULT).toString();
            break;
        case SOUNDEVENT_VOICEACTON:
            filename = ttSettings->value(SETTINGS_SOUNDEVENT_VOICEACTON, SETTINGS_SOUNDEVENT_VOICEACTON_DEFAULT).toString();
            break;
        case SOUNDEVENT_VOICEACTOFF:
            filename = ttSettings->value(SETTINGS_SOUNDEVENT_VOICEACTOFF, SETTINGS_SOUNDEVENT_VOICEACTOFF_DEFAULT).toString();
            break;
        case SOUNDEVENT_MUTEALLON:
            filename = ttSettings->value(SETTINGS_SOUNDEVENT_MUTEALLON, SETTINGS_SOUNDEVENT_MUTEALLON_DEFAULT).toString();
            break;
        case SOUNDEVENT_MUTEALLOFF:
            filename = ttSettings->value(SETTINGS_SOUNDEVENT_MUTEALLOFF, SETTINGS_SOUNDEVENT_MUTEALLOFF_DEFAULT).toString();
            break;
        case SOUNDEVENT_TRANSMITQUEUE_HEAD:
            filename = ttSettings->value(SETTINGS_SOUNDEVENT_TRANSMITQUEUE_HEAD, SETTINGS_SOUNDEVENT_TRANSMITQUEUE_HEAD_DEFAULT).toString();
            break;
        case SOUNDEVENT_TRANSMITQUEUE_STOP:
            filename = ttSettings->value(SETTINGS_SOUNDEVENT_TRANSMITQUEUE_STOP, SETTINGS_SOUNDEVENT_TRANSMITQUEUE_STOP_DEFAULT).toString();
            break;
        case SOUNDEVENT_VOICEACTTRIG:
            filename = ttSettings->value(SETTINGS_SOUNDEVENT_VOICEACTTRIG, SETTINGS_SOUNDEVENT_VOICEACTTRIG_DEFAULT).toString();
            break;
        case SOUNDEVENT_VOICEACTSTOP:
            filename = ttSettings->value(SETTINGS_SOUNDEVENT_VOICEACTSTOP, SETTINGS_SOUNDEVENT_VOICEACTSTOP_DEFAULT).toString();
            break;
        case SOUNDEVENT_VOICEACTMEON:
            filename = ttSettings->value(SETTINGS_SOUNDEVENT_VOICEACTMEON, SETTINGS_SOUNDEVENT_VOICEACTMEON_DEFAULT).toString();
            break;
        case SOUNDEVENT_VOICEACTMEOFF:
            filename = ttSettings->value(SETTINGS_SOUNDEVENT_VOICEACTMEOFF, SETTINGS_SOUNDEVENT_VOICEACTMEOFF_DEFAULT).toString();
            break;
        case SOUNDEVENT_INTERCEPT:
            filename = ttSettings->value(SETTINGS_SOUNDEVENT_INTERCEPT, SETTINGS_SOUNDEVENT_INTERCEPT_DEFAULT).toString();
            break;
        case SOUNDEVENT_INTERCEPTEND:
            filename = ttSettings->value(SETTINGS_SOUNDEVENT_INTERCEPTEND, SETTINGS_SOUNDEVENT_INTERCEPTEND_DEFAULT).toString();
            break;
        }
    }
    return filename;
}

PlaySoundEvent::PlaySoundEvent(QObject* parent) : QObject(parent)
{
}

void PlaySoundEvent::queueSoundEvent(SoundEvent event)
{
    m_playbackQueue.enqueue(event);
    playSoundEvent();
}

void PlaySoundEvent::playbackUpdate(int playbackid, const MediaFileInfo& mfi)
{
    switch (mfi.nStatus)
    {
    case MFS_FINISHED :
    case MFS_ABORTED :
    case MFS_ERROR :
    {
        if (m_activeSessions.contains(playbackid))
        {
            m_activeSessions.remove(playbackid);
            playSoundEvent();
        }
        break;
    }
    default : break;
    }
}

void PlaySoundEvent::playSoundEvent()
{
    QString filename;
    while (m_playbackQueue.size() && filename.isEmpty())
    {
        filename = getSoundEventFilename(m_playbackQueue.front());
        if (filename.isEmpty())
            m_playbackQueue.dequeue();
    }

    if (m_playbackQueue.empty())
        return;

    if (filename.size())
    {
        PlaybackMode pbm = PlaybackMode(ttSettings->value(SETTINGS_SOUNDEVENT_PLAYBACKMODE, SETTINGS_SOUNDEVENT_PLAYBACKMODE_DEFAULT).toInt());
        switch (pbm)
        {
        case PLAYBACKMODE_DEFAULT :
            playDefaultSoundEvent(filename);
            break;
        case PLAYBACKMODE_ONEBYONE :
            if (m_activeSessions.size())
                return;
        case PLAYBACKMODE_TEAMTALK :
        case PLAYBACKMODE_OVERLAPPING :
            if (!playTeamTalkSoundEvent(filename))
            {
                m_playbackQueue.dequeue();
                playSoundEvent();
                return;
            }
            break;
        case PLAYBACKMODE_NONE :
            break;
        }
    }
    m_playbackQueue.dequeue();
}

bool PlaySoundEvent::playTeamTalkSoundEvent(const QString& filename)
{
    MediaFilePlayback mfp = {};
    mfp.uOffsetMSec = TT_MEDIAPLAYBACK_OFFSET_IGNORE;
    mfp.bPaused = FALSE;
    mfp.audioPreprocessor = initDefaultAudioPreprocessor(TEAMTALK_AUDIOPREPROCESSOR);
    int volumePct = ttSettings->value(SETTINGS_SOUNDEVENT_VOLUME, SETTINGS_SOUNDEVENT_VOLUME_DEFAULT).toInt();
    // sound event volume is historically 0-100% but refGain() has 50% as default volume,
    // i.e. play at original volume level
    volumePct /= 2;
    mfp.audioPreprocessor.ttpreprocessor.nGainLevel = refGain(volumePct);

    int id = TT_InitLocalPlayback(ttInst, _W(filename), &mfp);
    if (id > 0)
        m_activeSessions.insert(id);
    return id > 0;
}

void PlaySoundEvent::playDefaultSoundEvent(const QString& filename)
{
#if defined(QT_MULTIMEDIA_LIB)
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    QSound::play(filename);
#else
    static QSoundEffect* effect = new QSoundEffect(ttSettings);
    effect->setSource(QUrl::fromLocalFile(filename));
    effect->setVolume(ttSettings->value(SETTINGS_SOUNDEVENT_VOLUME, SETTINGS_SOUNDEVENT_VOLUME_DEFAULT).toInt()/100.0);
    effect->play();
#endif
#endif
}

void playSoundEvent(SoundEvent event)
{
    Q_ASSERT(playsoundevent);
    playsoundevent->queueSoundEvent(event);
}

void resetDefaultSoundsPack()
{
    ttSettings->setValue(SETTINGS_SOUNDEVENT_NEWUSER, SETTINGS_SOUNDEVENT_NEWUSER_DEFAULT);
    ttSettings->setValue(SETTINGS_SOUNDEVENT_REMOVEUSER, SETTINGS_SOUNDEVENT_REMOVEUSER_DEFAULT);
    ttSettings->setValue(SETTINGS_SOUNDEVENT_SERVERLOST, SETTINGS_SOUNDEVENT_SERVERLOST_DEFAULT);
    ttSettings->setValue(SETTINGS_SOUNDEVENT_USERMSG, SETTINGS_SOUNDEVENT_USERMSG_DEFAULT);
    ttSettings->setValue(SETTINGS_SOUNDEVENT_USERMSGSENT, SETTINGS_SOUNDEVENT_USERMSGSENT_DEFAULT);
    ttSettings->setValue(SETTINGS_SOUNDEVENT_CHANNELMSG, SETTINGS_SOUNDEVENT_CHANNELMSG_DEFAULT);
    ttSettings->setValue(SETTINGS_SOUNDEVENT_CHANNELMSGSENT, SETTINGS_SOUNDEVENT_CHANNELMSGSENT_DEFAULT);
    ttSettings->setValue(SETTINGS_SOUNDEVENT_BROADCASTMSG, SETTINGS_SOUNDEVENT_BROADCASTMSG_DEFAULT);
    ttSettings->setValue(SETTINGS_SOUNDEVENT_HOTKEY, SETTINGS_SOUNDEVENT_HOTKEY_DEFAULT);
    ttSettings->setValue(SETTINGS_SOUNDEVENT_NEWVIDEO, SETTINGS_SOUNDEVENT_NEWVIDEO_DEFAULT);
    ttSettings->setValue(SETTINGS_SOUNDEVENT_NEWDESKTOP, SETTINGS_SOUNDEVENT_NEWDESKTOP_DEFAULT);
    ttSettings->setValue(SETTINGS_SOUNDEVENT_FILESUPD, SETTINGS_SOUNDEVENT_FILESUPD_DEFAULT);
    ttSettings->setValue(SETTINGS_SOUNDEVENT_FILETXDONE, SETTINGS_SOUNDEVENT_FILETXDONE_DEFAULT);
    ttSettings->setValue(SETTINGS_SOUNDEVENT_QUESTIONMODE, SETTINGS_SOUNDEVENT_QUESTIONMODE_DEFAULT);
    ttSettings->setValue(SETTINGS_SOUNDEVENT_DESKTOPACCESS, SETTINGS_SOUNDEVENT_DESKTOPACCESS_DEFAULT);
    ttSettings->setValue(SETTINGS_SOUNDEVENT_USERLOGGEDIN, SETTINGS_SOUNDEVENT_USERLOGGEDIN_DEFAULT);
    ttSettings->setValue(SETTINGS_SOUNDEVENT_USERLOGGEDOUT, SETTINGS_SOUNDEVENT_USERLOGGEDOUT_DEFAULT);
    ttSettings->setValue(SETTINGS_SOUNDEVENT_VOICEACTON, SETTINGS_SOUNDEVENT_VOICEACTON_DEFAULT);
    ttSettings->setValue(SETTINGS_SOUNDEVENT_VOICEACTOFF, SETTINGS_SOUNDEVENT_VOICEACTOFF_DEFAULT);
    ttSettings->setValue(SETTINGS_SOUNDEVENT_MUTEALLON, SETTINGS_SOUNDEVENT_MUTEALLON_DEFAULT);
    ttSettings->setValue(SETTINGS_SOUNDEVENT_MUTEALLOFF, SETTINGS_SOUNDEVENT_MUTEALLOFF_DEFAULT);
    ttSettings->setValue(SETTINGS_SOUNDEVENT_TRANSMITQUEUE_HEAD, SETTINGS_SOUNDEVENT_TRANSMITQUEUE_HEAD_DEFAULT);
    ttSettings->setValue(SETTINGS_SOUNDEVENT_TRANSMITQUEUE_STOP, SETTINGS_SOUNDEVENT_TRANSMITQUEUE_STOP_DEFAULT);
    ttSettings->setValue(SETTINGS_SOUNDEVENT_VOICEACTTRIG, SETTINGS_SOUNDEVENT_VOICEACTTRIG_DEFAULT);
    ttSettings->setValue(SETTINGS_SOUNDEVENT_VOICEACTSTOP, SETTINGS_SOUNDEVENT_VOICEACTSTOP_DEFAULT);
    ttSettings->setValue(SETTINGS_SOUNDEVENT_VOICEACTMEON, SETTINGS_SOUNDEVENT_VOICEACTMEON_DEFAULT);
    ttSettings->setValue(SETTINGS_SOUNDEVENT_VOICEACTMEOFF, SETTINGS_SOUNDEVENT_VOICEACTMEOFF_DEFAULT);
    ttSettings->setValue(SETTINGS_SOUNDEVENT_INTERCEPT, SETTINGS_SOUNDEVENT_INTERCEPT_DEFAULT);
    ttSettings->setValue(SETTINGS_SOUNDEVENT_INTERCEPTEND, SETTINGS_SOUNDEVENT_INTERCEPTEND_DEFAULT);
    ttSettings->setValue(SETTINGS_SOUNDS_PACK, QCoreApplication::translate("MainWindow", SETTINGS_SOUNDS_PACK_DEFAULT));
}

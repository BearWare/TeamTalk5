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

#include "common.h"
#include "settings.h"
#include "appinfo.h"
#include <math.h>

#include <QSound>
#include <QDateTime>
#include <QDialog>
#include <QStack>

extern QSettings* ttSettings;
extern TTInstance* ttInst;

QString makeCustomCommand(const QString& cmd, const QString& value)
{
    return QString("%1\r\n%2").arg(cmd).arg(value);
}

QStringList getCustomCommand(const TextMessage& msg)
{
    return _Q(msg.szMessage).split("\r\n");
}

void initDefaultAudioCodec(AudioCodec& audiocodec)
{
    audiocodec.nCodec = DEFAULT_AUDIOCODEC;
    switch(DEFAULT_AUDIOCODEC)
    {
    case SPEEX_CODEC :
        audiocodec.speex.nQuality = DEFAULT_SPEEX_QUALITY;
        audiocodec.speex.nBandmode = DEFAULT_SPEEX_BANDMODE;
        audiocodec.speex.nTxIntervalMSec = DEFAULT_SPEEX_DELAY;
        audiocodec.speex.bStereoPlayback = DEFAULT_SPEEX_SIMSTEREO;
        break;
    case SPEEX_VBR_CODEC :
        audiocodec.speex_vbr.nQuality = DEFAULT_SPEEX_VBR_QUALITY;
        audiocodec.speex_vbr.nBandmode = DEFAULT_SPEEX_VBR_BANDMODE;
        audiocodec.speex_vbr.nTxIntervalMSec = DEFAULT_SPEEX_VBR_DELAY;
        audiocodec.speex_vbr.bStereoPlayback = DEFAULT_SPEEX_VBR_SIMSTEREO;
        audiocodec.speex_vbr.nBitRate = DEFAULT_SPEEX_VBR_BITRATE;
        audiocodec.speex_vbr.nMaxBitRate = DEFAULT_SPEEX_VBR_MAXBITRATE;
        audiocodec.speex_vbr.bDTX = DEFAULT_SPEEX_VBR_DTX;
        break;
    case OPUS_CODEC :
        audiocodec.opus.nApplication = DEFAULT_OPUS_APPLICATION;
        audiocodec.opus.nSampleRate = DEFAULT_OPUS_SAMPLERATE;
        audiocodec.opus.nChannels = DEFAULT_OPUS_CHANNELS;
        audiocodec.opus.nTxIntervalMSec = DEFAULT_OPUS_DELAY;
        audiocodec.opus.nComplexity = DEFAULT_OPUS_COMPLEXITY;
        audiocodec.opus.bVBR = DEFAULT_OPUS_VBR;
        audiocodec.opus.bVBRConstraint = DEFAULT_OPUS_VBRCONSTRAINT;
        audiocodec.opus.bDTX = DEFAULT_OPUS_DTX;
        audiocodec.opus.bFEC = DEFAULT_OPUS_FEC;
        audiocodec.opus.nBitRate = DEFAULT_OPUS_BITRATE;
        break;
    default :
        audiocodec.nCodec = NO_CODEC;
        break;
    }
}

bool getVideoCaptureCodec(VideoCodec& vidcodec)
{
    Codec codec = (Codec)ttSettings->value(SETTINGS_VIDCAP_CODEC,
                                           SETTINGS_VIDCAP_CODEC_DEFAULT).toInt();
    vidcodec.nCodec = codec;
    
    switch(vidcodec.nCodec)
    {
    case WEBM_VP8_CODEC :
        vidcodec.webm_vp8.nRcTargetBitrate = ttSettings->value(SETTINGS_VIDCAP_WEBMVP8_BITRATE,
                                                               SETTINGS_VIDCAP_WEBMVP8_BITRATE_DEFAULT).toInt();
        vidcodec.webm_vp8.nEncodeDeadline = DEFAULT_WEBMVP8_DEADLINE;
        break;
    case SPEEX_CODEC :
    case SPEEX_VBR_CODEC :
    case OPUS_CODEC :
    case NO_CODEC :
        break;
    }
    return codec != NO_CODEC;
}

void initDefaultVideoFormat(VideoFormat& vidfmt)
{
    vidfmt.nWidth = DEFAULT_VIDEO_WIDTH;
    vidfmt.nHeight = DEFAULT_VIDEO_HEIGHT;
    vidfmt.nFPS_Numerator = DEFAULT_VIDEO_FPS;
    vidfmt.nFPS_Denominator = 1;
    vidfmt.picFourCC = DEFAULT_VIDEO_FOURCC;
}

void initDefaultAudioPreprocessor(AudioPreprocessor& preprocessor)
{
    switch (preprocessor.nPreprocessor)
    {
    case NO_AUDIOPREPROCESSOR :
        break;
    case SPEEXDSP_AUDIOPREPROCESSOR :
        preprocessor.speexdsp.bEnableAGC = DEFAULT_AGC_ENABLE;
        preprocessor.speexdsp.nGainLevel = DEFAULT_AGC_GAINLEVEL;
        preprocessor.speexdsp.nMaxIncDBSec = DEFAULT_AGC_INC_MAXDB;
        preprocessor.speexdsp.nMaxDecDBSec = DEFAULT_AGC_DEC_MAXDB;
        preprocessor.speexdsp.nMaxGainDB = DEFAULT_AGC_GAINMAXDB;
        preprocessor.speexdsp.bEnableDenoise = DEFAULT_DENOISE_ENABLE;
        preprocessor.speexdsp.nMaxNoiseSuppressDB = DEFAULT_DENOISE_SUPPRESS;
        break;
    case TEAMTALK_AUDIOPREPROCESSOR :
        preprocessor.ttpreprocessor.nGainLevel = SOUND_GAIN_DEFAULT;
        preprocessor.ttpreprocessor.bMuteLeftSpeaker = preprocessor.ttpreprocessor.bMuteRightSpeaker = FALSE;
        break;
    }
}

void loadAudioPreprocessor(AudioPreprocessor& preprocessor)
{
    switch(preprocessor.nPreprocessor)
    {
    case NO_AUDIOPREPROCESSOR:
        break;
    case SPEEXDSP_AUDIOPREPROCESSOR:
        preprocessor.speexdsp.bEnableAGC = ttSettings->value(SETTINGS_STREAMMEDIA_SPX_AGC_ENABLE, DEFAULT_AGC_ENABLE).toBool();
        preprocessor.speexdsp.nGainLevel = ttSettings->value(SETTINGS_STREAMMEDIA_SPX_AGC_GAINLEVEL, DEFAULT_AGC_GAINLEVEL).toInt();
        preprocessor.speexdsp.nMaxIncDBSec = ttSettings->value(SETTINGS_STREAMMEDIA_SPX_AGC_INC_MAXDB, DEFAULT_AGC_INC_MAXDB).toInt();
        preprocessor.speexdsp.nMaxDecDBSec = ttSettings->value(SETTINGS_STREAMMEDIA_SPX_AGC_DEC_MAXDB, DEFAULT_AGC_DEC_MAXDB).toInt();
        preprocessor.speexdsp.nMaxGainDB = ttSettings->value(SETTINGS_STREAMMEDIA_SPX_AGC_GAINMAXDB, DEFAULT_AGC_GAINMAXDB).toInt();
        preprocessor.speexdsp.bEnableDenoise = ttSettings->value(SETTINGS_STREAMMEDIA_SPX_DENOISE_ENABLE, DEFAULT_DENOISE_ENABLE).toBool();
        preprocessor.speexdsp.nMaxNoiseSuppressDB = ttSettings->value(SETTINGS_STREAMMEDIA_SPX_DENOISE_SUPPRESS, DEFAULT_DENOISE_SUPPRESS).toInt();
        break;
    case TEAMTALK_AUDIOPREPROCESSOR:
        preprocessor.ttpreprocessor.bMuteLeftSpeaker = ttSettings->value(SETTINGS_STREAMMEDIA_TTAP_MUTELEFT, false).toBool();
        preprocessor.ttpreprocessor.bMuteRightSpeaker = ttSettings->value(SETTINGS_STREAMMEDIA_TTAP_MUTERIGHT, false).toBool();
        preprocessor.ttpreprocessor.nGainLevel = ttSettings->value(SETTINGS_STREAMMEDIA_TTAP_GAINLEVEL, SOUND_GAIN_DEFAULT).toInt();
        break;
    }
}

bool initVideoCaptureFromSettings()
{
    QString devid = ttSettings->value(SETTINGS_VIDCAP_DEVICEID).toString();

    QStringList fps = ttSettings->value(SETTINGS_VIDCAP_FPS, SETTINGS_VIDCAP_FPS_DEFAULT).toString().split("/");
    QStringList res = ttSettings->value(SETTINGS_VIDCAP_RESOLUTION, SETTINGS_VIDCAP_RESOLUTION_DEFAULT).toString().split("x");
    FourCC fourcc = (FourCC)ttSettings->value(SETTINGS_VIDCAP_FOURCC, SETTINGS_VIDCAP_FOURCC_DEFAULT).toInt();

    if(fps.size() == 2 && res.size() == 2)
    {
        VideoFormat format;

        format.nFPS_Numerator = fps[0].toInt();
        format.nFPS_Denominator = fps[1].toInt();
        format.nWidth = res[0].toInt();
        format.nHeight = res[1].toInt();
        format.picFourCC = fourcc;

        return initVideoCapture(devid, format);
    }
    return false;
}

bool initVideoCapture(const QString& devid, const VideoFormat& fmt)
{
    QString use_devid = devid;
    if(use_devid.isEmpty())
    {
        int count = 1;
        QVector<VideoCaptureDevice> devs(1);
        TT_GetVideoCaptureDevices(&devs[0], &count);
        if(count)
            use_devid = _Q(devs[0].szDeviceID);
    }

    return TT_InitVideoCaptureDevice(ttInst, _W(use_devid), &fmt);
}

bool isValid(const VideoFormat& fmt)
{
    return fmt.nWidth>0 && fmt.nHeight>0 && fmt.nFPS_Numerator>0 && 
        fmt.nFPS_Denominator>0 && fmt.picFourCC != FOURCC_NONE;
}

QVector<SoundDevice> getSoundDevices()
{
    //try getting all sound devices at once
    int count = 25;
    QVector<SoundDevice> result(count);
    TT_GetSoundDevices(&result[0], &count);
    if(result.size() == count)
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

bool getSoundDevice(int deviceid, const QVector<SoundDevice>& devs,
                    SoundDevice& dev)
{
    for(int i=0;i<devs.size();i++)
        if(devs[i].nDeviceID == deviceid)
        {
            dev = devs[i];
            return true;
        }
    return false;
}

bool getSoundDevice(const QString& devid, const QVector<SoundDevice>& devs,
                    SoundDevice& dev)
{
    if(devid.isEmpty())
        return false;

    for(int i=0;i<devs.size();i++)
        if(_Q(devs[i].szDeviceID) == devid)
        {
            dev = devs[i];
            return true;
        }
    return false;
}

int getDefaultSndInputDevice()
{
    SoundSystem sndsys = (SoundSystem)ttSettings->value(SETTINGS_SOUND_SOUNDSYSTEM,
                                                        SOUNDSYSTEM_NONE).toInt();
    int inputid = ttSettings->value(SETTINGS_SOUND_INPUTDEVICE, TT_SOUNDDEVICE_ID_TEAMTALK_VIRTUAL).toInt();
    if(sndsys != SOUNDSYSTEM_NONE)
        TT_GetDefaultSoundDevicesEx(sndsys, &inputid, nullptr);
    else
        TT_GetDefaultSoundDevices(&inputid, nullptr);
    return inputid;
}

int getDefaultSndOutputDevice()
{
    SoundSystem sndsys = (SoundSystem)ttSettings->value(SETTINGS_SOUND_SOUNDSYSTEM,
                                                        SOUNDSYSTEM_NONE).toInt();
    int outputid = ttSettings->value(SETTINGS_SOUND_OUTPUTDEVICE, TT_SOUNDDEVICE_ID_TEAMTALK_VIRTUAL).toInt();
    if(sndsys != SOUNDSYSTEM_NONE)
        TT_GetDefaultSoundDevicesEx(sndsys, nullptr, &outputid);
    else
        TT_GetDefaultSoundDevices(nullptr, &outputid);
    return outputid;
}

int getSoundInputFromUID(int inputid, const QString& uid)
{
    if(uid.isEmpty())
        return inputid;

    QVector<SoundDevice> inputdev = getSoundDevices();

    SoundDevice dev;
    if(getSoundDevice(uid, inputdev, dev))
        inputid = dev.nDeviceID;

    return inputid;
}

int getSoundOutputFromUID(int outputid, const QString& uid)
{
    if(uid.isEmpty())
        return outputid;

    QVector<SoundDevice> outputdev = getSoundDevices();

    SoundDevice dev;
    if(getSoundDevice(uid, outputdev, dev))
        outputid = dev.nDeviceID;
    return outputid;
}

int getSelectedSndInputDevice()
{
    int inputid = ttSettings->value(SETTINGS_SOUND_INPUTDEVICE,
                                    SOUNDDEVICEID_DEFAULT).toInt();
    if(inputid == SOUNDDEVICEID_DEFAULT)
        inputid = getDefaultSndInputDevice();
    else
    {
        //check if device-id has changed since last
        QString uid = ttSettings->value(SETTINGS_SOUND_INPUTDEVICE_UID, "").toString();
        if(uid.size())
            inputid = getSoundInputFromUID(inputid, uid);
    }
    return inputid;
}

int getSelectedSndOutputDevice()
{
    int outputid = ttSettings->value(SETTINGS_SOUND_OUTPUTDEVICE,
                                     SOUNDDEVICEID_DEFAULT).toInt();
    if(outputid == SOUNDDEVICEID_DEFAULT)
        outputid = getDefaultSndOutputDevice();
    else
    {
        //check if device-id has changed since last
        QString uid = ttSettings->value(SETTINGS_SOUND_OUTPUTDEVICE_UID, "").toString();
        if(uid.size())
            outputid = getSoundOutputFromUID(outputid, uid);
    }
    return outputid;
}

QStringList initSelectedSoundDevices()
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
    SoundDevice indev = {};
    getSoundDevice(inputid, devs, indev);

    // toggle sound device effect if it's supported by input device
    SoundDeviceEffects effects = {};
    effects.bEnableAGC = (indev.uSoundDeviceFeatures & SOUNDDEVICEFEATURE_AGC) && ttSettings->value(SETTINGS_SOUND_AGC, SETTINGS_SOUND_AGC_DEFAULT).toBool();
    effects.bEnableDenoise = (indev.uSoundDeviceFeatures & SOUNDDEVICEFEATURE_DENOISE) && ttSettings->value(SETTINGS_SOUND_DENOISING, SETTINGS_SOUND_DENOISING_DEFAULT).toBool();
    effects.bEnableEchoCancellation = (indev.uSoundDeviceFeatures & SOUNDDEVICEFEATURE_AEC) && ttSettings->value(SETTINGS_SOUND_ECHOCANCEL, SETTINGS_SOUND_ECHOCANCEL_DEFAULT).toBool();

    TT_SetSoundDeviceEffects(ttInst, &effects);

    bool duplex = ttSettings->value(SETTINGS_SOUND_DUPLEXMODE, SETTINGS_SOUND_DUPLEXMODE_DEFAULT).toBool();
    // WASAPI has to know which speaker device to echo cancel therefore force duplex mode
    duplex |= (indev.nSoundSystem == SOUNDSYSTEM_WASAPI && (effects.bEnableAGC || effects.bEnableDenoise || effects.bEnableEchoCancellation));

    if (duplex)
    {
        if (!TT_InitSoundDuplexDevices(ttInst, inputid, outputid))
        {
            result.append(QObject::tr("Failed to initialize sound duplex mode"));
        }
    }
    else
    {
        if (!TT_InitSoundInputDevice(ttInst, inputid))
        {
            result.append(QObject::tr("Failed to initialize sound input device"));
        }
        if (!TT_InitSoundOutputDevice(ttInst, outputid))
        {
            result.append(QObject::tr("Failed to initialize sound output device"));
        }
    }
    return result;
}

QStringList initDefaultSoundDevices()
{
    QStringList result;

    TT_CloseSoundInputDevice(ttInst);
    TT_CloseSoundOutputDevice(ttInst);
    TT_CloseSoundDuplexDevices(ttInst);

    result.append(QObject::tr("Switching to default sound devices"));
    int inputid, outputid;
    if (!TT_GetDefaultSoundDevices(&inputid, &outputid))
    {
        result.append(QObject::tr("Unable to get default sound devices"));
    }
    else
    {
        // reset sound device effects
        SoundDeviceEffects effects = {};
        TT_SetSoundDeviceEffects(ttInst, &effects);

        if (!TT_InitSoundInputDevice(ttInst, inputid) || !TT_InitSoundOutputDevice(ttInst, outputid))
        {
            TT_CloseSoundInputDevice(ttInst);
            TT_CloseSoundOutputDevice(ttInst);
            result.append(QObject::tr("Failed to initialize default sound devices"));
        }
    }
    return result;
}

#ifdef Q_OS_DARWIN
QString QCFStringToQString(CFStringRef str)
{
    if(!str)
        return QString();
    CFIndex length = CFStringGetLength(str);
    const UniChar *chars = CFStringGetCharactersPtr(str);
    if (chars)
        return QString(reinterpret_cast<const QChar *>(chars), length);

    QVector<UniChar> buffer(length);
    CFStringGetCharacters(str, CFRangeMake(0, length), buffer.data());
    return QString(reinterpret_cast<const QChar *>(buffer.constData()), length);
}

QString TranslateKey(quint8 vk)
{
    TISInputSourceRef currentKeyboard = TISCopyCurrentKeyboardInputSource();
    const CFDataRef layoutData = 
        reinterpret_cast<const CFDataRef>(TISGetInputSourceProperty(currentKeyboard,
                                          kTISPropertyUnicodeKeyLayoutData) );

    const UCKeyboardLayout *keyboardLayout =
        reinterpret_cast<const UCKeyboardLayout*>(CFDataGetBytePtr(layoutData));

    UInt32 keysDown = 0;
    UniChar chars[10];
    UniCharCount realLength = 0;

    OSStatus oss = UCKeyTranslate(keyboardLayout,
                                  vk,
                                  kUCKeyActionDown,
                                  0,
                                  LMGetKbdType(),
                                  0,//kUCKeyTranslateNoDeadKeysBit,
                                  &keysDown,
                                  sizeof(chars) / sizeof(chars[0]),
                                  &realLength,
                                  chars);
    Q_ASSERT(oss == 0);

    CFStringRef ptr_str = CFStringCreateWithCharacters(kCFAllocatorDefault, 
                                                       chars, (int)realLength);
    QString ss = QCFStringToQString(ptr_str);
    CFRelease(ptr_str);
    CFRelease(currentKeyboard);
    return ss;
}

QString GetMacOSHotKeyText(const hotkey_t& hotkey)
{
    if(hotkey.size() != MAC_HOTKEY_SIZE)
        return QString();

    QString comp;

    if(hotkey[0] != (INT32)MAC_NO_KEY)
    {
        if(hotkey[0] & cmdKey)
            comp += comp.size()? " + Cmd" : "Cmd";
        if(hotkey[0] & shiftKey)
            comp += comp.size()? " + Shift" : "Shift";
        if(hotkey[0] & optionKey)
            comp += comp.size()? " + Option" : "Option";
        if(hotkey[0] & controlKey)
            comp += comp.size()? "+ Ctrl" : "Ctrl";
    }

    QString tmp;
    if(hotkey[1] != (INT32)MAC_NO_KEY)
    {
        quint8 vk = hotkey[1];
        switch(vk)
        {
        case kVK_ANSI_A:
        case kVK_ANSI_S:
        case kVK_ANSI_D:
        case kVK_ANSI_F:
        case kVK_ANSI_H:
        case kVK_ANSI_G:
        case kVK_ANSI_Z:
        case kVK_ANSI_X:
        case kVK_ANSI_C:
        case kVK_ANSI_V:
        case kVK_ANSI_B:
        case kVK_ANSI_Q:
        case kVK_ANSI_W:
        case kVK_ANSI_E:
        case kVK_ANSI_R:
        case kVK_ANSI_Y:
        case kVK_ANSI_T:
        case kVK_ANSI_1:
        case kVK_ANSI_2:
        case kVK_ANSI_3:
        case kVK_ANSI_4:
        case kVK_ANSI_6:
        case kVK_ANSI_5:
        case kVK_ANSI_Equal:
        case kVK_ANSI_9:
        case kVK_ANSI_7:
        case kVK_ANSI_Minus:
        case kVK_ANSI_8:
        case kVK_ANSI_0:
        case kVK_ANSI_RightBracket:
        case kVK_ANSI_O:
        case kVK_ANSI_U:
        case kVK_ANSI_LeftBracket:
        case kVK_ANSI_I:
        case kVK_ANSI_P:
        case kVK_ANSI_L:
        case kVK_ANSI_J:
        case kVK_ANSI_Quote:
        case kVK_ANSI_K:
        case kVK_ANSI_Semicolon:
        case kVK_ANSI_Backslash:
        case kVK_ANSI_Comma:
        case kVK_ANSI_Slash:
        case kVK_ANSI_N:
        case kVK_ANSI_M:
        case kVK_ANSI_Period:
        case kVK_ANSI_Grave:
        case kVK_ANSI_KeypadDecimal:
        case kVK_ANSI_KeypadMultiply:
        case kVK_ANSI_KeypadPlus:
        case kVK_ANSI_KeypadClear:
        case kVK_ANSI_KeypadDivide:
        case kVK_ANSI_KeypadEnter:
        case kVK_ANSI_KeypadMinus:
        case kVK_ANSI_KeypadEquals:
        case kVK_ANSI_Keypad0:
        case kVK_ANSI_Keypad1:
        case kVK_ANSI_Keypad2:
        case kVK_ANSI_Keypad3:
        case kVK_ANSI_Keypad4:
        case kVK_ANSI_Keypad5:
        case kVK_ANSI_Keypad6:
        case kVK_ANSI_Keypad7:
        case kVK_ANSI_Keypad8:
        case kVK_ANSI_Keypad9:
            tmp = TranslateKey(vk);
            break;
        case kVK_Return:
            tmp = "Return";break;
        case kVK_Tab:
            tmp = "Tab";break;
        case kVK_Space:
            tmp = "Space";break;
        case kVK_Delete:
            tmp = "BackSpace";break;
        case kVK_Escape:
            tmp = "Esc";break;
            //case kVK_Command:
            //case kVK_Shift:
        case kVK_CapsLock:
            tmp = "CapsLock";break;
            //case kVK_Option:
            //case kVK_Control:
            //case kVK_RightShift:
            //case kVK_RightOption:
            //case kVK_RightControl:
        case kVK_Function:
            tmp = "Fn";break;
        case kVK_F17:
            tmp = "F17";break;
        case kVK_VolumeUp:
            tmp = "VolUp";break;
        case kVK_VolumeDown:
            tmp = "VolDown";break;
        case kVK_Mute:
            tmp = "Mute";break;
        case kVK_F18: 
            tmp = "F18";break;
        case kVK_F19:     
            tmp = "F19";break;
        case kVK_F20:  
            tmp = "F20";break;
        case kVK_F5: 
            tmp = "F5";break;
        case kVK_F6:
            tmp = "F6";break;
        case kVK_F7:
            tmp = "F7";break;
        case kVK_F3:
            tmp = "F3";break;
        case kVK_F8:
            tmp = "F8";break;
        case kVK_F9:
            tmp = "F9";break;
        case kVK_F11:
            tmp = "F11";break;
        case kVK_F13:
            tmp = "F13";break;
        case kVK_F16:
            tmp = "F16";break;
        case kVK_F14:
            tmp = "F14";break;
        case kVK_F10:
            tmp = "F10";break;
        case kVK_F12:
            tmp = "F12";break;
        case kVK_F15:
            tmp = "F15";break;
        case kVK_Help:
            tmp = "Help";break;
        case kVK_Home:
            tmp = "Home";break;
        case kVK_PageUp:
            tmp = "PgUp";break;
        case kVK_ForwardDelete:
            tmp = "Delete";break;
        case kVK_F4:
            tmp = "F4";break;
        case kVK_End:
            tmp = "End";break;
        case kVK_F2:
            tmp = "F2";break;
        case kVK_PageDown:
            tmp = "PgDown";break;
        case kVK_F1:
            tmp = "F1";break;
        case kVK_LeftArrow:
            tmp = "Left";break;
        case kVK_RightArrow:
            tmp = "Right";break;
        case kVK_DownArrow:
            tmp = "Down";break;
        case kVK_UpArrow:
            tmp = "Up";break;
        default:
            tmp += QString::number(vk);
        }
    }
    comp += comp.size()? " + " + tmp : tmp;
    return comp;
}
#endif


QString getHotKeyText(const hotkey_t& hotkey)
{
#ifdef Q_OS_WIN32
    QString key;
    for(int i=0;i<hotkey.size();i++)
    {
        TTCHAR buff[TT_STRLEN] = {};
        TT_HotKey_GetKeyString(ttInst, hotkey[i], buff);
        key += (i == hotkey.size()-1)? _Q(buff):_Q(buff) + " + ";
    }
    return key;
#elif defined(Q_OS_LINUX)
    Q_UNUSED(ttInst);
    int keys[4] = {0, 0, 0, 0};
    for(int i=0;i<hotkey.size();i++)
        keys[i] = hotkey[i];
        
    QKeySequence keyseq(keys[0], keys[1], keys[2], keys[3]);
    return keyseq.toString();
#elif defined(Q_OS_DARWIN)
    Q_UNUSED(ttInst);
    /*
    QString key;
    if(hotkey.size())
        key = QString::number(hotkey[0]);

    for(int i=1;i<hotkey.size();i++)
        key += " + " + QString::number(hotkey[i]);

    return key;
    */
    return GetMacOSHotKeyText(hotkey);
#else
    return QString("Unknown");
#endif

}


#if defined(Q_OS_WIN32) && !defined(Q_OS_WINCE)
bool isComputerIdle(int idle_secs)
{
    LASTINPUTINFO info;
    info.cbSize = sizeof(LASTINPUTINFO);
    if( GetLastInputInfo(&info))
         return (::GetTickCount() - info.dwTime) / 1000 >= (UINT)idle_secs;
    else
        return false;
}
#elif defined(Q_OS_DARWIN)
#include <IOKit/IOKitLib.h>
bool isComputerIdle(int idle_secs)
{
    int64_t os_idle_secs = 0;
    io_iterator_t iter = 0;
    if (IOServiceGetMatchingServices(kIOMasterPortDefault, 
                                     IOServiceMatching("IOHIDSystem"), 
                                     &iter) == KERN_SUCCESS)
    {
        io_registry_entry_t entry = IOIteratorNext(iter);
        if (entry)
        {
            CFMutableDictionaryRef dict = nullptr;
            if (IORegistryEntryCreateCFProperties(entry, &dict, 
                                                  kCFAllocatorDefault, 
                                                  0) == KERN_SUCCESS)
            {
                CFNumberRef obj = static_cast<CFNumberRef>
                (CFDictionaryGetValue(dict, 
                                      CFSTR("HIDIdleTime")));
                if (obj)
                {
                    int64_t nanoseconds = 0;
                    if (CFNumberGetValue(obj, kCFNumberSInt64Type, 
                                         &nanoseconds))
                    {
// Divide by 10^9 to convert from nanoseconds to seconds.
                        os_idle_secs = (nanoseconds >> 30); 
                    }
                }
            }
            IOObjectRelease(entry);
        }
        IOObjectRelease(iter);
    }
    return (int)os_idle_secs > idle_secs;
}
#else
bool isComputerIdle(int /*idle_secs*/)
{
    return false;
}
#endif

bool isMyselfTalking()
{
    bool talking = false;
    ClientFlags flags = TT_GetFlags(ttInst);
    talking = (flags & CLIENT_TX_VOICE) ||
              ((flags & CLIENT_SNDINPUT_VOICEACTIVATED) &&
              (flags & CLIENT_SNDINPUT_VOICEACTIVE));
    return talking;
}

bool isMyselfStreaming()
{
    return (TT_GetFlags(ttInst) & (CLIENT_STREAM_AUDIO | CLIENT_STREAM_VIDEO)) != CLIENT_CLOSED;
}

QString getHotKeyString(HotKeyID keyid)
{
    switch(keyid)
    {
    case HOTKEY_PUSHTOTALK :
        return SETTINGS_GENERAL_PUSHTOTALK_KEY;
    case HOTKEY_VOICEACTIVATION :
        return SETTINGS_SHORTCUTS_VOICEACTIVATION;
    case HOTKEY_INCVOLUME :
        return SETTINGS_SHORTCUTS_INCVOLUME;
    case HOTKEY_DECVOLUME :
        return SETTINGS_SHORTCUTS_DECVOLUME;
    case HOTKEY_MUTEALL :
        return SETTINGS_SHORTCUTS_MUTEALL;
    case HOTKEY_MICROPHONEGAIN_INC :
        return SETTINGS_SHORTCUTS_INCVOICEGAIN;
    case HOTKEY_MICROPHONEGAIN_DEC :
        return SETTINGS_SHORTCUTS_DECVOICEGAIN;
    case HOTKEY_VIDEOTX :
        return SETTINGS_SHORTCUTS_VIDEOTX;
    default :
        Q_ASSERT(0); //unknown hotkey id
    }
    return QString();
}

void saveHotKeySettings(HotKeyID hotkeyid, const hotkey_t& hotkey)
{
    QStringList hklst;
    for(int i=0;i<hotkey.size();i++)
        hklst.push_back(QString::number(hotkey[i]));
    ttSettings->setValue(getHotKeyString(hotkeyid), hklst);
}

bool loadHotKeySettings(HotKeyID hotkeyid, hotkey_t& hotkey)
{
    QStringList hklst = ttSettings->value(getHotKeyString(hotkeyid)).toStringList();
    for(int i=0;i<hklst.size();i++)
        hotkey.push_back(hklst[i].toInt());
    return hklst.size();
}

void deleteHotKeySettings(HotKeyID hotkeyid)
{
    ttSettings->remove(getHotKeyString(hotkeyid));
}

void saveVideoFormat(const VideoFormat& vidfmt)
{
    QString resolution, fps;
    resolution = QString("%1x%2").arg(vidfmt.nWidth).arg(vidfmt.nHeight);
    fps = QString("%1/%2").arg(vidfmt.nFPS_Numerator).arg(vidfmt.nFPS_Denominator);

    ttSettings->setValue(SETTINGS_VIDCAP_RESOLUTION, resolution);
    ttSettings->setValue(SETTINGS_VIDCAP_FPS, fps);
    ttSettings->setValue(SETTINGS_VIDCAP_FOURCC, (int)vidfmt.picFourCC);
}

bool loadVideoFormat(VideoFormat& vidfmt)
{
    QStringList fps_tokens = ttSettings->value(SETTINGS_VIDCAP_FPS, "0/0").toString().split("/");
    QStringList res_tokens = ttSettings->value(SETTINGS_VIDCAP_RESOLUTION, "0x0").toString().split("x");
    if(fps_tokens.size() == 2 && res_tokens.size() == 2 &&
       fps_tokens[0].toInt() && fps_tokens[1].toInt() && 
       res_tokens[0].toInt() && res_tokens[1].toInt())
    {
        vidfmt.nFPS_Numerator = fps_tokens[0].toInt();
        vidfmt.nFPS_Denominator = fps_tokens[1].toInt();
        vidfmt.nWidth = res_tokens[0].toInt();
        vidfmt.nHeight = res_tokens[1].toInt();
        vidfmt.picFourCC = (FourCC)ttSettings->value(SETTINGS_VIDCAP_FOURCC, 0).toInt();
        return true;
    }
    return false;
}

void playSoundEvent(SoundEvent event)
{
    QString filename;
    switch(event)
    {
    case SOUNDEVENT_NEWUSER:
        filename = ttSettings->value(SETTINGS_SOUNDEVENT_NEWUSER).toString();
        break;
    case SOUNDEVENT_REMOVEUSER:
        filename = ttSettings->value(SETTINGS_SOUNDEVENT_REMOVEUSER).toString();
        break;
    case SOUNDEVENT_SERVERLOST:
        filename = ttSettings->value(SETTINGS_SOUNDEVENT_SERVERLOST).toString();
        break;
    case SOUNDEVENT_USERMSG:
        filename = ttSettings->value(SETTINGS_SOUNDEVENT_USERMSG).toString();
        break;
    case SOUNDEVENT_CHANNELMSG:
        filename = ttSettings->value(SETTINGS_SOUNDEVENT_CHANNELMSG).toString();
        break;
    case SOUNDEVENT_BROADCASTMSG :
        filename = ttSettings->value(SETTINGS_SOUNDEVENT_BROADCASTMSG).toString();
        break;
    case SOUNDEVENT_HOTKEY:
        filename = ttSettings->value(SETTINGS_SOUNDEVENT_HOTKEY).toString();
        break;
    case SOUNDEVENT_SILENCE:   
        filename = ttSettings->value(SETTINGS_SOUNDEVENT_SILENCE).toString();
        break;
    case SOUNDEVENT_NEWVIDEO:   
        filename = ttSettings->value(SETTINGS_SOUNDEVENT_NEWVIDEO).toString();
        break;
    case SOUNDEVENT_NEWDESKTOP:   
        filename = ttSettings->value(SETTINGS_SOUNDEVENT_NEWDESKTOP).toString();
        break;
    case SOUNDEVENT_FILESUPD:  
        filename = ttSettings->value(SETTINGS_SOUNDEVENT_FILESUPD).toString();
        break;
    case SOUNDEVENT_FILETXDONE:
        filename = ttSettings->value(SETTINGS_SOUNDEVENT_FILETXDONE).toString();
        break;
    case SOUNDEVENT_QUESTIONMODE:
        filename = ttSettings->value(SETTINGS_SOUNDEVENT_QUESTIONMODE).toString();
        break;
    case SOUNDEVENT_DESKTOPACCESS:
        filename = ttSettings->value(SETTINGS_SOUNDEVENT_DESKTOPACCESS).toString();
        break;
    }
    if(filename.size())
        QSound::play(filename);
}

void addLatestHost(const HostEntry& host)
{
    QList<HostEntry> hosts;
    HostEntry tmp;
    int index = 0;
    while(getLatestHost(index, tmp))
    {
        hosts.push_back(tmp);
        tmp = HostEntry();
        index++;
    }
    for(int i=0;i<hosts.size();)
    {
        if(hosts[i].ipaddr == host.ipaddr &&
            hosts[i].tcpport == host.tcpport && 
            hosts[i].udpport == host.udpport &&
            /*hosts[i].srvpasswd == host.srvpasswd &&*/ //don't include passwords
            hosts[i].username == host.username &&
            /*hosts[i].password == host.password &&*/
            hosts[i].channel == host.channel/* &&
            hosts[i].chanpasswd == host.chanpasswd*/)
        {
            hosts.erase(hosts.begin()+i);
        }
        else i++;
    }
    hosts.insert(0, host);
    while(hosts.size()>5)hosts.removeLast();

    for(int i=0;i<hosts.size();i++)
    {
        ttSettings->setValue(QString(SETTINGS_LATESTHOST_HOSTADDR).arg(i), hosts[i].ipaddr);
        ttSettings->setValue(QString(SETTINGS_LATESTHOST_TCPPORT).arg(i), hosts[i].tcpport);
        ttSettings->setValue(QString(SETTINGS_LATESTHOST_UDPPORT).arg(i), hosts[i].udpport);
        ttSettings->setValue(QString(SETTINGS_LATESTHOST_ENCRYPTED).arg(i), hosts[i].encrypted);
        ttSettings->setValue(QString(SETTINGS_LATESTHOST_USERNAME).arg(i), hosts[i].username); 
        ttSettings->setValue(QString(SETTINGS_LATESTHOST_PASSWORD).arg(i), hosts[i].password); 
        ttSettings->setValue(QString(SETTINGS_LATESTHOST_CHANNEL).arg(i), hosts[i].channel); 
        ttSettings->setValue(QString(SETTINGS_LATESTHOST_CHANNELPASSWD).arg(i), hosts[i].chanpasswd); 
    }
}

void deleteLatestHost(int index)
{
    QStack<HostEntry> hosts;
    HostEntry tmp;
    int i = 0;
    while(getLatestHost(i, tmp))
    {
        hosts.push(tmp);
        i++;
        tmp = HostEntry();
    }
    
    for(i=0;i<hosts.size();i++)
    {
        ttSettings->remove(QString(SETTINGS_LATESTHOST_HOSTADDR).arg(i));
        ttSettings->remove(QString(SETTINGS_LATESTHOST_TCPPORT).arg(i));
        ttSettings->remove(QString(SETTINGS_LATESTHOST_UDPPORT).arg(i));
        ttSettings->remove(QString(SETTINGS_LATESTHOST_ENCRYPTED).arg(i));
        ttSettings->remove(QString(SETTINGS_LATESTHOST_USERNAME).arg(i));
        ttSettings->remove(QString(SETTINGS_LATESTHOST_PASSWORD).arg(i));
        ttSettings->remove(QString(SETTINGS_LATESTHOST_CHANNEL).arg(i));
        ttSettings->remove(QString(SETTINGS_LATESTHOST_CHANNELPASSWD).arg(i));
    }

    i=0;
    while(!hosts.isEmpty())
    {
        if(i != index)
            addLatestHost(hosts.pop());
        else
            hosts.pop();
        i++;
    }
}

bool getLatestHost(int index, HostEntry& host)
{
    host.ipaddr = ttSettings->value(QString(SETTINGS_LATESTHOST_HOSTADDR).arg(index)).toString();
    host.tcpport = ttSettings->value(QString(SETTINGS_LATESTHOST_TCPPORT).arg(index)).toInt();
    host.udpport = ttSettings->value(QString(SETTINGS_LATESTHOST_UDPPORT).arg(index)).toInt();
    host.encrypted = ttSettings->value(QString(SETTINGS_LATESTHOST_ENCRYPTED).arg(index), false).toBool();
    host.username = ttSettings->value(QString(SETTINGS_LATESTHOST_USERNAME).arg(index)).toString();
    host.password = ttSettings->value(QString(SETTINGS_LATESTHOST_PASSWORD).arg(index)).toString();
    host.channel = ttSettings->value(QString(SETTINGS_LATESTHOST_CHANNEL).arg(index)).toString();
    host.chanpasswd = ttSettings->value(QString(SETTINGS_LATESTHOST_CHANNELPASSWD).arg(index)).toString();
    return host.ipaddr.size();
}

void addServerEntry(const HostEntry& host)
{
    QList<HostEntry> hosts;
    HostEntry tmp;
    int index = 0;
    while(getServerEntry(index, tmp))
    {
        hosts.push_back(tmp);
        tmp = HostEntry();
        index++;
    }
    hosts.append(host);

    for(int i=0;i<hosts.size();i++)
        setServerEntry(i, hosts[i]);
}

void setServerEntry(int index, const HostEntry& host)
{
    ttSettings->setValue(QString(SETTINGS_SERVERENTRIES_NAME).arg(index), host.name);
    ttSettings->setValue(QString(SETTINGS_SERVERENTRIES_HOSTADDR).arg(index), host.ipaddr);
    ttSettings->setValue(QString(SETTINGS_SERVERENTRIES_TCPPORT).arg(index), host.tcpport);  
    ttSettings->setValue(QString(SETTINGS_SERVERENTRIES_UDPPORT).arg(index), host.udpport);  
    ttSettings->setValue(QString(SETTINGS_SERVERENTRIES_ENCRYPTED).arg(index), host.encrypted);
    ttSettings->setValue(QString(SETTINGS_SERVERENTRIES_USERNAME).arg(index), host.username); 
    ttSettings->setValue(QString(SETTINGS_SERVERENTRIES_PASSWORD).arg(index), host.password); 
    ttSettings->setValue(QString(SETTINGS_SERVERENTRIES_CHANNEL).arg(index), host.channel); 
    ttSettings->setValue(QString(SETTINGS_SERVERENTRIES_CHANNELPASSWD).arg(index), host.chanpasswd); 
}

bool getServerEntry(int index, HostEntry& host)
{
    host.name = ttSettings->value(QString(SETTINGS_SERVERENTRIES_NAME).arg(index)).toString();
    host.ipaddr = ttSettings->value(QString(SETTINGS_SERVERENTRIES_HOSTADDR).arg(index)).toString();
    host.tcpport = ttSettings->value(QString(SETTINGS_SERVERENTRIES_TCPPORT).arg(index)).toInt();
    host.udpport = ttSettings->value(QString(SETTINGS_SERVERENTRIES_UDPPORT).arg(index)).toInt();
    host.encrypted = ttSettings->value(QString(SETTINGS_SERVERENTRIES_ENCRYPTED).arg(index), false).toBool();
    host.username = ttSettings->value(QString(SETTINGS_SERVERENTRIES_USERNAME).arg(index)).toString();
    host.password = ttSettings->value(QString(SETTINGS_SERVERENTRIES_PASSWORD).arg(index)).toString();
    host.channel = ttSettings->value(QString(SETTINGS_SERVERENTRIES_CHANNEL).arg(index)).toString();
    host.chanpasswd = ttSettings->value(QString(SETTINGS_SERVERENTRIES_CHANNELPASSWD).arg(index)).toString();
    return host.name.size();
}

void deleteServerEntry(const QString& name)
{
    QList<HostEntry> hosts;
    HostEntry tmp;
    int index = 0;
    while(getServerEntry(index, tmp))
    {
        if(tmp.name != name)
            hosts.push_back(tmp);
        ttSettings->remove(QString(SETTINGS_SERVERENTRIES_NAME).arg(index));
        ttSettings->remove(QString(SETTINGS_SERVERENTRIES_HOSTADDR).arg(index));
        ttSettings->remove(QString(SETTINGS_SERVERENTRIES_TCPPORT).arg(index));
        ttSettings->remove(QString(SETTINGS_SERVERENTRIES_UDPPORT).arg(index));
        ttSettings->remove(QString(SETTINGS_SERVERENTRIES_ENCRYPTED).arg(index));
        ttSettings->remove(QString(SETTINGS_SERVERENTRIES_USERNAME).arg(index));
        ttSettings->remove(QString(SETTINGS_SERVERENTRIES_PASSWORD).arg(index));
        ttSettings->remove(QString(SETTINGS_SERVERENTRIES_CHANNEL).arg(index));
        ttSettings->remove(QString(SETTINGS_SERVERENTRIES_CHANNELPASSWD).arg(index));
        index++;
        tmp = HostEntry();
    }

    for(int i=0;i<hosts.size();i++)
        setServerEntry(i, hosts[i]);
}

bool getServerEntry(const QDomElement& hostElement, HostEntry& entry)
{
    Q_ASSERT(hostElement.tagName() == "host");
    bool ok = true;
    QDomElement tmp = hostElement.firstChildElement("name");
    if(!tmp.isNull())
        entry.name = tmp.text();
    else ok = false;

    tmp = hostElement.firstChildElement("address");
    if(!tmp.isNull())
        entry.ipaddr = tmp.text();
    else ok = false;

    tmp = hostElement.firstChildElement("tcpport");
    if(!tmp.isNull())
        entry.tcpport = tmp.text().toInt();
    else ok = false;

    tmp = hostElement.firstChildElement("udpport");
    if(!tmp.isNull())
        entry.udpport = tmp.text().toInt();
    else ok = false;

    tmp = hostElement.firstChildElement("encrypted");
    if(!tmp.isNull())
        entry.encrypted = (tmp.text().toLower() == "true" || tmp.text() == "1");

    QDomElement auth = hostElement.firstChildElement("auth");
    if(!auth.isNull())
    {
        tmp = auth.firstChildElement("username");
        if(!tmp.isNull())
            entry.username = tmp.text();

        tmp = auth.firstChildElement("password");
        if(!tmp.isNull())
            entry.password = tmp.text();
    }

    QDomElement join = hostElement.firstChildElement("join");
    if(!join.isNull())
    {
        tmp = join.firstChildElement("channel");
        if(!tmp.isNull())
            entry.channel = tmp.text();
        tmp = join.firstChildElement("password");
        if(!tmp.isNull())
            entry.chanpasswd = tmp.text();
    }

    QDomElement client = hostElement.firstChildElement(CLIENTSETUP_TAG);
    if(!client.isNull())
    {
        tmp = client.firstChildElement("nickname");
        if(!tmp.isNull())
            entry.nickname = tmp.text();
        tmp = client.firstChildElement("gender");
        if(!tmp.isNull())
            entry.gender = tmp.text().toInt() == GENDER_FEMALE? GENDER_FEMALE : GENDER_MALE;
#if defined(Q_OS_WIN32)
        tmp = client.firstChildElement("win-hotkey");
#elif defined(Q_OS_DARWIN)
        tmp = client.firstChildElement("mac-hotkey");
#elif defined(Q_OS_LINUX)
        tmp = client.firstChildElement("x11-hotkey");
#else
#error Unknown OS
#endif
        if(!tmp.isNull())
        {
            tmp = tmp.firstChildElement("key");
            while(!tmp.isNull())
            {
                entry.hotkey.push_back(tmp.text().toInt());
                tmp = tmp.nextSiblingElement("key");
            }
        }
        tmp = client.firstChildElement("voice-activated");
        if(!tmp.isNull())
            entry.voiceact = tmp.text().toInt();
        tmp = client.firstChildElement("videoformat");
        if(!tmp.isNull())
        {
            QDomElement cap = tmp.firstChildElement("width");
            if(!cap.isNull())
                entry.capformat.nWidth = cap.text().toInt();
            cap = tmp.firstChildElement("height");
            if(!cap.isNull())
                entry.capformat.nHeight = cap.text().toInt();
            
            cap = tmp.firstChildElement("fps-numerator");
            if(!cap.isNull())
                entry.capformat.nFPS_Numerator = cap.text().toInt();
            cap = tmp.firstChildElement("fps-denominator");
            if(!cap.isNull())
                entry.capformat.nFPS_Denominator = cap.text().toInt();

            cap = tmp.firstChildElement("fourcc");
            if(!cap.isNull())
                entry.capformat.picFourCC = (FourCC)cap.text().toInt();
        }
        tmp = client.firstChildElement("videocodec");
        if(!tmp.isNull())
        {
            QDomElement vid = tmp.firstChildElement("codec");
            if(!vid.isNull())
                entry.vidcodec.nCodec = (Codec)vid.text().toInt();
            switch(entry.vidcodec.nCodec)
            {
            case WEBM_VP8_CODEC :
                vid = tmp.firstChildElement("webm-vp8-bitrate");
                if(!vid.isNull())
                {
                    entry.vidcodec.webm_vp8.nRcTargetBitrate = vid.text().toInt();
                    entry.vidcodec.webm_vp8.nEncodeDeadline = DEFAULT_WEBMVP8_DEADLINE;
                }
                break;
            case SPEEX_CODEC :
            case SPEEX_VBR_CODEC :
            case OPUS_CODEC :
            case NO_CODEC :
                break;
            }
        }
    }
    return ok;
}

void addDesktopAccessEntry(const DesktopAccessEntry& entry)
{
    QVector<DesktopAccessEntry> entries;
    getDesktopAccessList(entries);
    entries.push_back(entry);

    for(int c=0;c<entries.size();c++)
    {
        ttSettings->setValue(QString(SETTINGS_DESKTOPACCESS_HOSTADDR).arg(c),
                             entries[c].ipaddr);
        ttSettings->setValue(QString(SETTINGS_DESKTOPACCESS_TCPPORT).arg(c),
                             entries[c].tcpport);
        for(int i=0;i<entries[c].channels.size();i++)
            ttSettings->setValue(QString(SETTINGS_DESKTOPACCESS_CHANNEL).arg(c).arg(i),
                                 entries[c].channels[i]);
        for(int i=0;i<entries[c].usernames.size();i++)
            ttSettings->setValue(QString(SETTINGS_DESKTOPACCESS_USERNAME).arg(c).arg(i),
                                 entries[c].usernames[i]);
    }
}

void getDesktopAccessList(QVector<DesktopAccessEntry>& entries)
{
    int c = 0;
    while(ttSettings->value(QString(SETTINGS_DESKTOPACCESS_HOSTADDR).arg(c)).toString().size())
    {
        DesktopAccessEntry entry;
        entry.ipaddr = ttSettings->value(QString(SETTINGS_DESKTOPACCESS_HOSTADDR).arg(c)).toString();
        entry.tcpport = ttSettings->value(QString(SETTINGS_DESKTOPACCESS_TCPPORT).arg(c)).toInt();

        int i = 0;
        while(ttSettings->value(QString(SETTINGS_DESKTOPACCESS_CHANNEL).arg(c).arg(i)).toString().size())
        {
            entry.channels.push_back(ttSettings->value(QString(SETTINGS_DESKTOPACCESS_CHANNEL).arg(c).arg(i)).toString());
            i++;
        }
        i = 0;
        while(ttSettings->value(QString(SETTINGS_DESKTOPACCESS_USERNAME).arg(c).arg(i)).toString().size())
        {
            entry.usernames.push_back(ttSettings->value(QString(SETTINGS_DESKTOPACCESS_USERNAME).arg(c).arg(i)).toString());
            i++;
        }
        entries.push_back(entry);
        c++;
    }
}

void getDesktopAccessList(QVector<DesktopAccessEntry>& entries,
                          const QString& ipaddr, int tcpport)
{
    QVector<DesktopAccessEntry> tmp;
    getDesktopAccessList(tmp);
    foreach(DesktopAccessEntry entry, tmp)
        if(entry.ipaddr == ipaddr && tcpport == entry.tcpport)
            entries.push_back(entry);
}

bool hasDesktopAccess(const QVector<DesktopAccessEntry>& entries,
                      const User& user)
{
    QString username = _Q(user.szUsername);
    for(int i=0;i<entries.size();i++)
    {
        if(username.size() && entries[i].usernames.size() &&
            entries[i].usernames.contains(username, Qt::CaseInsensitive))
            return true;

        //channel match must be done with TT-API since path might not 
        //match exactly, i.e. case-insensitive, pre '/' and post '/'
        if(user.nChannelID>0 && entries[i].channels.size())
        {
            for(int j=0;j<entries[i].channels.size();j++)
            {
                int chanid = TT_GetChannelIDFromPath(ttInst,
                                                     _W(entries[i].channels[j]));
                if(user.nChannelID > 0 && chanid == user.nChannelID)
                    return true;
            }
        }
    }
    return false;
}

void deleteDesktopAccessEntries()
{
    QVector<DesktopAccessEntry> entries;
    getDesktopAccessList(entries);

    for(int c=0;c<entries.size();c++)
    {
        ttSettings->remove(QString(SETTINGS_DESKTOPACCESS_HOSTADDR).arg(c));
        ttSettings->remove(QString(SETTINGS_DESKTOPACCESS_TCPPORT).arg(c));

        for(int i=0;i<entries[c].channels.size();i++)
            ttSettings->remove(QString(SETTINGS_DESKTOPACCESS_CHANNEL).arg(c).arg(i));
        for(int i=0;i<entries[c].usernames.size();i++)
            ttSettings->remove(QString(SETTINGS_DESKTOPACCESS_USERNAME).arg(c).arg(i));
    }
}

QString parseXML(const QDomDocument& doc, QString elements)
{
    QDomElement element(doc.documentElement());
    QStringList tokens = elements.split("/");
    while (tokens.size())
    {
        if (element.isNull() || element.nodeName() != tokens[0])
            return QString();

        tokens.removeFirst();
        if (tokens.isEmpty())
            return element.text();

        element = element.firstChildElement(tokens[0]);
    }
    return QString();
}

QString newVersionAvailable(const QDomDocument& updateDoc)
{
    return parseXML(updateDoc, "teamtalk/update/name");
    //if (element.nodeName() == "teamtalk")
    //{
    //    element = element.firstChildElement("update");
    //    if(!element.isNull())
    //    {
    //        element = element.firstChildElement("name");
    //        if (!element.isNull())
    //            return element.text();
    //    }
    //}
    //return QString();
}

QString getBearWareRegistrationUrl(const QDomDocument& doc)
{
    return parseXML(doc, "teamtalk/bearware/register-url");
}

QByteArray generateTTFile(const HostEntry& entry)
{
    QDomDocument doc(TTFILE_ROOT);
    QDomProcessingInstruction pi = doc.createProcessingInstruction("xml", "version=\"1.0\" encoding=\"UTF-8\" ");
    doc.appendChild(pi);

    QDomElement root = doc.createElement(TTFILE_ROOT);
    root.setAttribute("version", TTFILE_VERSION);
    doc.appendChild(root);

    QDomElement host = doc.createElement("host");

    QDomElement name = doc.createElement("name");
    name.appendChild(doc.createTextNode(entry.name));

    QDomElement address = doc.createElement("address");
    address.appendChild(doc.createTextNode(entry.ipaddr));

    QDomElement tcpport = doc.createElement("tcpport");
    tcpport.appendChild(doc.createTextNode(QString::number(entry.tcpport)));

    QDomElement udpport = doc.createElement("udpport");
    udpport.appendChild(doc.createTextNode(QString::number(entry.udpport)));

    QDomElement encrypted = doc.createElement("encrypted");
    encrypted.appendChild(doc.createTextNode(entry.encrypted?"true":"false"));

    host.appendChild(name);
    host.appendChild(address);
    host.appendChild(tcpport);
    host.appendChild(udpport);
    host.appendChild(encrypted);

    if(entry.username.size())
    {
        QDomElement auth = doc.createElement("auth");

        QDomElement username = doc.createElement("username");
        username.appendChild(doc.createTextNode(entry.username));

        QDomElement password = doc.createElement("password");
        password.appendChild(doc.createTextNode(entry.password));

        auth.appendChild(username);
        auth.appendChild(password);

        host.appendChild(auth);
    }

    if(entry.channel.size())
    {
        QDomElement join = doc.createElement("join");

        QDomElement channel = doc.createElement("channel");
        channel.appendChild(doc.createTextNode(entry.channel));

        QDomElement password = doc.createElement("password");
        password.appendChild(doc.createTextNode(entry.chanpasswd));

        join.appendChild(channel);
        join.appendChild(password);

        host.appendChild(join);
    }

    QDomElement client = doc.createElement(CLIENTSETUP_TAG);

    if(entry.nickname.size())
    {
        QDomElement nickname = doc.createElement("nickname");
        nickname.appendChild(doc.createTextNode(entry.nickname));
        client.appendChild(nickname);
    }
    if(entry.gender != GENDER_NONE)
    {
        QDomElement gender = doc.createElement("gender");
        gender.appendChild(doc.createTextNode(QString::number(entry.gender)));
        client.appendChild(gender);
    }
    if(entry.hotkey.size())
    {
#if defined(Q_OS_WIN32)
        QDomElement hotkey = doc.createElement("win-hotkey");
#elif defined(Q_OS_DARWIN)
        QDomElement hotkey = doc.createElement("mac-hotkey");
#elif defined(Q_OS_LINUX)
        QDomElement hotkey = doc.createElement("x11-hotkey");
#else
#error Unknown OS
#endif
        foreach(int k, entry.hotkey)
        {
            QDomElement key = doc.createElement("key");
            key.appendChild(doc.createTextNode(QString::number(k)));
            hotkey.appendChild(key);
        }
        client.appendChild(hotkey);
    }
    if(entry.voiceact >= 0)
    {
        QDomElement vox = doc.createElement("voice-activated");
        vox.appendChild(doc.createTextNode(QString::number(entry.voiceact)));
        client.appendChild(vox);
    }
    if(entry.capformat.nWidth)
    {
        QDomElement cap = doc.createElement("videoformat");

        QDomElement newElement = doc.createElement("width");
        newElement.appendChild(doc.createTextNode(QString::number(entry.capformat.nWidth)));
        cap.appendChild(newElement);
        newElement = doc.createElement("height");
        newElement.appendChild(doc.createTextNode(QString::number(entry.capformat.nHeight)));
        cap.appendChild(newElement);

        newElement = doc.createElement("fps-numerator");
        newElement.appendChild(doc.createTextNode(QString::number(entry.capformat.nFPS_Numerator)));
        cap.appendChild(newElement);
        newElement = doc.createElement("fps-denominator");
        newElement.appendChild(doc.createTextNode(QString::number(entry.capformat.nFPS_Denominator)));
        cap.appendChild(newElement);

        newElement = doc.createElement("fourcc");
        newElement.appendChild(doc.createTextNode(QString::number(entry.capformat.picFourCC)));
        cap.appendChild(newElement);

        client.appendChild(cap);
    }
    if(entry.vidcodec.nCodec != NO_CODEC)
    {
        QDomElement vidcodec = doc.createElement("videocodec");

        QDomElement newElement = doc.createElement("codec");
        newElement.appendChild(doc.createTextNode(QString::number(entry.vidcodec.nCodec)));
        vidcodec.appendChild(newElement);
        switch(entry.vidcodec.nCodec)
        {
        case WEBM_VP8_CODEC :
            newElement = doc.createElement("webm-vp8-bitrate");
            newElement.appendChild(doc.createTextNode(QString::number(entry.vidcodec.webm_vp8.nRcTargetBitrate)));
            vidcodec.appendChild(newElement);
            break;
        case SPEEX_CODEC :
        case SPEEX_VBR_CODEC :
        case OPUS_CODEC :
        case NO_CODEC :
            break;
        }
        client.appendChild(vidcodec);
    }

    if(client.hasChildNodes())
        host.appendChild(client);

    root.appendChild(host);

    return doc.toByteArray();
}

void setVolume(int userid, int vol_diff, StreamType stream_type)
{
    User user;
    if(TT_GetUser(ttInst, userid, &user))
    {
        int vol = 0;
        switch(stream_type)
        {
        case STREAMTYPE_VOICE :
            vol = refVolumeToPercent(user.nVolumeVoice);
            vol = refVolume(vol + vol_diff);
            break;
        case STREAMTYPE_MEDIAFILE_AUDIO :
            vol = refVolumeToPercent(user.nVolumeMediaFile);
            vol = refVolume(vol + vol_diff);
            break;
        default :
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
    if(percent == 0)
        return 0;
    double d = 82.832 * exp(0.0508 * percent) - 50;
    return d;
}

int refVolumeToPercent(int volume)
{
    if(volume == 0)
        return 0;

    double d = (volume + 50) / 82.832;
    d = log(d) / 0.0508;
    return d + .5;
}

int refGain(double percent)
{
    if(percent == 0)
        return 0;

    return  82.832 * exp(0.0508 * percent) - 50;
}

bool versionSameOrLater(const QString& check, const QString& against)
{
    if(check == against) return true;

    QStringList chk_tokens = check.split(".");
    QStringList against_tokens = against.split(".");

    QVector<int> vec_chk, vec_against;
    for(int i=0;i<chk_tokens.size();i++)
        vec_chk.push_back(chk_tokens[i].toInt());
    for(int i=0;i<against_tokens.size();i++)
        vec_against.push_back(against_tokens[i].toInt());

    int less = vec_chk.size() < vec_against.size()?vec_chk.size():vec_against.size();
    
    for(int i=0;i<less;i++)
        if(vec_chk[i] < vec_against[i])
            return false;
        else if(vec_chk[i] > vec_against[i])
            return true;

    return true;
}

QString getVersion(const User& user)
{
    return QString("%1.%2.%3")
        .arg(user.uVersion >> 16)
        .arg((user.uVersion >> 8) & 0xFF)
        .arg(user.uVersion & 0xFF);
}

QString limitText(const QString& text)
{
    int len = ttSettings->value(SETTINGS_DISPLAY_MAX_STRING, DEFAULT_MAX_STRING_LENGTH).toInt();
    if(text.size()>len+3)
        return text.left(len) + "...";
    return text;
}

QString getDisplayName(const User& user)
{
    if(ttSettings->value(SETTINGS_DISPLAY_SHOWUSERNAME,
                         SETTINGS_DISPLAY_SHOWUSERNAME_DEFAULT).toBool())
    {
        return limitText(_Q(user.szUsername));
    }

    QString nickname = _Q(user.szNickname);
    if (nickname.isEmpty())
        nickname = QString("%1 - #%2").arg(SETTINGS_GENERAL_NICKNAME_DEFAULT).arg(user.nUserID);
    return limitText(nickname);
}

QString getDateTimeStamp()
{
    return QDateTime::currentDateTime().toString("yyyyMMdd-hhmmss");
}

QString generateAudioStorageFilename(AudioFileFormat aff)
{
    QString filename = getDateTimeStamp() + " ";
    filename += QObject::tr("Conference");
    switch(aff)
    {
    case AFF_WAVE_FORMAT :
        filename += ".wav";
        break;
    case AFF_CHANNELCODEC_FORMAT :
        filename += ".ogg";
        break;
    case AFF_MP3_16KBIT_FORMAT :
    case AFF_MP3_32KBIT_FORMAT :
    case AFF_MP3_64KBIT_FORMAT :
    case AFF_MP3_128KBIT_FORMAT :
    case AFF_MP3_256KBIT_FORMAT :
        filename += ".mp3";
        break;
    case AFF_NONE :
        break;
    }
    return filename;
}


QString generateLogFileName(const QString& name)
{
    static QString invalidPath("?:*\"|<>/\\");

    QString filename = getDateTimeStamp();
    filename += " - " + name;
    for(int i=0;i<filename.size();i++)
    {
        if(invalidPath.contains(filename[i]))
            filename[i] = '_';
    }
    return filename;
}

bool openLogFile(QFile& file, const QString& folder, const QString& name)
{
    if(file.isOpen())
        file.close();

    QString filename = folder + "/";
    filename += generateLogFileName(name);

    file.setFileName(filename);
    return file.open(QFile::WriteOnly | QFile::Append);
}

bool writeLogEntry(QFile& file, const QString& line)
{
    return file.write(QString(line + "\r\n").toUtf8())>0;
}

void setVideoTextBox(const QRect& rect, const QColor& bgcolor,
                     const QColor& fgcolor, const QString& text,
                     quint32 text_pos, int w_percent, int h_percent,
                     QPainter& painter)
{
    int w = w_percent / 100. * rect.width();
    int h = h_percent / 100. * rect.height();

    int x, y;
    switch(text_pos & VIDTEXT_POSITION_MASK)
    {
    case VIDTEXT_POSITION_TOPLEFT :
        x = 0; y = 0;
        break;
    case VIDTEXT_POSITION_TOPRIGHT :
        x = rect.width() - w;
        y = 0;
        break;
    case VIDTEXT_POSITION_BOTTOMLEFT :
        x = 0;
        y = rect.height() - h;
        break;
    case VIDTEXT_POSITION_BOTTOMRIGHT :
    default :
        x = rect.width() - w;
        y = rect.height() - h;
        break;
    }

    if(h>0 && w>0)
    {
        const QFont font = painter.font();
        if(font.pixelSize() != h)
        {
            QFont newFont(font);
            newFont.setPixelSize(h);
            painter.setFont(newFont);
        }
        painter.fillRect(x, y, w, h, bgcolor);
        painter.setPen(fgcolor);
        painter.drawText(x, y, w, h, Qt::AlignHCenter | Qt::AlignCenter, text);

        if(font.pixelSize() != h)
            painter.setFont(font);
    }
}

void setTransmitUsers(const QSet<int>& users, INT32* dest_array,
                      INT32 max_elements)
{
    QSet<int>::const_iterator ite = users.begin();
    for(int i=0;i<max_elements;i++)
    {
        if(ite != users.end())
        {
            dest_array[i] = *ite;
            ite++;
        }
        else
            dest_array[i] = 0;
    }
}

#if defined(Q_OS_DARWIN)
void setMacResizeMargins(QDialog* dlg, QLayout* layout)
{
    QSize size = dlg->size();
    QMargins margins = layout->contentsMargins();
    margins.setBottom(margins.bottom()+12);
    layout->setContentsMargins(margins);
    size += QSize(0, 12);
    dlg->resize(size);
}
#endif /* Q_OS_DARWIN */

void setCurrentItemData(QComboBox* cbox, const QVariant& itemdata)
{
    int index = cbox->findData(itemdata);
    if(index>=0)
        cbox->setCurrentIndex(index);
}

QVariant getCurrentItemData(QComboBox* cbox, const QVariant& not_found/* = QVariant()*/)
{
    if(cbox->currentIndex()>=0)
        return cbox->itemData(cbox->currentIndex());
    return not_found;
}


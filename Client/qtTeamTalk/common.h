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

#ifndef COMMON_H
#define COMMON_H

#include <QDebug>
#include <QSettings>
#include <QList>
#include <QDomDocument>
#include <QPainter>
#include <QLayout>
#include <QComboBox>
#include <QFile>
#include <QDateTime>
#include <QCoreApplication>
#include "stdint.h"

#if defined(_MSC_VER)
#define _CRT_SECURE_NO_WARNINGS 1
#pragma warning(disable:4800)
#endif

#if defined(Q_OS_WIN32)
#define NOMINMAX //prevent std::...::min() collision
#include <windows.h>
#endif

#if defined(ENABLE_TOLK)
#include <Tolk.h>
#endif

/*
 * The TeamTalk header and library files are located in 
 * the TeamTalk 5 SDK.
 *
 * To download the TeamTalk 5 SDK go to the GitHub website:
 *
 * https://github.com/BearWare/TeamTalk5
 */

#include <TeamTalk.h>

#if defined(Q_OS_DARWIN) //For hotkeys on Mac
#include <Carbon/Carbon.h>
#endif

#if defined(Q_OS_WINCE)
#define _W(qstr) qstr.utf16()
#define _Q(wstr) QString::fromWCharArray(wstr)
#define COPY_TTSTR(wstr, qstr)                                      \
    do {                                                            \
        wcsncpy(wstr, _W(qstr), TT_STRLEN);                         \
        if(qstr.size() >= TT_STRLEN)                                \
            wstr[TT_STRLEN-1] = '\0';                               \
    } while(0)

#elif defined(Q_OS_WIN32)
//Conversion from/to TTCHAR
#define _W(qstr) qstr.toStdWString().c_str()
#define _Q(wstr) QString::fromWCharArray(wstr)
#define COPY_TTSTR(wstr, qstr)                                      \
    do {                                                            \
        wcsncpy(wstr, _W(qstr), TT_STRLEN);                         \
        if(qstr.size() >= TT_STRLEN)                                \
            wstr[TT_STRLEN-1] = '\0';                               \
    } while(0)

#else
#define _W(qstr) qstr.toUtf8().constData()
#define _Q(wstr) QString::fromUtf8(wstr)
#define COPY_TTSTR(wstr, qstr)                                      \
    do {                                                            \
        strncpy(wstr, _W(qstr), TT_STRLEN);                         \
        if(qstr.size() >= TT_STRLEN)                                \
            wstr[TT_STRLEN-1] = '\0';                               \
    } while(0)
#endif

#define DEFAULT_TCPPORT 10333
#define DEFAULT_UDPPORT 10333

//Client spefic VU max SOUND_VU_MAX (voice act slider)
#define DEFAULT_SOUND_VU_MAX            20

// Channel-struct's AudioConfig
#define CHANNEL_AUDIOCONFIG_MAX             32000
#define DEFAULT_CHANNEL_AUDIOCONFIG_ENABLE  FALSE
#define DEFAULT_CHANNEL_AUDIOCONFIG_LEVEL   9600 /* CHANNEL_AUDIOCONFIG_MAX * (DEFAULT_WEBRTC_GAINDB / WEBRTC_GAINCONTROLLER2_FIXEDGAIN_MAX) */

#define DEFAULT_ECHO_ENABLE             FALSE
#define DEFAULT_AGC_ENABLE              FALSE
#define DEFAULT_DENOISE_ENABLE          FALSE

#define DEFAULT_TEAMTALK_GAINLEVEL              SOUND_GAIN_DEFAULT
#define DEFAULT_TEAMTALK_MUTELEFT               FALSE
#define DEFAULT_TEAMTALK_MUTERIGHT              FALSE

#define DEFAULT_SPEEXDSP_AGC_ENABLE             DEFAULT_AGC_ENABLE
#define DEFAULT_SPEEXDSP_AGC_GAINLEVEL          8000
#define DEFAULT_SPEEXDSP_AGC_INC_MAXDB          12
#define DEFAULT_SPEEXDSP_AGC_DEC_MAXDB          -40
#define DEFAULT_SPEEXDSP_AGC_GAINMAXDB          30
#define DEFAULT_SPEEXDSP_DENOISE_ENABLE         DEFAULT_DENOISE_ENABLE
#define DEFAULT_SPEEXDSP_DENOISE_SUPPRESS       -30
#define DEFAULT_SPEEXDSP_ECHO_ENABLE            FALSE /* requires duplex mode */
#define DEFAULT_SPEEXDSP_ECHO_SUPPRESS          -40
#define DEFAULT_SPEEXDSP_ECHO_SUPPRESSACTIVE    -15

#define DEFAULT_WEBRTC_PREAMPLIFIER_ENABLE      FALSE
#define DEFAULT_WEBRTC_PREAMPLIFIER_GAINFACTOR  1.0f
#define DEFAULT_WEBRTC_VAD_ENABLE               FALSE
#define DEFAULT_WEBRTC_LEVELESTIMATION_ENABLE   FALSE
#define DEFAULT_WEBRTC_GAINCTL_ENABLE           DEFAULT_AGC_ENABLE
#define DEFAULT_WEBRTC_GAINDB                   15
#define DEFAULT_WEBRTC_SAT_PROT_ENABLE          TRUE
#define DEFAULT_WEBRTC_INIT_SAT_MARGIN_DB       20
#define DEFAULT_WEBRTC_EXTRA_SAT_MARGIN_DB      2
#define DEFAULT_WEBRTC_MAXGAIN_DBSEC            3
#define DEFAULT_WEBRTC_MAX_OUT_NOISE            -50
#define DEFAULT_WEBRTC_NOISESUPPRESS_ENABLE     DEFAULT_DENOISE_ENABLE
#define DEFAULT_WEBRTC_NOISESUPPRESS_LEVEL      2
#define DEFAULT_WEBRTC_ECHO_CANCEL_ENABLE       FALSE /* requires duplex mode */

#define DEFAULT_MSEC_PER_PACKET         40

#define DEFAULT_AUDIOCODEC              OPUS_CODEC

//Default Speex codec settings
#define DEFAULT_SPEEX_BANDMODE          1
#define DEFAULT_SPEEX_QUALITY           4
#define DEFAULT_SPEEX_DELAY             DEFAULT_MSEC_PER_PACKET
#define DEFAULT_SPEEX_SIMSTEREO         FALSE

//Default Speex VBR codec settings
#define DEFAULT_SPEEX_VBR_BANDMODE      1
#define DEFAULT_SPEEX_VBR_QUALITY       4
#define DEFAULT_SPEEX_VBR_BITRATE       0
#define DEFAULT_SPEEX_VBR_MAXBITRATE    0
#define DEFAULT_SPEEX_VBR_DTX           TRUE
#define DEFAULT_SPEEX_VBR_DELAY         DEFAULT_MSEC_PER_PACKET
#define DEFAULT_SPEEX_VBR_SIMSTEREO     FALSE

//Default OPUS codec settings
#define DEFAULT_OPUS_SAMPLERATE     48000
#define DEFAULT_OPUS_CHANNELS       1
#define DEFAULT_OPUS_APPLICATION    OPUS_APPLICATION_VOIP
#define DEFAULT_OPUS_COMPLEXITY     10
#define DEFAULT_OPUS_FEC            TRUE
#define DEFAULT_OPUS_DTX            FALSE
#define DEFAULT_OPUS_VBR            TRUE
#define DEFAULT_OPUS_VBRCONSTRAINT  FALSE
#define DEFAULT_OPUS_BITRATE        32000
#define DEFAULT_OPUS_DELAY          DEFAULT_MSEC_PER_PACKET
#define DEFAULT_OPUS_FRAMESIZE      0 // implies same as DEFAULT_OPUS_DELAY

//Default video capture settings
#define DEFAULT_VIDEO_WIDTH     320
#define DEFAULT_VIDEO_HEIGHT    240
#define DEFAULT_VIDEO_FPS       10
#define DEFAULT_VIDEO_FOURCC    FOURCC_I420

//Default video codec settings
#define DEFAULT_VIDEO_CODEC         WEBM_VP8_CODEC
#define DEFAULT_WEBMVP8_DEADLINE    WEBM_VPX_DL_REALTIME
#define DEFAULT_WEBMVP8_BITRATE     256

//Default user right for default user-type
#define USERRIGHT_DEFAULT   (USERRIGHT_MULTI_LOGIN |                \
                             USERRIGHT_VIEW_ALL_USERS |             \
                             USERRIGHT_CREATE_TEMPORARY_CHANNEL |   \
                             USERRIGHT_UPLOAD_FILES |               \
                             USERRIGHT_DOWNLOAD_FILES |             \
                             USERRIGHT_TRANSMIT_VOICE |             \
                             USERRIGHT_TRANSMIT_VIDEOCAPTURE |      \
                             USERRIGHT_TRANSMIT_DESKTOP |           \
                             USERRIGHT_TRANSMIT_DESKTOPINPUT |      \
                             USERRIGHT_TRANSMIT_MEDIAFILE_AUDIO |   \
                             USERRIGHT_TRANSMIT_MEDIAFILE_VIDEO)

#define DEFAULT_MAX_STRING_LENGTH       50

//whether to enable key-translation
#define ENABLE_KEY_TRANSLATION 1

//user ID to pass to store local user's voice stream
#define AUDIOSTORAGE_LOCAL_USERID 0

#ifdef Q_OS_WIN32
#define WM_TEAMALK_CLIENTEVENT     (WM_APP + 1)
#define WM_TEAMTALK_HOTKEYEVENT     (WM_APP + 2)
#endif

//For TT_DoChangeStatus
enum StatusMode
{
    STATUSMODE_AVAILABLE        = 0x00000000,
    STATUSMODE_AWAY             = 0x00000001, 
    STATUSMODE_QUESTION         = 0x00000002,
    STATUSMODE_MODE             = 0x000000FF,

    STATUSMODE_FLAGS            = 0xFFFFFF00,
    STATUSMODE_MALE             = 0x00000000,
    STATUSMODE_FEMALE           = 0x00000100,
    STATUSMODE_NEUTRAL          = 0x00001000,
    STATUSMODE_VIDEOTX          = 0x00000200,
    STATUSMODE_DESKTOP          = 0x00000400,
    STATUSMODE_STREAM_MEDIAFILE = 0x00000800,
    STATUSMODE_GENDER_MASK = (STATUSMODE_MALE | STATUSMODE_FEMALE | STATUSMODE_NEUTRAL),
};

enum Gender
{
    GENDER_NONE               = 0,
    GENDER_MALE               = 1,
    GENDER_FEMALE             = 2,
    GENDER_NEUTRAL            = 3
};

enum SoundEvent
{
    SOUNDEVENT_NEWUSER,
    SOUNDEVENT_REMOVEUSER,
    SOUNDEVENT_SERVERLOST,
    SOUNDEVENT_USERMSG,
    SOUNDEVENT_SENTMSG,
    SOUNDEVENT_CHANNELMSG,
    SOUNDEVENT_SENTCHANNELMSG,
    SOUNDEVENT_BROADCASTMSG,
    SOUNDEVENT_HOTKEY,
    SOUNDEVENT_SILENCE,   
    SOUNDEVENT_NEWVIDEO,   
    SOUNDEVENT_NEWDESKTOP,   
    SOUNDEVENT_FILESUPD,  
    SOUNDEVENT_FILETXDONE,
    SOUNDEVENT_QUESTIONMODE,
    SOUNDEVENT_DESKTOPACCESS,
    SOUNDEVENT_USERLOGGEDIN,
    SOUNDEVENT_USERLOGGEDOUT,
    SOUNDEVENT_VOICEACTON,
    SOUNDEVENT_VOICEACTOFF,
    SOUNDEVENT_MUTEALLON,
    SOUNDEVENT_MUTEALLOFF,
    SOUNDEVENT_TRANSMITQUEUE_HEAD,
    SOUNDEVENT_TRANSMITQUEUE_STOP,
    SOUNDEVENT_VOICEACTTRIG,
    SOUNDEVENT_VOICEACTSTOP,
    SOUNDEVENT_VOICEACTMEON,
    SOUNDEVENT_VOICEACTMEOFF,
};

enum TextToSpeechEvent : qulonglong
{
    TTS_NONE                                        = 0x0000000000000000,
    TTS_USER_LOGGEDIN                               = 0x0000000000000001,
    TTS_USER_LOGGEDOUT                              = 0x0000000000000002,
    TTS_USER_JOINED                                 = 0x0000000000000004,
    TTS_USER_LEFT                                   = 0x0000000000000008,
    TTS_USER_JOINED_SAME                            = 0x0000000000000010,
    TTS_USER_LEFT_SAME                              = 0x0000000000000020,
    TTS_USER_TEXTMSG_PRIVATE                        = 0x0000000000000040,
    TTS_USER_TEXTMSG_PRIVATE_SEND                   = 0x0000000000000080,
    TTS_USER_TEXTMSG_CHANNEL                        = 0x0000000000000100,
    TTS_USER_TEXTMSG_CHANNEL_SEND                   = 0x0000000000000200,
    TTS_USER_TEXTMSG_BROADCAST                      = 0x0000000000000400,
    TTS_USER_TEXTMSG_BROADCAST_SEND                 = 0x0000000000000800,

    TTS_SUBSCRIPTIONS_TEXTMSG_PRIVATE               = 0x0000000000001000,
    TTS_SUBSCRIPTIONS_TEXTMSG_CHANNEL               = 0x0000000000002000,
    TTS_SUBSCRIPTIONS_TEXTMSG_BROADCAST             = 0x0000000000004000,
    TTS_SUBSCRIPTIONS_VOICE                         = 0x0000000000008000,
    TTS_SUBSCRIPTIONS_VIDEO                         = 0x0000000000010000,
    TTS_SUBSCRIPTIONS_DESKTOP                       = 0x0000000000020000,
    TTS_SUBSCRIPTIONS_DESKTOPINPUT                  = 0x0000000000040000,
    TTS_SUBSCRIPTIONS_MEDIAFILE                     = 0x0000000000080000,

    TTS_SUBSCRIPTIONS_INTERCEPT_TEXTMSG_PRIVATE     = 0x0000000000100000,
    TTS_SUBSCRIPTIONS_INTERCEPT_TEXTMSG_CHANNEL     = 0x0000000000200000,
    TTS_SUBSCRIPTIONS_INTERCEPT_VOICE               = 0x0000000000400000,
    TTS_SUBSCRIPTIONS_INTERCEPT_VIDEO               = 0x0000000000800000,
    TTS_SUBSCRIPTIONS_INTERCEPT_DESKTOP             = 0x0000000001000000,
    TTS_SUBSCRIPTIONS_INTERCEPT_MEDIAFILE           = 0x0000000002000000,

    TTS_CLASSROOM_CHANMSG_TX                        = 0x0000000004000000,
    TTS_CLASSROOM_VOICE_TX                          = 0x0000000008000000,
    TTS_CLASSROOM_VIDEO_TX                          = 0x0000000010000000,
    TTS_CLASSROOM_DESKTOP_TX                        = 0x0000000020000000,
    TTS_CLASSROOM_MEDIAFILE_TX                      = 0x0000000040000000,

    TTS_FILE_ADD                                    = 0x0000000080000000,
    TTS_FILE_REMOVE                                 = 0x0000000100000000,

    TTS_MENU_ACTIONS                                = 0x0000000200000000,

    TTS_SERVER_CONNECTIVITY                         = 0x0000000400000000,
};

typedef qulonglong TTSEvents;

enum TextToSpeechEngine
{
    TTSENGINE_NONE          = 0,
    TTSENGINE_QT            = 1,
    TTSENGINE_TOLK          = 2,
    TTSENGINE_NOTIFY        = 3,
};

#define TTSENGINE_NOTIFY_PATH "/usr/bin/notify-send"

enum StatusBarEvent : qulonglong
{
    STATUSBAR_NONE                                        = 0x0000000000000000,
    STATUSBAR_USER_LOGGEDIN                               = 0x0000000000000001,
    STATUSBAR_USER_LOGGEDOUT                              = 0x0000000000000002,
    STATUSBAR_USER_JOINED                                 = 0x0000000000000004,
    STATUSBAR_USER_LEFT                                   = 0x0000000000000008,
    STATUSBAR_USER_JOINED_SAME                            = 0x0000000000000010,
    STATUSBAR_USER_LEFT_SAME                              = 0x0000000000000020,

    STATUSBAR_SUBSCRIPTIONS_TEXTMSG_PRIVATE               = 0x0000000000000040,
    STATUSBAR_SUBSCRIPTIONS_TEXTMSG_CHANNEL               = 0x0000000000000080,
    STATUSBAR_SUBSCRIPTIONS_TEXTMSG_BROADCAST             = 0x0000000000000100,
    STATUSBAR_SUBSCRIPTIONS_VOICE                         = 0x0000000000000200,
    STATUSBAR_SUBSCRIPTIONS_VIDEO                         = 0x0000000000000400,
    STATUSBAR_SUBSCRIPTIONS_DESKTOP                       = 0x0000000000000800,
    STATUSBAR_SUBSCRIPTIONS_DESKTOPINPUT                  = 0x0000000000001000,
    STATUSBAR_SUBSCRIPTIONS_MEDIAFILE                     = 0x0000000000002000,

    STATUSBAR_SUBSCRIPTIONS_INTERCEPT_TEXTMSG_PRIVATE     = 0x0000000000004000,
    STATUSBAR_SUBSCRIPTIONS_INTERCEPT_TEXTMSG_CHANNEL     = 0x0000000000008000,
    STATUSBAR_SUBSCRIPTIONS_INTERCEPT_VOICE               = 0x0000000000010000,
    STATUSBAR_SUBSCRIPTIONS_INTERCEPT_VIDEO               = 0x0000000000020000,
    STATUSBAR_SUBSCRIPTIONS_INTERCEPT_DESKTOP             = 0x0000000000040000,
    STATUSBAR_SUBSCRIPTIONS_INTERCEPT_MEDIAFILE           = 0x0000000000080000,

    STATUSBAR_CLASSROOM_CHANMSG_TX                        = 0x0000000000100000,
    STATUSBAR_CLASSROOM_VOICE_TX                          = 0x0000000000200000,
    STATUSBAR_CLASSROOM_VIDEO_TX                          = 0x0000000000400000,
    STATUSBAR_CLASSROOM_DESKTOP_TX                        = 0x0000000000800000,
    STATUSBAR_CLASSROOM_MEDIAFILE_TX                      = 0x0000000001000000,

    STATUSBAR_FILE_ADD                                    = 0x0000000002000000,
    STATUSBAR_FILE_REMOVE                                 = 0x0000000004000000,

    STATUSBAR_SAVE_SERVER_CONFIG                          = 0x0000000008000000,

    STATUSBAR_START_RECORD                                = 0x0000000010000000,
};

typedef qulonglong StatusBarEvents;

enum HotKeyID
{
    HOTKEY_PUSHTOTALK,
    HOTKEY_VOICEACTIVATION,
    HOTKEY_INCVOLUME,
    HOTKEY_DECVOLUME,
    HOTKEY_MUTEALL,
    HOTKEY_MICROPHONEGAIN_INC,
    HOTKEY_MICROPHONEGAIN_DEC,
    HOTKEY_VIDEOTX
};

typedef QList<INT32> hotkey_t;

#if defined(Q_OS_DARWIN)
#define MAC_HOTKEY_SIZE 2
#define MAC_NO_KEY 0xFFFFFFFF
#endif

enum AudioStorageMode
{
    AUDIOSTORAGE_NONE               = 0x0,
    AUDIOSTORAGE_SINGLEFILE         = 0x1,
    AUDIOSTORAGE_SEPARATEFILES      = 0x2,

    AUDIOSTORAGE_BOTH               = AUDIOSTORAGE_SINGLEFILE | AUDIOSTORAGE_SEPARATEFILES,
};

enum DesktopShareMode
{
    DESKTOPSHARE_NONE,
    DESKTOPSHARE_DESKTOP,
    DESKTOPSHARE_ACTIVE_WINDOW,
    DESKTOPSHARE_SPECIFIC_WINDOW,
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

struct HostEntry
{
    QString name;
    QString ipaddr;
    int tcpport;
    int udpport;
    bool encrypted;
    QString username;
    QString password;
    QString nickname;
    QString channel;
    QString chanpasswd;
    //tt-file specific
    Gender gender;
    hotkey_t hotkey;
    int voiceact;
    VideoFormat capformat;
    VideoCodec vidcodec;

    HostEntry()
    : tcpport(0), udpport(0), encrypted(false), gender(GENDER_NONE)
    , voiceact(-1), capformat(), vidcodec() {}

    // doesn't include 'name'
    bool sameHost(const HostEntry& host) const;
    // same as sameHost() but also host.name == name
    bool sameHostEntry(const HostEntry& host) const;
};

struct DesktopAccessEntry
{
    QString ipaddr;
    int tcpport;
    QStringList channels;
    QStringList usernames;
    DesktopAccessEntry()
    : tcpport(0) { }
};

struct UserCached
{
    bool valid = false;
    Subscriptions subscriptions = SUBSCRIBE_NONE;
    bool voiceMute = false, mediaMute = false;
    int voiceVolume = SOUND_VOLUME_DEFAULT, mediaVolume = SOUND_VOLUME_DEFAULT;
    bool voiceLeftSpeaker = true, voiceRightSpeaker = true,
        mediaLeftSpeaker = true, mediaRightSpeaker = true;
    UserCached() {}
    UserCached(const User& user);
    void sync(TTInstance* ttInst, const User& user);
};

//internal TeamTalk 5 commands. Use with TT_DoTextMessage() and 
//message type MSGTYPE_CUSTOM
#define TT_INTCMD_TYPING_TEXT         "typing"
#define TT_INTCMD_DESKTOP_ACCESS      "desktopaccess"

struct MyTextMessage : TextMessage
{
    QDateTime receiveTime;
    MyTextMessage()
    {
        this->szFromUsername[0] = '\0';
        this->szMessage[0] = '\0';
        receiveTime = QDateTime::currentDateTime();
    }

    explicit MyTextMessage(const TextMessage& msg)
        : MyTextMessage()
    {
        this->nChannelID = msg.nChannelID;
        this->nFromUserID = msg.nFromUserID;
        this->nMsgType = msg.nMsgType;
        this->nToUserID = msg.nToUserID;
#if defined(Q_OS_WIN32)
        wcsncpy(this->szFromUsername, msg.szFromUsername, TT_STRLEN);
        wcsncpy(this->szMessage, msg.szMessage, TT_STRLEN);
#else
        strncpy(this->szFromUsername, msg.szFromUsername, TT_STRLEN);
        strncpy(this->szMessage, msg.szMessage, TT_STRLEN);
#endif
    }
};
typedef QList<MyTextMessage> textmessages_t;

QString makeCustomCommand(const QString& cmd, const QString& value);
QStringList getCustomCommand(const TextMessage& msg);

void initDefaultAudioCodec(AudioCodec& codec);
bool getVideoCaptureCodec(VideoCodec& vidcodec);
void initDefaultVideoFormat(VideoFormat& vidfmt);
AudioPreprocessor initDefaultAudioPreprocessor(AudioPreprocessorType preprocessortype);
AudioPreprocessor loadAudioPreprocessor(AudioPreprocessorType preprocessortype);

bool initVideoCaptureFromSettings();
bool initVideoCapture(const QString& devid, const VideoFormat& fmt);
bool isValid(const VideoFormat& fmt);

QVector<SoundDevice> getSoundDevices();
bool getSoundDevice(int deviceid, const QVector<SoundDevice>& devs, SoundDevice& dev);
bool getSoundDevice(const QString& devid, const QVector<SoundDevice>& devs, SoundDevice& dev);
int getSoundDuplexSampleRate(const SoundDevice& indev, const SoundDevice& outdev);
bool isSoundDeviceEchoCapable(const SoundDevice& indev, const SoundDevice& outdev);

int getDefaultSndInputDevice();
int getDefaultSndOutputDevice();

int getSoundInputFromUID(int inputid, const QString& uid);
int getSoundOutputFromUID(int outputid, const QString& uid);

int getSelectedSndInputDevice();
int getSelectedSndOutputDevice();

QStringList initSelectedSoundDevices(SoundDevice& indev, SoundDevice& outdev);
QStringList initDefaultSoundDevices(SoundDevice& indev, SoundDevice& outdev);

QString getHotKeyText(const hotkey_t& hotkey);

bool isComputerIdle(int idle_secs);
bool isMyselfTalking();
bool isMyselfStreaming();

void saveHotKeySettings(HotKeyID hotkeyid, const hotkey_t& hotkey);

bool loadHotKeySettings(HotKeyID hotkeyid, hotkey_t& hotkey);

void deleteHotKeySettings(HotKeyID hotkeyid);

void saveVideoFormat(const VideoFormat& vidfmt);
bool loadVideoFormat(VideoFormat& vidfmt);

void playSoundEvent(SoundEvent event);
void resetDefaultSoundsPack();
void addTextToSpeechMessage(TextToSpeechEvent event, const QString& msg);

void addLatestHost(const HostEntry& host);
void deleteLatestHost(int index);
bool getLatestHost(int index, HostEntry& host);

//server entries in settings file
void addServerEntry(const HostEntry& host);
void setServerEntry(int index, const HostEntry& host);
bool getServerEntry(int index, HostEntry& host);
void deleteServerEntry(const QString& name);

//server entry from XML
bool getServerEntry(const QDomElement& hostElement, HostEntry& entry);
#define CLIENTSETUP_TAG "clientsetup"

//get desktop access list
void addDesktopAccessEntry(const DesktopAccessEntry& entry);
void getDesktopAccessList(QVector<DesktopAccessEntry>& entries);
void getDesktopAccessList(QVector<DesktopAccessEntry>& entries,
                          const QString& ipaddr, int tcpport);
bool hasDesktopAccess(const QVector<DesktopAccessEntry>& entries,
                      const User& user);
void deleteDesktopAccessEntries();

QString parseXML(const QDomDocument& doc, QString elements);
QString newVersionAvailable(const QDomDocument& updateDoc);
QString downloadUpdateURL(const QDomDocument& updateDoc);
QString getBearWareRegistrationUrl(const QDomDocument& doc);
QString userCacheID(const User& user);

QByteArray generateTTFile(const HostEntry& entry);

void incVolume(int userid, StreamType stream_type);
void decVolume(int userid, StreamType stream_type);
int refVolume(double percent);
int refVolumeToPercent(int volume);
int refGain(double percent);

bool versionSameOrLater(const QString& check, const QString& against);
QString getVersion(const User& user);
QString limitText(const QString& text);
QString getDisplayName(const User& user);

QString getDateTimeStamp();
QString generateAudioStorageFilename(AudioFileFormat aff);

QString generateLogFileName(const QString& name);
bool openLogFile(QFile& file, const QString& folder, const QString& name);
bool writeLogEntry(QFile& file, const QString& line);

void setVideoTextBox(const QRect& rect, const QColor& bgcolor,
                     const QColor& fgcolor, const QString& text,
                     quint32 text_pos, int w_percent, int h_percent,
                     QPainter& painter);

void setTransmitUsers(const QSet<int>& users, INT32* dest_array, INT32 max_elements);
#if defined(Q_OS_DARWIN)
void setMacResizeMargins(QDialog* dlg, QLayout* layout);
#endif /* Q_OS_DARWIN */

void setCurrentItemData(QComboBox* cbox, const QVariant& itemdata);
QVariant getCurrentItemData(QComboBox* cbox, const QVariant& not_found = QVariant());
#endif

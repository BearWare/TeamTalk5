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

#include <QList>
#include <QDomDocument>
#include <QFile>
#include <QDateTime>
#include <QMap>
#include <QVector>

#if defined(Q_OS_WIN32)
#define NOMINMAX //prevent std::...::min() collision
#include <windows.h>
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

#if defined(Q_OS_WIN32)
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

typedef QVector<int> userids_t;
typedef QMap<int, Channel> channels_t;
typedef QMap<int, User> users_t;

bool userCanChanMessage(int userid, const Channel& chan, bool includeFreeForAll = false);
bool userCanVoiceTx(int userid, const Channel& chan, bool includeFreeForAll = false);
bool userCanVideoTx(int userid, const Channel& chan, bool includeFreeForAll = false);
bool userCanDesktopTx(int userid, const Channel& chan, bool includeFreeForAll = false);
bool userCanMediaFileTx(int userid, const Channel& chan, bool includeFreeForAll = false);

channels_t getSubChannels(int channelid, const channels_t& channels, bool recursive = false);
channels_t getParentChannels(int channelid, const channels_t& channels);
users_t getChannelUsers(int channelid, const users_t& users, const channels_t& channels, bool recursive = false);
bool isFreeForAll(StreamTypes stream_type, const int transmitUsers[][2],
                  int max_userids = TT_TRANSMITUSERS_MAX);
void setTransmitUsers(const QSet<int>& users, INT32* dest_array, INT32 max_elements);

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

typedef std::vector<INT32> hotkey_t;

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

struct HostEntry
{
    QString name;
    QString ipaddr;
    int tcpport = DEFAULT_TCPPORT;
    int udpport = DEFAULT_UDPPORT;
    bool encrypted = false;
    QString username;
    QString password;
    QString nickname;
    QString channel;
    QString chanpasswd;
    //tt-file specific
    Gender gender = GENDER_NONE;
    hotkey_t hotkey;
    int voiceact = -1;
    VideoFormat capformat = {};
    VideoCodec vidcodec = {};

    // doesn't include 'name'
    bool sameHost(const HostEntry& host, bool nickcheck = true) const;
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
    MyTextMessage() : TextMessage()
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
        this->bMore = msg.bMore;
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
AudioPreprocessor initDefaultAudioPreprocessor(AudioPreprocessorType preprocessortype);
AudioPreprocessor loadAudioPreprocessor(AudioPreprocessorType preprocessortype);

bool isComputerIdle(int idle_secs);
bool isMyselfTalking();
bool isMyselfStreaming();

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
QString newBetaVersionAvailable(const QDomDocument& updateDoc);
QString downloadBetaUpdateURL(const QDomDocument& updateDoc);
QString getBearWareRegistrationUrl(const QDomDocument& doc);
bool isWebLogin(const QString& username, bool includeParentLoginName);
QString userCacheID(const User& user);

QByteArray generateTTFile(const HostEntry& entry);

bool versionSameOrLater(const QString& check, const QString& against);
QString getVersion(const User& user);
QString limitText(const QString& text);
QString getDisplayName(const User& user);

QString generateAudioStorageFilename(AudioFileFormat aff);

QString getDateTimeStamp();
QString generateLogFileName(const QString& name);
bool openLogFile(QFile& file, const QString& folder, const QString& name);
bool writeLogEntry(QFile& file, const QString& line);

#endif

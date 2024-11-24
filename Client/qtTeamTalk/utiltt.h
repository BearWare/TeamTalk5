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

#ifndef UTILTT_H
#define UTILTT_H

#include <QtGlobal>

#if defined(Q_OS_WIN32)
#define NOMINMAX //prevent std::...::min() collision
#include <windows.h>
#endif

#include <QMap>
#include <QSet>
#include <QVector>

/*
 * The TeamTalk header and library files are located in
 * the TeamTalk 5 SDK.
 *
 * To download the TeamTalk 5 SDK go to the GitHub website:
 *
 * https://github.com/BearWare/TeamTalk5
 */

#include <TeamTalk.h>

#if defined(Q_OS_WIN32)
//Conversion from/to TTCHAR
#define _W(qstr) qstr.toStdWString().c_str()
#define _Q(wstr) QString::fromWCharArray(wstr)
#define COPY_TTSTR(wstr, qstr)                                  \
do {                                                            \
    wcsncpy(wstr, _W(qstr), TT_STRLEN);                         \
    if(qstr.size() >= TT_STRLEN)                                \
        wstr[TT_STRLEN-1] = '\0';                               \
} while(0)

#else
#define _W(qstr) qstr.toUtf8().constData()
#define _Q(wstr) QString::fromUtf8(wstr)
#define COPY_TTSTR(wstr, qstr)                                  \
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
/* gain controller 2 */
#define DEFAULT_WEBRTC_GAINCTL_ENABLE           DEFAULT_AGC_ENABLE
/* gain controller 2 - fixed digital */
#define DEFAULT_WEBRTC_GAINDB                   0.0f
/* gain controller 2 - adaptive digital */
#define DEFAULT_WEBRTC_SAT_PROT_ENABLE          TRUE
#define DEFAULT_WEBRTC_HEADROOM_DB              5.0f
#define DEFAULT_WEBRTC_MAXGAIN_DB               50.0f
#define DEFAULT_WEBRTC_INITIAL_GAIN_DB          15.0f
#define DEFAULT_WEBRTC_MAXGAIN_DBSEC            6.0f
#define DEFAULT_WEBRTC_MAX_OUT_NOISE            -50.0f
/* noise suppression */
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
                             USERRIGHT_TRANSMIT_MEDIAFILE_VIDEO |   \
                             USERRIGHT_TEXTMESSAGE_USER |           \
                             USERRIGHT_TEXTMESSAGE_CHANNEL)

#define DEFAULT_MAX_STRING_LENGTH       50

//whether to enable key-translation
#define ENABLE_KEY_TRANSLATION 1

//user ID to pass to store local user's voice stream
#define AUDIOSTORAGE_LOCAL_USERID 0

#define SPEEXDSP_AGC_GAINLEVEL_MIN 0
#define SPEEXDSP_AGC_GAINLEVEL_MAX 0x8000

#ifdef Q_OS_WIN32
#define WM_TEAMALK_CLIENTEVENT     (WM_APP + 1)
#define WM_TEAMTALK_HOTKEYEVENT     (WM_APP + 2)
#endif

typedef QVector<int> userids_t;
typedef QMap<int, Channel> channels_t;
typedef QMap<int, User> users_t;
typedef QVector<UserAccount> useraccounts_t;

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

void initDefaultAudioCodec(AudioCodec& codec);
AudioPreprocessor initDefaultAudioPreprocessor(AudioPreprocessorType preprocessortype);

bool isMyselfTalking();
bool isMyselfStreaming();

#endif // UTILTT_H

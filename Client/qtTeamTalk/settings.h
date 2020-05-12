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

#ifndef SETTINGS_H
#define SETTINGS_H

#include <QSettings>

#define SETTINGS_VERSION                            "5.1"
#define SETTINGS_GENERAL_VERSION                    "version"
#define SETTINGS_GENERAL_VERSION_DEFAULT            "5.0"

#define SETTINGS_GENERAL_NICKNAME                   "general_/nickname"
#define SETTINGS_GENERAL_NICKNAME_DEFAULT           "NoName"
#define SETTINGS_GENERAL_GENDER                     "general_/gender"
#define SETTINGS_GENERAL_GENDER_DEFAULT             true
#define SETTINGS_GENERAL_BEARWARE_USERNAME          "general_/bearwareid"
#define SETTINGS_GENERAL_BEARWARE_TOKEN             "general_/bearwaretoken"
#define SETTINGS_GENERAL_AUTOAWAY                   "general_/auto-away"
#define SETTINGS_GENERAL_PUSHTOTALK                 "general_/push-to-talk"
#define SETTINGS_GENERAL_PUSHTOTALK_KEY             "general_/ptt-key"
#define SETTINGS_GENERAL_VOICEACTIVATED             "general_/voice-activated"
#define SETTINGS_GENERAL_VOICEACTIVATED_DEFAULT     false
#define SETTINGS_GENERAL_STATUSMESSAGE              "general_/statusmsg"
#define SETTINGS_GENERAL_PROFILENAME                "general_/profilename"

#define SETTINGS_DISPLAY_MESSAGEPOPUP               "display/msgpopup"
#define SETTINGS_DISPLAY_VIDEOPOPUP                 "display/videopopup"
#define SETTINGS_DISPLAY_VIDEORETURNTOGRID          "display/videoreturntogrid"
#define SETTINGS_DISPLAY_VIDEORETURNTOGRID_DEFAULT  true
#define SETTINGS_DISPLAY_DESKTOPPOPUP               "display/desktoppopup"
#define SETTINGS_DISPLAY_VIDEOTEXT_WIDTH            "display/videotext-pane-width"
#define SETTINGS_DISPLAY_VIDEOTEXT_WIDTH_DEFAULT    30.0
#define SETTINGS_DISPLAY_VIDEOTEXT_HEIGHT           "display/videotext-pane-height"
#define SETTINGS_DISPLAY_VIDEOTEXT_HEIGHT_DEFAULT   10.0
#define SETTINGS_DISPLAY_VIDEOTEXT_FGCOLOR          "display/videotext-pane-fgcolor"
#define SETTINGS_DISPLAY_VIDEOTEXT_FGCOLOR_DEFAULT  QColor(Qt::white)
#define SETTINGS_DISPLAY_VIDEOTEXT_BGCOLOR          "display/videotext-pane-bgcolor"
#define SETTINGS_DISPLAY_VIDEOTEXT_BGCOLOR_DEFAULT  QColor(Qt::blue)
#define SETTINGS_DISPLAY_VIDEOTEXT                  "display/videotext-pane"
#define SETTINGS_DISPLAY_VIDEOTEXT_DEFAULT          (VIDTEXT_SHOW_NICKNAME | VIDTEXT_POSITION_BOTTOMRIGHT)
#define SETTINGS_DISPLAY_VIDEOTEXT_SHOW             "display/videotext-pane-show"
#define SETTINGS_DISPLAY_TRAYMINIMIZE               "display/trayminimize"
#define SETTINGS_DISPLAY_ALWAYSONTOP                "display/alwaysontop"
#define SETTINGS_DISPLAY_VU_METER_UPDATES           "display/vumeter-updates"
#define SETTINGS_DISPLAY_VU_METER_UPDATES_DEFAULT   true
#define SETTINGS_DISPLAY_USERSCOUNT                 "display/userscount"
#define SETTINGS_DISPLAY_USERSCOUNT_DEFAULT         true
#define SETTINGS_DISPLAY_SHOWUSERNAME               "display/showusername"
#define SETTINGS_DISPLAY_SHOWUSERNAME_DEFAULT       false
#define SETTINGS_DISPLAY_LASTTALK                   "display/display-last-talk"
#define SETTINGS_DISPLAY_LASTTALK_DEFAULT           true
#define SETTINGS_DISPLAY_MSGTIMESTAMP               "display/msgtimestamp"
#define SETTINGS_DISPLAY_STARTMINIMIZED             "display/startminimized"
#define SETTINGS_DISPLAY_WINDOWPOS                  "display/windowposition"
#define SETTINGS_DISPLAY_SPLITTER                   "display/splitter"
#define SETTINGS_DISPLAY_VIDEOSPLITTER              "display/video-splitter"
#define SETTINGS_DISPLAY_DESKTOPSPLITTER            "display/desktop-splitter"
#define SETTINGS_DISPLAY_FILESHEADER                "display/filesheader"
#define SETTINGS_DISPLAY_LOGSTATUSBAR               "display/log-statusbar"
#define SETTINGS_DISPLAY_FREESERVERS                "display/showfreeservers"
#define SETTINGS_DISPLAY_LANGUAGE                   "display/language"
#define SETTINGS_DISPLAY_APPUPDATE                  "display/check-appupdate"
#define SETTINGS_DISPLAY_MAX_STRING                 "display/channel-max-string"
#define SETTINGS_DISPLAY_MAX_STRING_DEFAULT         DEFAULT_MAX_STRING_LENGTH

#define SETTINGS_CONNECTION_AUTOCONNECT             "connection/autoconnect"
#define SETTINGS_CONNECTION_RECONNECT               "connection/reconnect"
#define SETTINGS_CONNECTION_AUTOJOIN                "connection/autojoin"
#define SETTINGS_CONNECTION_QUERYMAXPAYLOAD         "connection/query-max-payload"
#define SETTINGS_CONNECTION_SUBSCRIBE_USERMSG       "connection/subscribe-usermsg"
#define SETTINGS_CONNECTION_SUBSCRIBE_CHANNELMSG    "connection/subscribe-channelmsg"
#define SETTINGS_CONNECTION_SUBSCRIBE_BROADCASTMSG  "connection/subscribe-broadcastmsg"
#define SETTINGS_CONNECTION_SUBSCRIBE_VOICE         "connection/subscribe-audio"
#define SETTINGS_CONNECTION_SUBSCRIBE_VIDEOCAPTURE  "connection/subscribe-video"
#define SETTINGS_CONNECTION_SUBSCRIBE_DESKTOP       "connection/subscribe-desktop"
#define SETTINGS_CONNECTION_SUBSCRIBE_MEDIAFILE     "connection/subscribe-media-file"
#define SETTINGS_CONNECTION_TCPPORT                 "connection/localtcpport"
#define SETTINGS_CONNECTION_UDPPORT                 "connection/localudpport"

#define SOUNDDEVICEID_DEFAULT  -2

#define SETTINGS_SOUND_SOUNDSYSTEM                  "soundsystem/soundsystem"
#define SETTINGS_SOUND_INPUTDEVICE                  "soundsystem/inputdeviceid"
#define SETTINGS_SOUND_INPUTDEVICE_DEFAULT          SOUNDDEVICEID_DEFAULT
#define SETTINGS_SOUND_INPUTDEVICE_UID              "soundsystem/inputdeviceuid"
#define SETTINGS_SOUND_OUTPUTDEVICE                 "soundsystem/outputdeviceid"
#define SETTINGS_SOUND_OUTPUTDEVICE_DEFAULT         SOUNDDEVICEID_DEFAULT
#define SETTINGS_SOUND_OUTPUTDEVICE_UID             "soundsystem/outputdeviceuid"
#define SETTINGS_SOUND_MASTERVOLUME                 "soundsystem/mastervolume"
#define SETTINGS_SOUND_MASTERVOLUME_DEFAULT         50
#define SETTINGS_SOUND_MICROPHONEGAIN               "soundsystem/microphonegain"
#define SETTINGS_SOUND_MICROPHONEGAIN_AGC_DEFAULT   (100 * DEFAULT_AGC_GAINLEVEL / SOUND_GAIN_MAX)
#define SETTINGS_SOUND_MICROPHONEGAIN_GAIN_DEFAULT  50
#define SETTINGS_SOUND_MEDIASTREAM_VOLUME           "soundsystem/mediastream"
#define SETTINGS_SOUND_MEDIASTREAM_VOLUME_DEFAULT   100
#define SETTINGS_SOUND_VOICEACTIVATIONLEVEL         "soundsystem/voice-activation-level"
#define SETTINGS_SOUND_VOICEACTIVATIONLEVEL_DEFAULT 2
#define SETTINGS_SOUND_ECHOCANCEL                   "soundsystem/echocancellation"
#define SETTINGS_SOUND_ECHOCANCEL_DEFAULT           DEFAULT_ECHO_ENABLE
#define SETTINGS_SOUND_DUPLEXMODE                   "soundsystem/duplex-mode"
#define SETTINGS_SOUND_DUPLEXMODE_DEFAULT           DEFAULT_SOUND_DUPLEXMODE
#define SETTINGS_SOUND_AGC                          "soundsystem/agc"
#define SETTINGS_SOUND_AGC_DEFAULT                  DEFAULT_AGC_ENABLE
#define SETTINGS_SOUND_DENOISING                    "soundsystem/denoising"
#define SETTINGS_SOUND_DENOISING_DEFAULT            DEFAULT_DENOISE_ENABLE
#define SETTINGS_SOUND_SOUNDOUT_MUTE_LEFT           "soundsystem/sndoutput-mute-left"
#define SETTINGS_SOUND_SOUNDOUT_MUTE_LEFT_DEFAULT   false
#define SETTINGS_SOUND_SOUNDOUT_MUTE_RIGHT          "soundsystem/sndoutput-mute-right"
#define SETTINGS_SOUND_SOUNDOUT_MUTE_RIGHT_DEFAULT  false

#define SETTINGS_SOUNDEVENT_NEWUSER                 "soundevents/newuser"
#define SETTINGS_SOUNDEVENT_REMOVEUSER              "soundevents/removeuser"
#define SETTINGS_SOUNDEVENT_SERVERLOST              "soundevents/server-lost"
#define SETTINGS_SOUNDEVENT_USERMSG                 "soundevents/user-msg"
#define SETTINGS_SOUNDEVENT_CHANNELMSG              "soundevents/channel-msg"
#define SETTINGS_SOUNDEVENT_BROADCASTMSG            "soundevents/broadcast-msg"
#define SETTINGS_SOUNDEVENT_HOTKEY                  "soundevents/hotkey-pressed"
#define SETTINGS_SOUNDEVENT_SILENCE                 "soundevents/channel-silent"
#define SETTINGS_SOUNDEVENT_NEWVIDEO                "soundevents/new-video-session"
#define SETTINGS_SOUNDEVENT_NEWDESKTOP              "soundevents/new-desktop-session"
#define SETTINGS_SOUNDEVENT_FILESUPD                "soundevents/files-updated"
#define SETTINGS_SOUNDEVENT_FILETXDONE              "soundevents/file-tx-done"
#define SETTINGS_SOUNDEVENT_QUESTIONMODE            "soundevents/question-mode"
#define SETTINGS_SOUNDEVENT_DESKTOPACCESS           "soundevents/desktop-access-request"

#define SETTINGS_SHORTCUTS_VOICEACTIVATION          "shortcuts/voiceactivation"
#define SETTINGS_SHORTCUTS_INCVOLUME                "shortcuts/volume-inc"
#define SETTINGS_SHORTCUTS_DECVOLUME                "shortcuts/volume-dec"
#define SETTINGS_SHORTCUTS_MUTEALL                  "shortcuts/muteall"
#define SETTINGS_SHORTCUTS_INCVOICEGAIN             "shortcuts/voicegain-inc"
#define SETTINGS_SHORTCUTS_DECVOICEGAIN             "shortcuts/voicegain-dec"
#define SETTINGS_SHORTCUTS_VIDEOTX                  "shortcuts/video-tx"

#define SETTINGS_VIDCAP_DEVICEID                    "videocapture/deviceid"
#define SETTINGS_VIDCAP_RESOLUTION                  "videocapture/resolution"
#define SETTINGS_VIDCAP_RESOLUTION_DEFAULT          (QString("%1x%2").arg(DEFAULT_VIDEO_WIDTH).arg(DEFAULT_VIDEO_HEIGHT))
#define SETTINGS_VIDCAP_FPS                         "videocapture/fps"
#define SETTINGS_VIDCAP_FPS_DEFAULT                 (QString("%1/1").arg(DEFAULT_VIDEO_FPS))
#define SETTINGS_VIDCAP_FOURCC                      "videocapture/fourcc"
#define SETTINGS_VIDCAP_FOURCC_DEFAULT              DEFAULT_VIDEO_FOURCC
#define SETTINGS_VIDCAP_CODEC                       "videocapture/codec"
#define SETTINGS_VIDCAP_CODEC_DEFAULT               DEFAULT_VIDEO_CODEC
#define SETTINGS_VIDCAP_WEBMVP8_BITRATE             "videocapture/codec/webmvp8-bitrate"
#define SETTINGS_VIDCAP_WEBMVP8_BITRATE_DEFAULT     DEFAULT_WEBMVP8_BITRATE
#define SETTINGS_VIDCAP_ENABLE                      "videocapture/enable"
#define SETTINGS_VIDCAP_ENABLE_DEFAULT              false

#define SETTINGS_MEDIASTORAGE_MODE                  "media-storage/audio-storage-mode"
#define SETTINGS_MEDIASTORAGE_AUDIOFOLDER           "media-storage/audio-storage-folder"
#define SETTINGS_MEDIASTORAGE_FILEFORMAT            "media-storage/audio-storage-format"
#define SETTINGS_MEDIASTORAGE_CHANLOGFOLDER         "media-storage/channellog-folder"
#define SETTINGS_MEDIASTORAGE_USERLOGFOLDER         "media-storage/userlog-folder"

#define SETTINGS_STREAMMEDIA_FILENAME               "stream-media/%1_stream-media-filename"
#define SETTINGS_STREAMMEDIA_LOOP                   "stream-media/stream-media-file-loop"
#define SETTINGS_STREAMMEDIA_CODEC                  "stream-media/codec"
#define SETTINGS_STREAMMEDIA_WEBMVP8_BITRATE        "stream-media/codec/webmvp8-bitrate"
#define SETTINGS_STREAMMEDIA_OFFSET                 "stream-media/offset"
#define SETTINGS_STREAMMEDIA_OFFSET_DEFAULT         0
#define SETTINGS_STREAMMEDIA_AUDIOPREPROCESSOR      "stream-media/audiopreprocessor"
#define SETTINGS_STREAMMEDIA_AUDIOPREPROCESSOR_DEFAULT NO_AUDIOPREPROCESSOR
#define SETTINGS_STREAMMEDIA_TTAP_MUTELEFT          "stream-media/ttaudiopreprocessor/muteleft"
#define SETTINGS_STREAMMEDIA_TTAP_MUTERIGHT         "stream-media/ttaudiopreprocessor/muteright"
#define SETTINGS_STREAMMEDIA_TTAP_GAINLEVEL         "stream-media/ttaudiopreprocessor/gainlevel"
#define SETTINGS_STREAMMEDIA_SPX_AGC_ENABLE         "stream-media/spxaudiopreprocessor/agc-enable"
#define SETTINGS_STREAMMEDIA_SPX_AGC_GAINLEVEL      "stream-media/spxaudiopreprocessor/agc-gain-level"
#define SETTINGS_STREAMMEDIA_SPX_AGC_INC_MAXDB      "stream-media/spxaudiopreprocessor/agc-incmaxdbsec"
#define SETTINGS_STREAMMEDIA_SPX_AGC_DEC_MAXDB      "stream-media/spxaudiopreprocessor/agc-devmaxdbsec"
#define SETTINGS_STREAMMEDIA_SPX_AGC_GAINMAXDB      "stream-media/spxaudiopreprocessor/agc-gainmaxdb"
#define SETTINGS_STREAMMEDIA_SPX_DENOISE_ENABLE     "stream-media/spxaudiopreprocessor/denoise-enable"
#define SETTINGS_STREAMMEDIA_SPX_DENOISE_SUPPRESS   "stream-media/spxaudiopreprocessor/denoise-suppress"

#define SETTINGS_DESKTOPSHARE_MODE                  "desktop-share/desktop-share-mode"
#define SETTINGS_DESKTOPSHARE_BMPMODE               "desktop-share/desktop-bmp-mode"
#define SETTINGS_DESKTOPSHARE_INTERVAL              "desktop-share/desktop-share-interval"
#define SETTINGS_DESKTOPSHARE_CURSOR                "desktop-share/desktop-cursor"
#define SETTINGS_DESKTOPSHARE_WINDOWTITLE           "desktop-share/desktop-window-title"

#define SETTINGS_LATESTHOST_HOSTADDR                "latesthosts/%1_hostaddr"
#define SETTINGS_LATESTHOST_TCPPORT                 "latesthosts/%1_tcpport"
#define SETTINGS_LATESTHOST_UDPPORT                 "latesthosts/%1_udpport"
#define SETTINGS_LATESTHOST_ENCRYPTED               "latesthosts/%1_encrypted"
#define SETTINGS_LATESTHOST_USERNAME                "latesthosts/%1_username"
#define SETTINGS_LATESTHOST_PASSWORD                "latesthosts/%1_password"
#define SETTINGS_LATESTHOST_CHANNEL                 "latesthosts/%1_channel"
#define SETTINGS_LATESTHOST_CHANNELPASSWD           "latesthosts/%1_chanpassword"

#define SETTINGS_SERVERENTRIES_NAME                 "serverentries/%1_name"
#define SETTINGS_SERVERENTRIES_HOSTADDR             "serverentries/%1_hostaddr"
#define SETTINGS_SERVERENTRIES_TCPPORT              "serverentries/%1_tcpport"
#define SETTINGS_SERVERENTRIES_UDPPORT              "serverentries/%1_udpport"
#define SETTINGS_SERVERENTRIES_ENCRYPTED            "serverentries/%1_encrypted"
#define SETTINGS_SERVERENTRIES_USERNAME             "serverentries/%1_username"
#define SETTINGS_SERVERENTRIES_PASSWORD             "serverentries/%1_password"
#define SETTINGS_SERVERENTRIES_CHANNEL              "serverentries/%1_channel"
#define SETTINGS_SERVERENTRIES_CHANNELPASSWD        "serverentries/%1_chanpassword"

#define SETTINGS_DESKTOPACCESS_HOSTADDR             "desktopaccess/%1_hostaddr"
#define SETTINGS_DESKTOPACCESS_TCPPORT              "desktopaccess/%1_tcpport"
#define SETTINGS_DESKTOPACCESS_CHANNEL              "desktopaccess/%1_%2_channel"
#define SETTINGS_DESKTOPACCESS_USERNAME             "desktopaccess/%1_%2_username"

#define SETTINGS_LAST_DIRECTORY                     "history/last-directory"
#define SETTINGS_FIREWALL_ADD                       "history/firewall-add"

#endif

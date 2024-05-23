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

#ifndef SETTINGS_H
#define SETTINGS_H

#include <QSettings>
#include <QApplication>

#if defined(Q_OS_WIN32)
#define SOUNDSPATH                                     QApplication::applicationDirPath()+"/Sounds"
#elif defined(Q_OS_LINUX)
#define SOUNDSPATH                                     QApplication::applicationDirPath()+"/sounds"
#elif defined(Q_OS_DARWIN)
#define SOUNDSPATH                                     "/Applications/TeamTalk5.app/Contents/Resources/Sounds"
#endif
#define SETTINGS_VERSION                            "5.4"
#define SETTINGS_GENERAL_VERSION                    "version"
#define SETTINGS_GENERAL_VERSION_DEFAULT            SETTINGS_VERSION
#if defined(ENABLE_TOLK) || defined(Q_OS_LINUX)
#define SETTINGS_GENERAL_FIRSTSTART                 "general_/first-start"
#define SETTINGS_GENERAL_FIRSTSTART_DEFAULT         true
#endif

#define SETTINGS_GENERAL_NICKNAME                   "general_/nickname"
#define SETTINGS_GENERAL_NICKNAME_DEFAULT           ""
#define SETTINGS_GENERAL_GENDER                     "general_/gender"
#define SETTINGS_GENERAL_GENDER_DEFAULT             GENDER_NEUTRAL
#define SETTINGS_GENERAL_BEARWARE_USERNAME          "general_/bearwareid"
#define SETTINGS_GENERAL_BEARWARE_TOKEN             "general_/bearwaretoken"
#define SETTINGS_GENERAL_RESTOREUSERSETTINGS        "general_/restore-user-settings"
#define SETTINGS_GENERAL_RESTOREUSERSETTINGS_DEFAULT true
#define SETTINGS_GENERAL_AUTOAWAY                   "general_/auto-away"
#define SETTINGS_GENERAL_AUTOAWAY_DEFAULT                   180
#define SETTINGS_GENERAL_AWAY_STATUSMSG                   "general_/idle-message"
#define SETTINGS_GENERAL_INACTIVITY_DISABLE_VOICEACT        "general_/inactivity-disable-voiceact"
#define SETTINGS_GENERAL_INACTIVITY_DISABLE_VOICEACT_DEFAULT false
#define SETTINGS_GENERAL_PUSHTOTALK                 "general_/push-to-talk"
#define SETTINGS_GENERAL_PUSHTOTALK_KEY             "general_/ptt-key"
#define SETTINGS_GENERAL_PUSHTOTALKLOCK             "general_/ptt-key-lock"
#define SETTINGS_GENERAL_PUSHTOTALKLOCK_DEFAULT     false
#define SETTINGS_GENERAL_VOICEACTIVATED             "general_/voice-activated"
#define SETTINGS_GENERAL_VOICEACTIVATED_DEFAULT     false
#define SETTINGS_GENERAL_STATUSMESSAGE              "general_/statusmsg"
#define SETTINGS_GENERAL_STREAMING_STATUS              "general_/streaming-status"
#define SETTINGS_GENERAL_STREAMING_STATUS_DEFAULT              false
#define SETTINGS_GENERAL_PROFILENAME                "general_/profilename"

#define SETTINGS_DISPLAY_MESSAGEPOPUP               "display/msgpopup"
#define SETTINGS_DISPLAY_MESSAGEPOPUP_DEFAULT       true
#define SETTINGS_DISPLAY_VIDEOPOPUP                 "display/videopopup"
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
#define SETTINGS_DISPLAY_EMOJI                     "display/show-emoji"
#define SETTINGS_DISPLAY_EMOJI_DEFAULT              true
#define SETTINGS_DISPLAY_ANIM                     "display/show-animations"
#define SETTINGS_DISPLAY_ANIM_DEFAULT              true
#define SETTINGS_DISPLAY_SERVNAME                     "display/show-servname"
#define SETTINGS_DISPLAY_SERVNAME_DEFAULT              true
#define SETTINGS_DISPLAY_MSGTIMESTAMP               "display/msgtimestamp"
#define SETTINGS_DISPLAY_STARTMINIMIZED             "display/startminimized"
#define SETTINGS_DISPLAY_MAINWINDOWPOS              "display/windowposition"
#define SETTINGS_DISPLAY_TEXTMSGWINDOWPOS           "display/textmsg-windowposition"
#define SETTINGS_DISPLAY_TEXTMSGWINDOWPOS_SPLITTER  "display/textmsg-windowsplitter"
#define SETTINGS_DISPLAY_ONLINEUSERSWINDOWPOS       "display/onlineusers-windowposition"
#define SETTINGS_DISPLAY_ONLINEUSERS_HEADERSIZES          "display/onlineusers-headersize"
#define SETTINGS_DISPLAY_USERACCOUNTSWINDOWPOS      "display/useraccount-windowposition"
#define SETTINGS_DISPLAY_CHANNELWINDOWPOS           "display/channel-windowposition"
#define SETTINGS_DISPLAY_BANNEDUSERSWINDOWPOS       "display/bannedusers-windowposition"
#define SETTINGS_DISPLAY_BANNEDUSERS_HEADERSIZES    "display/bannedusers-headersize"
#define SETTINGS_DISPLAY_MEDIASTORAGEWINDOWPOS      "display/mediastorage-windowposition"
#define SETTINGS_DISPLAY_PREFERENCESWINDOWPOS       "display/preferences-windowposition"
#define SETTINGS_DISPLAY_SERVERPROPERTIESWINDOWPOS  "display/serverproperties-windowposition"
#define SETTINGS_DISPLAY_SPLITTER                   "display/splitter"
#define SETTINGS_DISPLAY_VIDEOSPLITTER              "display/video-splitter"
#define SETTINGS_DISPLAY_DESKTOPSPLITTER            "display/desktop-splitter"
#define SETTINGS_DISPLAY_FILESHEADER                "display/filesheader2"
#define SETTINGS_DISPLAY_SOUNDEVENTSHEADER                 "display/soundeventsheader"
#define SETTINGS_DISPLAY_TTSHEADER                  "display/ttsheader"
#define SETTINGS_DISPLAY_LOGSTATUSBAR               "display/log-statusbar"
#define SETTINGS_DISPLAY_LOGSTATUSBAR_DEFAULT       true
#define SETTINGS_DISPLAY_OFFICIALSERVERS            "display/show-official-servers"
#define SETTINGS_DISPLAY_OFFICIALSERVERS_DEFAULT    true
#define SETTINGS_DISPLAY_UNOFFICIALSERVERS          "display/show-unofficial-servers"
#define SETTINGS_DISPLAY_UNOFFICIALSERVERS_DEFAULT  false
#define SETTINGS_DISPLAY_SERVERLISTFILTER_NAME      "display/filter-server-name"
#define SETTINGS_DISPLAY_SERVERLISTFILTER_NAME_DEFAULT ""
#define SETTINGS_DISPLAY_SERVERLISTFILTER_USERSCOUNT "display/filter-server-users-count"
#define SETTINGS_DISPLAY_SERVERLISTFILTER_USERSCOUNT_DEFAULT      0
#define SETTINGS_DISPLAY_LANGUAGE                   "display/language"
#define SETTINGS_DISPLAY_LANGUAGE_DEFAULT           ""
#define SETTINGS_DISPLAY_APPUPDATE                  "display/check-appupdate"
#define SETTINGS_DISPLAY_APPUPDATE_DEFAULT                  true
#define SETTINGS_DISPLAY_APPUPDATE_BETA                  "display/check-beta-versions"
#define SETTINGS_DISPLAY_APPUPDATE_BETA_DEFAULT                  false
#define SETTINGS_DISPLAY_APPUPDATE_DLG                  "display/show-dlg-appupdate"
#define SETTINGS_DISPLAY_APPUPDATE_DLG_DEFAULT                  true
#define SETTINGS_DISPLAY_MAX_STRING                 "display/channel-max-string"
#define SETTINGS_DISPLAY_MAX_STRING_DEFAULT         DEFAULT_MAX_STRING_LENGTH
#define SETTINGS_DISPLAY_STATUSBAR_EVENTS_HEADER    "display/status-bar-events-header"
#define SETTINGS_DISPLAY_STATUSBARDLG_SIZE          "display/status-bar-dialog-size"
#define SETTINGS_STATUSBAR_ACTIVEEVENTS                   "display/status-bar-active-events"
#define SETTINGS_STATUSBAR_ACTIVEEVENTS_DEFAULT         StatusBarEvents(STATUSBAR_USER_LOGGEDIN | \
                                                        STATUSBAR_USER_LOGGEDOUT | \
                                                        STATUSBAR_USER_JOINED | \
                                                        STATUSBAR_USER_LEFT | \
                                                        STATUSBAR_USER_JOINED_SAME | \
                                                        STATUSBAR_USER_LEFT_SAME | \
                                                        STATUSBAR_SUBSCRIPTIONS_TEXTMSG_PRIVATE | \
                                                        STATUSBAR_SUBSCRIPTIONS_TEXTMSG_CHANNEL | \
                                                        STATUSBAR_SUBSCRIPTIONS_TEXTMSG_BROADCAST | \
                                                        STATUSBAR_SUBSCRIPTIONS_VOICE | \
                                                        STATUSBAR_SUBSCRIPTIONS_VIDEO | \
                                                        STATUSBAR_SUBSCRIPTIONS_DESKTOP | \
                                                        STATUSBAR_SUBSCRIPTIONS_DESKTOPINPUT | \
                                                        STATUSBAR_SUBSCRIPTIONS_MEDIAFILE | \
                                                        STATUSBAR_SUBSCRIPTIONS_INTERCEPT_TEXTMSG_PRIVATE | \
                                                        STATUSBAR_SUBSCRIPTIONS_INTERCEPT_TEXTMSG_CHANNEL | \
                                                        STATUSBAR_SUBSCRIPTIONS_INTERCEPT_VOICE | \
                                                        STATUSBAR_SUBSCRIPTIONS_INTERCEPT_VIDEO | \
                                                        STATUSBAR_SUBSCRIPTIONS_INTERCEPT_DESKTOP | \
                                                        STATUSBAR_SUBSCRIPTIONS_INTERCEPT_MEDIAFILE | \
                                                        STATUSBAR_CLASSROOM_CHANMSG_TX | \
                                                        STATUSBAR_CLASSROOM_VOICE_TX | \
                                                        STATUSBAR_CLASSROOM_VIDEO_TX | \
                                                        STATUSBAR_CLASSROOM_DESKTOP_TX | \
                                                        STATUSBAR_CLASSROOM_MEDIAFILE_TX | \
                                                        STATUSBAR_FILE_ADD | \
                                                        STATUSBAR_FILE_REMOVE | \
                                                        STATUSBAR_SAVE_SERVER_CONFIG | \
                                                        STATUSBAR_START_RECORD | \
                                                        STATUSBAR_TRANSMISSION_BLOCKED)
#define SETTINGS_DISPLAY_SERVERLOG_EVENTS_HEADER    "display/serverlog-events-header"
#define SETTINGS_DISPLAY_CHANEXP                 "display/expand-channels"
#define SETTINGS_DISPLAY_CHANEXP_DEFAULT         false
#define SETTINGS_DISPLAY_CHANDBCLICK             "display/action-channel-double-click"
#define SETTINGS_DISPLAY_CHANDBCLICK_DEFAULT             ACTION_JOINLEAVE
#define SETTINGS_DISPLAY_CLOSE_FILEDIALOG             "display/close-file-dlg"
#define SETTINGS_DISPLAY_CLOSE_FILEDIALOG_DEFAULT             false
#define SETTINGS_DISPLAY_CHANEXCLUDE_DLG             "display/dlg-alert-exclude"
#define SETTINGS_DISPLAY_CHANEXCLUDE_DLG_DEFAULT             false
#define SETTINGS_DISPLAY_MOTD_DLG             "display/MOTD-dlg"
#define SETTINGS_DISPLAY_MOTD_DLG_DEFAULT             false
#define SETTINGS_DISPLAY_CHANNEL_TOPIC             "display/channel-topic"
#define SETTINGS_DISPLAY_CHANNEL_TOPIC_DEFAULT             false
#define SETTINGS_DISPLAY_START_SERVERLIST             "display/serverlist-on-start"
#define SETTINGS_DISPLAY_START_SERVERLIST_DEFAULT             false
#define SETTINGS_DISPLAY_CHANNELSORT                 "display/sort-channels"
#define SETTINGS_DISPLAY_CHANNELSORT_DEFAULT         CHANNELSORT_ASCENDING
#define SETTINGS_DISPLAY_WINDOW_MAXIMIZE                          "display/window-maximized"
#define SETTINGS_DISPLAY_SERVERLIST_HEADERSIZES     "display/serverlist-header"
#define SETTINGS_DISPLAY_SERVERLISTDLG_SIZE         "display/serverlist-dialog-size"
#define SETTINGS_DISPLAY_USERACCOUNTS_HEADERSIZES     "display/useraccounts-header"
#define SETTINGS_DISPLAY_USERACCOUNTSDLG_SIZE         "display/useraccounts-dialog-size"
#define SETTINGS_DISPLAY_SERVERLIST_SORT                            "display/server-list-sort"
#define SETTINGS_DISPLAY_SERVERLIST_SORT_DEFAULT                            "default"
#define SETTINGS_DISPLAY_USERACCOUNT_SORT                            "display/user-account-sort"
#define SETTINGS_DISPLAY_USERACCOUNT_SORT_DEFAULT                            "username"
#define SETTINGS_DISPLAY_BANLIST_SORT                            "display/ban-list-sort"
#define SETTINGS_DISPLAY_BANLIST_SORT_DEFAULT                            "bantime"
#define SETTINGS_DISPLAY_ONLINEUSERS_SORT                            "display/online-users-sort"
#define SETTINGS_DISPLAY_ONLINEUSERS_SORT_DEFAULT                            "nickname"
#define SETTINGS_DISPLAY_FILESLIST_SORT                            "display/files-list-sort"
#define SETTINGS_DISPLAY_FILESLIST_SORT_DEFAULT                            "name"
#define SETTINGS_DISPLAY_SERVERWINDOWPOS     "display/server-dialog-pos"
#define SETTINGS_DISPLAY_USERWINDOWPOS     "display/user-dialog-pos"

#define SETTINGS_CONNECTION_AUTOCONNECT             "connection/autoconnect"
#define SETTINGS_CONNECTION_AUTOCONNECT_DEFAULT     false
#define SETTINGS_CONNECTION_RECONNECT               "connection/reconnect"
#define SETTINGS_CONNECTION_RECONNECT_DEFAULT       true
#define SETTINGS_CONNECTION_AUTOJOIN                "connection/autojoin"
#define SETTINGS_CONNECTION_AUTOJOIN_DEFAULT        true
#define SETTINGS_CONNECTION_QUERYMAXPAYLOAD         "connection/query-max-payload"
#define SETTINGS_CONNECTION_QUERYMAXPAYLOAD_DEFAULT false
#define SETTINGS_CONNECTION_SUBSCRIBE_USERMSG       "connection/subscribe-usermsg"
#define SETTINGS_CONNECTION_SUBSCRIBE_USERMSG_DEFAULT true
#define SETTINGS_CONNECTION_SUBSCRIBE_CHANNELMSG    "connection/subscribe-channelmsg"
#define SETTINGS_CONNECTION_SUBSCRIBE_CHANNELMSG_DEFAULT true
#define SETTINGS_CONNECTION_SUBSCRIBE_BROADCASTMSG  "connection/subscribe-broadcastmsg"
#define SETTINGS_CONNECTION_SUBSCRIBE_BROADCASTMSG_DEFAULT true
#define SETTINGS_CONNECTION_SUBSCRIBE_VOICE         "connection/subscribe-audio"
#define SETTINGS_CONNECTION_SUBSCRIBE_VOICE_DEFAULT true
#define SETTINGS_CONNECTION_SUBSCRIBE_VIDEOCAPTURE  "connection/subscribe-video"
#define SETTINGS_CONNECTION_SUBSCRIBE_VIDEOCAPTURE_DEFAULT true
#define SETTINGS_CONNECTION_SUBSCRIBE_DESKTOP       "connection/subscribe-desktop"
#define SETTINGS_CONNECTION_SUBSCRIBE_DESKTOP_DEFAULT true
#define SETTINGS_CONNECTION_SUBSCRIBE_MEDIAFILE     "connection/subscribe-media-file"
#define SETTINGS_CONNECTION_SUBSCRIBE_MEDIAFILE_DEFAULT true
#define SETTINGS_CONNECTION_TCPPORT                 "connection/localtcpport"
#define SETTINGS_CONNECTION_UDPPORT                 "connection/localudpport"
#define SETTINGS_CONNECTION_HEAR_MYSELF                            "connection/hear-myself"
#define SETTINGS_CONNECTION_HEAR_MYSELF_DEFAULT                            false

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
#define SETTINGS_SOUND_MICROPHONEGAIN_GAIN_DEFAULT  50
#define SETTINGS_SOUND_MEDIASTREAM_VOLUME           "soundsystem/mediastream"
#define SETTINGS_SOUND_MEDIASTREAM_VOLUME_DEFAULT   100
#define SETTINGS_SOUND_VOICEACTIVATIONLEVEL         "soundsystem/voice-activation-level"
#define SETTINGS_SOUND_VOICEACTIVATIONLEVEL_DEFAULT 2
#define SETTINGS_SOUND_ECHOCANCEL                   "soundsystem/echocancellation"
#define SETTINGS_SOUND_ECHOCANCEL_DEFAULT           DEFAULT_ECHO_ENABLE
#define SETTINGS_SOUND_AGC                          "soundsystem/agc"
#define SETTINGS_SOUND_AGC_DEFAULT                  DEFAULT_AGC_ENABLE
#define SETTINGS_SOUND_DENOISING                    "soundsystem/denoising"
#define SETTINGS_SOUND_DENOISING_DEFAULT            DEFAULT_DENOISE_ENABLE
#define SETTINGS_SOUND_SOUNDOUT_MUTE_LEFT           "soundsystem/sndoutput-mute-left"
#define SETTINGS_SOUND_SOUNDOUT_MUTE_LEFT_DEFAULT   false
#define SETTINGS_SOUND_SOUNDOUT_MUTE_RIGHT          "soundsystem/sndoutput-mute-right"
#define SETTINGS_SOUND_SOUNDOUT_MUTE_RIGHT_DEFAULT  false

#define SETTINGS_SOUNDEVENT_NEWUSER                 "soundevents/newuser"
#define SETTINGS_SOUNDEVENT_NEWUSER_DEFAULT                 (SOUNDSPATH"/newuser.wav")
#define SETTINGS_SOUNDEVENT_REMOVEUSER              "soundevents/removeuser"
#define SETTINGS_SOUNDEVENT_REMOVEUSER_DEFAULT              (SOUNDSPATH"/removeuser.wav")
#define SETTINGS_SOUNDEVENT_SERVERLOST              "soundevents/server-lost"
#define SETTINGS_SOUNDEVENT_SERVERLOST_DEFAULT              (SOUNDSPATH"/serverlost.wav")
#define SETTINGS_SOUNDEVENT_USERMSG                 "soundevents/user-msg"
#define SETTINGS_SOUNDEVENT_USERMSG_DEFAULT                 (SOUNDSPATH"/user_msg.wav")
#define SETTINGS_SOUNDEVENT_USERMSGSENT             "soundevents/sound-send"
#define SETTINGS_SOUNDEVENT_USERMSGSENT_DEFAULT                 (SOUNDSPATH"/user_msg_sent.wav")
#define SETTINGS_SOUNDEVENT_CHANNELMSG              "soundevents/channel-msg"
#define SETTINGS_SOUNDEVENT_CHANNELMSG_DEFAULT              (SOUNDSPATH"/channel_msg.wav")
#define SETTINGS_SOUNDEVENT_CHANNELMSGSENT             "soundevents/sound-channel-send"
#define SETTINGS_SOUNDEVENT_CHANNELMSGSENT_DEFAULT              (SOUNDSPATH"/channel_msg_sent.wav")
#define SETTINGS_SOUNDEVENT_BROADCASTMSG            "soundevents/broadcast-msg"
#define SETTINGS_SOUNDEVENT_BROADCASTMSG_DEFAULT            (SOUNDSPATH"/broadcast_msg.wav")
#define SETTINGS_SOUNDEVENT_HOTKEY                  "soundevents/hotkey-pressed"
#define SETTINGS_SOUNDEVENT_HOTKEY_DEFAULT                  (SOUNDSPATH"/hotkey.wav")
#define SETTINGS_SOUNDEVENT_SILENCE                 "soundevents/channel-silent"
#define SETTINGS_SOUNDEVENT_NEWVIDEO                "soundevents/new-video-session"
#define SETTINGS_SOUNDEVENT_NEWVIDEO_DEFAULT                (SOUNDSPATH"/videosession.wav")
#define SETTINGS_SOUNDEVENT_NEWDESKTOP              "soundevents/new-desktop-session"
#define SETTINGS_SOUNDEVENT_NEWDESKTOP_DEFAULT              (SOUNDSPATH"/desktopsession.wav")
#define SETTINGS_SOUNDEVENT_FILESUPD                "soundevents/files-updated"
#define SETTINGS_SOUNDEVENT_FILESUPD_DEFAULT                (SOUNDSPATH"/fileupdate.wav")
#define SETTINGS_SOUNDEVENT_FILETXDONE              "soundevents/file-tx-done"
#define SETTINGS_SOUNDEVENT_FILETXDONE_DEFAULT              (SOUNDSPATH"/filetx_complete.wav")
#define SETTINGS_SOUNDEVENT_QUESTIONMODE            "soundevents/question-mode"
#define SETTINGS_SOUNDEVENT_QUESTIONMODE_DEFAULT            (SOUNDSPATH"/questionmode.wav")
#define SETTINGS_SOUNDEVENT_DESKTOPACCESS           "soundevents/desktop-access-request"
#define SETTINGS_SOUNDEVENT_DESKTOPACCESS_DEFAULT           (SOUNDSPATH"/desktopaccessreq.wav")
#define SETTINGS_SOUNDEVENT_USERLOGGEDIN           "soundevents/user-logged-in"
#define SETTINGS_SOUNDEVENT_USERLOGGEDIN_DEFAULT           (SOUNDSPATH"/logged_on.wav")
#define SETTINGS_SOUNDEVENT_USERLOGGEDOUT           "soundevents/user-logged-out"
#define SETTINGS_SOUNDEVENT_USERLOGGEDOUT_DEFAULT           (SOUNDSPATH"/logged_off.wav")
#define SETTINGS_SOUNDEVENT_VOICEACTON           "soundevents/voice-act-on"
#define SETTINGS_SOUNDEVENT_VOICEACTON_DEFAULT           (SOUNDSPATH"/vox_enable.wav")
#define SETTINGS_SOUNDEVENT_VOICEACTOFF           "soundevents/voice-act-off"
#define SETTINGS_SOUNDEVENT_VOICEACTOFF_DEFAULT           (SOUNDSPATH"/vox_disable.wav")
#define SETTINGS_SOUNDEVENT_MUTEALLON           "soundevents/mute-all"
#define SETTINGS_SOUNDEVENT_MUTEALLON_DEFAULT           (SOUNDSPATH"/mute_all.wav")
#define SETTINGS_SOUNDEVENT_MUTEALLOFF           "soundevents/unmute-all"
#define SETTINGS_SOUNDEVENT_MUTEALLOFF_DEFAULT           (SOUNDSPATH"/unmute_all.wav")
#define SETTINGS_SOUNDEVENT_TRANSMITQUEUE_HEAD           "soundevents/transmit-queue-head"
#define SETTINGS_SOUNDEVENT_TRANSMITQUEUE_HEAD_DEFAULT           (SOUNDSPATH"/txqueue_start.wav")
#define SETTINGS_SOUNDEVENT_TRANSMITQUEUE_STOP           "soundevents/transmit-queue-stop"
#define SETTINGS_SOUNDEVENT_TRANSMITQUEUE_STOP_DEFAULT           (SOUNDSPATH"/txqueue_stop.wav")
#define SETTINGS_SOUNDEVENT_VOICEACTTRIG           "soundevents/voice-activation-triggered"
#define SETTINGS_SOUNDEVENT_VOICEACTTRIG_DEFAULT           (SOUNDSPATH"/voiceact_on.wav")
#define SETTINGS_SOUNDEVENT_VOICEACTSTOP           "soundevents/voice-activation-stop"
#define SETTINGS_SOUNDEVENT_VOICEACTSTOP_DEFAULT           (SOUNDSPATH"/voiceact_off.wav")
#define SETTINGS_SOUNDEVENT_VOICEACTMEON           "soundevents/voice-act-me-on"
#define SETTINGS_SOUNDEVENT_VOICEACTMEON_DEFAULT           (SOUNDSPATH"/vox_me_enable.wav")
#define SETTINGS_SOUNDEVENT_VOICEACTMEOFF           "soundevents/voice-act-me-off"
#define SETTINGS_SOUNDEVENT_VOICEACTMEOFF_DEFAULT           (SOUNDSPATH"/vox_me_disable.wav")
#define SETTINGS_SOUNDEVENT_INTERCEPT           "soundevents/intercept"
#define SETTINGS_SOUNDEVENT_INTERCEPT_DEFAULT           (SOUNDSPATH"/intercept.wav")
#define SETTINGS_SOUNDEVENT_INTERCEPTEND           "soundevents/intercept-stopped"
#define SETTINGS_SOUNDEVENT_INTERCEPTEND_DEFAULT           (SOUNDSPATH"/interceptEnd.wav")
#define SETTINGS_SOUNDEVENT_TYPING           "soundevents/user-typing"
#define SETTINGS_SOUNDEVENT_TYPING_DEFAULT           (SOUNDSPATH"/typing.wav")
#define SETTINGS_SOUNDS_PACK           "soundevents/sounds-pack"
#define SETTINGS_SOUNDS_PACK_DEFAULT           QT_TRANSLATE_NOOP("MainWindow", "Default")
#define SETTINGS_SOUNDEVENT_ENABLE                   "soundevents/sounds-enable"
#define SETTINGS_SOUNDEVENT_ENABLE_DEFAULT           true
#define SETTINGS_SOUNDEVENT_VOLUME                 "soundevents/volume"
#define SETTINGS_SOUNDEVENT_VOLUME_DEFAULT                 100
#define SETTINGS_SOUNDEVENT_PLAYBACKMODE            "soundevents/playback-mode"
#define SETTINGS_SOUNDEVENT_PLAYBACKMODE_DEFAULT    PLAYBACKMODE_DEFAULT
#define SETTINGS_SOUNDEVENT_ACTIVEEVENTS                   "soundevents/active-events"
#define SETTINGS_SOUNDEVENT_ACTIVEEVENTS_DEFAULT    SoundEvents(SOUNDEVENT_NEWUSER | \
                                                    SOUNDEVENT_REMOVEUSER | \
                                                    SOUNDEVENT_SERVERLOST | \
                                                    SOUNDEVENT_USERMSG | \
                                                    SOUNDEVENT_USERMSGSENT | \
                                                    SOUNDEVENT_CHANNELMSG | \
                                                    SOUNDEVENT_CHANNELMSGSENT | \
                                                    SOUNDEVENT_BROADCASTMSG | \
                                                    SOUNDEVENT_HOTKEY | \
                                                    SOUNDEVENT_NEWVIDEO | \
                                                    SOUNDEVENT_NEWDESKTOP | \
                                                    SOUNDEVENT_FILESUPD | \
                                                    SOUNDEVENT_FILETXDONE | \
                                                    SOUNDEVENT_QUESTIONMODE | \
                                                    SOUNDEVENT_DESKTOPACCESS | \
                                                    SOUNDEVENT_USERLOGGEDIN | \
                                                    SOUNDEVENT_USERLOGGEDOUT | \
                                                    SOUNDEVENT_VOICEACTON | \
                                                    SOUNDEVENT_VOICEACTOFF | \
                                                    SOUNDEVENT_MUTEALLON | \
                                                    SOUNDEVENT_MUTEALLOFF | \
                                                    SOUNDEVENT_TRANSMITQUEUE_HEAD | \
                                                    SOUNDEVENT_TRANSMITQUEUE_STOP | \
                                                    SOUNDEVENT_VOICEACTTRIG | \
                                                    SOUNDEVENT_VOICEACTSTOP | \
                                                    SOUNDEVENT_VOICEACTMEON | \
                                                    SOUNDEVENT_VOICEACTMEOFF | \
                                                    SOUNDEVENT_INTERCEPT | \
                                                    SOUNDEVENT_INTERCEPTEND | \
                                                    SOUNDEVENT_TYPING)

#define SETTINGS_SHORTCUTS_VOICEACTIVATION          "shortcuts/voiceactivation"
#define SETTINGS_SHORTCUTS_INCVOLUME                "shortcuts/volume-inc"
#define SETTINGS_SHORTCUTS_DECVOLUME                "shortcuts/volume-dec"
#define SETTINGS_SHORTCUTS_MUTEALL                  "shortcuts/muteall"
#define SETTINGS_SHORTCUTS_INCVOICEGAIN             "shortcuts/voicegain-inc"
#define SETTINGS_SHORTCUTS_DECVOICEGAIN             "shortcuts/voicegain-dec"
#define SETTINGS_SHORTCUTS_VIDEOTX                  "shortcuts/video-tx"
#define SETTINGS_SHORTCUTS_RESTARTSOUNDDEVICES      "shortcuts/reinit-sounddevs"
#define SETTINGS_SHORTCUTS_SHOWHIDEWINDOW           "shortcuts/show-hide-window"

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

#define SETTINGS_TTS_ENABLE                   "texttospeech/tts-enable"
#define SETTINGS_TTS_ENABLE_DEFAULT           true
#define SETTINGS_TTS_ACTIVEEVENTS                   "texttospeech/active-events"
#define SETTINGS_TTS_ACTIVEEVENTS_DEFAULT           TTSEvents(TTS_USER_LOGGEDIN | \
                                                    TTS_USER_LOGGEDOUT | \
                                                    TTS_USER_JOINED_SAME | \
                                                    TTS_USER_LEFT_SAME | \
                                                    TTS_USER_TEXTMSG_PRIVATE | \
                                                    TTS_USER_TEXTMSG_CHANNEL | \
                                                    TTS_USER_TEXTMSG_BROADCAST | \
                                                    TTS_FILE_ADD | \
                                                    TTS_FILE_REMOVE | \
                                                    TTS_MENU_ACTIONS)
#define SETTINGS_TTS_ENGINE                         "texttospeech/tts-engine"
#define SETTINGS_TTS_ENGINE_DEFAULT                 TTSENGINE_NONE
#define SETTINGS_TTS_LOCALE                         "texttospeech/tts-language"
#define SETTINGS_TTS_VOICE                         "texttospeech/tts-voice"
#define SETTINGS_TTS_RATE                         "texttospeech/tts-rate"
#define SETTINGS_TTS_RATE_DEFAULT                 0.0
#define SETTINGS_TTS_VOLUME                         "texttospeech/tts-volume"
#define SETTINGS_TTS_VOLUME_DEFAULT                 0.5
#if defined(Q_OS_LINUX)
#define SETTINGS_TTS_TIMESTAMP                         "texttospeech/tts-timestamp"
#define SETTINGS_TTS_TIMESTAMP_DEFAULT                 1000
#elif defined(Q_OS_WIN)
#define SETTINGS_TTS_SAPI                         "texttospeech/force-sapi"
#define SETTINGS_TTS_SAPI_DEFAULT                 false
#define SETTINGS_TTS_TRY_SAPI                         "texttospeech/try-sapi"
#define SETTINGS_TTS_TRY_SAPI_DEFAULT                 true
#define SETTINGS_TTS_OUTPUT_MODE                         "texttospeech/output-mode"
#define SETTINGS_TTS_OUTPUT_MODE_DEFAULT                 TTS_OUTPUTMODE_SPEECHBRAILLE
#elif defined(Q_OS_DARWIN)
#if QT_VERSION < QT_VERSION_CHECK(6,4,0)
#define SETTINGS_TTS_SPEAKLISTS                         "texttospeech/speak-lists"
#define SETTINGS_TTS_SPEAKLISTS_DEFAULT                 true
#endif
#endif

#define SETTINGS_TTSMSG_USER_LOGGEDIN                         "texttospeech/messages/user-logged-in"
#define SETTINGS_TTSMSG_USER_LOGGEDIN_DEFAULT                 QT_TRANSLATE_NOOP("UtilTTS", "{user} has logged in on {server}")
#define SETTINGS_TTSMSG_USER_LOGGEDOUT                         "texttospeech/messages/user-logged-out"
#define SETTINGS_TTSMSG_USER_LOGGEDOUT_DEFAULT                 QT_TRANSLATE_NOOP("UtilTTS", "{user} has logged out from {server}")
#define SETTINGS_TTSMSG_USER_JOINED                         "texttospeech/messages/user-joined-channel"
#define SETTINGS_TTSMSG_USER_JOINED_DEFAULT                 QT_TRANSLATE_NOOP("UtilTTS", "{user} joined channel {channel}")
#define SETTINGS_TTSMSG_USER_LEFT                         "texttospeech/messages/user-left-channel"
#define SETTINGS_TTSMSG_USER_LEFT_DEFAULT                 QT_TRANSLATE_NOOP("UtilTTS", "{user} left channel {channel}")
#define SETTINGS_TTSMSG_USER_JOINED_SAME                         "texttospeech/messages/user-joined-same-channel"
#define SETTINGS_TTSMSG_USER_JOINED_SAME_DEFAULT                 QT_TRANSLATE_NOOP("UtilTTS", "{user} joined channel")
#define SETTINGS_TTSMSG_USER_LEFT_SAME                         "texttospeech/messages/user-left-same-channel"
#define SETTINGS_TTSMSG_USER_LEFT_SAME_DEFAULT                 QT_TRANSLATE_NOOP("UtilTTS", "{user} left channel")
#define SETTINGS_TTSMSG_PRIVATEMSG                         "texttospeech/messages/private-message"
#define SETTINGS_TTSMSG_PRIVATEMSG_DEFAULT                 QT_TRANSLATE_NOOP("UtilTTS", "Private message from {user}: {message}")
#define SETTINGS_TTSMSG_PRIVATEMSGSEND                         "texttospeech/messages/private-message-send"
#define SETTINGS_TTSMSG_PRIVATEMSGSEND_DEFAULT                 QT_TRANSLATE_NOOP("UtilTTS", "Private message sent: {message}")
#define SETTINGS_TTSMSG_TYPING                         "texttospeech/messages/private-message-typing"
#define SETTINGS_TTSMSG_TYPING_DEFAULT                 QT_TRANSLATE_NOOP("UtilTTS", "{user} is typing...")
#define SETTINGS_TTSMSG_QUESTIONMODE                         "texttospeech/messages/user-set-question-mode"
#define SETTINGS_TTSMSG_QUESTIONMODE_DEFAULT                 QT_TRANSLATE_NOOP("UtilTTS", "{user} set question mode")
#define SETTINGS_TTSMSG_CHANNELMSG                         "texttospeech/messages/channel-message"
#define SETTINGS_TTSMSG_CHANNELMSG_DEFAULT                 QT_TRANSLATE_NOOP("UtilTTS", "Channel message from {user}: {message}")
#define SETTINGS_TTSMSG_CHANNELMSGSEND                         "texttospeech/messages/channel-message-send"
#define SETTINGS_TTSMSG_CHANNELMSGSEND_DEFAULT                 QT_TRANSLATE_NOOP("UtilTTS", "Channel message sent: {message}")
#define SETTINGS_TTSMSG_BROADCASTMSG                         "texttospeech/messages/broadcast-message"
#define SETTINGS_TTSMSG_BROADCASTMSG_DEFAULT                 QT_TRANSLATE_NOOP("UtilTTS", "Broadcast message from {user}: {message}")
#define SETTINGS_TTSMSG_BROADCASTMSGSEND                         "texttospeech/messages/broadcast-message-send"
#define SETTINGS_TTSMSG_BROADCASTMSGSEND_DEFAULT                 QT_TRANSLATE_NOOP("UtilTTS", "Broadcast message sent: {message}")
#define SETTINGS_TTSMSG_SUBCHANGE                         "texttospeech/messages/subscription-change"
#define SETTINGS_TTSMSG_SUBCHANGE_DEFAULT                 QT_TRANSLATE_NOOP("UtilTTS", "Subscription \"{type}\" {state} for {user}")
#define SETTINGS_TTSMSG_CLASSROOM                         "texttospeech/messages/classroom-transmission-change"
#define SETTINGS_TTSMSG_CLASSROOM_DEFAULT                 QT_TRANSLATE_NOOP("UtilTTS", "Transmission \"{type}\" {state} for {user}")
#define SETTINGS_TTSMSG_FILE_ADDED                         "texttospeech/messages/file-added"
#define SETTINGS_TTSMSG_FILE_ADDED_DEFAULT                 QT_TRANSLATE_NOOP("UtilTTS", "File {filename} added by {user}")
#define SETTINGS_TTSMSG_FILE_REMOVED                         "texttospeech/messages/file-removed"
#define SETTINGS_TTSMSG_FILE_REMOVED_DEFAULT                 QT_TRANSLATE_NOOP("UtilTTS", "File {file} removed by {user}")

#define SETTINGS_STATUSBARMSG_USER_LOGGEDIN                         "status-bar-messages/user-logged-in"
#define SETTINGS_STATUSBARMSG_USER_LOGGEDIN_DEFAULT                 QT_TRANSLATE_NOOP("UtilUI", "{user} has logged in")
#define SETTINGS_STATUSBARMSG_USER_LOGGEDOUT                         "status-bar-messages/user-logged-out"
#define SETTINGS_STATUSBARMSG_USER_LOGGEDOUT_DEFAULT                 QT_TRANSLATE_NOOP("UtilUI", "{user} has logged out")
#define SETTINGS_STATUSBARMSG_USER_JOINED                         "status-bar-messages/user-joined-channel"
#define SETTINGS_STATUSBARMSG_USER_JOINED_DEFAULT                 QT_TRANSLATE_NOOP("UtilUI", "{user} joined channel {channel}")
#define SETTINGS_STATUSBARMSG_USER_LEFT                         "status-bar-messages/user-left-channel"
#define SETTINGS_STATUSBARMSG_USER_LEFT_DEFAULT                 QT_TRANSLATE_NOOP("UtilUI", "{user} left channel {channel}")
#define SETTINGS_STATUSBARMSG_USER_JOINED_SAME                         "status-bar-messages/user-joined-same-channel"
#define SETTINGS_STATUSBARMSG_USER_JOINED_SAME_DEFAULT                 QT_TRANSLATE_NOOP("UtilUI", "{user} joined channel")
#define SETTINGS_STATUSBARMSG_USER_LEFT_SAME                         "status-bar-messages/user-left-same-channel"
#define SETTINGS_STATUSBARMSG_USER_LEFT_SAME_DEFAULT                 QT_TRANSLATE_NOOP("UtilUI", "{user} left channel")
#define SETTINGS_STATUSBARMSG_SUBCHANGE                         "status-bar-messages/subscription-change"
#define SETTINGS_STATUSBARMSG_SUBCHANGE_DEFAULT                 QT_TRANSLATE_NOOP("UtilUI", "Subscription \"{type}\" {state} for {user}")
#define SETTINGS_STATUSBARMSG_CLASSROOM                         "status-bar-messages/classroom-transmission-change"
#define SETTINGS_STATUSBARMSG_CLASSROOM_DEFAULT                 QT_TRANSLATE_NOOP("UtilUI", "Transmission \"{type}\" {state} for {user}")
#define SETTINGS_STATUSBARMSG_FILE_ADDED                         "status-bar-messages/file-added"
#define SETTINGS_STATUSBARMSG_FILE_ADDED_DEFAULT                 QT_TRANSLATE_NOOP("UtilUI", "File {filename} added by {user}")
#define SETTINGS_STATUSBARMSG_FILE_REMOVED                         "status-bar-messages/file-removed"
#define SETTINGS_STATUSBARMSG_FILE_REMOVED_DEFAULT                 QT_TRANSLATE_NOOP("UtilTTS", "File {file} removed by {user}")

#define SETTINGS_MEDIASTORAGE_MODE                  "media-storage/audio-storage-mode"
#define SETTINGS_MEDIASTORAGE_AUDIOFOLDER           "media-storage/audio-storage-folder"
#define SETTINGS_MEDIASTORAGE_FILEFORMAT            "media-storage/audio-storage-format"
#define SETTINGS_MEDIASTORAGE_STREAMTYPES           "media-storage/audio-storage-streamtypes"
#define SETTINGS_MEDIASTORAGE_STREAMTYPES_DEFAULT   STREAMTYPE_VOICE
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
#define SETTINGS_STREAMMEDIA_WEBRTC_GAINCTL_ENABLE          "stream-media/webrtcaudiopreprocessor/gain-enable"
#define SETTINGS_STREAMMEDIA_WEBRTC_GAINDB                  "stream-media/webrtcaudiopreprocessor/gain-db"
#define SETTINGS_STREAMMEDIA_WEBRTC_SAT_PROT_ENABLE         "stream-media/webrtcaudiopreprocessor/sat-protection-enable"
#define SETTINGS_STREAMMEDIA_WEBRTC_INIT_SAT_MARGIN_DB      "stream-media/webrtcaudiopreprocessor/init-sat-margin-db"
#define SETTINGS_STREAMMEDIA_WEBRTC_EXTRA_SAT_MARGIN_DB     "stream-media/webrtcaudiopreprocessor/extra-sat-margin-db"
#define SETTINGS_STREAMMEDIA_WEBRTC_MAXGAIN_DBSEC           "stream-media/webrtcaudiopreprocessor/maxgain-dbsec"
#define SETTINGS_STREAMMEDIA_WEBRTC_MAX_OUT_NOISE           "stream-media/webrtcaudiopreprocessor/max-out-noise"
#define SETTINGS_STREAMMEDIA_WEBRTC_NOISESUPPRESS_ENABLE    "stream-media/webrtcaudiopreprocessor/noise-suppress-enable"
#define SETTINGS_STREAMMEDIA_WEBRTC_NOISESUPPRESS_LEVEL     "stream-media/webrtcaudiopreprocessor/noise-suppress-level"

#define SETTINGS_DESKTOPSHARE_MODE                  "desktop-share/desktop-share-mode"
#define SETTINGS_DESKTOPSHARE_BMPMODE               "desktop-share/desktop-bmp-mode"
#define SETTINGS_DESKTOPSHARE_INTERVAL              "desktop-share/desktop-share-interval"
#define SETTINGS_DESKTOPSHARE_CURSOR                "desktop-share/desktop-cursor"
#define SETTINGS_DESKTOPSHARE_WINDOWTITLE           "desktop-share/desktop-window-title"

#define SETTINGS_LATESTHOST_NAME                "latesthosts/%1_name"
#define SETTINGS_LATESTHOST_HOSTADDR                "latesthosts/%1_hostaddr"
#define SETTINGS_LATESTHOST_TCPPORT                 "latesthosts/%1_tcpport"
#define SETTINGS_LATESTHOST_UDPPORT                 "latesthosts/%1_udpport"
#define SETTINGS_LATESTHOST_ENCRYPTED               "latesthosts/%1_encrypted"
#define SETTINGS_LATESTHOST_ENCRYPTED_CADATA        "latesthosts/%1_cadata"
#define SETTINGS_LATESTHOST_ENCRYPTED_CERTDATA      "latesthosts/%1_certdata"
#define SETTINGS_LATESTHOST_ENCRYPTED_KEYDATA       "latesthosts/%1_keydata"
#define SETTINGS_LATESTHOST_ENCRYPTED_VERIFYPEER    "latesthosts/%1_verifypeer"
#define SETTINGS_LATESTHOST_USERNAME                "latesthosts/%1_username"
#define SETTINGS_LATESTHOST_PASSWORD                "latesthosts/%1_password"
#define SETTINGS_LATESTHOST_NICKNAME                "latesthosts/%1_nickname"
#define SETTINGS_LATESTHOST_CHANNEL                 "latesthosts/%1_channel"
#define SETTINGS_LATESTHOST_CHANNELPASSWD           "latesthosts/%1_chanpassword"

#define SETTINGS_SERVERENTRIES_NAME                 "serverentries/%1_name"
#define SETTINGS_SERVERENTRIES_HOSTADDR             "serverentries/%1_hostaddr"
#define SETTINGS_SERVERENTRIES_TCPPORT              "serverentries/%1_tcpport"
#define SETTINGS_SERVERENTRIES_UDPPORT              "serverentries/%1_udpport"
#define SETTINGS_SERVERENTRIES_ENCRYPTED            "serverentries/%1_encrypted"
#define SETTINGS_SERVERENTRIES_ENCRYPTED_CADATA     "serverentries/%1_cadata"
#define SETTINGS_SERVERENTRIES_ENCRYPTED_CERTDATA   "serverentries/%1_certdata"
#define SETTINGS_SERVERENTRIES_ENCRYPTED_KEYDATA    "serverentries/%1_keydata"
#define SETTINGS_SERVERENTRIES_ENCRYPTED_VERIFYPEER "serverentries/%1_verifypeer"
#define SETTINGS_SERVERENTRIES_USERNAME             "serverentries/%1_username"
#define SETTINGS_SERVERENTRIES_PASSWORD             "serverentries/%1_password"
#define SETTINGS_SERVERENTRIES_NICKNAME             "serverentries/%1_nickname"
#define SETTINGS_SERVERENTRIES_LASTCHANNEL              "serverentries/%1_join-last-channel"
#define SETTINGS_SERVERENTRIES_CHANNEL              "serverentries/%1_channel"
#define SETTINGS_SERVERENTRIES_CHANNELPASSWD        "serverentries/%1_chanpassword"

#define SETTINGS_DESKTOPACCESS_HOSTADDR             "desktopaccess/%1_hostaddr"
#define SETTINGS_DESKTOPACCESS_TCPPORT              "desktopaccess/%1_tcpport"
#define SETTINGS_DESKTOPACCESS_CHANNEL              "desktopaccess/%1_%2_channel"
#define SETTINGS_DESKTOPACCESS_USERNAME             "desktopaccess/%1_%2_username"

#define SETTINGS_LAST_DIRECTORY                     "history/last-directory"
#define SETTINGS_FIREWALL_ADD                       "history/firewall-add"

#define SETTINGS_KEEP_DISCONNECTED_USERS                            "online-users/keep-disconnected-users"
#define SETTINGS_KEEP_DISCONNECTED_USERS_DEFAULT                            false

#endif

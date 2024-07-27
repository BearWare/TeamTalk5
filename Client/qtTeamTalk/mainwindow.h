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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ui_mainwindow.h"

#include <QMap>
#include <QSet>
#include <QQueue>
#include <QSystemTrayIcon>
#include <QNetworkAccessManager>
#include <QSortFilterProxyModel>

#include "common.h"
#include "textmessagecontainer.h"
#include "utilsound.h"
#include "utilui.h"
#include "utilhotkey.h"

#ifdef Q_OS_LINUX
#include <QX11Info>
#endif

#if defined(Q_OS_DARWIN)
#include <Carbon/Carbon.h>
#endif

//Commands which we're interested in knowning when have completed
enum CommandComplete
{
    CMD_COMPLETE_UNKNOWN,

    CMD_COMPLETE_LOGIN,
    CMD_COMPLETE_JOINCHANNEL,
    CMD_COMPLETE_LEAVECHANNEL,
    CMD_COMPLETE_LIST_SERVERBANS,
    CMD_COMPLETE_LIST_CHANNELBANS,
    CMD_COMPLETE_LISTACCOUNTS,
    CMD_COMPLETE_SUBSCRIBE,
    CMD_COMPLETE_UNSUBSCRIBE,

    CMD_COMPLETE_SAVECONFIG
};

enum TimerEvent
{
    TIMER_PROCESS_TTEVENT = 1,
    TIMER_ONE_SECOND,
    TIMER_VUMETER_UPDATE,
    TIMER_SEND_DESKTOPCURSOR,
    TIMER_RECONNECT,
    TIMER_STATUSMSG,
    TIMER_SEND_DESKTOPWINDOW,
    TIMER_APP_UPDATE,
};

enum
{
    TAB_CHAT,
    TAB_FILES,
    TAB_MEDIA,
    TAB_VIDEO,
    TAB_DESKTOP,

    TAB_COUNT
};

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    MainWindow(const QString& cfgfile);
    ~MainWindow();

    void loadSettings();

    bool parseArgs(const QStringList& args);
#ifdef Q_OS_LINUX
    //X11 hotkeys
    void keysActive(quint32 keycode, quint32 mods, bool active);
#endif

    void hotkeyToggle(HotKeyID id, bool active);

protected:
    void timerEvent(QTimerEvent *event) override;
    void changeEvent(QEvent* event) override;
    void keyPressEvent(QKeyEvent* e) override;
    void closeEvent(QCloseEvent* event) override;

#if defined(Q_OS_WIN32)

#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    bool nativeEvent(const QByteArray& eventType, void* message,
                     long* result);
#else
    bool nativeEvent(const QByteArray& eventType, void* message,
                     qintptr* result);
#endif /* QT_VERSION */

#endif /* Q_OS_WIN32 */
private:
    Ui::MainWindow ui;
    class FilesModel* m_filesmodel;
    QSortFilterProxyModel* m_proxyFilesModel;
    QSystemTrayIcon* m_sysicon;
    QMenu* m_sysmenu;
    QLabel* m_pttlabel;
    QLabel* m_pinglabel;
    QLabel* m_dtxlabel;
    QProgressBar* m_dtxprogress;

    //keep track for active commands awaiting replies
    typedef QMap<int, CommandComplete> cmdreply_t;
    cmdreply_t m_commands;
    int m_current_cmdid;

    //queue of messages in statusbar
    QQueue<QString> m_statusmsg;
    typedef QMap<int, TimerEvent> timers_t;
    timers_t m_timers;

    //keep track of who is talking
    typedef QSet<int> talkingset_t;
    talkingset_t m_talking;

    //ignore video frames from these users
    typedef QSet<int> video_exclude_user_t;
    video_exclude_user_t m_vid_exclude;

    typedef QMap<int, QString> chanpasswd_t;
    chanpasswd_t m_channel_passwd;

    //audio storage mode
    quint32 m_audiostorage_mode;

    //idle time caused away status mode
    bool m_idled_out;
    int m_statusmode;

    //current host
    HostEntry m_host;
    //client instance's user account
    UserAccount m_myuseraccount;
    //ping and tx/rx information
    ClientStatistics m_clientstats;
    //last channel that were joined by the client
    Channel m_last_channel;
    //server properties
    ServerProperties m_srvprop;
    //current channel
    Channel m_mychannel;
    //channel log file
    QFile m_logChan;
    // active sound devices
    SoundDevice m_devin = {}, m_devout = {};

    //op text messsage dialogs (userid -> dlg)
    typedef QMap<int, class TextMessageDlg*> usermsg_t;
    usermsg_t m_usermsg;
    //active video dialogs (userid -> dlg)
    typedef QMap<int, class UserVideoDlg*> uservideo_t;
    //video map, both video capture and media files
    uservideo_t m_user_video;
    //active desktop dialogs (userid -> dlg)
    typedef QMap<int, class UserDesktopDlg*> userdesktop_t;
    userdesktop_t m_userdesktop;
    //users to move to another channel (drag/drop)
    userids_t m_moveusers;
    // user-cache of previous settings
    QMap<QString, UserCached> m_usercache;
    //online users in separate dialog
    class OnlineUsersDlg* m_onlineusersdlg;
    //user accounts in separate dialog
    class UserAccountsDlg* m_useraccountsdlg;
    //banned users in seperate dialog
    class BannedUsersDlg* m_bannedusersdlg;
    //server statistics in separate dialog
    class ServerStatisticsDlg* m_serverstatsdlg;
    //current desktop update session (0 reserved)
    int m_desktopsession_id, m_prev_desktopsession_id;
    //progress for current desktop update
    int m_desktopsession_total;
    int m_desktopsession_remain;
    bool m_desktopsend_on_completion;
    TextMessageContainer m_textmessages;
    QVector<UserAccount> m_useraccounts;
    QVector<BannedUser> m_bannedusers;
    //position of mouse cursor
    QPoint m_lastCursorPos;
    //list of desktop access users
    QVector<DesktopAccessEntry> m_desktopaccess_entries;
    // user being relayed
    int m_relayvoice_userid = 0, m_relaymediafile_userid = 0;

    void processTTMessage(const TTMessage& msg);
    void addStatusMsg(StatusBarEvent event, const QString& msg);
    void connectToServer();
    void disconnectFromServer();
    void login();
    void showTTErrorMessage(const ClientErrorMsg& msg,
                            CommandComplete cmd_type);

    // init selected sound devices
    void initSound();

    //command replies
    void cmdCompleteLoggedIn(int myuserid);
    void cmdCompleteListServers(CommandComplete complete);
    void cmdCompleteListUserAccounts();

    QString getTitle();
    void updateWindowTitle();
#if defined(Q_OS_WIN32)
    void firewallInstall();
#endif
    void subscribeCommon(bool checked, Subscriptions subs, int userid = 0);
    void processTextMessage(const MyTextMessage& textmsg);
    void processMyselfJoined(int channelid);
    void processMyselfLeft(int channelid);
    TextMessageDlg* getTextMessageDlg(int userid);
    void killLocalTimer(TimerEvent e);
    bool timerExists(TimerEvent e);
    void updateChannelFiles(int channelid);
    void updateUserSubscription(int userid);
    void updateIdleTimeout();
    /* 'mode' is mask of 'AudioStorageMode' */
    void updateAudioStorage(bool enable, AudioStorageMode mode);
    void updateAudioConfig();
    bool sendDesktopWindow();
    void restartSendDesktopWindowTimer();
    void sendDesktopCursor();
    QRect getSharedWindowRect();
    void processDesktopInput(int userid, const DesktopInput& input);
    void startStreamMediaFile();
    void stopStreamMediaFile();
    void updateMediaFileProgress(const MediaFileInfo& mfi);
    void loadHotKeys();
    void enableHotKey(HotKeyID id, const hotkey_t& hk);
    void disableHotKey(HotKeyID id);
    void pttHotKey(bool active);
    void checkAppUpdate(bool manualCheck = false);
    void toggleAllowStreamType(bool checked, int userid, int channelid, StreamType st);
    void toggleAllowStreamTypeForAll(bool checked, StreamType st);
    void toggleAllowStreamType(bool checked, StreamType st);
    void transmitOn(StreamType st);
    void relayAudioStream(int userid, StreamType st, bool enable);
    void enableVoiceActivation(bool checked, SoundEvent on = SOUNDEVENT_VOICEACTON, SoundEvent off = SOUNDEVENT_VOICEACTOFF);
    void updateClassroomChannel(const Channel& oldchan, const Channel& newchan);
    void moveUsersToChannel(int chanid);
#if defined(Q_OS_LINUX)
    void executeDesktopInput(const DesktopInput& input);
#endif

#if defined(Q_OS_WIN32)
    HWND m_hShareWnd;
#endif
#if defined(Q_OS_LINUX)
    //set of native key codes
    typedef QSet<quint32> keycomp_t;
    typedef QMap<HotKeyID, keycomp_t> reghotkeys_t;
    //the registered hotkeys
    reghotkeys_t m_hotkeys;
    Display* m_display;
    INT64 m_nWindowShareWnd;
#elif defined(Q_OS_DARWIN)
    typedef QMap<int,EventHotKeyRef> reghotkeys_t;
    reghotkeys_t m_hotkeys;
    INT64 m_nCGShareWnd;
#endif

private:
    /* menu items begin */
    void slotClientNewInstance(bool checked=false);
    void slotClientConnect(bool checked=false);
    void slotClientPreferences(bool checked=false);
    void slotClientSoundDevices();
    void slotClientAudioEffect();
    void slotClientRecordConversations(bool checked=false);
    void slotClientExit(bool checked=false);

    void slotMeChangeNickname(bool checked=false);
    void slotMeChangeStatus(bool checked=false);
    void slotMeEnablePushToTalk(bool checked=false);
    void slotMeHearMyself(bool checked=false);
    void slotMeEnableVoiceActivation(bool checked=false);
    void slotMeEnableVideoTransmission(bool checked=false);
    void slotMeEnableDesktopSharing(bool checked=false);
    void slotMeEnableTTS(bool checked=false);
    void slotMeEnableSounds(bool checked=false);

    void slotUsersViewUserInformationGrid(bool checked=false);
    void slotUsersSpeakUserInformationGrid(bool checked=false);
    void slotUsersMessagesGrid(bool checked=false);
    void slotUsersMuteVoiceGrid(bool checked=false);
    void slotUsersMuteMediaFileGrid(bool checked=false);
    void slotUsersVolumeGrid(bool checked=false);
    void slotUsersOpGrid(bool checked=false);
    void slotUsersKickFromChannel(bool checked=false);
    void slotUsersKickBanFromChannel(bool checked = false);
    void slotUsersKickFromServer(bool checked=false);
    void slotUsersKickBanFromServer(bool checked=false);
    void slotUsersSubscriptionsUserMsg(bool checked=false);
    void slotUsersSubscriptionsChannelMsg(bool checked=false);
    void slotUsersSubscriptionsBCastMsg(bool checked=false);
    void slotUsersSubscriptionsVoice(bool checked=false);
    void slotUsersSubscriptionsVideo(bool checked=false);
    void slotUsersSubscriptionsDesktop(bool checked=false);
    void slotUsersSubscriptionsDesktopInput(bool checked=false);
    void slotUsersSubscriptionsMediaFile(bool checked=false);
    void slotUsersSubscriptionsInterceptUserMsg(bool checked=false);
    void slotUsersSubscriptionsInterceptChannelMsg(bool checked=false);
    void slotUsersSubscriptionsInterceptVoice(bool checked=false);
    void slotUsersSubscriptionsInterceptVideo(bool checked=false);
    void slotUsersSubscriptionsInterceptDesktop(bool checked=false);
    void slotUsersSubscriptionsInterceptMediaFile(bool checked=false);
    void slotUsersAdvancedIncVolumeVoice();
    void slotUsersAdvancedDecVolumeVoice();
    void slotUsersAdvancedIncVolumeMediaFile();
    void slotUsersAdvancedDecVolumeMediaFile();
    void slotUsersAdvancedStoreForMove(int userid = 0);
    void slotUsersAdvancedMoveUsers();
    void slotUsersAdvancedMoveUsersDialog();
    void slotUsersAdvancedChanMsgAllowed(bool checked=false);
    void slotUsersAdvancedVoiceAllowed(bool checked=false);
    void slotUsersAdvancedVideoAllowed(bool checked=false);
    void slotUsersAdvancedDesktopAllowed(bool checked=false);
    void slotUsersAdvancedMediaFileAllowed(bool checked=false);
    void slotUsersAdvancedRelayUserVoice(bool checked = false);
    void slotUsersAdvancedRelayUserMediaFile(bool checked=false);
    void slotUsersMuteVoiceAll(bool checked=false);

    void slotChannelsCreateChannel(bool checked=false);
    void slotChannelsUpdateChannel(bool checked=false);
    void slotChannelsDeleteChannel(bool checked=false);
    void slotChannelsJoinChannel(bool checked=false);
    void slotChannelsLeaveChannel(bool checked=false);
    void slotChannelsViewChannelInfo(bool checked=false);
    void slotChannelsSpeakChannelInformationGrid(bool checked=false);
    void slotChannelsSpeakChannelStatusGrid(bool checked=false);
    void slotChannelsSpeakChannelStatus();
    void slotChannelsListBans(bool checked=false);
    void slotChannelsStreamMediaFile(bool checked=false);
    void slotPauseResumeStream();
    void slotChannelsUploadFile(bool checked=false);
    void slotChannelsDownloadFile(bool checked=false);
    void slotChannelsDeleteFile(bool checked=false);
    void slotChannelsGenerateTTUrl(bool checked=false);
    
    void slotServerUserAccounts(bool checked=false);
    void slotServerBannedUsers(bool checked=false);
    void slotServerOnlineUsers(bool checked=false);
    void slotServerBroadcastMessage(bool checked=false);
    void slotServerServerProperties(bool checked=false);
    void slotServerSaveConfiguration(bool checked=false);
    void slotServerServerStatistics(bool checked=false);

    void slotHelpManual(bool checked=false);
    void slotHelpResetPreferences(bool checked=false);
    void slotHelpVisitBearWare(bool checked=false);
    void slotHelpAbout(bool checked=false);
    /* menu items end */

    void slotConnectToLatest();
    void slotUsersViewUserInformation(int userid);
    void slotUsersSpeakUserInformation(int userid);
    void slotUsersMessages(int userid);
    void slotUsersMuteVoice(int userid, bool mute);
    void slotUsersMuteMediaFile(int userid, bool mute);
    void slotUsersVolume(int userid);
    void slotUsersOp(int userid, int chanid);
    void slotUsersKick(int userid, int chanid);
    void slotUsersKickBan(const User& user);

    void slotTreeSelectionChanged();
    void slotTreeContextMenu(const QPoint& pos);
    void slotFilesContextMenu(const QPoint& pos);
    void slotUpdateUI();
    void slotUpdateVideoTabUI();
    void slotUpdateDesktopTabUI();
    void slotUploadFiles(const QStringList& files);
    int getRemoteFileID(int channelid, const QString& filename);
    void slotSendChannelMessage();
    void slotUserDoubleClicked(int userid);
    void slotChannelDoubleClicked(int channelid);
    void slotNewMyselfTextMessage(const MyTextMessage& textmsg);
    void slotTextMessageClosed(int userid);
    void slotTransmitUsersChanged(int channelid, const QMap<int,StreamTypes>&);
    void slotChannelUpdate(const Channel& chan);
    void slotInitVideo();
    void slotAddUserVideo();
    void slotRemoveUserVideoGrid();
    void slotRemoveUserVideo(int userid);
    void slotDetachUserVideo(int userid = 0);
    void slotNewUserVideoDlg(int userid, const QSize& size);
    void slotUserVideoDlgClosing(int userid);
    void slotAddUserDesktopGrid();
    void slotRemoveUserDesktopGrid();
    void slotRemoveUserDesktop(int userid);
    void slotAccessUserDesktop(bool enable);
    void slotDetachUserDesktopGrid();
    void slotDetachUserDesktop(int userid, const QSize& size);
    void slotUserDesktopDlgClosing(int userid);
    void slotUserJoin(int channelid, const User& user);
    void slotUserLeft(int channelid, const User& user);
    void slotUserUpdate(const User& user);
    void slotToggleQuestionMode(bool checked);

    void slotUpdateVideoCount(int count);
    void slotUpdateDesktopCount(int count);

    void slotMasterVolumeChanged(int value);
    void slotMicrophoneGainChanged(int value);
    void slotVoiceActivationLevelChanged(int value);

    void slotTrayIconChange(QSystemTrayIcon::ActivationReason reason);

    void slotLoadTTFile(const QString& filepath);

    void slotSoftwareUpdateReply(QNetworkReply* reply, bool manualCheck = false);
    void slotBearWareAuthReply(QNetworkReply* reply);
    void slotCmdSuccess(int cmdid);
    void slotClosedOnlineUsersDlg(int);
    void slotClosedServerStatsDlg(int);
    void slotClosedUserAccountsDlg(int);
    void slotClosedBannedUsersDlg(int);
    void initialScreenReaderSetup();
    void startTTS();
    void slotTextChanged();
    void slotSpeakClientStats(bool checked=false);

    void clienteventConSuccess();
    void clienteventConFailed();
    void clienteventConCryptError(const TTMessage& msg);
    void clienteventConLost();
    void clienteventMyselfKicked(const TTMessage& msg);
    void clienteventCmdProcessing(int cmdid, bool complete);
    void clienteventCmdChannelUpdate(const Channel& channel);
    void clienteventCmdUserLoggedIn(const User& user);
    void clienteventCmdUserLoggedOut(const User& user);
    void clienteventCmdUserJoined(const User& user);
    void clienteventCmdUserLeft(int prevchannelid, const User& user);
    void clienteventCmdUserUpdate(const User& user);
    void clienteventCmdFileNew(const RemoteFile& file);
    void clienteventCmdFileRemove(const RemoteFile& file);
    void clienteventFileTransfer(const FileTransfer& filetransfer);
    void clienteventInternalError(const ClientErrorMsg& clienterrormsg);
    void clienteventUserStateChange(const User& user);
    void clienteventVoiceActivation(bool active);
    void clienteventStreamMediaFile(const MediaFileInfo& mediafileinfo);
    void clienteventUserVideoCapture(int source, int streamid);
    void clienteventUserMediaFileVideo(int source, int streamid);
    void clienteventUserDesktopWindow(int source, int streamid);
    void clienteventDesktopWindowTransfer(int source, int bytesremain);
    void clienteventUserRecordMediaFile(int source, const MediaFileInfo& mediafileinfo);
    void clienteventUserAudioBlock(int source, StreamTypes streamtypes);
    void clienteventSoundDeviceAdded(const SoundDevice& snddev);
    void clienteventSoundDeviceRemoved(const SoundDevice& snddev);
    MediaFilePlayback m_mfp = {};
    VideoCodec m_mfp_videocodec = {};

signals:
    /* Begin - CLIENTEVENT_* based events */
    void serverUpdate(const ServerProperties& srvprop);
    void serverStatistics(const ServerStatistics& stats);
    void newChannel(const Channel& chan);
    void updateChannel(const Channel& chan);
    void removeChannel(const Channel& chan);
    void userLogin(const User& user);
    void userLogout(const User& user);
    void userUpdate(const User& user);
    void userJoined(int channelid, const User& user);
    void userLeft(int channelid, const User& user);
    void userStateChange(const User& user);
    void updateMyself();
    void newVideoCaptureFrame(int userid, int stream_id);
    void newMediaVideoFrame(int userid, int stream_id);
    void newDesktopWindow(int userid, int sessionid);
    void userDesktopCursor(int src_userid, const DesktopInput&);
    void newTextMessage(const MyTextMessage& textmsg);
    void filetransferUpdate(const FileTransfer& transfer);
    void mediaStreamUpdate(const MediaFileInfo& mfi);
    void mediaPlaybackUpdate(int sessionID, const MediaFileInfo& mfi);
    void cmdProcessing(int cmdid, bool active);
    void cmdSuccess(int cmdid);
    void cmdError(int errorno, int cmdid);
    /* End - CLIENTEVENT_* based events */

    void preferencesModified();
};
#endif

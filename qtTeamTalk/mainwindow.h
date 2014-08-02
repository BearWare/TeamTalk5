/*
 * Copyright (c) 2005-2014, BearWare.dk
 * 
 * Contact Information:
 *
 * Bjoern D. Rasmussen
 * Skanderborgvej 40 4-2
 * DK-8000 Aarhus C
 * Denmark
 * Email: contact@bearware.dk
 * Phone: +45 20 20 54 59
 * Web: http://www.bearware.dk
 *
 * This source code is part of the TeamTalk 5 SDK owned by
 * BearWare.dk. All copyright statements may not be removed 
 * or altered from any source distribution. If you use this
 * software in a product, an acknowledgment in the product 
 * documentation is required.
 *
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning(disable:4100)
#endif

#include "ui_mainwindow.h"

#ifdef _MSC_VER 
#pragma warning( pop )
#endif

#include <QMap>
#include <QSet>
#include <QQueue>
#include <QSystemTrayIcon>
#include <QNetworkAccessManager>

#include "common.h"

#if defined(Q_OS_WIN32)
#define USE_POLL 0
#endif

//Commands which we're interested in knowning when have completed
enum CommandComplete
{
    CMD_COMPLETE_UNKNOWN,

    CMD_COMPLETE_LOGIN,
    CMD_COMPLETE_JOINCHANNEL,
    CMD_COMPLETE_LEAVECHANNEL,
    CMD_COMPLETE_LISTBANS,
    CMD_COMPLETE_LISTACCOUNTS,
    CMD_COMPLETE_SUBSCRIBE,
    CMD_COMPLETE_UNSUBSCRIBE
};

enum TimerEvent
{
    TIMER_PROCESS_TTEVENT = 1,
    TIMER_ONE_SECOND,
    TIMER_UI_UPDATE,
    TIMER_RECONNECT,
    TIMER_STATUSMSG,
    TIMER_SEND_DESKTOPWINDOW
};

enum
{
    TAB_CHAT,
    TAB_VIDEO,
    TAB_DESKTOP,
    TAB_FILES,

    TAB_COUNT
};

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    MainWindow();
    ~MainWindow();

    void loadSettings();

    bool parseArgs(const QStringList& args);

    void processTTMessage(const TTMessage& msg);

    void addStatusMsg(const QString& msg);

    void Connect();
    void Disconnect();

    void showTTErrorMessage(const ClientErrorMsg& msg,
                            CommandComplete cmd_type);

#ifdef Q_OS_LINUX
    //X11 hotkeys
    void keysActive(quint32 keycode, quint32 mods, bool active);
#endif

    void hotkeyToggle(HotKeyID id, bool active);

protected:
    void timerEvent(QTimerEvent *event);
    void closeEvent(QCloseEvent *event);
    void changeEvent(QEvent* event);

#if defined(Q_OS_WIN32) && QT_VERSION >= 0x050000
    bool nativeEvent(const QByteArray& eventType, void* message,
                     long* result);
#endif
private:
    Ui::MainWindow ui;
    class FilesModel* m_filesmodel;
    QSystemTrayIcon* m_sysicon;
    QMenu* m_sysmenu;
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

    //audio storage mode
    quint32 m_audiostorage_mode;

    //idle time caused away status mode
    bool m_idled_out;
    int m_statusmode;

    //ping and tx/rx information
    ClientStatistics m_clientstats;
    //last channel that were joined by the client
    Channel m_last_channel;
    //server properties
    ServerProperties m_srvprop;
    //current channel
    Channel m_mychannel;

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
    //handle xml request for updated versions
    QNetworkAccessManager* m_http_manager;
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
    typedef QMap<int, textmessages_t> usermessages_t;
    usermessages_t m_usermessages;
    //position of mouse cursor
    QPoint m_lastCursorPos;
    //list of desktop access users
    QVector<DesktopAccessEntry> m_desktopaccess_entries;

    //current command reply processing
    void commandProcessing(int cmdid, bool complete);
    //command replies
    void cmdLoggedIn(int myuserid);
    void cmdJoinedChannel(int channelid);

    void updateWindowTitle();
#if defined(Q_OS_WIN32)
    void firewallInstall();
#endif
    void subscribeCommon(bool checked, Subscriptions subs, int userid = 0);
    void processTextMessage(const TextMessage& textmsg);
    void processMyselfJoined(int channelid);
    void processMyselfLeft(int channelid);
    TextMessageDlg* getTextMessageDlg(int userid);
    int getVideoStreamID(int userid) const;
    void killLocalTimer(TimerEvent e);
    bool timerExists(TimerEvent e);
    void updateChannelFiles(int channelid);
    void updateUserGainLevel(int userid = -1 /* means all */);
    void updateUserSubscription(int userid);
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
    void loadHotKeys();
    void enableHotKey(HotKeyID id, const hotkey_t& hk);
    void disableHotKey(HotKeyID id);
#if defined(Q_OS_LINUX)
    void executeDesktopInput(const DesktopInput& input);
#endif

#if defined(Q_OS_WIN32)
    QString m_pttKeys;
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

private slots:
    /* menu items begin */
    void slotClientNewInstance(bool checked=false);
    void slotClientConnect(bool checked=false);
    void slotClientPreferences(bool checked=false);
    void slotClientExit(bool checked=false);

    void slotMeChangeNickname(bool checked=false);
    void slotMeChangeStatus(bool checked=false);
    void slotMeEnablePushToTalk(bool checked=false);
    void slotMeEnableVoiceActivation(bool checked=false);
    void slotMeEnableVideoTransmission(bool checked=false);
    void slotMeEnableDesktopSharing(bool checked=false);

    void slotUsersViewUserInformation(bool checked=false);
    void slotUsersMessages(bool checked=false);
    void slotUsersMuteVoice(bool checked=false);
    void slotUsersMuteMediaFile(bool checked=false);
    void slotUsersVolume(bool checked=false);
    void slotUsersOp(bool checked=false);
    void slotUsersKickFromChannel(bool checked=false);
    void slotUsersKickFromServer(bool checked=false);
    void slotUsersKickBan(bool checked=false);
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
    void slotUsersAdvancedStoreForMove();
    void slotUsersAdvancedMoveUsers();
    void slotUsersAdvancedVoiceAllowed(bool checked=false);
    void slotUsersAdvancedVideoAllowed(bool checked=false);
    void slotUsersAdvancedDesktopAllowed(bool checked=false);
    void slotUsersAdvancedMediaFileAllowed(bool checked=false);
    void slotUsersMuteVoiceAll(bool checked=false);
    void slotUsersStoreAudioToDisk(bool checked=false);

    void slotChannelsCreateChannel(bool checked=false);
    void slotChannelsUpdateChannel(bool checked=false);
    void slotChannelsDeleteChannel(bool checked=false);
    void slotChannelsJoinChannel(bool checked=false);
    void slotChannelsViewChannelInfo(bool checked=false);
    void slotChannelsStreamMediaFile(bool checked=false);
    void slotChannelsUploadFile(bool checked=false);
    void slotChannelsDownloadFile(bool checked=false);
    void slotChannelsDeleteFile(bool checked=false);
    
    void slotServerUserAccounts(bool checked=false);
    void slotServerBannedUsers(bool checked=false);
    void slotServerOnlineUsers(bool checked=false);
    void slotServerBroadcastMessage(bool checked=false);
    void slotServerServerProperties(bool checked=false);
    void slotServerSaveConfiguration(bool checked=false);
    void slotServerServerStatistics(bool checked=false);

    void slotHelpManual(bool checked=false);
    void slotHelpVisitBearWare(bool checked=false);
    void slotHelpAbout(bool checked=false);
    /* menu items end */

    void slotConnectToLatest();
    void slotUsersViewUserInformation(int userid);
    void slotUsersMessages(int userid);
    void slotUsersMuteVoice(int userid, bool mute);
    void slotUsersMuteMediaFile(int userid, bool mute);
    void slotUsersVolume(int userid);
    void slotUsersOp(int userid, int chanid);
    void slotUsersKick(int userid, int chanid);
    void slotUsersKickBan(int userid, int chanid);

    void slotTreeSelectionChanged();
    void slotTreeContextMenu(const QPoint& pos);
    void slotUpdateUI();
    void slotUpdateVideoTabUI();
    void slotUpdateDesktopTabUI();
    void slotUploadFiles(const QStringList& files);
    void slotSendChannelMessage();
    void slotUserDoubleClicked(int userid);
    void slotChannelDoubleClicked(int channelid);
    void slotNewTextMessage(const TextMessage& textmsg);
    void slotNewMyselfTextMessage(const TextMessage& textmsg);
    void slotTextMessageClosed(int userid);
    void slotClassroomChanged(int channelid, const QMap<int,StreamTypes>&);
    void slotChannelUpdate(const Channel& chan);
    void slotInitVideo();
    void slotAddUserVideo();
    void slotRemoveUserVideo();
    void slotRemoveUserVideo(int userid);
    void slotDetachUserVideo(int userid = 0);
    void slotNewUserVideoDlg(int userid, const QSize& size);
    void slotUserVideoDlgClosing(int userid);
    void slotAddUserDesktop();
    void slotRemoveUserDesktop();
    void slotRemoveUserDesktop(int userid);
    void slotAccessUserDesktop(bool enable);
    void slotDetachUserDesktop();
    void slotDetachUserDesktop(int userid, const QSize& size);
    void slotUserDesktopDlgClosing(int userid);
    void slotUserJoin(int channelid, const User& user);
    void slotUserLeft(int channelid, const User& user);
    void slotUserUpdate(const User& user);
    void slotEnableQuestionMode(bool checked);

    void slotUpdateVideoCount(int count);
    void slotUpdateDesktopCount(int count);


    void slotMasterVolumeChanged(int value);
    void slotMicrophoneGainChanged(int value);
    void slotVoiceActivationLevelChanged(int value);
    void slotEnableAudioRecording(bool checked);

    void slotTrayIconChange(QSystemTrayIcon::ActivationReason reason);

    void slotLoadTTFile(const QString& filepath);

    void slotHttpUpdateReply(QNetworkReply* reply);
    void slotClosedOnlineUsersDlg(int);
    void slotClosedServerStatsDlg(int);
    void slotClosedUserAccountsDlg(int);
    void slotClosedBannedUsersDlg(int);

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
    void newTextMessage(const TextMessage& textmsg);
    void filetransferUpdate(const FileTransfer& transfer);
    void cmdSuccess(int cmdid);
    void cmdError(int error, int cmdid);
    /* End - CLIENTEVENT_* based events */

    void preferencesModified();
};
#endif

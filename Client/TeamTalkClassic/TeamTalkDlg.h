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

#pragma once

#include "gui/SysColStatic.h"
#include "gui/dialogex/DialogEx.h"
#include "gui/SessionTreeCtrlWin32.h"
#include "gui/ChatRichEditCtrl.h"
#include "gui/MessageDlg.h"
#include "gui/SystemTray.h"
#include "gui/SplitterBar.h"
#include "gui/MyEdit.h"
#include "gui/MyTabCtrl.h"
#include "gui/ChatTab.h"
#include "gui/FilesTab.h"
#include "settings/ClientXML.h"
#include "HttpRequest.h"
#include "PlaySoundThread.h"

#if defined(ENABLE_TOLK)
#include <Tolk.h>
#endif

#include <vector>
#include <map>
#include <queue>
#include <set>
#include <memory>

#include "afxcmn.h"
#include "afxwin.h"
#include <afxpriv.h>

#define WM_TRAY_MSG                 (WM_USER+900)
#define WM_TEAMTALKDLG_TTFILE       (WM_USER+901)
#define WM_TEAMTALKDLG_TTLINK       (WM_USER+902)

#define WM_TEAMTALK_CLIENTEVENT     (WM_APP + 1)
#define WM_TEAMTALK_HOTKEYEVENT     (WM_APP + 2)

#define CONNECT_TIMEOUT 30000
#define RECONNECT_TIMEOUT 7000
#define VUMETER_UPDATE_TIMEOUT 50

enum
{
    HOTKEY_PUSHTOTALK_ID = 1,
    HOTKEY_VOICEACT_ID = 2,
    HOTKEY_VOLUME_PLUS = 3,
    HOTKEY_VOLUME_MINUS = 4,
    HOTKEY_MUTEALL = 5,
    HOTKEY_VOICEGAIN_PLUS = 6,
    HOTKEY_VOICEGAIN_MINUS = 7,
    HOTKEY_MIN_RESTORE = 8,
};

enum CommmandComplete
{
    CMD_COMPLETE_NONE,

    CMD_COMPLETE_LOGIN,
    CMD_COMPLETE_JOIN,
    CMD_COMPLETE_LISTACCOUNTS,
    CMD_COMPLETE_LIST_SERVERBANS,
    CMD_COMPLETE_LIST_CHANNELBANS,
    CMD_COMPLETE_SUBSCRIBE,
    CMD_COMPLETE_UNSUBSCRIBE,
    CMD_COMPLETE_SERVERSTATS
};

enum
{
    DESKTOPSHARE_NONE,
    DESKTOPSHARE_DESKTOP,
    DESKTOPSHARE_ACTIVE_WINDOW,
    DESKTOPSHARE_SPECIFIC_WINDOW,
};

class CFileTransferDlg;
class CUserVideoDlg;
class CUserDesktopDlg;

// CTeamTalkDlg dialog
class CTeamTalkDlg : public CDialogExx
{
    DECLARE_DYNAMIC(CTeamTalkDlg);

    // Construction
public:
    CTeamTalkDlg(CWnd* pParent = NULL);    // standard constructor
    virtual ~CTeamTalkDlg();

    BOOL Connect(LPCTSTR szAddress, UINT nTcpPort, UINT nUdpPort, BOOL bEncrypted);
    void Disconnect();

    void Login();

    void UpdateWindowTitle();

    void EnableVoiceActivation(BOOL bEnable, SoundEvent on = SOUNDEVENT_ENABLE_VOICEACTIVATION, SoundEvent off = SOUNDEVENT_DISABLE_VOICEACTIVATION);
    void EnableSpeech(BOOL bEnable);

    void CloseMessageSessions();
    CMessageDlg* GetUsersMessageSession(int nUserID, BOOL bCreateNew, BOOL* lpbNew = NULL);
    void OnUsersMessages(int nUserID);

    void OpenVideoSession(int nUserID);
    void CloseVideoSession(int nUserID);
    void CloseDesktopSession(int nUserID);
    void StartMediaStream();
    void StopMediaStream();
    void SwitchFont();
    void ResizeItems();

    void AddStatusText(LPCTSTR szText);
    void AddLogMessage(LPCTSTR szMsg);
    void AddVoiceMessage(LPCTSTR szMsg);

    void RunWizard();
    void Translate();
    void UpdateHotKeys();
    void ParseArgs();
    void DefaultUnsubscribe(int nUserID);
    void SubscribeToggle(int nUserID, Subscription sub);
    void SubscribeCommon(int nUserID, Subscription sub, BOOL bEnable);

    void FirewallInstall();

    void UpdateAudioStorage(BOOL bEnable);
    void UpdateMasterVolume(int nVol);
    void UpdateGainLevel(int nGain);
    void UpdateAudioConfig();
    void UpdateChannelLog();
    HWND GetSharedDesktopWindowHWND();
    BOOL SendDesktopWindow();
    void RestartSendDesktopWindowTimer();
    void ToggleTransmitUsers(int nUserID, StreamTypes uStreamTypes);
    void PlaySoundEvent(SoundEvent event);

    void RunAppUpdate();

    CString m_szTTLink;
    CStringList m_cmdArgs;

    // Dialog Data
    enum { IDD = IDD_DIALOG_TEAMTALK };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    // Implementation
protected:
    HICON m_hIcon;
    CBrush m_brush;
    BOOL m_bTwoPanes;
    BOOL m_bIgnoreResize;
    BOOL m_bMinimized;
    BOOL m_bSpeech;
    HACCEL m_hAccel; // accelerator table
    int m_nLastMoveChannel;
    HWND m_hShareWnd;

    CSplitterBar m_wndSplitter;
    //tray
    CSystemTray* m_pTray;

    INT64 m_nLastRecvBytes;
    INT64 m_nLastSentBytes;
    CFont m_Font;
    teamtalk::HostEntry m_host;
    CString m_szStatusBar;
    CRect m_rectLast;
    CPlaySoundThread* m_pPlaySndThread;

    UINT_PTR m_nConnectTimerID, m_nReconnectTimerID;
    //pictures
    CSysColStatic m_wndVolPic;
    CSysColStatic m_wndVoicePic;
    CSysColStatic m_wndMikePic;

    //all users
    typedef std::set<int> userids_t;
    userids_t m_users;
    //user message session
    typedef std::map<int, CMessageDlg*> mapuserdlg_t;
    mapuserdlg_t m_mUserDlgs;
    typedef std::map<int, CUserVideoDlg*> mapvideodlg_t;
    mapvideodlg_t m_videodlgs;
    std::set<int> m_videoignore;
    typedef std::map<int, CUserDesktopDlg*> mapdesktopdlg_t;
    mapdesktopdlg_t m_desktopdlgs;
    std::set<int> m_desktopignore;
    class COnlineUsersDlg* m_pOnlineUsersDlg = nullptr;
    //store used channel passwords
    typedef std::map<int, CString> chanpasswd_t;
    chanpasswd_t m_channelPasswords;

    std::queue<CString> m_qStatusMsgs;
    UINT_PTR m_nStatusTimerID;
    int m_nStatusMode;
    CString m_szAwayMessage;
    BOOL m_bResizeReady;
    BOOL m_bHotKey;
    CString m_szHotKey;
    BOOL m_bSendDesktopOnCompletion;

    //right side controls
    CMyTabCtrl m_wndTabCtrl;
    CChatTab m_tabChat;
    CFilesTab m_tabFiles;
    typedef std::map<int, CFileTransferDlg*> mtransferdlg_t;
    mtransferdlg_t m_mTransfers;

    //user stopped talking
    typedef std::set<int> talking_t;
    talking_t m_Talking;

    typedef std::map<int, CommmandComplete> cmdreply_t;
    cmdreply_t m_commands;
    int m_nCurrentCmdID;

    std::unique_ptr<class CStreamMediaDlg> m_pStreamMediaDlg;

    teamtalk::ClientXML m_xmlSettings;
    BOOL m_bResetSettings;

    std::vector<UserAccount> m_useraccounts;
    std::vector<BannedUser> m_bannedusers;

    //from tt instance
    afx_msg LRESULT OnClientEvent(WPARAM wParam, LPARAM lParam);

    // ClientNode message handlers
    void OnConnectFailed(const TTMessage& msg);
    void OnConnectSuccess(const TTMessage& msg);
    void OnConnectionLost(const TTMessage& msg);
    void OnLoggedIn(const TTMessage& msg);
    void OnLoggedOut(const TTMessage& msg);
    void OnKicked(const TTMessage& msg);
    void OnServerUpdate(const TTMessage& msg);
    void OnServerStatistics(const TTMessage& msg);
    void OnCommandError(const TTMessage& msg);
    void OnCommandProc(const TTMessage& msg);

    void OnUserLogin(const TTMessage& msg);
    void OnUserLogout(const TTMessage& msg);
    void OnUserAdd(const TTMessage& msg);
    void OnUserUpdate(const TTMessage& msg);
    void OnUserRemove(const TTMessage& msg);
    
    void OnChannelAdd(const TTMessage& msg);
    void OnChannelUpdate(const TTMessage& msg);
    void OnChannelRemove(const TTMessage& msg);

    void OnChannelJoined(const Channel& chan);
    void OnChannelLeft(const Channel& chan);
    
    void OnFileAdd(const TTMessage& msg);
    void OnFileRemove(const TTMessage& msg);

    void OnUserAccount(const TTMessage& msg);
    void OnBannedUser(const TTMessage& msg);
    
    void OnUserMessage(const TTMessage& msg);
    
    void OnUserStateChange(const TTMessage& msg);
    void OnUserVideoCaptureFrame(const TTMessage& msg);
    void OnUserMediaVideoFrame(const TTMessage& msg);
    void OnUserDesktopWindow(const TTMessage& msg);
    void OnUserDesktopInput(const TTMessage& msg);
    void OnUserAudioFile(const TTMessage& msg);
    void OnVoiceActivated(const TTMessage& msg);
    void OnFileTransfer(const TTMessage& msg);
    void OnStreamMediaFile(const TTMessage& msg);
    void OnInternalError(const TTMessage& msg);
    void OnDesktopWindowTransfer(const TTMessage& msg);

    void OnHotKey(const TTMessage& msg);

    //from Dialogs
    afx_msg LRESULT OnSendChannelMessage(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnMessageDlgClosed(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnFileTransferDlgClosed(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnFilesDropped(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnMoveUser(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnVideoDlgClosed(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnVideoDlgEnded(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnDesktopDlgClosed(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnDesktopDlgEnded(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnOnlineUsersDlgClosed(WPARAM wParam, LPARAM lParam);

    //tray icon msgs
    afx_msg LRESULT OnTrayMessage(WPARAM wParam, LPARAM lParam);
    //from splitter
    afx_msg LRESULT OnSplitterMoved(WPARAM wParam, LPARAM lParam);
    void OnWindowRestore();

    //used for inter-process comm.
    afx_msg BOOL OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pCopyDataStruct);
    LRESULT OnTeamTalkLink(WPARAM wParam, LPARAM lParam);
    LRESULT OnTeamTalkFile(WPARAM wParam, LPARAM lParam);

    BOOL OnToolTipText(UINT, NMHDR* pNMHDR, LRESULT* pResult);
    virtual BOOL OnInitDialog();
    afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
    afx_msg void Exit();
    afx_msg void OnClose();
    virtual void OnOK();
    virtual void OnCancel();
    DECLARE_MESSAGE_MAP()

public:
    CSessionTreeCtrlWin32 m_wndTree;
    CSliderCtrl m_wndVolSlider;
    CSliderCtrl m_wndVoiceSlider;
    CSliderCtrl m_wndGainSlider;
    CProgressCtrl m_wndVUProgress;
    CStatic m_wndVU;

    BOOL m_bIdledOut;
    BOOL m_bPreferencesOpen;
    std::unique_ptr<CHttpRequest> m_httpUpdate, m_httpWebLogin;
    std::set<int> m_moveusers;
    CFile m_logChan;

    afx_msg void OnUpdateStats(CCmdUI *pCmdUI);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
    afx_msg void OnUpdateChannelsViewchannelmessages(CCmdUI *pCmdUI);
    afx_msg void OnChannelsViewchannelmessages();
    afx_msg void OnUpdateFileConnect(CCmdUI *pCmdUI);
    afx_msg void OnFileConnect();
    afx_msg void OnFileHostmanager();
    afx_msg void OnFileExit();
    afx_msg void OnMeChangenick();
    afx_msg void OnUpdateMeChangenick(CCmdUI *pCmdUI);
    afx_msg void OnMeChangestatus();
    afx_msg void OnUpdateMeChangestatus(CCmdUI *pCmdUI);
    afx_msg void OnMeEnablehotkey();
    afx_msg void OnUpdateMeEnablehotkey(CCmdUI *pCmdUI);
    afx_msg void OnMeEnablevoiceactivation();
    afx_msg void OnUpdateMeEnablevoiceactivation(CCmdUI *pCmdUI);
    afx_msg void OnUsersViewinfo();
    afx_msg void OnUpdateUsersViewinfo(CCmdUI *pCmdUI);
    afx_msg void OnUsersMessages();
    afx_msg void OnUpdateUsersMessages(CCmdUI *pCmdUI);
    afx_msg void OnUsersMuteVoice();
    afx_msg void OnUpdateUsersMuteVoice(CCmdUI *pCmdUI);
    afx_msg void OnUsersVolume();
    afx_msg void OnUpdateUsersVolume(CCmdUI *pCmdUI);
    afx_msg void OnUsersKickFromChannel();
    afx_msg void OnUsersMuteVoiceall();
    afx_msg void OnUpdateUsersMuteVoiceall(CCmdUI *pCmdUI);
    afx_msg void OnUsersPositionusers();
    afx_msg void OnUpdateUsersPositionusers(CCmdUI *pCmdUI);
    virtual BOOL PreTranslateMessage(MSG* pMsg);
    afx_msg void OnChannelsCreatechannel();
    afx_msg void OnUpdateChannelsCreatechannel(CCmdUI *pCmdUI);
    afx_msg void OnChannelsJoinchannel();
    afx_msg void OnUpdateChannelsJoinchannel(CCmdUI *pCmdUI);
    afx_msg void OnChannelsViewchannelinfo();
    afx_msg void OnUpdateChannelsViewchannelinfo(CCmdUI *pCmdUI);
    afx_msg void OnNMDblclkTreeSession(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    afx_msg void OnNMCustomdrawSliderVolume(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnHelpAbout();
    afx_msg void OnHelpWebsite();
    afx_msg void OnHelpManual();
    afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
    afx_msg void OnEndSession(BOOL bEnding);
    afx_msg void OnNMCustomdrawSliderVoiceact(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnFilePreferences();
    afx_msg void OnDropFiles(HDROP hDropInfo);
    afx_msg void OnUpdateUsersOp(CCmdUI *pCmdUI);
    afx_msg void OnUsersOp();
    afx_msg void OnUpdateMeUsespeechonevents(CCmdUI *pCmdUI);
    afx_msg void OnMeUsespeechonevents();
    afx_msg void OnHelpRunwizard();
    afx_msg void OnUpdateChannelsUploadfile(CCmdUI *pCmdUI);
    afx_msg void OnChannelsUploadfile();
    afx_msg void OnUpdateChannelsDownloadfile(CCmdUI *pCmdUI);
    afx_msg void OnChannelsDownloadfile();
    afx_msg void OnUpdateChannelsDeletefile(CCmdUI *pCmdUI);
    afx_msg void OnChannelsDeletefile();
    afx_msg void OnUpdateChannelsLeavechannel(CCmdUI *pCmdUI);
    afx_msg void OnChannelsLeavechannel();
    afx_msg void OnUpdateChannelsUpdatechannel(CCmdUI *pCmdUI);
    afx_msg void OnChannelsUpdatechannel();
    afx_msg void OnUpdateChannelsDeletechannel(CCmdUI *pCmdUI);
    afx_msg void OnChannelsDeletechannel();
    afx_msg void OnUpdateServerServerproperties(CCmdUI *pCmdUI);
    afx_msg void OnServerServerproperties();
    afx_msg void OnUpdateServerListuseraccounts(CCmdUI *pCmdUI);
    afx_msg void OnServerListuseraccounts();
    afx_msg void OnUpdateServerSaveconfiguration(CCmdUI *pCmdUI);
    afx_msg void OnServerSaveconfiguration();
    afx_msg void OnUpdateAdvancedStoreformove(CCmdUI *pCmdUI);
    afx_msg void OnAdvancedStoreformove();
    afx_msg void OnUpdateAdvancedMoveuser(CCmdUI *pCmdUI);
    afx_msg void OnAdvancedMoveuser();
    afx_msg void OnUpdateServerListbannedusers(CCmdUI *pCmdUI);
    afx_msg void OnServerListbannedusers();
    afx_msg void OnUpdateUsersKickchannel(CCmdUI *pCmdUI);
    afx_msg void OnUpdateUsersKickandban(CCmdUI *pCmdUI);
    afx_msg void OnUsersKickandban();
    afx_msg void OnUpdateUsersStoreconversationstodisk(CCmdUI *pCmdUI);
    afx_msg void OnUsersStoreconversationstodisk();
    afx_msg void OnUpdateAdvancedMoveuserdialog(CCmdUI *pCmdUI);
    afx_msg void OnAdvancedMoveuserdialog();
    afx_msg void OnUpdateSubscriptionsUsermessages(CCmdUI *pCmdUI);
    afx_msg void OnSubscriptionsUsermessages();
    afx_msg void OnUpdateSubscriptionsChannelmessages(CCmdUI *pCmdUI);
    afx_msg void OnSubscriptionsChannelmessages();
    afx_msg void OnUpdateSubscriptionsBroadcastmessages(CCmdUI *pCmdUI);
    afx_msg void OnSubscriptionsBroadcastmessages();
    afx_msg void OnUpdateSubscriptionsAudio(CCmdUI *pCmdUI);
    afx_msg void OnSubscriptionsAudio();
    afx_msg void OnUpdateSubscriptionsInterceptusermessages(CCmdUI *pCmdUI);
    afx_msg void OnSubscriptionsInterceptusermessages();
    afx_msg void OnUpdateSubscriptionsInterceptchannelmessages(CCmdUI *pCmdUI);
    afx_msg void OnSubscriptionsInterceptchannelmessages();
    afx_msg void OnUpdateSubscriptionsInterceptaudio(CCmdUI *pCmdUI);
    afx_msg void OnSubscriptionsInterceptaudio();
    afx_msg void OnUpdateAdvancedAllowvoicetransmission(CCmdUI *pCmdUI);
    afx_msg void OnAdvancedAllowvoicetransmission();
    afx_msg void OnUpdateAdvancedAllowvideotransmission(CCmdUI *pCmdUI);
    afx_msg void OnAdvancedAllowvideotransmission();
    afx_msg void OnUpdateServerServerstatistics(CCmdUI *pCmdUI);
    afx_msg void OnServerServerstatistics();
    afx_msg void OnNMCustomdrawSliderGainlevel(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnUpdateSubscriptionsVideo(CCmdUI *pCmdUI);
    afx_msg void OnSubscriptionsVideo();
    afx_msg void OnUpdateServerOnlineusers(CCmdUI *pCmdUI);
    afx_msg void OnServerOnlineusers();
    afx_msg void OnUpdateServerBroadcastmessage(CCmdUI *pCmdUI);
    afx_msg void OnServerBroadcastmessage();
    afx_msg void OnUpdateMeEnablevideotransmission(CCmdUI *pCmdUI);
    afx_msg void OnMeEnablevideotransmission();
    afx_msg void OnUpdateSubscriptionsInterceptvideo(CCmdUI *pCmdUI);
    afx_msg void OnSubscriptionsInterceptvideo();
    afx_msg void OnUpdateSubscriptionsDesktop(CCmdUI *pCmdUI);
    afx_msg void OnSubscriptionsDesktop();
    afx_msg void OnUpdateSubscriptionsInterceptdesktop(CCmdUI *pCmdUI);
    afx_msg void OnSubscriptionsInterceptdesktop();
    afx_msg void OnUpdateMeEnabledesktopsharing(CCmdUI *pCmdUI);
    afx_msg void OnMeEnabledesktopsharing();
    afx_msg void OnUpdateChannelsStreamMediaFileToChannel(CCmdUI *pCmdUI);
    afx_msg void OnChannelsStreamMediaFileToChannel();
    afx_msg void OnUpdateSubscriptionsDesktopacces(CCmdUI *pCmdUI);
    afx_msg void OnSubscriptionsDesktopacces();
    afx_msg void OnUpdateUsersAllowdesktopaccess(CCmdUI *pCmdUI);
    afx_msg void OnUsersAllowdesktopaccess();
    afx_msg void OnUpdateUsersMuteMediafile(CCmdUI *pCmdUI);
    afx_msg void OnUsersMuteMediafile();
    afx_msg void OnUpdateUsersKickfromserver(CCmdUI *pCmdUI);
    afx_msg void OnUsersKickfromserver();
    afx_msg void OnUpdateAdvancedIncvolumemediafile(CCmdUI *pCmdUI);
    afx_msg void OnAdvancedIncvolumemediafile();
    afx_msg void OnUpdateAdvancedIncvolumevoice(CCmdUI *pCmdUI);
    afx_msg void OnAdvancedIncvolumevoice();
    afx_msg void OnUpdateAdvancedLowervolumemediafile(CCmdUI *pCmdUI);
    afx_msg void OnAdvancedLowervolumemediafile();
    afx_msg void OnUpdateAdvancedLowervolumevoice(CCmdUI *pCmdUI);
    afx_msg void OnAdvancedLowervolumevoice();
    afx_msg void OnUpdateAdvancedAllowdesktoptransmission(CCmdUI *pCmdUI);
    afx_msg void OnAdvancedAllowdesktoptransmission();
    afx_msg void OnUpdateAdvancedAllowmediafiletransmission(CCmdUI *pCmdUI);
    afx_msg void OnAdvancedAllowmediafiletransmission();
    afx_msg void OnUpdateSubscriptionsMediafilestream(CCmdUI *pCmdUI);
    afx_msg void OnSubscriptionsMediafilestream();
    afx_msg void OnUpdateSubscriptionsInterceptmediafilestream(CCmdUI *pCmdUI);
    afx_msg void OnSubscriptionsInterceptmediafilestream();
    afx_msg void OnUpdateUserinfoSpeakuserinfo(CCmdUI *pCmdUI);
    afx_msg void OnUserinfoSpeakuserinfo();
    afx_msg void OnUpdateChannelinfoSpeakchannelinfo(CCmdUI *pCmdUI);
    afx_msg void OnChannelinfoSpeakchannelinfo();
    afx_msg void OnHelpResetpreferencestodefault();
    afx_msg void OnUpdateAdvancedAllowallvoicetransmission(CCmdUI *pCmdUI);
    afx_msg void OnAdvancedAllowallvoicetransmission();
    afx_msg void OnUpdateAdvancedAllowallvideotransmission(CCmdUI *pCmdUI);
    afx_msg void OnAdvancedAllowallvideotransmission();
    afx_msg void OnUpdateAdvancedAllowallmediafiletransmission(CCmdUI *pCmdUI);
    afx_msg void OnAdvancedAllowallmediafiletransmission();
    afx_msg void OnUpdateAdvancedAllowalldesktoptransmission(CCmdUI *pCmdUI);
    afx_msg void OnAdvancedAllowalldesktoptransmission();
    afx_msg void OnUpdateChannelinfoSpeakchannelstate(CCmdUI *pCmdUI);
    afx_msg void OnChannelinfoSpeakchannelstate();
    afx_msg void OnUpdateKickKickandbanfromchannel(CCmdUI *pCmdUI);
    afx_msg void OnKickKickandbanfromchannel();
    afx_msg void OnUpdateChannelsBannedusersinchannel(CCmdUI *pCmdUI);
    afx_msg void OnChannelsBannedusersinchannel();
    afx_msg void OnClientNewclientinstance();
};


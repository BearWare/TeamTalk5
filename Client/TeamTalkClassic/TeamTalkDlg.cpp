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


#include "stdafx.h"
#include "TeamTalkApp.h"
#include "TeamTalkDlg.h"

#include "gui/AboutBox.h"
#include "gui/HostManagerDlg.h"
#include "gui/ChangeStatusDlg.h"
#include "gui/InputDlg.h"
#include "gui/PositionUsersDlg.h"
#include "gui/UserInfoDlg.h"
#include "gui/UserVolumeDlg.h"
#include "gui/ChannelDlg.h"
#include "gui/GeneralPage.h"
#include "gui/WindowPage.h"
#include "gui/ClientPage.h"
#include "gui/SoundSysPage.h"
#include "gui/SoundEventsPage.h"
#include "gui/VideoCapturePage.h"
#include "gui/AdvancedPage.h"
#include "gui/ShortcutsPage.h"
#include "gui/TextToSpeechPage.h"
#include "gui/MyPropertySheet.h"
#include "gui/KeyCompDlg.h"
#include "gui/FileTransferDlg.h"
#include "gui/UserVideoDlg.h"
#include "gui/ServerPropertiesDlg.h"
#include "gui/UserAccountsDlg.h"
#include "gui/BannedDlg.h"
#include "gui/MoveToChannelDlg.h"
#include "gui/ServerStatisticsDlg.h"
#include "gui/OnlineUsersDlg.h"
#include "gui/MediaStorageDlg.h"
#include "gui/UserDesktopDlg.h"
#include "gui/DesktopShareDlg.h"
#include "gui/StreamMediaDlg.h"
#include "gui/WebLoginDlg.h"
#include "gui/BanTypeDlg.h"
#include "gui/BearWareLoginDlg.h"

#include "wizard/WizMasterSheet.h"
#include "wizard/WizWelcomePage.h"
#include "wizard/WizGeneralPage.h"
#include "wizard/WizSoundSysPage.h"
#include "wizard/WizCompletionPage.h"

#include "Helper.h"
#include "AppInfo.h"

#include <string>
#include <iterator>
#include <regex>
#include <WinInet.h>

using namespace std;
using namespace teamtalk;

TTInstance* ttInst = NULL;

//Limit text lengths for nickname, etc.
extern int nTextLimit;
extern BOOL bShowUsernames;

enum : UINT_PTR
{
    TIMER_VOICELEVEL_ID = 1,
    TIMER_ONESECOND_ID,
    TIMER_CONNECT_TIMEOUT_ID,
    TIMER_STATUSMSG_ID,
    TIMER_RECONNECT_ID,
    TIMER_DESKTOPSHARE_ID,
    TIMER_APPUPDATE_ID,
    TIMER_HTTPREQUEST_APPUPDATE_UPDATE_ID,
    TIMER_HTTPREQUEST_APPUPDATE_TIMEOUT_ID,
    TIMER_HTTPREQUEST_WEBLOGIN_ID,
    TIMER_HTTPREQUEST_WEBLOGIN_TIMEOUT_ID,
};

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// CTeamTalkDlg dialog

IMPLEMENT_DYNAMIC(CTeamTalkDlg, CDialogExx);
CTeamTalkDlg::CTeamTalkDlg(CWnd* pParent /*=NULL*/)
: CDialogExx(CTeamTalkDlg::IDD, pParent)
, m_bTwoPanes(TRUE)
, m_bIgnoreResize(FALSE)
, m_bHotKey(FALSE)
, m_nLastRecvBytes(0)
, m_nLastSentBytes(0)
, m_pTray(NULL)
, m_pPlaySndThread(NULL)
, m_bMinimized(FALSE)
, m_nConnectTimerID(0)
, m_nReconnectTimerID(0)
, m_nStatusTimerID(0)
, m_bResizeReady(FALSE)
, m_bIdledOut(FALSE)
, m_bPreferencesOpen(FALSE)
, m_bSpeech(FALSE)
, m_xmlSettings(TT_XML_ROOTNAME)
, m_nLastMoveChannel(0)
, m_nStatusMode(STATUSMODE_AVAILABLE)
, m_bSendDesktopOnCompletion(FALSE)
, m_hShareWnd(NULL)
, m_nCurrentCmdID(0)
, m_bResetSettings(FALSE)
{
#ifndef _WIN32_WCE
    EnableActiveAccessibility();
#endif

    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    m_host.nTcpPort = DEFAULT_TEAMTALK_TCPPORT;
    m_host.nUdpPort = DEFAULT_TEAMTALK_UDPPORT;
}

CTeamTalkDlg::~CTeamTalkDlg()
{
}


void CTeamTalkDlg::EnableVoiceActivation(BOOL bEnable,
                                         SoundEvent on /*= SOUNDEVENT_ENABLE_VOICEACTIVATION*/,
                                         SoundEvent off /*= SOUNDEVENT_DISABLE_VOICEACTIVATION*/)
{
    if(bEnable)
        m_wndVoiceSlider.ShowWindow(SW_SHOW);
    else
        m_wndVoiceSlider.ShowWindow(SW_HIDE);

    m_wndVoiceSlider.EnableWindow(bEnable);
    TT_SetVoiceActivationLevel(ttInst, m_wndVoiceSlider.GetPos());
    TT_EnableVoiceActivation(ttInst, bEnable);
    PlaySoundEvent(bEnable? on :  off);
}

void CTeamTalkDlg::EnableSpeech(BOOL bEnable)
{
#if defined(ENABLE_TOLK)
    if(m_bSpeech)
    {
        Tolk_Unload();
    }

    if(bEnable)
    {
        Tolk_Load();
        Tolk_TrySAPI(true);
    }
#endif
    m_bSpeech = bEnable;
}

BOOL CTeamTalkDlg::Connect(LPCTSTR szAddress, UINT nTcpPort, UINT nUdpPort, BOOL bEncrypted)
{
    if( TT_GetFlags(ttInst) & CLIENT_CONNECTION)
    {
        ASSERT(FALSE);
        return FALSE;
    }
    ASSERT(szAddress);
    ASSERT(nTcpPort);
    ASSERT(nUdpPort);

    UINT nLocalTcpPort = m_xmlSettings.GetClientTcpPort(0);
    UINT nLocalUdpPort = m_xmlSettings.GetClientUdpPort(0);

    if (!InitSoundSystem(m_xmlSettings))
    {
        AddStatusText(LoadText(IDS_SNDINITFAILED));
    }

    //clear session tree
    m_wndTree.ClearChannels();

    TT_SetVoiceActivationLevel(ttInst, m_xmlSettings.GetVoiceActivationLevel());

    BOOL bSuccess = FALSE;
    bSuccess = TT_Connect(ttInst, szAddress, nTcpPort, nUdpPort, nLocalTcpPort, nLocalUdpPort, bEncrypted);

    if(!bSuccess)
    {
        //cleanup
        Disconnect();
        return FALSE;
    }
    else
    {
        SetTimer(TIMER_ONESECOND_ID, 1000, NULL);
        m_nConnectTimerID = SetTimer(TIMER_CONNECT_TIMEOUT_ID, CONNECT_TIMEOUT, NULL);
        if(m_xmlSettings.GetVuMeterUpdate())
            SetTimer(TIMER_VOICELEVEL_ID, VUMETER_UPDATE_TIMEOUT, NULL);

        CString szText;
        szText.Format(LoadText(IDS_CONNECTING), szAddress, nTcpPort, nUdpPort);
        AddStatusText(szText);

        return TRUE;
    }
}

void CTeamTalkDlg::Disconnect()
{
    KillTimer(TIMER_VOICELEVEL_ID);

    if(m_nConnectTimerID)
        KillTimer(TIMER_CONNECT_TIMEOUT_ID);
    m_nConnectTimerID = 0;

    KillTimer(TIMER_ONESECOND_ID);

    TT_Disconnect(ttInst);
    TT_CloseSoundDuplexDevices(ttInst);
    TT_CloseSoundInputDevice(ttInst);
    TT_CloseSoundOutputDevice(ttInst);

    TTMessage msg;
    INT32 nZero = 0;
    while(TT_GetMessage(ttInst, &msg, &nZero));

    m_nLastRecvBytes = m_nLastSentBytes = 0;

    //clear channels view?
    if(m_xmlSettings.GetQuitClearChannels())
        m_wndTree.ClearChannels();

    //close video sessions
    while(m_videodlgs.size())
        CloseVideoSession(m_videodlgs.begin()->first);
    m_videoignore.clear();

    //close desktop sessions
    while(m_desktopdlgs.size())
        CloseDesktopSession(m_desktopdlgs.begin()->first);
    m_desktopignore.clear();

    if (m_pOnlineUsersDlg)
        m_pOnlineUsersDlg->ResetUsers();

    //add to stopped talking (for event)
    m_Talking.clear();
    m_users.clear();
    m_useraccounts.clear();
    m_bannedusers.clear();
    m_moveusers.clear();

    UpdateWindowTitle();
}

void CTeamTalkDlg::Login()
{
    if(STR_UTF8(m_host.szUsername) == WEBLOGIN_FACEBOOK_USERNAME)
    {
        CWebLoginDlg dlg;
        if(dlg.DoModal() == IDOK)
        {
            m_host.szPassword = STR_UTF8(dlg.m_szPassword);
        }
    }

    int cmd = TT_DoLoginEx(ttInst,
        STR_UTF8(m_xmlSettings.GetNickname(STR_UTF8(DEFAULT_NICKNAME)).c_str()),
        STR_UTF8(m_host.szUsername.c_str()),
        STR_UTF8(m_host.szPassword.c_str()), APPTITLE_SHORT);

    m_commands[cmd] = CMD_COMPLETE_LOGIN;

    AddStatusText(LoadText(IDS_CONLOGIN, _T("Connected... logging in")));
}

void CTeamTalkDlg::UpdateWindowTitle()
{
    CString szProfileName = STR_UTF8(m_xmlSettings.GetProfileName());

    Channel chan = {};
    TT_GetChannel(ttInst, TT_GetMyChannelID(ttInst), &chan);

    //set window title
    CString szTitle;
    if(chan.nChannelID>0 && TT_GetRootChannelID(ttInst) != chan.nChannelID)
        szTitle.Format(_T("%s - %s"), LimitText(chan.szName), APPTITLE);
    else
        szTitle.Format(_T("%s"), APPTITLE);

    if(szProfileName.GetLength())
        szTitle += _T(" - ") + szProfileName;

    SetWindowText(szTitle);
}

LRESULT CTeamTalkDlg::OnMessageDlgClosed(WPARAM wParam, LPARAM lParam)
{
    mapuserdlg_t::iterator ite = m_mUserDlgs.find(int(wParam));
    if(ite != m_mUserDlgs.end())
    {
        m_wndTree.SetUserMessages(int(wParam), ite->second->m_messages);
        m_mUserDlgs.erase(ite);
    }
    m_wndTree.SetUserMessage(int(wParam), FALSE);

    return TRUE;
}

void CTeamTalkDlg::CloseMessageSessions()
{
    for(mapuserdlg_t::iterator ite = m_mUserDlgs.begin();ite != m_mUserDlgs.end();ite++)
        (*ite).second->SetAlive(FALSE);

    m_mUserDlgs.clear();
}

CMessageDlg* CTeamTalkDlg::GetUsersMessageSession(int nUserID, BOOL bCreateNew, BOOL* lpbNew)
{
    //wParam contains user id
    mapuserdlg_t::iterator ite = m_mUserDlgs.find(nUserID);

    if(ite != m_mUserDlgs.end())
    {
        if(lpbNew)
            *lpbNew = FALSE;
        return (*ite).second;
    }
    else if(bCreateNew)
    {
        User user = {}, myself = {};
        TT_GetUser(ttInst, nUserID, &user);
        TT_GetUser(ttInst, TT_GetMyUserID(ttInst), &myself);

        CString szLogFolder = STR_UTF8(m_xmlSettings.GetUserTextLogStorage());

        CMessageDlg* pMsgDlg = new CMessageDlg(this, myself, user, szLogFolder);
        pMsgDlg->m_messages = m_wndTree.GetUserMessages(nUserID);
        m_mUserDlgs[user.nUserID] = pMsgDlg;
        MyFont font;
        string szFaceName;
        int nSize;
        bool bBold, bUnderline, bItalic;
        if(m_xmlSettings.GetFont(szFaceName, nSize, bBold, bUnderline, bItalic))
        {
            font.nSize = nSize;
            font.bBold = bBold;
            font.bUnderline = bUnderline;
            font.bItalic = bItalic;
            font.szFaceName = STR_UTF8( szFaceName.c_str() );
            ConvertFont(font, pMsgDlg->m_lf);
        }
        pMsgDlg->m_bShowTimeStamp = m_xmlSettings.GetMessageTimeStamp();
        VERIFY(pMsgDlg->Create(CMessageDlg::IDD, GetDesktopWindow()));

        if(lpbNew)
            *lpbNew = TRUE;
        return pMsgDlg;
    }
    return NULL;
}

void CTeamTalkDlg::OpenVideoSession(int nUserID)
{
    ASSERT(nUserID & VIDEOTYPE_MASK);

    CUserVideoDlg* dlg = new CUserVideoDlg(nUserID, this);
    BOOL b = dlg->Create(CUserVideoDlg::IDD, GetDesktopWindow());
    ASSERT(b);
    dlg->ShowWindow(SW_SHOW);
    m_videodlgs[nUserID] = dlg;
    PlaySoundEvent(SOUNDEVENT_USER_VIDEOSESSION_NEW);
}

void CTeamTalkDlg::CloseVideoSession(int nUserID)
{
    ASSERT(nUserID & VIDEOTYPE_MASK);
    mapvideodlg_t::iterator ii = m_videodlgs.find(nUserID);
    if(ii != m_videodlgs.end())
    {
        ii->second->DestroyWindow();
        delete ii->second;
        m_videodlgs.erase(nUserID);
    }
}

void CTeamTalkDlg::CloseDesktopSession(int nUserID)
{
    ASSERT(nUserID>0);
    mapdesktopdlg_t::iterator ii = m_desktopdlgs.find(nUserID);
    if(ii != m_desktopdlgs.end())
    {
        ii->second->DestroyWindow();
        delete ii->second;
        m_desktopdlgs.erase(nUserID);
    }
}

void CTeamTalkDlg::StartMediaStream()
{
    if (m_xmlSettings.GetLastMediaFiles().empty())
        return;

    CString szFileName = STR_UTF8(m_xmlSettings.GetLastMediaFiles().front());
    VideoCodec vidcodec = m_xmlSettings.GetVideoCodec();
    MediaFilePlayback mfp = m_xmlSettings.GetMediaFilePlayback();

    if (!TT_StartStreamingMediaFileToChannelEx(ttInst, szFileName, &mfp, &vidcodec))
    {
        MessageBox(LoadText(IDS_STREAMFAILEDBOX, _T("Failed to stream media file.")),
            LoadText(IDD_DIALOG_STREAMMEDIA, _T("Stream Media File")), MB_OK);
        return;
    }

    m_nStatusMode |= STATUSMODE_STREAM_MEDIAFILE;
    TT_DoChangeStatus(ttInst, m_nStatusMode, m_szAwayMessage);
}

void CTeamTalkDlg::StopMediaStream()
{
    TT_StopStreamingMediaFileToChannel(ttInst);

    m_nStatusMode &= ~STATUSMODE_STREAM_MEDIAFILE;
    TT_DoChangeStatus(ttInst, m_nStatusMode, m_szAwayMessage);
}

void CTeamTalkDlg::AddStatusText(LPCTSTR szText)
{
    m_qStatusMsgs.push(szText);
	
	while(m_qStatusMsgs.size()>10)
	    m_qStatusMsgs.pop();
	
    if(m_qStatusMsgs.size() == 1)
        if(!m_nStatusTimerID)
            m_nStatusTimerID = SetTimer(TIMER_STATUSMSG_ID, 500, NULL);
    AddLogMessage(szText);
}

void CTeamTalkDlg::AddLogMessage(LPCTSTR szMsg)
{
    m_tabChat.m_wndRichEdit.AddLogMesage(szMsg);
}

void CTeamTalkDlg::AddVoiceMessage(LPCTSTR szMsg)
{
#if defined(ENABLE_TOLK)
    if(m_bSpeech)
        Tolk_Output(szMsg);
#endif
}

void CTeamTalkDlg::RunWizard()
{
    CString szTitle;
    szTitle.LoadString(IDS_WIZCAPTION);
    TRANSLATE_ITEM(IDS_WIZCAPTION, szTitle);
    CWizMasterSheet dlg(szTitle);
    CWizWelcomePage welcomepage;
    CWizGeneralPage generalpage;
    CWizSoundSysPage soundpage;
    CWizCompletionPage completepage;

    /// Welcome page
    welcomepage.m_bLanguage = !m_xmlSettings.GetLanguageFile().empty();
    welcomepage.m_szLanguage = STR_UTF8( m_xmlSettings.GetLanguageFile().c_str() );

    /// General page
    generalpage.m_sNickname = STR_UTF8( m_xmlSettings.GetNickname(STR_UTF8(DEFAULT_NICKNAME)).c_str() );
    generalpage.m_bPush = m_xmlSettings.GetPushToTalk();
    HotKey hotkey;

    m_xmlSettings.GetPushToTalkKey(hotkey);

    generalpage.m_Hotkey = hotkey;
    generalpage.m_bVoiceAct = m_xmlSettings.GetVoiceActivated();
    generalpage.m_nInactivity = m_xmlSettings.GetInactivityDelay();

    /// Sound system
    soundpage.m_nInputDevice = m_xmlSettings.GetSoundInputDevice(UNDEFINED);
    soundpage.m_nOutputDevice = m_xmlSettings.GetSoundOutputDevice(UNDEFINED);

    dlg.AddPage(&welcomepage);
    dlg.AddPage(&generalpage);
    dlg.AddPage(&soundpage);
    dlg.AddPage(&completepage);

    if(dlg.DoModal() == ID_WIZFINISH)
    {
        /// Welcome page
        if(welcomepage.m_bLanguage)
            m_xmlSettings.SetLanguageFile( STR_UTF8( welcomepage.m_szLanguage.GetBuffer() ) );

        Translate();
        TRANSLATE(*this, IDD);

        /// General page
        m_xmlSettings.SetNickname( STR_UTF8( generalpage.m_sNickname.GetBuffer() ));
        m_xmlSettings.SetPushToTalk(generalpage.m_bPush);
        HotKey hotkey = generalpage.m_Hotkey;
        if(generalpage.m_bPush && hotkey.size())
        {
            m_xmlSettings.SetPushToTalkKey(hotkey);
            TT_HotKey_Register(ttInst, HOTKEY_PUSHTOTALK_ID, &hotkey[0], INT32(hotkey.size()));
            m_szHotKey = CKeyCompDlg::GetHotkeyString(hotkey);
        }
        else
        {
            m_szHotKey.Empty();
        }
        m_xmlSettings.SetVoiceActivated(generalpage.m_bVoiceAct);
        m_xmlSettings.SetInactivityDelay(generalpage.m_nInactivity);

        /// Sound system
        m_xmlSettings.SetSoundInputDevice(soundpage.m_nInputDevice);
        m_xmlSettings.SetSoundOutputDevice(soundpage.m_nOutputDevice);

        if(completepage.m_bWebsite)
            OnHelpWebsite();
        if(completepage.m_bManual)
            OnHelpManual();
    }
}

void CTeamTalkDlg::UpdateHotKeys()
{
    TT_HotKey_Unregister(ttInst, HOTKEY_VOICEACT_ID);
    TT_HotKey_Unregister(ttInst, HOTKEY_VOLUME_PLUS);
    TT_HotKey_Unregister(ttInst, HOTKEY_VOLUME_MINUS);
    TT_HotKey_Unregister(ttInst, HOTKEY_MUTEALL);
    TT_HotKey_Unregister(ttInst, HOTKEY_VOICEGAIN_PLUS);
    TT_HotKey_Unregister(ttInst, HOTKEY_VOICEGAIN_MINUS);
    TT_HotKey_Unregister(ttInst, HOTKEY_MIN_RESTORE);

    HotKey hk;
    m_xmlSettings.GetHotKeyVoiceAct(hk);
    if(hk.size())
        TT_HotKey_Register(ttInst, HOTKEY_VOICEACT_ID, &hk[0], INT32(hk.size()));
    hk.clear();

    m_xmlSettings.GetHotKeyVolumePlus(hk);
    if(hk.size())
        TT_HotKey_Register(ttInst, HOTKEY_VOLUME_PLUS, &hk[0], INT32(hk.size()));
    hk.clear();

    m_xmlSettings.GetHotKeyVolumeMinus(hk);
    if(hk.size())
        TT_HotKey_Register(ttInst, HOTKEY_VOLUME_MINUS, &hk[0], INT32(hk.size()));
    hk.clear();

    m_xmlSettings.GetHotKeyMuteAll(hk);
    if(hk.size())
        TT_HotKey_Register(ttInst, HOTKEY_MUTEALL, &hk[0], INT32(hk.size()));
    hk.clear();

    m_xmlSettings.GetHotKeyVoiceGainPlus(hk);
    if(hk.size())
        TT_HotKey_Register(ttInst, HOTKEY_VOICEGAIN_PLUS, &hk[0], INT32(hk.size()));
    hk.clear();

    m_xmlSettings.GetHotKeyVoiceGainMinus(hk);
    if(hk.size())
        TT_HotKey_Register(ttInst, HOTKEY_VOICEGAIN_MINUS, &hk[0], INT32(hk.size()));

    m_xmlSettings.GetHotKeyMinRestore(hk);
    if(hk.size())
        TT_HotKey_Register(ttInst, HOTKEY_MIN_RESTORE, &hk[0], INT32(hk.size()));
}

BOOL CTeamTalkDlg::PreTranslateMessage(MSG* pMsg)
{
    if (WM_KEYFIRST <= pMsg->message && pMsg->message <= WM_KEYLAST)
        if (m_hAccel && ::TranslateAccelerator(m_hWnd, m_hAccel, pMsg))
        {
            return TRUE;
        }

        return CDialogExx::PreTranslateMessage(pMsg);
}

void CTeamTalkDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogExx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_TREE_SESSION, m_wndTree);
    DDX_Control(pDX, IDC_SLIDER_VOLUME, m_wndVolSlider);
    DDX_Control(pDX, IDC_SLIDER_VOICEACT, m_wndVoiceSlider);
    DDX_Control(pDX, IDC_SLIDER_GAINLEVEL, m_wndGainSlider);
    DDX_Control(pDX, IDC_PROGRESS_VOICEACT, m_wndVUProgress);
    DDX_Control(pDX, IDC_STATIC_VU, m_wndVU);
    DDX_Control(pDX, IDC_TAB_CTRL, m_wndTabCtrl);
}

BEGIN_MESSAGE_MAP(CTeamTalkDlg, CDialogExx)
    ON_WM_SYSCOMMAND()
    ON_WM_CLOSE()
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_WM_SIZE()
    ON_WM_CTLCOLOR()

    ON_MESSAGE(WM_TEAMTALK_CLIENTEVENT, OnClientEvent)
    ON_MESSAGE(WM_FILETRANSFERDLG_CLOSED, OnFileTransferDlgClosed)
    ON_MESSAGE(WM_FILESLISTCTRL_FILESDROPPED, OnFilesDropped)
    ON_MESSAGE(WM_SESSIONTREECTRL_MOVEUSER, OnMoveUser)

    ON_MESSAGE(WM_TRAY_MSG, OnTrayMessage)
    ON_MESSAGE(WM_MESSAGEDLG_CLOSED, OnMessageDlgClosed)
    ON_MESSAGE(WM_USERVIDEODLG_CLOSED, OnVideoDlgClosed)
    ON_MESSAGE(WM_USERVIDEODLG_ENDED, OnVideoDlgEnded)
    ON_MESSAGE(WM_USERDESKTOPDLG_CLOSED, OnDesktopDlgClosed)
    ON_MESSAGE(WM_USERDESKTOPDLG_ENDED, OnDesktopDlgEnded)
    ON_MESSAGE(WM_ONLINEUSERSDLG_CLOSED, OnOnlineUsersDlgClosed)

    ON_COMMAND(ID_POPUP_RESTORE, OnWindowRestore)
    ON_MESSAGE(WM_SPLITTER_MOVED, OnSplitterMoved)

    ON_MESSAGE(WM_TEAMTALKDLG_TTFILE, OnTeamTalkFile)
    ON_MESSAGE(WM_TEAMTALKDLG_TTLINK, OnTeamTalkLink)

    //tool tips
    ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTW, 0, 0xFFFF, OnToolTipText)
    ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, OnToolTipText)

    ON_UPDATE_COMMAND_UI(ID_INDICATOR_STATS, OnUpdateStats)
    ON_UPDATE_COMMAND_UI(ID_CHANNELS_VIEWCHANNELMESSAGES, OnUpdateChannelsViewchannelmessages)
    ON_COMMAND(ID_CHANNELS_VIEWCHANNELMESSAGES, OnChannelsViewchannelmessages)
    ON_UPDATE_COMMAND_UI(ID_FILE_CONNECT, OnUpdateFileConnect)
    ON_COMMAND(ID_FILE_CONNECT, OnFileConnect)
    ON_COMMAND(ID_FILE_EXIT, OnFileExit)
    ON_COMMAND(ID_ME_CHANGENICK, OnMeChangenick)
    ON_UPDATE_COMMAND_UI(ID_ME_CHANGENICK, OnUpdateMeChangenick)
    ON_COMMAND(ID_ME_CHANGESTATUS, OnMeChangestatus)
    ON_UPDATE_COMMAND_UI(ID_ME_CHANGESTATUS, OnUpdateMeChangestatus)
    ON_COMMAND(ID_ME_ENABLEHOTKEY, OnMeEnablehotkey)
    ON_UPDATE_COMMAND_UI(ID_ME_ENABLEHOTKEY, OnUpdateMeEnablehotkey)
    ON_COMMAND(ID_ME_ENABLEVOICEACTIVATION, OnMeEnablevoiceactivation)
    ON_UPDATE_COMMAND_UI(ID_ME_ENABLEVOICEACTIVATION, OnUpdateMeEnablevoiceactivation)
    ON_COMMAND(ID_USERS_VIEWINFO, OnUsersViewinfo)
    ON_UPDATE_COMMAND_UI(ID_USERS_VIEWINFO, OnUpdateUsersViewinfo)
    ON_COMMAND(ID_USERS_MESSAGES, OnUsersMessages)
    ON_UPDATE_COMMAND_UI(ID_USERS_MESSAGES, OnUpdateUsersMessages)
    ON_COMMAND(ID_USERS_MUTE_VOICE, OnUsersMuteVoice)
    ON_UPDATE_COMMAND_UI(ID_USERS_MUTE_VOICE, OnUpdateUsersMuteVoice)
    ON_COMMAND(ID_USERS_VOLUME, OnUsersVolume)
    ON_UPDATE_COMMAND_UI(ID_USERS_VOLUME, OnUpdateUsersVolume)
    ON_COMMAND(ID_USERS_KICKCHANNEL, OnUsersKickFromChannel)
    ON_UPDATE_COMMAND_UI(ID_USERS_KICKANDBAN, &CTeamTalkDlg::OnUpdateUsersKickandban)
    ON_COMMAND(ID_USERS_KICKANDBAN, &CTeamTalkDlg::OnUsersKickandban)
    ON_UPDATE_COMMAND_UI(ID_USERS_KICKCHANNEL, &CTeamTalkDlg::OnUpdateUsersKickchannel)
    ON_COMMAND(ID_USERS_MUTEALL, OnUsersMuteVoiceall)
    ON_UPDATE_COMMAND_UI(ID_USERS_MUTEALL, OnUpdateUsersMuteVoiceall)
    ON_COMMAND(ID_USERS_POSITIONUSERS, OnUsersPositionusers)
    ON_UPDATE_COMMAND_UI(ID_USERS_POSITIONUSERS, OnUpdateUsersPositionusers)
    ON_COMMAND(ID_CHANNELS_CREATECHANNEL, OnChannelsCreatechannel)
    ON_UPDATE_COMMAND_UI(ID_CHANNELS_CREATECHANNEL, OnUpdateChannelsCreatechannel)
    ON_COMMAND(ID_CHANNELS_JOINCHANNEL, OnChannelsJoinchannel)
    ON_UPDATE_COMMAND_UI(ID_CHANNELS_JOINCHANNEL, OnUpdateChannelsJoinchannel)
    ON_COMMAND(ID_CHANNELS_VIEWCHANNELINFO, OnChannelsViewchannelinfo)
    ON_UPDATE_COMMAND_UI(ID_CHANNELS_VIEWCHANNELINFO, OnUpdateChannelsViewchannelinfo)
    ON_NOTIFY(NM_DBLCLK, IDC_TREE_SESSION, OnNMDblclkTreeSession)
    ON_WM_TIMER()
    ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_VOLUME, OnNMCustomdrawSliderVolume)
    ON_COMMAND(ID_HELP_ABOUT, OnHelpAbout)
    ON_COMMAND(ID_HELP_WEBSITE, OnHelpWebsite)
    ON_COMMAND(ID_HELP_MANUAL, OnHelpManual)
    ON_WM_SHOWWINDOW()
    ON_WM_ENDSESSION()
    ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_VOICEACT, OnNMCustomdrawSliderVoiceact)
    ON_COMMAND(ID_FILE_PREFERENCES, OnFilePreferences)
    ON_WM_COPYDATA()
    ON_WM_DROPFILES()
    ON_UPDATE_COMMAND_UI(ID_USERS_OP, OnUpdateUsersOp)
    ON_COMMAND(ID_USERS_OP, OnUsersOp)
    ON_UPDATE_COMMAND_UI(ID_ME_USESPEECHONEVENTS, OnUpdateMeUsespeechonevents)
    ON_COMMAND(ID_ME_USESPEECHONEVENTS, OnMeUsespeechonevents)
    ON_COMMAND(ID_HELP_RUNWIZARD, OnHelpRunwizard)
    ON_UPDATE_COMMAND_UI(ID_CHANNELS_UPLOADFILE, OnUpdateChannelsUploadfile)
    ON_COMMAND(ID_CHANNELS_UPLOADFILE, OnChannelsUploadfile)
    ON_UPDATE_COMMAND_UI(ID_CHANNELS_DOWNLOADFILE, OnUpdateChannelsDownloadfile)
    ON_COMMAND(ID_CHANNELS_DOWNLOADFILE, OnChannelsDownloadfile)
    ON_UPDATE_COMMAND_UI(ID_CHANNELS_DELETEFILE, OnUpdateChannelsDeletefile)
    ON_COMMAND(ID_CHANNELS_DELETEFILE, OnChannelsDeletefile)
    ON_UPDATE_COMMAND_UI(ID_CHANNELS_LEAVECHANNEL, OnUpdateChannelsLeavechannel)
    ON_COMMAND(ID_CHANNELS_LEAVECHANNEL, OnChannelsLeavechannel)
    ON_UPDATE_COMMAND_UI(ID_CHANNELS_UPDATECHANNEL, &CTeamTalkDlg::OnUpdateChannelsUpdatechannel)
    ON_COMMAND(ID_CHANNELS_UPDATECHANNEL, &CTeamTalkDlg::OnChannelsUpdatechannel)
    ON_UPDATE_COMMAND_UI(ID_CHANNELS_DELETECHANNEL, &CTeamTalkDlg::OnUpdateChannelsDeletechannel)
    ON_COMMAND(ID_CHANNELS_DELETECHANNEL, &CTeamTalkDlg::OnChannelsDeletechannel)
    ON_UPDATE_COMMAND_UI(ID_SERVER_SERVERPROPERTIES, &CTeamTalkDlg::OnUpdateServerServerproperties)
    ON_COMMAND(ID_SERVER_SERVERPROPERTIES, &CTeamTalkDlg::OnServerServerproperties)
    ON_UPDATE_COMMAND_UI(ID_SERVER_LISTUSERACCOUNTS, &CTeamTalkDlg::OnUpdateServerListuseraccounts)
    ON_COMMAND(ID_SERVER_LISTUSERACCOUNTS, &CTeamTalkDlg::OnServerListuseraccounts)
    ON_UPDATE_COMMAND_UI(ID_SERVER_SAVECONFIGURATION, &CTeamTalkDlg::OnUpdateServerSaveconfiguration)
    ON_COMMAND(ID_SERVER_SAVECONFIGURATION, &CTeamTalkDlg::OnServerSaveconfiguration)
    ON_UPDATE_COMMAND_UI(ID_ADVANCED_STOREFORMOVE, &CTeamTalkDlg::OnUpdateAdvancedStoreformove)
    ON_COMMAND(ID_ADVANCED_STOREFORMOVE, &CTeamTalkDlg::OnAdvancedStoreformove)
    ON_UPDATE_COMMAND_UI(ID_ADVANCED_MOVEUSER, &CTeamTalkDlg::OnUpdateAdvancedMoveuser)
    ON_COMMAND(ID_ADVANCED_MOVEUSER, &CTeamTalkDlg::OnAdvancedMoveuser)
    ON_UPDATE_COMMAND_UI(ID_SERVER_LISTBANNEDUSERS, &CTeamTalkDlg::OnUpdateServerListbannedusers)
    ON_COMMAND(ID_SERVER_LISTBANNEDUSERS, &CTeamTalkDlg::OnServerListbannedusers)
    ON_UPDATE_COMMAND_UI(ID_USERS_STORECONVERSATIONSTODISK, &CTeamTalkDlg::OnUpdateUsersStoreconversationstodisk)
    ON_COMMAND(ID_USERS_STORECONVERSATIONSTODISK, &CTeamTalkDlg::OnUsersStoreconversationstodisk)
    ON_UPDATE_COMMAND_UI(ID_ADVANCED_MOVEUSERDIALOG, &CTeamTalkDlg::OnUpdateAdvancedMoveuserdialog)
    ON_COMMAND(ID_ADVANCED_MOVEUSERDIALOG, &CTeamTalkDlg::OnAdvancedMoveuserdialog)
    ON_UPDATE_COMMAND_UI(ID_SUBSCRIPTIONS_USERMESSAGES, &CTeamTalkDlg::OnUpdateSubscriptionsUsermessages)
    ON_COMMAND(ID_SUBSCRIPTIONS_USERMESSAGES, &CTeamTalkDlg::OnSubscriptionsUsermessages)
    ON_UPDATE_COMMAND_UI(ID_SUBSCRIPTIONS_CHANNELMESSAGES, &CTeamTalkDlg::OnUpdateSubscriptionsChannelmessages)
    ON_COMMAND(ID_SUBSCRIPTIONS_CHANNELMESSAGES, &CTeamTalkDlg::OnSubscriptionsChannelmessages)
    ON_UPDATE_COMMAND_UI(ID_SUBSCRIPTIONS_BROADCASTMESSAGES, &CTeamTalkDlg::OnUpdateSubscriptionsBroadcastmessages)
    ON_COMMAND(ID_SUBSCRIPTIONS_BROADCASTMESSAGES, &CTeamTalkDlg::OnSubscriptionsBroadcastmessages)
    ON_UPDATE_COMMAND_UI(ID_SUBSCRIPTIONS_VOICE, &CTeamTalkDlg::OnUpdateSubscriptionsAudio)
    ON_COMMAND(ID_SUBSCRIPTIONS_VOICE, &CTeamTalkDlg::OnSubscriptionsAudio)
    ON_UPDATE_COMMAND_UI(ID_SUBSCRIPTIONS_INTERCEPTUSERMESSAGES, &CTeamTalkDlg::OnUpdateSubscriptionsInterceptusermessages)
    ON_COMMAND(ID_SUBSCRIPTIONS_INTERCEPTUSERMESSAGES, &CTeamTalkDlg::OnSubscriptionsInterceptusermessages)
    ON_UPDATE_COMMAND_UI(ID_SUBSCRIPTIONS_INTERCEPTVOICE, &CTeamTalkDlg::OnUpdateSubscriptionsInterceptaudio)
    ON_COMMAND(ID_SUBSCRIPTIONS_INTERCEPTVOICE, &CTeamTalkDlg::OnSubscriptionsInterceptaudio)
    ON_UPDATE_COMMAND_UI(ID_SUBSCRIPTIONS_INTERCEPTCHANNELMESSAGES, &CTeamTalkDlg::OnUpdateSubscriptionsInterceptchannelmessages)
    ON_COMMAND(ID_SUBSCRIPTIONS_INTERCEPTCHANNELMESSAGES, &CTeamTalkDlg::OnSubscriptionsInterceptchannelmessages)
    ON_UPDATE_COMMAND_UI(ID_ADVANCED_ALLOWVIDEOTRANSMISSION, &CTeamTalkDlg::OnUpdateAdvancedAllowvideotransmission)
    ON_COMMAND(ID_ADVANCED_ALLOWVIDEOTRANSMISSION, &CTeamTalkDlg::OnAdvancedAllowvideotransmission)
    ON_UPDATE_COMMAND_UI(ID_ADVANCED_ALLOWVOICETRANSMISSION, &CTeamTalkDlg::OnUpdateAdvancedAllowvoicetransmission)
    ON_COMMAND(ID_ADVANCED_ALLOWVOICETRANSMISSION, &CTeamTalkDlg::OnAdvancedAllowvoicetransmission)
    ON_UPDATE_COMMAND_UI(ID_SERVER_SERVERSTATISTICS, &CTeamTalkDlg::OnUpdateServerServerstatistics)
    ON_COMMAND(ID_SERVER_SERVERSTATISTICS, &CTeamTalkDlg::OnServerServerstatistics)
    ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDER_GAINLEVEL, &CTeamTalkDlg::OnNMCustomdrawSliderGainlevel)
    ON_UPDATE_COMMAND_UI(ID_SUBSCRIPTIONS_VIDEO, &CTeamTalkDlg::OnUpdateSubscriptionsVideo)
    ON_COMMAND(ID_SUBSCRIPTIONS_VIDEO, &CTeamTalkDlg::OnSubscriptionsVideo)
    ON_UPDATE_COMMAND_UI(ID_SERVER_ONLINEUSERS, &CTeamTalkDlg::OnUpdateServerOnlineusers)
    ON_COMMAND(ID_SERVER_ONLINEUSERS, &CTeamTalkDlg::OnServerOnlineusers)
    ON_UPDATE_COMMAND_UI(ID_SERVER_BROADCASTMESSAGE, &CTeamTalkDlg::OnUpdateServerBroadcastmessage)
    ON_COMMAND(ID_SERVER_BROADCASTMESSAGE, &CTeamTalkDlg::OnServerBroadcastmessage)
    ON_UPDATE_COMMAND_UI(ID_ME_ENABLEVIDEOTRANSMISSION, &CTeamTalkDlg::OnUpdateMeEnablevideotransmission)
    ON_COMMAND(ID_ME_ENABLEVIDEOTRANSMISSION, &CTeamTalkDlg::OnMeEnablevideotransmission)
    ON_UPDATE_COMMAND_UI(ID_SUBSCRIPTIONS_INTERCEPTVIDEO, &CTeamTalkDlg::OnUpdateSubscriptionsInterceptvideo)
    ON_COMMAND(ID_SUBSCRIPTIONS_INTERCEPTVIDEO, &CTeamTalkDlg::OnSubscriptionsInterceptvideo)
    ON_UPDATE_COMMAND_UI(ID_SUBSCRIPTIONS_DESKTOP, &CTeamTalkDlg::OnUpdateSubscriptionsDesktop)
    ON_COMMAND(ID_SUBSCRIPTIONS_DESKTOP, &CTeamTalkDlg::OnSubscriptionsDesktop)
    ON_UPDATE_COMMAND_UI(ID_SUBSCRIPTIONS_INTERCEPTDESKTOP, &CTeamTalkDlg::OnUpdateSubscriptionsInterceptdesktop)
    ON_COMMAND(ID_SUBSCRIPTIONS_INTERCEPTDESKTOP, &CTeamTalkDlg::OnSubscriptionsInterceptdesktop)
    ON_UPDATE_COMMAND_UI(ID_ME_ENABLEDESKTOPSHARING, &CTeamTalkDlg::OnUpdateMeEnabledesktopsharing)
    ON_COMMAND(ID_ME_ENABLEDESKTOPSHARING, &CTeamTalkDlg::OnMeEnabledesktopsharing)
    ON_UPDATE_COMMAND_UI(ID_CHANNELS_STREAMMEDIAFILETOCHANNEL, &CTeamTalkDlg::OnUpdateChannelsStreamMediaFileToChannel)
    ON_COMMAND(ID_CHANNELS_STREAMMEDIAFILETOCHANNEL, &CTeamTalkDlg::OnChannelsStreamMediaFileToChannel)
    ON_UPDATE_COMMAND_UI(ID_SUBSCRIPTIONS_DESKTOPACCES, &CTeamTalkDlg::OnUpdateSubscriptionsDesktopacces)
    ON_COMMAND(ID_SUBSCRIPTIONS_DESKTOPACCES, &CTeamTalkDlg::OnSubscriptionsDesktopacces)
    ON_UPDATE_COMMAND_UI(ID_USERS_ALLOWDESKTOPACCESS, &CTeamTalkDlg::OnUpdateUsersAllowdesktopaccess)
    ON_COMMAND(ID_USERS_ALLOWDESKTOPACCESS, &CTeamTalkDlg::OnUsersAllowdesktopaccess)
    ON_UPDATE_COMMAND_UI(ID_USERS_MUTE_MEDIAFILE, &CTeamTalkDlg::OnUpdateUsersMuteMediafile)
    ON_COMMAND(ID_USERS_MUTE_MEDIAFILE, &CTeamTalkDlg::OnUsersMuteMediafile)
    ON_UPDATE_COMMAND_UI(ID_USERS_KICKFROMSERVER, &CTeamTalkDlg::OnUpdateUsersKickfromserver)
    ON_COMMAND(ID_USERS_KICKFROMSERVER, &CTeamTalkDlg::OnUsersKickfromserver)
    ON_UPDATE_COMMAND_UI(ID_ADVANCED_INCVOLUMEMEDIAFILE, &CTeamTalkDlg::OnUpdateAdvancedIncvolumemediafile)
    ON_COMMAND(ID_ADVANCED_INCVOLUMEMEDIAFILE, &CTeamTalkDlg::OnAdvancedIncvolumemediafile)
    ON_UPDATE_COMMAND_UI(ID_ADVANCED_INCVOLUMEVOICE, &CTeamTalkDlg::OnUpdateAdvancedIncvolumevoice)
    ON_COMMAND(ID_ADVANCED_INCVOLUMEVOICE, &CTeamTalkDlg::OnAdvancedIncvolumevoice)
    ON_UPDATE_COMMAND_UI(ID_ADVANCED_LOWERVOLUMEMEDIAFILE, &CTeamTalkDlg::OnUpdateAdvancedLowervolumemediafile)
    ON_COMMAND(ID_ADVANCED_LOWERVOLUMEMEDIAFILE, &CTeamTalkDlg::OnAdvancedLowervolumemediafile)
    ON_UPDATE_COMMAND_UI(ID_ADVANCED_LOWERVOLUMEVOICE, &CTeamTalkDlg::OnUpdateAdvancedLowervolumevoice)
    ON_COMMAND(ID_ADVANCED_LOWERVOLUMEVOICE, &CTeamTalkDlg::OnAdvancedLowervolumevoice)
    ON_UPDATE_COMMAND_UI(ID_ADVANCED_ALLOWDESKTOPTRANSMISSION, &CTeamTalkDlg::OnUpdateAdvancedAllowdesktoptransmission)
    ON_COMMAND(ID_ADVANCED_ALLOWDESKTOPTRANSMISSION, &CTeamTalkDlg::OnAdvancedAllowdesktoptransmission)
    ON_UPDATE_COMMAND_UI(ID_ADVANCED_ALLOWMEDIAFILETRANSMISSION, &CTeamTalkDlg::OnUpdateAdvancedAllowmediafiletransmission)
    ON_COMMAND(ID_ADVANCED_ALLOWMEDIAFILETRANSMISSION, &CTeamTalkDlg::OnAdvancedAllowmediafiletransmission)
    ON_UPDATE_COMMAND_UI(ID_SUBSCRIPTIONS_MEDIAFILESTREAM, &CTeamTalkDlg::OnUpdateSubscriptionsMediafilestream)
    ON_COMMAND(ID_SUBSCRIPTIONS_MEDIAFILESTREAM, &CTeamTalkDlg::OnSubscriptionsMediafilestream)
    ON_UPDATE_COMMAND_UI(ID_SUBSCRIPTIONS_INTERCEPTMEDIAFILESTREAM, &CTeamTalkDlg::OnUpdateSubscriptionsInterceptmediafilestream)
    ON_COMMAND(ID_SUBSCRIPTIONS_INTERCEPTMEDIAFILESTREAM, &CTeamTalkDlg::OnSubscriptionsInterceptmediafilestream)
    ON_UPDATE_COMMAND_UI(ID_USERINFO_SPEAKUSERINFO, &CTeamTalkDlg::OnUpdateUserinfoSpeakuserinfo)
    ON_COMMAND(ID_USERINFO_SPEAKUSERINFO, &CTeamTalkDlg::OnUserinfoSpeakuserinfo)
    ON_UPDATE_COMMAND_UI(ID_CHANNELINFO_SPEAKCHANNELINFO, &CTeamTalkDlg::OnUpdateChannelinfoSpeakchannelinfo)
    ON_COMMAND(ID_CHANNELINFO_SPEAKCHANNELINFO, &CTeamTalkDlg::OnChannelinfoSpeakchannelinfo)
    ON_COMMAND(ID_HELP_RESETPREFERENCESTODEFAULT, &CTeamTalkDlg::OnHelpResetpreferencestodefault)
    ON_UPDATE_COMMAND_UI(ID_ADVANCED_ALLOWALLVOICETRANSMISSION, &CTeamTalkDlg::OnUpdateAdvancedAllowallvoicetransmission)
    ON_COMMAND(ID_ADVANCED_ALLOWALLVOICETRANSMISSION, &CTeamTalkDlg::OnAdvancedAllowallvoicetransmission)
    ON_UPDATE_COMMAND_UI(ID_ADVANCED_ALLOWALLVIDEOTRANSMISSION, &CTeamTalkDlg::OnUpdateAdvancedAllowallvideotransmission)
    ON_COMMAND(ID_ADVANCED_ALLOWALLVIDEOTRANSMISSION, &CTeamTalkDlg::OnAdvancedAllowallvideotransmission)
    ON_UPDATE_COMMAND_UI(ID_ADVANCED_ALLOWALLMEDIAFILETRANSMISSION, &CTeamTalkDlg::OnUpdateAdvancedAllowallmediafiletransmission)
    ON_COMMAND(ID_ADVANCED_ALLOWALLMEDIAFILETRANSMISSION, &CTeamTalkDlg::OnAdvancedAllowallmediafiletransmission)
    ON_UPDATE_COMMAND_UI(ID_ADVANCED_ALLOWALLDESKTOPTRANSMISSION, &CTeamTalkDlg::OnUpdateAdvancedAllowalldesktoptransmission)
    ON_COMMAND(ID_ADVANCED_ALLOWALLDESKTOPTRANSMISSION, &CTeamTalkDlg::OnAdvancedAllowalldesktoptransmission)
    ON_UPDATE_COMMAND_UI(ID_CHANNELINFO_SPEAKCHANNELSTATE, &CTeamTalkDlg::OnUpdateChannelinfoSpeakchannelstate)
    ON_COMMAND(ID_CHANNELINFO_SPEAKCHANNELSTATE, &CTeamTalkDlg::OnChannelinfoSpeakchannelstate)
        ON_UPDATE_COMMAND_UI(ID_KICK_KICKANDBANFROMCHANNEL, &CTeamTalkDlg::OnUpdateKickKickandbanfromchannel)
        ON_COMMAND(ID_KICK_KICKANDBANFROMCHANNEL, &CTeamTalkDlg::OnKickKickandbanfromchannel)
        ON_UPDATE_COMMAND_UI(ID_CHANNELS_BANNEDUSERSINCHANNEL, &CTeamTalkDlg::OnUpdateChannelsBannedusersinchannel)
        ON_COMMAND(ID_CHANNELS_BANNEDUSERSINCHANNEL, &CTeamTalkDlg::OnChannelsBannedusersinchannel)
        ON_COMMAND(ID_CLIENT_NEWCLIENTINSTANCE, &CTeamTalkDlg::OnClientNewclientinstance)
        END_MESSAGE_MAP()


// CTeamTalkDlg message handlers
    
LRESULT CTeamTalkDlg::OnClientEvent(WPARAM wParam, LPARAM lParam)
{
    TTMessage msg;
    ZERO_STRUCT(msg);

    int nWait = 0;
    while(TT_GetMessage(ttInst, &msg, &nWait))
    {
        switch(msg.nClientEvent)
        {
        case CLIENTEVENT_CON_SUCCESS :
            OnConnectSuccess(msg);
            break;
        case CLIENTEVENT_CON_FAILED :
            OnConnectFailed(msg);
            break;
        case CLIENTEVENT_CON_LOST :
            OnConnectionLost(msg);
            break;
        case CLIENTEVENT_CMD_PROCESSING :
            OnCommandProc(msg);
            break;
        case CLIENTEVENT_CMD_ERROR :
            OnCommandError(msg);
            break;
        case CLIENTEVENT_CMD_MYSELF_LOGGEDIN :
            OnLoggedIn(msg);
            break;
        case CLIENTEVENT_CMD_MYSELF_LOGGEDOUT :
            OnLoggedOut(msg);
            break;
        case CLIENTEVENT_CMD_MYSELF_KICKED :
            OnKicked(msg);
            break;
        case CLIENTEVENT_CMD_USER_LOGGEDIN :
            OnUserLogin(msg);
            break;
        case CLIENTEVENT_CMD_USER_LOGGEDOUT :
            OnUserLogout(msg);
            break;
        case CLIENTEVENT_CMD_USER_UPDATE :
            OnUserUpdate(msg);
            break;
        case CLIENTEVENT_CMD_USER_JOINED :
            OnUserAdd(msg);
            break;
        case CLIENTEVENT_CMD_USER_LEFT :
            OnUserRemove(msg);
            break;
        case CLIENTEVENT_CMD_USER_TEXTMSG :
            OnUserMessage(msg);
            break;
        case CLIENTEVENT_CMD_CHANNEL_NEW :
            OnChannelAdd(msg);
            break;
        case CLIENTEVENT_CMD_CHANNEL_UPDATE :
            OnChannelUpdate(msg);
            break;
        case CLIENTEVENT_CMD_CHANNEL_REMOVE :
            OnChannelRemove(msg);
            break;
        case CLIENTEVENT_CMD_SERVER_UPDATE :
            OnServerUpdate(msg);
            break;
        case CLIENTEVENT_CMD_SERVERSTATISTICS :
            OnServerStatistics(msg);
            break;
        case CLIENTEVENT_CMD_FILE_NEW :
            OnFileAdd(msg);
            break;
        case CLIENTEVENT_CMD_FILE_REMOVE :
            OnFileRemove(msg);
            break;
        case CLIENTEVENT_CMD_USERACCOUNT :
            OnUserAccount(msg);
            break;
        case CLIENTEVENT_CMD_BANNEDUSER :
            OnBannedUser(msg);
            break;
        case CLIENTEVENT_USER_STATECHANGE :
            OnUserStateChange(msg);
            break;
        case CLIENTEVENT_USER_VIDEOCAPTURE :
            OnUserVideoCaptureFrame(msg);
            break;
        case CLIENTEVENT_USER_MEDIAFILE_VIDEO :
            OnUserMediaVideoFrame(msg);
            break;
        case CLIENTEVENT_USER_DESKTOPWINDOW :
            OnUserDesktopWindow(msg);
            break;
        case CLIENTEVENT_USER_RECORD_MEDIAFILE :
            OnUserAudioFile(msg);
            break;
        case CLIENTEVENT_USER_DESKTOPINPUT :
            OnUserDesktopInput(msg);
            break;
        case CLIENTEVENT_VOICE_ACTIVATION :
            OnVoiceActivated(msg);
            break;
        case CLIENTEVENT_HOTKEY :
            OnHotKey(msg);
            break;
        case CLIENTEVENT_FILETRANSFER :
            OnFileTransfer(msg);
            break;
        case CLIENTEVENT_DESKTOPWINDOW_TRANSFER :
            OnDesktopWindowTransfer(msg);
            break;
        case CLIENTEVENT_STREAM_MEDIAFILE :
            OnStreamMediaFile(msg);
            break;
        case CLIENTEVENT_INTERNAL_ERROR :
            OnInternalError(msg);
            break;
        case CLIENTEVENT_LOCAL_MEDIAFILE :
            if (m_pStreamMediaDlg)
                m_pStreamMediaDlg->ProcessTTMessage(msg);
            break;
        }
    }
    return TRUE;
}

void CTeamTalkDlg::OnConnectSuccess(const TTMessage& msg)
{
    //kill connect timeout
    if(m_nConnectTimerID)
        KillTimer(TIMER_CONNECT_TIMEOUT_ID);
    m_nConnectTimerID = 0;
    //kill reconnect timer
    if(m_nReconnectTimerID)
        KillTimer(m_nReconnectTimerID);
    m_nReconnectTimerID = 0;

    std::smatch sm;
    if (m_host.szUsername == WEBLOGIN_BEARWARE_USERNAME ||
        std::regex_search(m_host.szUsername, sm, std::regex(WEBLOGIN_BEARWARE_USERNAMEPOSTFIX "$")) && sm.size())
    {
        std::string username, token;
        m_xmlSettings.GetBearWareLogin(username, token);

        if(username.empty())
        {
            CBearWareLoginDlg dlg;
            if(dlg.DoModal() == IDOK)
            {
                username = STR_UTF8(dlg.m_szUsername);
                token = STR_UTF8(dlg.m_szToken);
                m_xmlSettings.SetBearWareLogin(STR_UTF8(dlg.m_szUsername), STR_UTF8(dlg.m_szToken));
            }
        }

        ServerProperties srvprop = {};
        TT_GetServerProperties(ttInst, &srvprop);

        CString szUsername = STR_UTF8(username), szToken = STR_UTF8(token);
        TCHAR szUrlUsername[INTERNET_MAX_URL_LENGTH] = _T("");
        TCHAR szUrlToken[INTERNET_MAX_URL_LENGTH] = _T("");
        TCHAR szUrlAccessToken[INTERNET_MAX_URL_LENGTH] = _T("");
        DWORD dwNewLen = INTERNET_MAX_URL_LENGTH;
        UrlEscape(szUsername, szUrlUsername, &dwNewLen, URL_ESCAPE_PERCENT | URL_ESCAPE_AS_UTF8);
        dwNewLen = INTERNET_MAX_URL_LENGTH;
        UrlEscape(szToken, szUrlToken, &dwNewLen, URL_ESCAPE_PERCENT | URL_ESCAPE_AS_UTF8);
        dwNewLen = INTERNET_MAX_URL_LENGTH;
        UrlEscape(srvprop.szAccessToken, szUrlAccessToken, &dwNewLen, URL_ESCAPE_PERCENT | URL_ESCAPE_AS_UTF8);

        m_httpWebLogin.reset(new CHttpRequest(WEBLOGIN_BEARWARE_URLTOKEN(szUrlUsername, szUrlToken, szUrlAccessToken)));
        SetTimer(TIMER_HTTPREQUEST_WEBLOGIN_ID, 500, NULL);
        SetTimer(TIMER_HTTPREQUEST_WEBLOGIN_TIMEOUT_ID, 10000, NULL);
    }
    else
    {
        Login();
    }
}

void CTeamTalkDlg::OnConnectFailed(const TTMessage& msg)
{
    Disconnect();

    CString s;
    s.Format(LoadText(IDS_CONNFAILED, _T("Failed to connect to %s TCP port %d UDP port %d")), STR_UTF8(m_host.szAddress.c_str()), m_host.nTcpPort, m_host.nUdpPort);
    AddStatusText(s);

    if(!m_nReconnectTimerID)
    {
        CString szError;
        szError.Format(LoadText(IDS_CONFAILED, _T("Failed to connect to host %s TCP port %d UDP port %d.\r\nCheck that the server is running on the specified address\r\nand that a firewall isn't preventing clients from connecting.")),
            STR_UTF8(m_host.szAddress.c_str()), m_host.nTcpPort, m_host.nUdpPort);
        AfxMessageBox(szError);
    }
}

void CTeamTalkDlg::OnConnectionLost(const TTMessage& msg)
{
    Disconnect();

    CString s;
    s.Format(LoadText(IDS_CONLOST, _T("Connection lost to %s TCP port %d UDP port %d")), STR_UTF8(m_host.szAddress.c_str()), m_host.nTcpPort, m_host.nUdpPort);
    AddStatusText(s);

    //reconnect to latest?
    if(m_xmlSettings.GetReconnectOnDropped())
    {
        ASSERT(m_nReconnectTimerID == 0);
        if(m_nReconnectTimerID)
            KillTimer(m_nReconnectTimerID);
        m_nReconnectTimerID = SetTimer(TIMER_RECONNECT_ID, RECONNECT_TIMEOUT, NULL);
    }

    PlaySoundEvent(SOUNDEVENT_CONNECTION_LOST);
}

void CTeamTalkDlg::OnLoggedIn(const TTMessage& msg)
{
    AddStatusText(LoadText(IDS_CONSUCCESS, _T("Successfully logged in")));

    switch(m_xmlSettings.GetGender(GENDER_NONE))
    {
    case GENDER_MALE :
        m_nStatusMode &= ~STATUSMODE_FEMALE;
        break;
    case GENDER_FEMALE :
        m_nStatusMode |= STATUSMODE_FEMALE;
        break;
    }
    TT_DoChangeStatus(ttInst, m_nStatusMode, m_szAwayMessage);
}

void CTeamTalkDlg::OnLoggedOut(const TTMessage& msg)
{
    AddStatusText(LoadText(IDS_LOGOUTSUCCESS, _T("Successfully logged out")));
}

void CTeamTalkDlg::OnKicked(const TTMessage& msg)
{
    PlaySoundEvent(SOUNDEVENT_CONNECTION_LOST);

    AfxMessageBox(LoadText(IDS_KICKEDFROMCHANNEL, _T("You have been kicked from the channel.")));
}

void CTeamTalkDlg::OnServerUpdate(const TTMessage& msg)
{
    ASSERT(msg.ttType == __SERVERPROPERTIES);

    m_wndTree.UpdServerName(msg.serverproperties);
    m_tabChat.m_wndRichEdit.SetServerInfo(msg.serverproperties.szServerName,
                                          msg.serverproperties.szMOTD);
}

void CTeamTalkDlg::OnServerStatistics(const TTMessage& msg)
{
    ASSERT(msg.ttType == __SERVERSTATISTICS);

    CServerStatisticsDlg dlg(this);
    const ServerStatistics& stats = msg.serverstatistics;
    dlg.m_szTotalRxTx.Format(_T("%I64d/%I64d"), stats.nTotalBytesRX/1024, stats.nTotalBytesTX/1024);
    dlg.m_szVoiceRxTx.Format(_T("%I64d/%I64d"), stats.nVoiceBytesRX/1024, stats.nVoiceBytesTX/1024);
    dlg.m_szVideoRxTx.Format(_T("%I64d/%I64d"), stats.nVideoCaptureBytesRX/1024, stats.nVideoCaptureBytesTX/1024);
    dlg.m_szMediaFileRXTX.Format(_T("%I64d/%I64d"), stats.nMediaFileBytesRX/1024, stats.nMediaFileBytesTX/1024);
    dlg.m_szDesktopRXTX.Format(_T("%I64d/%I64d"), stats.nDesktopBytesRX/1024, stats.nDesktopBytesTX/1024);
    dlg.m_szFilesRxTx.Format(_T("%I64d/%I64d"), stats.nFilesRx/1024, stats.nFilesTx/1024);
    dlg.m_szUsersServed.Format(_T("%I32d"), stats.nUsersServed);
    dlg.m_szUsersPeak.Format(_T("%I32d"), stats.nUsersPeak);
    dlg.DoModal();
}

void CTeamTalkDlg::OnCommandError(const TTMessage& msg)
{
    CString szError = LoadText(IDS_ERROROCCURREDCOMMAND, _T("An error occurred while perform a requested command:\r\n"));

    switch(msg.clienterrormsg.nErrorNo)
    {
    case CMDERR_SYNTAX_ERROR :
        szError += LoadText(IDS_CMDERR_SYNTAX_ERROR, msg.clienterrormsg.szErrorMsg);
        break;
    case CMDERR_UNKNOWN_COMMAND :
        szError += LoadText(IDS_CMDERR_UNKNOWN_COMMAND, msg.clienterrormsg.szErrorMsg);
        break;
    case CMDERR_MISSING_PARAMETER :
        szError += LoadText(IDS_CMDERR_MISSING_PARAMETER, msg.clienterrormsg.szErrorMsg);
        break;
    case CMDERR_INCOMPATIBLE_PROTOCOLS :
        szError += LoadText(IDS_CMDERR_INCOMPATIBLE_PROTOCOLS, msg.clienterrormsg.szErrorMsg);
        break;
    case CMDERR_UNKNOWN_AUDIOCODEC :
        szError += LoadText(IDS_CMDERR_UNKNOWN_AUDIOCODEC, msg.clienterrormsg.szErrorMsg);
        break;
    case CMDERR_INVALID_USERNAME :
        szError += LoadText(IDS_CMDERR_INVALID_USERNAME, msg.clienterrormsg.szErrorMsg);
        break;
    case CMDERR_INCORRECT_CHANNEL_PASSWORD :
        szError += LoadText(IDS_CMDERR_INCORRECT_CHANNEL_PASSWORD, msg.clienterrormsg.szErrorMsg);
        break;
    case CMDERR_INVALID_ACCOUNT :
        szError += LoadText(IDS_CMDERR_INVALID_ACCOUNT, msg.clienterrormsg.szErrorMsg);
        break;
    case CMDERR_MAX_SERVER_USERS_EXCEEDED :
        szError += LoadText(IDS_CMDERR_MAX_SERVER_USERS_EXCEEDED, msg.clienterrormsg.szErrorMsg);
        break;
    case CMDERR_MAX_CHANNEL_USERS_EXCEEDED :
        szError += LoadText(IDS_CMDERR_MAX_CHANNEL_USERS_EXCEEDED, msg.clienterrormsg.szErrorMsg);
        break;
    case CMDERR_SERVER_BANNED :
        szError += LoadText(IDS_CMDERR_SERVER_BANNED, msg.clienterrormsg.szErrorMsg);
        break;
    case CMDERR_NOT_AUTHORIZED :
        szError += LoadText(IDS_CMDERR_NOT_AUTHORIZED, msg.clienterrormsg.szErrorMsg);
        break;
    case CMDERR_MAX_DISKUSAGE_EXCEEDED :
        szError += LoadText(IDS_CMDERR_MAX_DISKUSAGE_EXCEEDED, msg.clienterrormsg.szErrorMsg);
        break;
    case CMDERR_INCORRECT_OP_PASSWORD :
        szError += LoadText(IDS_CMDERR_INCORRECT_OP_PASSWORD, msg.clienterrormsg.szErrorMsg);
        break;
    case CMDERR_AUDIOCODEC_BITRATE_LIMIT_EXCEEDED :
        szError += LoadText(IDS_CMDERR_AUDIOCODEC_BITRATE_LIMIT_EXCEEDED, msg.clienterrormsg.szErrorMsg);
        break;
    case CMDERR_MAX_LOGINS_PER_IPADDRESS_EXCEEDED :
        szError += LoadText(IDS_CMDERR_MAX_LOGINS_PER_IPADDRESS_EXCEEDED, msg.clienterrormsg.szErrorMsg);
        break;
    case CMDERR_MAX_CHANNELS_EXCEEDED :
        szError += LoadText(IDS_CMDERR_MAX_CHANNELS_EXCEEDED, msg.clienterrormsg.szErrorMsg);
        break;
    case CMDERR_COMMAND_FLOOD :
        szError += LoadText(IDS_CMDERR_COMMAND_FLOOD, msg.clienterrormsg.szErrorMsg);
        break;
    case CMDERR_CHANNEL_BANNED :
        szError += LoadText(IDS_CMDERR_CHANNEL_BANNED, msg.clienterrormsg.szErrorMsg);
        break;
    case CMDERR_NOT_LOGGEDIN :
        szError += LoadText(IDS_CMDERR_NOT_LOGGEDIN, msg.clienterrormsg.szErrorMsg);
        break;
    case CMDERR_ALREADY_LOGGEDIN :
        szError += LoadText(IDS_CMDERR_ALREADY_LOGGEDIN, msg.clienterrormsg.szErrorMsg);
        break;
    case CMDERR_NOT_IN_CHANNEL :
        szError += LoadText(IDS_CMDERR_NOT_IN_CHANNEL, msg.clienterrormsg.szErrorMsg);
        break;
    case CMDERR_ALREADY_IN_CHANNEL :
        szError += LoadText(IDS_CMDERR_ALREADY_IN_CHANNEL, msg.clienterrormsg.szErrorMsg);
        break;
    case CMDERR_CHANNEL_ALREADY_EXISTS :
        szError += LoadText(IDS_CMDERR_CHANNEL_ALREADY_EXISTS, msg.clienterrormsg.szErrorMsg);
        break;
    case CMDERR_CHANNEL_NOT_FOUND :
        szError += LoadText(IDS_CMDERR_CHANNEL_NOT_FOUND, msg.clienterrormsg.szErrorMsg);
        break;
    case CMDERR_BAN_NOT_FOUND :
        szError += LoadText(IDS_CMDERR_BAN_NOT_FOUND, msg.clienterrormsg.szErrorMsg);
        break;
    case CMDERR_FILETRANSFER_NOT_FOUND :
        szError += LoadText(IDS_CMDERR_FILETRANSFER_NOT_FOUND, msg.clienterrormsg.szErrorMsg);
        break;
    case CMDERR_OPENFILE_FAILED :
        szError += LoadText(IDS_CMDERR_OPENFILE_FAILED, msg.clienterrormsg.szErrorMsg);
        break;
    case CMDERR_ACCOUNT_NOT_FOUND :
        szError += LoadText(IDS_CMDERR_ACCOUNT_NOT_FOUND, msg.clienterrormsg.szErrorMsg);
        break;
    case CMDERR_FILE_NOT_FOUND :
        szError += LoadText(IDS_CMDERR_FILE_NOT_FOUND, msg.clienterrormsg.szErrorMsg);
        break;
    case CMDERR_FILE_ALREADY_EXISTS :
        szError += LoadText(IDS_CMDERR_FILE_ALREADY_EXISTS, msg.clienterrormsg.szErrorMsg);
        break;
    case CMDERR_FILESHARING_DISABLED :
        szError += LoadText(IDS_CMDERR_FILESHARING_DISABLED, msg.clienterrormsg.szErrorMsg);
        break;
    case CMDERR_CHANNEL_HAS_USERS :
        szError += LoadText(IDS_CMDERR_CHANNEL_HAS_USERS, msg.clienterrormsg.szErrorMsg);
        break;
    case CMDERR_LOGINSERVICE_UNAVAILABLE :
        szError += LoadText(IDS_CMDERR_LOGINSERVICE_UNAVAILABLE, msg.clienterrormsg.szErrorMsg);
        break;
    case CMDERR_USER_NOT_FOUND :
        //just ignore if reply to unsubscribe. It's used for closing streams
        if(m_commands[m_nCurrentCmdID] == CMD_COMPLETE_UNSUBSCRIBE)
        {
            szError.Empty();
            break;
        }
        szError += LoadText(IDS_CMDERR_USER_NOT_FOUND, msg.clienterrormsg.szErrorMsg);
        break;
    default :
        //unknown error occured
        szError = LoadText(IDS_UNKNOWNERROR, _T("An unknown error occurred. Check that the action you\r\nperformed is supported by the server. This error is most\r\nlikely caused by an incompability issue between the server\r\nand your client:\r\n"));
        szError += msg.clienterrormsg.szErrorMsg;
        break;
    }

    if (szError.GetLength())
    {
        AfxMessageBox(szError);
    }

    // remove command complete notification, since the command failed
    m_commands.erase(msg.nSource);
}

void CTeamTalkDlg::OnCommandProc(const TTMessage& msg)
{
    cmdreply_t::iterator ite = m_commands.find(msg.nSource);
    if(msg.bActive)
    {
        m_nCurrentCmdID = msg.nSource;
        return;
    }
    if(ite == m_commands.end())
    {
        m_nCurrentCmdID = 0;
        return;
    }

    switch(ite->second)
    {
    case CMD_COMPLETE_LOGIN :
    {
        if(m_xmlSettings.GetDefaultSubscriptions() != UNDEFINED)
        {
            vector<User> users;
            int count = 0;
            TT_GetServerUsers(ttInst, NULL, &count);
            if(count)
            {
                users.resize(count);
                TT_GetServerUsers(ttInst, &users[0], &count);
                for(size_t i=0;i<users.size();i++)
                    DefaultUnsubscribe(users[i].nUserID);
            }
        }

        UserAccount account;
        if(TT_GetMyUserAccount(ttInst, &account) && 
           !CString(account.szInitChannel).IsEmpty())
        {
            m_host.szChannel = STR_UTF8(account.szInitChannel);
            m_host.szChPasswd.clear();
        }

        if(m_host.szChannel.size())
        {
            int nChannelID = TT_GetChannelIDFromPath(ttInst,
                STR_UTF8(m_host.szChannel.c_str()));
            if(nChannelID>0) //join existing channel
            {
                int nCmdID = TT_DoJoinChannelByID(ttInst, nChannelID, 
                                                  STR_UTF8(m_host.szChPasswd.c_str()));
                m_commands[nCmdID] = CMD_COMPLETE_JOIN;
            }
            else //auto create channel
            {
                ServerProperties srvprop = {};
                TT_GetServerProperties(ttInst, &srvprop);

                Channel newchan = {};
                newchan.nParentID = TT_GetRootChannelID(ttInst);
                COPYTTSTR(newchan.szName, STR_UTF8(m_host.szChannel.c_str()));
                COPYTTSTR(newchan.szPassword, STR_UTF8(m_host.szChPasswd.c_str()));
                
                InitDefaultAudioCodec(newchan.audiocodec);

                newchan.audiocfg.bEnableAGC = DEFAULT_CHANNEL_AUDIOCONFIG;
                newchan.audiocfg.nGainLevel = DEFAULT_AGC_GAINLEVEL;

                newchan.nMaxUsers = srvprop.nMaxUsers;
                int nCmdID = TT_DoJoinChannel(ttInst, &newchan);
                m_commands[nCmdID] = CMD_COMPLETE_JOIN;
            }
        }
        else if(m_xmlSettings.GetAutoJoinRootChannel())
        {
            int nCmdID = TT_DoJoinChannelByID(ttInst, TT_GetRootChannelID(ttInst), _T(""));
            m_commands[nCmdID] = CMD_COMPLETE_JOIN;
        }
        break;
    }
    case CMD_COMPLETE_LISTACCOUNTS :
    {
        CUserAccountsDlg dlg(this, UAD_READWRITE);
        dlg.m_accounts = m_useraccounts;
        dlg.DoModal();
        m_useraccounts.clear();
    }
    break;
    case CMD_COMPLETE_LIST_SERVERBANS:
    case CMD_COMPLETE_LIST_CHANNELBANS :
    {
        CBannedDlg dlg;
        dlg.m_vecBanned = m_bannedusers;
        if(dlg.DoModal() == IDOK)
        {
            for(size_t i=0;i<dlg.m_vecUnBanned.size();i++)
                TT_DoUnBanUserEx(ttInst, &dlg.m_vecUnBanned[i]);
        }
        m_bannedusers.clear();
        break;
    }
    }

    m_commands.erase(ite);
    m_nCurrentCmdID = 0;
}

void CTeamTalkDlg::OnUserLogin(const TTMessage& msg)
{
    ASSERT(msg.ttType == __USER);
    const User& user = msg.user;
    m_users.insert(user.nUserID);
    m_wndTree.AddUser(user);

    if(m_pOnlineUsersDlg)
        m_pOnlineUsersDlg->AddUser(user);

    if(m_xmlSettings.GetAudioLogStorageMode() & AUDIOSTORAGE_SEPARATEFILES)
    {
        CString szAudioFolder = STR_UTF8(m_xmlSettings.GetAudioLogStorage());
        AudioFileFormat uAFF = (AudioFileFormat)m_xmlSettings.GetAudioLogStorageFormat();

        if(szAudioFolder.GetLength())
            TT_SetUserMediaStorageDir(ttInst, user.nUserID, szAudioFolder,
                                  NULL, uAFF);
    }

    DefaultUnsubscribe(user.nUserID);

    if (m_commands[m_nCurrentCmdID] == CMD_COMPLETE_NONE)
    {
        CString szMsg, szFormat;
        szFormat = LoadText(IDS_USERLOGIN);
        szMsg.Format(szFormat, GetDisplayName(user));
        if (m_xmlSettings.GetEventTTSEvents() & TTS_USER_LOGGEDIN)
            AddVoiceMessage(szMsg);
    }
}

void CTeamTalkDlg::OnUserLogout(const TTMessage& msg)
{
    ASSERT(msg.ttType == __USER);
    const User& user = msg.user;

    CMessageDlg* pDlg = GetUsersMessageSession(msg.user.nUserID, FALSE);
    if(pDlg)
    {
        pDlg->SetAlive(FALSE);
    }

    m_users.erase(user.nUserID);
    m_wndTree.RemoveUser(user);

    if (m_pOnlineUsersDlg)
        m_pOnlineUsersDlg->RemoveUser(msg.user.nUserID);

    CString szMsg, szFormat;
    szFormat = LoadText(IDS_USERLOGOUT);
    szMsg.Format(szFormat, GetDisplayName(user));
    if(m_xmlSettings.GetEventTTSEvents() & TTS_USER_LOGGEDOUT)
        AddVoiceMessage(szMsg);
}

/* SESSION_TREE UPDATES    */
void CTeamTalkDlg::OnUserAdd(const TTMessage& msg)
{
    ASSERT(msg.ttType == __USER);

    const User& user = msg.user;
    m_wndTree.AddUser(user);

    if(m_pOnlineUsersDlg)
        m_pOnlineUsersDlg->AddUser(user);

    Channel chan;
    m_wndTree.GetChannel(user.nChannelID, chan);

    if(user.nUserID != TT_GetMyUserID(ttInst))
    {
        CString szMsg, szFormat;

        if(TT_GetMyChannelID(ttInst) == user.nChannelID)
        {
            szFormat = LoadText(IDS_CHANNEL_JOINED);
            szMsg.Format(szFormat, GetDisplayName(user));
            AddStatusText(szMsg);
            if (m_xmlSettings.GetEventTTSEvents() & TTS_USER_JOINED_SAME)
                AddVoiceMessage(szMsg);

            //don't play sound when I join
            if(user.nUserID != TT_GetMyUserID(ttInst))
                PlaySoundEvent(SOUNDEVENT_USER_JOIN);
        }
        else if (m_commands[m_nCurrentCmdID] != CMD_COMPLETE_LOGIN)
        {
            CString szRoot;
            szRoot.LoadString(IDS_ROOTCHANNEL);
            TRANSLATE_ITEM(IDS_ROOTCHANNEL, szRoot);

            szFormat = LoadText(IDS_USERJOINEDCHANNEL);
            if(chan.nParentID == 0)
                szMsg.Format(szFormat, GetDisplayName(user), szRoot);
            else
                szMsg.Format(szFormat, GetDisplayName(user), chan.szName);

            if (m_xmlSettings.GetEventTTSEvents() & TTS_USER_JOINED)
                AddVoiceMessage(szMsg);
        }
    }
    else //myself joined channel
    {
        OnChannelJoined(chan);
    }

    if(m_xmlSettings.GetAudioLogStorageMode() & AUDIOSTORAGE_SEPARATEFILES)
    {
        CString szAudioFolder = STR_UTF8(m_xmlSettings.GetAudioLogStorage());
        AudioFileFormat uAFF = (AudioFileFormat)m_xmlSettings.GetAudioLogStorageFormat();
        if(szAudioFolder.GetLength())
            TT_SetUserMediaStorageDir(ttInst, user.nUserID, szAudioFolder, NULL, uAFF);
    }

    double d = m_xmlSettings.GetMediaStreamVsVoice(DEFAULT_MEDIA_VS_VOICE);
    d /= 100.;
    TT_SetUserVolume(ttInst, user.nUserID, STREAMTYPE_MEDIAFILE_AUDIO,
                     INT32(RefVolume(DEFAULT_SOUND_OUTPUT_VOLUME) * d));
}

void CTeamTalkDlg::OnUserUpdate(const TTMessage& msg)
{
    //new user info
    const User& user = msg.user;

    //get old user info
    User oldUser;
    m_wndTree.GetUser(user.nUserID, oldUser);
    m_wndTree.UpdateUser(user);

    if(user.nUserID != TT_GetMyUserID(ttInst) &&
       user.nChannelID == TT_GetMyChannelID(ttInst) &&
       (oldUser.nStatusMode & STATUSMODE_QUESTION) == 0 &&
       (user.nStatusMode & STATUSMODE_QUESTION))
    {
        PlaySoundEvent(SOUNDEVENT_USER_QUESTIONMODE);
    }

    if(m_pOnlineUsersDlg)
        m_pOnlineUsersDlg->AddUser(user);

    //if not in same channel, then ignore
    if(user.nChannelID != TT_GetMyChannelID(ttInst) || !user.nChannelID)
        return;

    CString szName = GetDisplayName(user);
    CString szText, szFormat;

    if((oldUser.uPeerSubscriptions & SUBSCRIBE_USER_MSG) !=
        (user.uPeerSubscriptions & SUBSCRIBE_USER_MSG))
    {
        szFormat = LoadText(IDS_SUB_TEXTMSG);
        szText.Format(szFormat, szName, int(user.uPeerSubscriptions & SUBSCRIBE_USER_MSG) != SUBSCRIBE_NONE);
        AddStatusText(szText);
        if (m_xmlSettings.GetEventTTSEvents() & TTS_SUBSCRIPTIONS_TEXTMSG_PRIVATE)
            AddVoiceMessage(szText);
    }
    if((oldUser.uPeerSubscriptions & SUBSCRIBE_CHANNEL_MSG) !=
        (user.uPeerSubscriptions & SUBSCRIBE_CHANNEL_MSG))
    {
        szFormat = LoadText(IDS_SUB_CHANTEXTMSG);
        szText.Format(szFormat, szName, int(user.uPeerSubscriptions & SUBSCRIBE_CHANNEL_MSG) != SUBSCRIBE_NONE);
        AddStatusText(szText);
        if (m_xmlSettings.GetEventTTSEvents() & TTS_SUBSCRIPTIONS_TEXTMSG_CHANNEL)
            AddVoiceMessage(szText);
    }
    if((oldUser.uPeerSubscriptions & SUBSCRIBE_BROADCAST_MSG) !=
        (user.uPeerSubscriptions & SUBSCRIBE_BROADCAST_MSG))
    {
        szFormat = LoadText(IDS_SUB_BCASTTEXTMSG);
        szText.Format(szFormat, szName, int(user.uPeerSubscriptions & SUBSCRIBE_BROADCAST_MSG) != SUBSCRIBE_NONE);
        AddStatusText(szText);
        if (m_xmlSettings.GetEventTTSEvents() & TTS_SUBSCRIPTIONS_TEXTMSG_BROADCAST)
            AddVoiceMessage(szText);
    }
    if((oldUser.uPeerSubscriptions & SUBSCRIBE_VOICE) !=
        (user.uPeerSubscriptions & SUBSCRIBE_VOICE))
    {
        szFormat = LoadText(IDS_SUB_VOICE);
        szText.Format(szFormat, szName, int(user.uPeerSubscriptions & SUBSCRIBE_VOICE) != SUBSCRIBE_NONE);
        AddStatusText(szText);
        if (m_xmlSettings.GetEventTTSEvents() & TTS_SUBSCRIPTIONS_VOICE)
            AddVoiceMessage(szText);
    }
    if((oldUser.uPeerSubscriptions & SUBSCRIBE_VIDEOCAPTURE) !=
        (user.uPeerSubscriptions & SUBSCRIBE_VIDEOCAPTURE))
    {
        szFormat = LoadText(IDS_SUB_VIDEO);
        szText.Format(szFormat, szName, int(user.uPeerSubscriptions & SUBSCRIBE_VIDEOCAPTURE) != SUBSCRIBE_NONE);
        AddStatusText(szText);
        if (m_xmlSettings.GetEventTTSEvents() & TTS_SUBSCRIPTIONS_VIDEO)
            AddVoiceMessage(szText);
    }
    if((oldUser.uPeerSubscriptions & SUBSCRIBE_DESKTOP) !=
        (user.uPeerSubscriptions & SUBSCRIBE_DESKTOP))
    {
        szFormat = LoadText(IDS_SUB_DESKTOP);
        szText.Format(szFormat, szName, int(user.uPeerSubscriptions & SUBSCRIBE_DESKTOP) != SUBSCRIBE_NONE);
        AddStatusText(szText);
        if (m_xmlSettings.GetEventTTSEvents() & TTS_SUBSCRIPTIONS_DESKTOP)
            AddVoiceMessage(szText);
    }
    if((oldUser.uPeerSubscriptions & SUBSCRIBE_DESKTOPINPUT) !=
        (user.uPeerSubscriptions & SUBSCRIBE_DESKTOPINPUT))
    {
        szFormat = LoadText(IDS_SUB_DESKTOPINPUT);
        szText.Format(szFormat, szName, int(user.uPeerSubscriptions & SUBSCRIBE_DESKTOPINPUT) != SUBSCRIBE_NONE);
        AddStatusText(szText);
        if(user.uPeerSubscriptions & SUBSCRIBE_DESKTOPINPUT)
        {
            szFormat = LoadText(IDS_DESKTOPINPUT_GRANTED);
            szText.Format(szFormat, szName);
            if (m_xmlSettings.GetEventTTSEvents() & TTS_SUBSCRIPTIONS_DESKTOPINPUT)
                AddVoiceMessage(szText);
        }
        else
        {
            szFormat = LoadText(IDS_DESKTOPINPUT_RETRACT);
            szText.Format(szFormat, szName);
            if (m_xmlSettings.GetEventTTSEvents() & TTS_SUBSCRIPTIONS_DESKTOPINPUT)
                AddVoiceMessage(szText);
        }
    }
    if((oldUser.uPeerSubscriptions & SUBSCRIBE_MEDIAFILE) !=
        (user.uPeerSubscriptions & SUBSCRIBE_MEDIAFILE))
    {
        szFormat = LoadText(IDS_SUB_MEDIAFILE);
        szText.Format(szFormat, szName, int(user.uPeerSubscriptions & SUBSCRIBE_MEDIAFILE) != SUBSCRIBE_NONE);
        AddStatusText(szText);
        if (m_xmlSettings.GetEventTTSEvents() & TTS_SUBSCRIPTIONS_MEDIAFILE)
            AddVoiceMessage(szText);
    }
    if((oldUser.uLocalSubscriptions & SUBSCRIBE_DESKTOPINPUT) !=
        (user.uLocalSubscriptions & SUBSCRIBE_DESKTOPINPUT))
    {
        if(user.uLocalSubscriptions & SUBSCRIBE_DESKTOPINPUT)
        {
            szFormat = LoadText(IDS_DESKTOPINPUT_ACTIVE);
            szText.Format(szFormat, szName);
            if (m_xmlSettings.GetEventTTSEvents() & TTS_SUBSCRIPTIONS_DESKTOPINPUT)
                AddVoiceMessage(szText);
        }
        else
        {
            szFormat = LoadText(IDS_DESKTOPINPUT_STOPPED);
            szText.Format(szFormat, szName);
            if (m_xmlSettings.GetEventTTSEvents() & TTS_SUBSCRIPTIONS_DESKTOPINPUT)
                AddVoiceMessage(szText);
        }
    }
    if((oldUser.uPeerSubscriptions & SUBSCRIBE_INTERCEPT_USER_MSG) !=
        (user.uPeerSubscriptions & SUBSCRIBE_INTERCEPT_USER_MSG))
    {
        szFormat = LoadText(IDS_SUBINT_TEXTMSG);
        szText.Format(szFormat, szName, int(user.uPeerSubscriptions & SUBSCRIBE_INTERCEPT_USER_MSG) != SUBSCRIBE_NONE);
        AddStatusText(szText);
        if (m_xmlSettings.GetEventTTSEvents() & TTS_SUBSCRIPTIONS_INTERCEPT_TEXTMSG_PRIVATE)
            AddVoiceMessage(szText);
    }
    if((oldUser.uPeerSubscriptions & SUBSCRIBE_INTERCEPT_CHANNEL_MSG) !=
        (user.uPeerSubscriptions & SUBSCRIBE_INTERCEPT_CHANNEL_MSG))
    {
        szFormat = LoadText(IDS_SUBINT_CHANTEXTMSG);
        szText.Format(szFormat, szName, int(user.uPeerSubscriptions & SUBSCRIBE_INTERCEPT_CHANNEL_MSG) != SUBSCRIBE_NONE);
        AddStatusText(szText);
        if (m_xmlSettings.GetEventTTSEvents() & TTS_SUBSCRIPTIONS_INTERCEPT_TEXTMSG_CHANNEL)
            AddVoiceMessage(szText);
    }
    if((oldUser.uPeerSubscriptions & SUBSCRIBE_INTERCEPT_VOICE) !=
        (user.uPeerSubscriptions & SUBSCRIBE_INTERCEPT_VOICE))
    {
        szFormat = LoadText(IDS_SUBINT_VOICE);
        szText.Format(szFormat, szName, int(user.uPeerSubscriptions & SUBSCRIBE_INTERCEPT_VOICE) != SUBSCRIBE_NONE);
        AddStatusText(szText);
        if (m_xmlSettings.GetEventTTSEvents() & TTS_SUBSCRIPTIONS_INTERCEPT_VOICE)
            AddVoiceMessage(szText);
    }
    if((oldUser.uPeerSubscriptions & SUBSCRIBE_INTERCEPT_VIDEOCAPTURE) !=
        (user.uPeerSubscriptions & SUBSCRIBE_INTERCEPT_VIDEOCAPTURE))
    {
        szFormat = LoadText(IDS_SUBINT_VIDEO);
        szText.Format(szFormat, szName, int(user.uPeerSubscriptions & SUBSCRIBE_INTERCEPT_VIDEOCAPTURE) != SUBSCRIBE_NONE);
        AddStatusText(szText);
        if (m_xmlSettings.GetEventTTSEvents() & TTS_SUBSCRIPTIONS_INTERCEPT_VIDEO)
            AddVoiceMessage(szText);
    }
    if((oldUser.uPeerSubscriptions & SUBSCRIBE_INTERCEPT_DESKTOP) !=
        (user.uPeerSubscriptions & SUBSCRIBE_INTERCEPT_DESKTOP))
    {
        szFormat = LoadText(IDS_SUBINT_DESKTOP);
        szText.Format(szFormat, szName, int(user.uPeerSubscriptions & SUBSCRIBE_INTERCEPT_DESKTOP) != SUBSCRIBE_NONE);
        AddStatusText(szText);
        if (m_xmlSettings.GetEventTTSEvents() & TTS_SUBSCRIPTIONS_INTERCEPT_DESKTOP)
            AddVoiceMessage(szText);
    }
    if((oldUser.uPeerSubscriptions & SUBSCRIBE_INTERCEPT_MEDIAFILE) !=
        (user.uPeerSubscriptions & SUBSCRIBE_INTERCEPT_MEDIAFILE))
    {
        szFormat = LoadText(IDS_SUBINT_MEDIAFILE);
        szText.Format(szFormat, szName, int(user.uPeerSubscriptions & SUBSCRIBE_INTERCEPT_MEDIAFILE) != SUBSCRIBE_NONE);
        AddStatusText(szText);
        if (m_xmlSettings.GetEventTTSEvents() & TTS_SUBSCRIPTIONS_INTERCEPT_MEDIAFILE)
            AddVoiceMessage(szText);
    }
}

void CTeamTalkDlg::OnUserRemove(const TTMessage& msg)
{
    const User& user = msg.user;
    Channel chan;
    m_wndTree.GetChannel(msg.nSource, chan);

    if(msg.user.nUserID == TT_GetMyUserID(ttInst))
    {
        //myself left channel
        OnChannelLeft(chan);
    }
    m_wndTree.RemoveUser(user);

    CString szMsg, szFormat;

    if(TT_GetMyChannelID(ttInst) == msg.nSource)
    {
        szFormat = LoadText(IDS_CHANNEL_LEFT);
        szMsg.Format(szFormat, GetDisplayName(user));

        PlaySoundEvent(SOUNDEVENT_USER_LEFT);

        AddStatusText(szMsg);
        if (m_xmlSettings.GetEventTTSEvents() & TTS_USER_LEFT_SAME)
            AddVoiceMessage(szMsg);
    }
    else if (m_commands[m_nCurrentCmdID] == CMD_COMPLETE_NONE)
    {
        CString szRoot;
        szRoot.LoadString(IDS_ROOTCHANNEL);
        TRANSLATE_ITEM(IDS_ROOTCHANNEL, szRoot);

        szFormat = LoadText(IDS_USERLEFTCHANNEL);
        if(chan.nParentID == 0)
            szMsg.Format(szFormat, GetDisplayName(user), szRoot);
        else
            szMsg.Format(szFormat, GetDisplayName(user), chan.szName);

        if (m_xmlSettings.GetEventTTSEvents() & TTS_USER_LEFT)
            AddVoiceMessage(szMsg);
    }
}

void CTeamTalkDlg::OnChannelAdd(const TTMessage& msg)
{
    ASSERT(msg.ttType == __CHANNEL);
    const Channel& chan = msg.channel;
    m_wndTree.AddChannel(chan);
}

int TransmitToggled(const Channel& chan, StreamTypes uPrev, StreamTypes uCur, StreamTypes uTypes)
{
    int ret;
    if ((uCur & uTypes) && (uPrev & uTypes) != (uCur & uTypes))
        ret = 1;
    else if((uCur & uTypes) == STREAMTYPE_NONE && (uPrev & uTypes) != (uCur & uTypes))
        ret = -1;
    else
        ret = 0;
    return (chan.uChannelType & CHANNEL_CLASSROOM) ? ret : -ret;
}

void CTeamTalkDlg::OnChannelUpdate(const TTMessage& msg)
{
    ASSERT(msg.ttType == __CHANNEL);
    const Channel& chan = msg.channel;

    Channel oldchan = {};
    if(!m_wndTree.GetChannel(chan.nChannelID, oldchan))
        return;

    m_wndTree.UpdateChannel(chan);

    if(chan.nChannelID == TT_GetMyChannelID(ttInst))
        UpdateAudioConfig();

    // Solo transmission
    if(chan.transmitUsersQueue[0] == TT_GetMyUserID(ttInst) &&
        oldchan.transmitUsersQueue[0] != TT_GetMyUserID(ttInst))
        PlaySoundEvent(SOUNDEVENT_TRANSMITQUEUE_HEAD);

    if(chan.transmitUsersQueue[0] != TT_GetMyUserID(ttInst) &&
        oldchan.transmitUsersQueue[0] == TT_GetMyUserID(ttInst))
        PlaySoundEvent(SOUNDEVENT_TRANSMITQUEUE_STOP);

    // remaining is for class room channel
    if(TT_GetMyChannelID(ttInst) != chan.nChannelID)
        return;

    transmitusers_t oldTransmit, newTransmit;
    transmitusers_t::iterator ii;
    GetTransmitUsers(oldchan, oldTransmit);
    GetTransmitUsers(chan, newTransmit);

    User user;
    CString szMsg, szFormat;

    std::set<int> modified;
    for(auto id : oldTransmit)
        modified.insert(id.first);
    for(auto id : newTransmit)
        modified.insert(id.first);

    for (auto userid : modified)
    {
        CString szName;
        if(userid == TT_CLASSROOM_FREEFORALL)
            szName = LoadText(IDS_EVERYONE);
        else if(!m_wndTree.GetUser(userid, user))
            continue;
        else
            szName = GetDisplayName(user);

        int ret = TransmitToggled(chan, oldTransmit[userid], newTransmit[userid], STREAMTYPE_VOICE);
        if(ret != 0)
        {
            if (ret < 0)
                szMsg.Format(LoadText(IDS_NOLONGERTRANSMIT, _T("%s can no longer transmit voice!")), szName);
            else
                szMsg.Format(LoadText(IDS_CANNOWTRANSMIT, _T("%s can now transmit voice!")), szName);

            AddStatusText(szMsg);
            if (m_xmlSettings.GetEventTTSEvents() & TTS_CLASSROOM_VIDEO_TX)
                AddVoiceMessage(szMsg);
        }
        ret = TransmitToggled(chan, oldTransmit[userid], newTransmit[userid], STREAMTYPE_VIDEOCAPTURE);
        if(ret != 0)
        {
            if (ret < 0)
                szMsg.Format(LoadText(IDS_NOLONGERTRANSMITVIDEO, _T("%s can no longer transmit video input!")), szName);
            else
                szMsg.Format(LoadText(IDS_CANNOWTRANSMITVIDEO, _T("%s can now transmit video input!")), szName);

            AddStatusText(szMsg);
            if (m_xmlSettings.GetEventTTSEvents() & TTS_CLASSROOM_VOICE_TX)
                AddVoiceMessage(szMsg);
        }
        ret = TransmitToggled(chan, oldTransmit[userid], newTransmit[userid], STREAMTYPE_DESKTOP);
        if(ret != 0)
        {
            if (ret < 0)
                szMsg.Format(LoadText(IDS_NOLONGERTRANSMITDESKTOP, _T("%s can no longer transmit shared desktops!")), szName);
            else
                szMsg.Format(LoadText(IDS_CANNOWTRANSMITDESKTOP, _T("%s can now transmit shared desktops!")), szName);

            AddStatusText(szMsg);
            if (m_xmlSettings.GetEventTTSEvents() & TTS_CLASSROOM_DESKTOP_TX)
                AddVoiceMessage(szMsg);
        }
        ret = TransmitToggled(chan, oldTransmit[userid], newTransmit[userid], STREAMTYPE_MEDIAFILE);
        if(ret != 0)
        {
            if (ret < 0)
                szMsg.Format(LoadText(IDS_NOLONGERTRANSMITMEDIAFILE, _T("%s can no longer transmit media files!")), szName);
            else
                szMsg.Format(LoadText(IDS_CANNOWTRANSMITMEDIAFILE, _T("%s can now transmit media files!")), szName);
            AddStatusText(szMsg);
            if (m_xmlSettings.GetEventTTSEvents() & TTS_CLASSROOM_MEDIAFILE_TX)
                AddVoiceMessage(szMsg);
        }
    }
}

void CTeamTalkDlg::OnChannelRemove(const TTMessage& msg)
{
    ASSERT(msg.ttType == __CHANNEL);
    const Channel& chan = msg.channel;
    m_wndTree.RemoveChannel(chan);
}

void CTeamTalkDlg::OnChannelJoined(const Channel& chan)
{
    m_tabFiles.UpdateFiles(chan.nChannelID);

    m_tabChat.m_wndRichEdit.SetChannelInfo(chan.nChannelID);

    CString szMsg, szFormat;
    if(chan.uChannelType & CHANNEL_CLASSROOM)
    {
        szFormat = LoadText(IDS_CLASSROOM_SELF_JOINED);
        szMsg.Format(szFormat, LimitText(chan.szName));
    }
    else
    {
        szFormat = LoadText(IDS_CHANNEL_SELF_JOINED);
        szMsg.Format(szFormat, LimitText(chan.szName));
    }

    AddStatusText(szMsg);
    if (m_xmlSettings.GetEventTTSEvents() & TTS_USER_JOINED)
        AddVoiceMessage(szMsg);

    UpdateChannelLog();

    UpdateAudioStorage(TRUE);
    UpdateAudioConfig();
    UpdateWindowTitle();
}

void CTeamTalkDlg::OnChannelLeft(const Channel& chan)
{
    m_tabFiles.UpdateFiles(-1);
    UpdateWindowTitle();

    CString szMsg, szFormat;
    szFormat = LoadText(IDS_CHANNEL_SELF_LEFT);
    szMsg.Format(szFormat, chan.szName);

    AddStatusText(szMsg);
    if (m_xmlSettings.GetEventTTSEvents() & TTS_USER_LEFT)
        AddVoiceMessage(szMsg);
}

void CTeamTalkDlg::OnFileAdd(const TTMessage& msg)
{
    ASSERT(msg.ttType == __REMOTEFILE);
    const RemoteFile& remotefile = msg.remotefile;
    User user;
    m_tabFiles.AddFile(remotefile.nChannelID, remotefile.nFileID);

    if(remotefile.nChannelID == TT_GetMyChannelID(ttInst) &&
       m_commands[m_nCurrentCmdID] != CMD_COMPLETE_LOGIN &&
       m_commands[m_nCurrentCmdID] != CMD_COMPLETE_JOIN)
    {
        PlaySoundEvent(SOUNDEVENT_FILES_UPDATED);
        TT_GetUserByUsername(ttInst, remotefile.szUsername, &user);
        CString szMsg;
        szMsg.Format(LoadText(IDS_FILEADDED), GetDisplayName(user), remotefile.szFileName);
        AddStatusText(szMsg);
        if (m_xmlSettings.GetEventTTSEvents() & TTS_FILE_ADD)
            AddVoiceMessage(szMsg);
    }
}

void CTeamTalkDlg::OnFileRemove(const TTMessage& msg)
{
    const RemoteFile& remotefile = msg.remotefile;
    m_tabFiles.RemoveFile(remotefile.nChannelID, remotefile.nFileID);

    if(remotefile.nChannelID == TT_GetMyChannelID(ttInst))
    {
        PlaySoundEvent(SOUNDEVENT_FILES_UPDATED);
        CString szMsg;
        szMsg.Format(LoadText(IDS_FILEREMOVED), remotefile.szFileName);
        AddStatusText(szMsg);
        if (m_xmlSettings.GetEventTTSEvents() & TTS_FILE_REMOVE)
            AddVoiceMessage(szMsg);
    }
}

void CTeamTalkDlg::OnUserAccount(const TTMessage& msg)
{
    m_useraccounts.push_back(msg.useraccount);
}

void CTeamTalkDlg::OnBannedUser(const TTMessage& msg)
{
    m_bannedusers.push_back(msg.banneduser);
}

void CTeamTalkDlg::OnUserMessage(const TTMessage& msg)
{
    //wParam = userid, lParam = message index

    const TextMessage& textmsg = msg.textmessage;

    switch(textmsg.nMsgType)
    {
    case MSGTYPE_USER :
    {
        m_wndTree.AddUserMessage(textmsg.nFromUserID, textmsg);

        PlaySoundEvent(SOUNDEVENT_USER_TEXTMSG);

        //find message session
        BOOL bNew = FALSE;
        CMessageDlg* pMsgDlg = GetUsersMessageSession(textmsg.nFromUserID,
                                                      TRUE, &bNew);

        if(pMsgDlg)
        {
            if(!bNew)
            {
                MyTextMessage msg;
                if(m_wndTree.GetLastUserMessage(textmsg.nFromUserID, msg))
                    pMsgDlg->AppendMessage(msg, TRUE);
            }

            if( m_xmlSettings.GetPopupOnMessage() )
            {
                pMsgDlg->ShowWindow(SW_SHOW);
                //pMsgDlg->SetForegroundWindow();
            }
            m_wndTree.SetUserMessage(textmsg.nFromUserID, TRUE);
        }
        else
        {
            if( m_xmlSettings.GetPopupOnMessage() )
            {
                HTREEITEM hItem = m_wndTree.GetUserItem(textmsg.nFromUserID);
                if(hItem)
                {
                    m_wndTree.SelectItem(hItem);
                    OnUsersMessages();
                }
            }
            else
            {
                m_wndTree.SetUserMessage(textmsg.nFromUserID, TRUE);
            }
        }

        User user;
        if(TT_GetUser(ttInst, textmsg.nFromUserID, &user))
        {
            CString szName = GetDisplayName(user);
            CString szFmt, szMsg;
            szFmt.LoadString(IDS_USERTEXTMSG);
            TRANSLATE_ITEM(IDS_USERTEXTMSG, szFmt);
            szMsg.Format(szFmt, szName, textmsg.szMessage);
            if (m_xmlSettings.GetEventTTSEvents() & TTS_USER_TEXTMSG_PRIVATE)
                AddVoiceMessage(szMsg);
        }
    }
    break;
    case MSGTYPE_CHANNEL :
    {
        User user;
        //add message to channel console
        if(TT_GetUser(ttInst, textmsg.nFromUserID, &user))
        {
            CString szName = GetDisplayName(user);
            CString szLine = m_tabChat.m_wndRichEdit.AddMessage(szName,
                                                                textmsg.szMessage);

            if(!m_bTwoPanes)
                m_wndTree.SetChannelMessage(textmsg.nFromUserID, TRUE);

            Channel chan;
            if(m_wndTree.GetChannel(textmsg.nChannelID, chan))
                WriteLogMsg(m_logChan, szLine);
            
            CString szFmt, szMsg;
            szFmt.LoadString(IDS_CHANTEXTMSG);
            TRANSLATE_ITEM(IDS_CHANTEXTMSG, szFmt);
            szMsg.Format(szFmt, szName, textmsg.szMessage);
            if (m_xmlSettings.GetEventTTSEvents() & TTS_USER_TEXTMSG_CHANNEL)
                AddVoiceMessage(szMsg);
        }

        PlaySoundEvent(SOUNDEVENT_USER_CHANNEL_TEXTMSG);
    }
    break;
    case MSGTYPE_BROADCAST :
    {
        User user;
        if(!TT_GetUser(ttInst, textmsg.nFromUserID, &user))
            break;
        CString szName = GetDisplayName(user);
        m_tabChat.m_wndRichEdit.AddBroadcastMessage(textmsg.szMessage);

        PlaySoundEvent(SOUNDEVENT_USER_BROADCAST_TEXTMSG);

        CString szFmt, szMsg;
        szFmt.LoadString(IDS_BCASTTEXTMSG);
        TRANSLATE_ITEM(IDS_BCASTTEXTMSG, szFmt);
        szMsg.Format(szFmt, szName, textmsg.szMessage);
        if (m_xmlSettings.GetEventTTSEvents() & TTS_USER_TEXTMSG_BROADCAST)
            AddVoiceMessage(szMsg);
    }
    break;
    case MSGTYPE_CUSTOM :
    {
        User user;
        if(!TT_GetUser(ttInst, textmsg.nFromUserID, &user))
            break;

        CStringList tokens;
        GetCustomCommand(textmsg.szMessage, tokens);
        POSITION pos = tokens.GetHeadPosition();
        if(tokens.GetCount()>1 &&
            tokens.GetNext(pos) == TT_INTCMD_DESKTOP_ACCESS)
        {
            CString szText, szFormat;
            if(tokens.GetNext(pos) != _T("0"))
            {
                szFormat = LoadText(IDS_DESKTOPINPUT_REQUEST);
                szText.Format(szFormat, GetDisplayName(user));
                if (m_xmlSettings.GetEventTTSEvents() & TTS_SUBSCRIPTIONS_DESKTOPINPUT)
                {
                    AddVoiceMessage(szText);
                }
                PlaySoundEvent(SOUNDEVENT_USER_DESKTOP_ACCESS);
            }
            else
            {
                szFormat = LoadText(IDS_DESKTOPINPUT_RETRACT);
                SubscribeCommon(textmsg.nFromUserID, SUBSCRIBE_DESKTOPINPUT, FALSE);
                szText.Format(szFormat, GetDisplayName(user));
            }
            AddStatusText(szText);
        }
    }
    break;
    }
}

void CTeamTalkDlg::OnUserStateChange(const TTMessage& msg)
{
    ASSERT(msg.ttType == __USER);
    const User& user = msg.user;

    User olduser = {};
    m_wndTree.GetUser(user.nUserID, olduser);

    m_wndTree.UpdateUser(msg.user);

    if((user.uUserState & USERSTATE_VOICE))
        m_Talking.insert(user.nUserID);
    else
    {
        //add to stopped talking (for event)
        m_Talking.erase(user.nUserID);

        if(m_xmlSettings.GetEventChannelSilent().size() && m_Talking.empty())
        {
            PlaySoundEvent(SOUNDEVENT_CHANNEL_SILENT);
        }
    }
}

void CTeamTalkDlg::OnUserVideoCaptureFrame(const TTMessage& msg)
{
    ASSERT(msg.ttType == __INT32);
    if(msg.nSource == 0)
    {
        if(!m_bPreferencesOpen)
        {
            //we don't show local frames, so just take out and delete
            VideoFrame* pFrame = TT_AcquireUserVideoCaptureFrame(ttInst, msg.nSource);
            if(pFrame)
                TT_ReleaseUserVideoCaptureFrame(ttInst, pFrame);
        }
        return;
    }

    int nUserID = (msg.nSource | VIDEOTYPE_CAPTURE);

    //ignore self
    if(m_videoignore.find(nUserID) != m_videoignore.end())
        return;

    mapvideodlg_t::iterator ii = m_videodlgs.find(nUserID);
    if(ii != m_videodlgs.end())
        ii->second->NewVideoFrame();
    else
        OpenVideoSession(nUserID);
}

void CTeamTalkDlg::OnUserMediaVideoFrame(const TTMessage& msg)
{
    int nUserID = (msg.nSource | VIDEOTYPE_MEDIAFILE);

    //ignore self
    if(m_videoignore.find(nUserID) != m_videoignore.end())
        return;

    mapvideodlg_t::iterator ii = m_videodlgs.find(nUserID);
    if(ii != m_videodlgs.end())
        ii->second->NewVideoFrame();
    else
        OpenVideoSession(nUserID);
    return;
}

void CTeamTalkDlg::OnUserDesktopWindow(const TTMessage& msg)
{
    //ignore self
    if(msg.nSource == 0 || m_desktopignore.find(msg.nSource) != m_desktopignore.end())
        return;

    mapdesktopdlg_t::iterator ii = m_desktopdlgs.find(msg.nSource);
    if(ii == m_desktopdlgs.end())
    {
        CUserDesktopDlg* dlg = new CUserDesktopDlg(msg.nSource, this);
        BOOL b = dlg->Create(CUserDesktopDlg::IDD, GetDesktopWindow());
        ASSERT(b);
        dlg->ShowWindow(SW_SHOW);
        m_desktopdlgs[msg.nSource] = dlg;
        PlaySoundEvent(SOUNDEVENT_USER_DESKTOPSESSION_NEW);
    }
}

void CTeamTalkDlg::OnUserDesktopInput(const TTMessage& msg)
{
    ASSERT(msg.ttType == __DESKTOPINPUT);

    //extract as many DesktopInputs as possible
    INT32 n_count = 0;
    vector<DesktopInput> inputs;
    inputs.push_back(msg.desktopinput);

    if(inputs.empty())
        return;

    //ignore input if we're not sharing a window
    if((TT_GetFlags(ttInst) & CLIENT_DESKTOP_ACTIVE) == 0)
        return;

    TTKeyTranslate key_trans = TTKEY_NO_TRANSLATE;

    key_trans = TTKEY_TTKEYCODE_TO_WINKEYCODE;

    vector<DesktopInput> executeInputs;

    CRect offset;
    if(!::GetWindowRect(GetSharedDesktopWindowHWND(), offset))
        return;

    for(size_t i=0;i<inputs.size();i++)
    {
        //calculate absolute offset
        if(inputs[i].uMousePosX != TT_DESKTOPINPUT_MOUSEPOS_IGNORE &&
           inputs[i].uMousePosY != TT_DESKTOPINPUT_MOUSEPOS_IGNORE)
        {
            //don't allow mouse input outside shared window
            if(inputs[i].uMousePosX > offset.Width() ||
               inputs[i].uMousePosY > offset.Height())
                continue;

            inputs[i].uMousePosX += UINT16(offset.left);
            inputs[i].uMousePosY += UINT16(offset.top);

        }

        DesktopInput trans_input;
        ZERO_STRUCT(trans_input);

        if(key_trans == TTKEY_NO_TRANSLATE)
            executeInputs.push_back(inputs[i]);
        else if(TT_DesktopInput_KeyTranslate(key_trans, &inputs[i], &trans_input, 1))
            executeInputs.push_back(trans_input);
        else
            TRACE(_T("Failed to translate received desktop input. KeyCode: 0x%X"), inputs[i].uKeyCode);
    }

    if(executeInputs.size())
    {
        TT_DesktopInput_Execute(&executeInputs[0], INT32(executeInputs.size()));

        //send desktop update immediately and restart desktop tx timer.
        if(SendDesktopWindow())
            RestartSendDesktopWindowTimer();
        else
            m_bSendDesktopOnCompletion = TRUE;
    }
}

void CTeamTalkDlg::OnUserAudioFile(const TTMessage& msg)
{
    User user;
    if(!TT_GetUser(ttInst, msg.nSource, &user))
        return;

    if(msg.mediafileinfo.nStatus == MFS_ERROR)
    {
        CString szMsg;
        szMsg.Format(LoadText(IDS_FAILEDTOWRITEAUDIOFILE, _T("Failed to write audio file for %s")),
                     GetDisplayName(user));
        AddStatusText(szMsg);
    }
}

void CTeamTalkDlg::OnVoiceActivated(const TTMessage& msg)
{
    m_wndTree.SetUserTalking(TT_GetMyUserID(ttInst),
                             IsMyselfTalking());
    PlaySoundEvent(msg.bActive? SOUNDEVENT_VOICEACTIVATED :  SOUNDEVENT_VOICEDEACTIVATED);
}

void CTeamTalkDlg::OnFileTransfer(const TTMessage& msg)
{
    const FileTransfer& filetransfer = msg.filetransfer;
    switch(filetransfer.nStatus)
    {
    case FILETRANSFER_ERROR :
        // play hotkey event if exists
        PlaySoundEvent(SOUNDEVENT_FILETX_COMPLETE);

        if(m_mTransfers.find(filetransfer.nTransferID) != m_mTransfers.end())
            m_mTransfers.find(filetransfer.nTransferID)->second->Failed();
        else
        {
            CString szError;
            szError.Format(LoadText(IDS_FAILEDTOSTARTFILETRANSFER, _T("Failed to start file transfer.\r\nFile name: %s")),
                (filetransfer.bInbound? filetransfer.szRemoteFileName :
                filetransfer.szRemoteFileName));
            AfxMessageBox(szError);
        }
        break;
    case FILETRANSFER_FINISHED :
        {
            if(m_mTransfers.find(filetransfer.nTransferID) != m_mTransfers.end())
                m_mTransfers.find(filetransfer.nTransferID)->second->Completed();

            // play hotkey event if exists
            PlaySoundEvent(SOUNDEVENT_FILETX_COMPLETE);
        }
        break;
    case FILETRANSFER_ACTIVE :
        {
            CFileTransferDlg* dlg = new CFileTransferDlg(filetransfer.nTransferID, FALSE, this);
            dlg->m_bAutoClose = m_xmlSettings.GetCloseTransferDialog();
            VERIFY(dlg->Create(CFileTransferDlg::IDD));
            dlg->ShowWindow(SW_SHOW);
            dlg->SetFocus();
            m_mTransfers[filetransfer.nTransferID] = dlg;
        }
    }
}

void CTeamTalkDlg::OnStreamMediaFile(const TTMessage& msg)
{
    switch(msg.mediafileinfo.nStatus)
    {
    case MFS_ERROR :
        AddStatusText(LoadText(IDS_ERRORSTREAMINGTOCHANNEL, _T("Error streaming media file to channel")));
        StopMediaStream();
        break;
    case MFS_STARTED :
        AddStatusText(LoadText(IDS_STREAMINGTOCHANNELSTART, _T("Started streaming media file to channel")));
        break;
    case MFS_FINISHED :
        AddStatusText(LoadText(IDS_STREAMINGTOCHANNELFINISH, _T("Finished streaming media file to channel")));
        StopMediaStream();

        if (m_xmlSettings.GetMediaFileRepeat(false))
            StartMediaStream();
        break;
    case MFS_ABORTED :
        AddStatusText(LoadText(IDS_STREAMINGTOCHANNELABORT, _T("Aborted streaming media file to channel")));
        StopMediaStream();
        break;
    }
}

void CTeamTalkDlg::OnInternalError(const TTMessage& msg)
{
    AddStatusText(msg.clienterrormsg.szErrorMsg);
}

void CTeamTalkDlg::OnDesktopWindowTransfer(const TTMessage& msg)
{
    ASSERT(msg.ttType == __INT32);
    if(msg.nSource == 0 && m_bSendDesktopOnCompletion && SendDesktopWindow())
    {
        m_bSendDesktopOnCompletion = FALSE;
        RestartSendDesktopWindowTimer();
    }
}

void CTeamTalkDlg::OnHotKey(const TTMessage& msg)
{
    switch(msg.nSource)
    {
    case HOTKEY_PUSHTOTALK_ID :
        if(msg.bActive)
        {
            TT_EnableVoiceTransmission(ttInst, TRUE);
            m_wndTree.SetUserTalking(TT_GetMyUserID(ttInst),
                                     IsMyselfTalking());

            // play hotkey event if exists
            PlaySoundEvent(SOUNDEVENT_PUSHTOTALK);
        }
        else    //key being released
        {
            TT_EnableVoiceTransmission(ttInst, FALSE);

            m_wndTree.SetUserTalking(TT_GetMyUserID(ttInst),
                                     IsMyselfTalking());

            //released event
            PlaySoundEvent(SOUNDEVENT_PUSHTOTALK);
        }
        break;
    case HOTKEY_VOICEACT_ID :
        if(msg.bActive)
            EnableVoiceActivation(!(TT_GetFlags(ttInst) & CLIENT_SNDINPUT_VOICEACTIVATED));
        break;
    case HOTKEY_VOLUME_PLUS :
        if(msg.bActive)
            UpdateMasterVolume(m_wndVolSlider.GetPos()+1);
        break;
    case HOTKEY_VOLUME_MINUS :
        if(msg.bActive)
            UpdateMasterVolume(m_wndVolSlider.GetPos()-1);
        break;
    case HOTKEY_MUTEALL :
        if(msg.bActive)
            TT_SetSoundOutputMute(ttInst, !(TT_GetFlags(ttInst) & CLIENT_SNDOUTPUT_MUTE));
        break;
    case HOTKEY_VOICEGAIN_PLUS :
        if(msg.bActive)
            UpdateGainLevel(m_wndGainSlider.GetPos() + 1);
        break;
    case HOTKEY_VOICEGAIN_MINUS :
        if(msg.bActive)
            UpdateGainLevel(m_wndGainSlider.GetPos() - 1);
        break;
    case HOTKEY_MIN_RESTORE :
        if(msg.bActive)
        {
            if(IsIconic())
            {
                SendMessage(WM_SYSCOMMAND, SC_RESTORE);
                SetForegroundWindow();
            }
            else
                SendMessage(WM_SYSCOMMAND, SC_MINIMIZE);
        }
        break;
    }
}

BOOL CTeamTalkDlg::OnToolTipText(UINT, NMHDR* pNMHDR, LRESULT* pResult)
{
    // need to handle both ANSI and UNICODE versions of the message
    TOOLTIPTEXTA* pTTTA = (TOOLTIPTEXTA*)pNMHDR;
    TOOLTIPTEXTW* pTTTW = (TOOLTIPTEXTW*)pNMHDR;
    TCHAR szFullText[256];
    CString strTipText;
    CString strPromptText;

    UINT_PTR nID = pNMHDR->idFrom;
    if (pNMHDR->code == TTN_NEEDTEXTA && (pTTTA->uFlags & TTF_IDISHWND) ||
        pNMHDR->code == TTN_NEEDTEXTW && (pTTTW->uFlags & TTF_IDISHWND))
    {
        // idFrom is actually the HWND of the tool
        nID = ::GetDlgCtrlID((HWND)nID);
    }

    switch(nID)
    {
    case 0 : // will be zero on a separator
        break;
    case IDC_SLIDER_GAINLEVEL :
        strPromptText.LoadString(IDS_MICGAIN);
        TRANSLATE_ITEM(IDS_MICGAIN,strPromptText);
        break;
    case IDC_SLIDER_VOLUME :
        strPromptText.LoadString(IDS_MASTERVOL);
        TRANSLATE_ITEM(IDS_MASTERVOL,strPromptText);
        break;
    case IDC_PROGRESS_VOICEACT :
        strPromptText.LoadString(IDS_VOICELEVEL);
        TRANSLATE_ITEM(IDS_VOICELEVEL,strPromptText);
        break;
    case IDC_SLIDER_VOICEACT :
        strPromptText.LoadString(IDS_VOICEACTLEVEL);
        TRANSLATE_ITEM(IDS_VOICEACTLEVEL,strPromptText);
        break;
    default :
        AfxLoadString(UINT(nID), szFullText);
        AfxExtractSubString(strTipText, szFullText, 1, '\n');
        AfxExtractSubString(strPromptText, szFullText, 0, '\n');
        break;
    }

#if defined(UNICODE) || defined(_UNICODE)
    ASSERT(pNMHDR->code != TTN_NEEDTEXTA);
    _tcsncpy(pTTTW->szText, strPromptText, sizeof(pTTTW->szText)/sizeof(TCHAR));
#else
    if (pNMHDR->code == TTN_NEEDTEXTA)
        lstrcpyn(pTTTA->szText, strPromptText, sizeof(pTTTA->szText));
    else
        _mbstowcsz(pTTTW->szText, strPromptText, sizeof(pTTTW->szText));
#endif

    ////old code
    //if (pNMHDR->code == TTN_NEEDTEXTA)
    //    lstrcpyn(pTTTA->szText, strPromptText, sizeof(pTTTA->szText));
    //else
    //    _mbstowcsz(pTTTW->szText, strPromptText, sizeof(pTTTW->szText));

    *pResult = 0;

    return TRUE;    // message was handled
}

LRESULT CTeamTalkDlg::OnSendChannelMessage(WPARAM wParam, LPARAM lParam)
{
    return TRUE;
}

BOOL CTeamTalkDlg::OnInitDialog()
{
    CDialogExx::OnInitDialog();

    //load toolbar & statusbar
    UINT indicators[] = {ID_INDICATOR_STATS/*, ID_SEPARATOR*/};
    InitDialogEx(TRUE, TRUE, indicators, 1, IDR_TOOLBAR1);

    //load accelerators
    m_hAccel = ::LoadAccelerators(AfxGetResourceHandle(), (LPCTSTR)IDR_ACCELERATOR2);
    if (!m_hAccel)
        MessageBox(LoadText(IDS_ACCELERATORNOTLOADDED, _T("The accelerator table was not loaded")));

    //load bmp for pictures
    //transparent pictures
    m_wndVolPic.SubclassDlgItem(IDC_STATIC_VOLUME, this);
    m_wndVolPic.ReloadBitmap(IDB_BITMAP_VOLUME);
    m_wndVoicePic.SubclassDlgItem(IDC_STATIC_VOICEACT, this);
    m_wndVoicePic.ReloadBitmap(IDB_BITMAP_VOICEACT);
    m_wndMikePic.SubclassDlgItem(IDC_STATIC_MIKE, this);
    m_wndMikePic.ReloadBitmap(IDB_BITMAP_MIKE);

    // Set the icon for this dialog.  The framework does this automatically
    //  when the application's main window is not a dialog
    SetIcon(m_hIcon, TRUE);            // Set big icon
    SetIcon(m_hIcon, FALSE);        // Set small icon

    //init session tree control
    m_wndTree.Initialize();

    m_pPlaySndThread = new CPlaySoundThread();
    m_pPlaySndThread->CreateThread();

    BOOL bRunWizard = FALSE;

    CString szXmlFile = _T( SETTINGS_FILE );
    //check if user wants to use a custom config
    POSITION pos = m_cmdArgs.Find(_T("cfg"));
    if(pos != NULL && m_cmdArgs.GetNext(pos) && pos != NULL)
    {
        szXmlFile = m_cmdArgs.GetAt(pos);
        if(!FileExists(szXmlFile))
        {
            ClientXML f(TT_XML_ROOTNAME);
            f.CreateFile(STR_LOCAL(szXmlFile));
        }
    }
    //load xml settings
    if(!FileExists(szXmlFile))
    {
        TCHAR buff[MAX_PATH];
        _tcsncpy(buff, _tgetenv(_T("APPDATA")), MAX_PATH);
        szXmlFile = buff;
        szXmlFile += _T("\\");
        szXmlFile += TEAMTALK_INSTALLDIR;
        szXmlFile += _T("\\");
        szXmlFile += _T( SETTINGS_FILE );
    }

    string ansiXml = STR_LOCAL(szXmlFile);
    if(FileExists(szXmlFile))
    {
        if(!m_xmlSettings.LoadFile( ansiXml ))
        {
            if(m_xmlSettings.HasErrors())
            {
                CString szMsg = LoadText(IDS_UNABLELOADSETFILE, _T("Unable to load the settings file:\r\n"));
                szMsg += szXmlFile + _T(".\r\n") + LoadText(IDS_CREATENEWSETFILE, _T("Create a new settings file?"));
                if(AfxMessageBox(szMsg, MB_YESNO) == IDYES)
                {
                    m_xmlSettings.CreateFile(ansiXml);
                    bRunWizard = TRUE;
                }
            }
            else
            {
                m_xmlSettings.CreateFile(ansiXml);
            }
        }
        else
        {
            string version = m_xmlSettings.GetFileVersion();
            if(!VersionSameOrLater(STR_UTF8(version), _T( TEAMTALK_XML_VERSION )))
            {
                // Volume defaults changed in 5.1 format
                m_xmlSettings.SetSoundOutputVolume(DEFAULT_SOUND_OUTPUT_VOLUME);
                m_xmlSettings.SetVoiceGainLevel(DEFAULT_SOUND_GAIN_LEVEL);
                m_xmlSettings.SetFileVersion(TEAMTALK_XML_VERSION);
            }
        }
    }
    else
    {
        // Create config folder if it doesn't exist already
        CString szCfgDir = szXmlFile;
        int nDirEnd = szCfgDir.ReverseFind('\\');
        if(nDirEnd>=0)
        {
            szCfgDir = szXmlFile.Left(nDirEnd);
            if(!DirectoryExists(szCfgDir))
                CreateDirectory(szCfgDir, NULL);
        }

        BOOL bIniCreated = FALSE;
        CString szDefPath = GetExecutableFolder() + _T("\\") + _T(SETTINGS_DEFAULT_FILE);
        if (FileExists(szDefPath))
        {
            CopyFile(szDefPath, STR_UTF8(ansiXml), FALSE);
            bIniCreated = m_xmlSettings.LoadFile(ansiXml);
        }
        
        if(!bIniCreated)
        {
            // Create settings file in current directory if it cannot be created in cfg-folder
            if(!m_xmlSettings.CreateFile(ansiXml))
                m_xmlSettings.CreateFile(SETTINGS_FILE);
            bRunWizard = TRUE;
        }
    }

    nTextLimit = m_xmlSettings.GetMaxTextLength(DEFAULT_MAX_STRING_LENGTH);

    //setup language
    CString szLanguage = STR_UTF8( m_xmlSettings.GetLanguageFile().c_str() );
    if(!szLanguage.IsEmpty())
    {
        Languages::Instance( szLanguage );
        TRANSLATE(*this, IDD);
        Translate();
    }

    //see if wizard should be invoked
    if(!bRunWizard)
        bRunWizard = m_cmdArgs.Find(_T("wizard")) > 0;

    ttInst = TT_InitTeamTalk(m_hWnd, WM_TEAMTALK_CLIENTEVENT);

    //Check whether we should run the wizard
    if(bRunWizard)
    {
        RunWizard();
    }

    m_wndVolSlider.SetRange(0, 100, TRUE);
    m_wndVolSlider.SetPos(m_xmlSettings.GetSoundOutputVolume(DEFAULT_SOUND_OUTPUT_VOLUME));
    m_wndVolSlider.SetPageSize(m_wndVolSlider.GetRangeMax() / 20);

    m_wndGainSlider.SetRange(0, 100, TRUE);
    m_wndGainSlider.SetPos(m_xmlSettings.GetVoiceGainLevel(DEFAULT_SOUND_GAIN_LEVEL));
    m_wndGainSlider.SetPageSize(m_wndGainSlider.GetRangeMax() / 20);

    m_wndVoiceSlider.SetRange(SOUND_VU_MIN, DEFAULT_SOUND_VU_MAX, TRUE);
    m_wndVoiceSlider.SetPageSize(m_wndVoiceSlider.GetRangeMax() / 20);
    m_wndVUProgress.SetRange(SOUND_VU_MIN, DEFAULT_SOUND_VU_MAX);

    //set vumeter and voice act-settings
    m_wndVoiceSlider.SetPos(m_xmlSettings.GetVoiceActivationLevel());

    //load hotkey
    ASSERT(IsWindow(GetSafeHwnd()));

    HotKey hotkey;
    m_xmlSettings.GetPushToTalkKey(hotkey);

    if(m_xmlSettings.GetPushToTalk() && hotkey.size())
    {
        TT_HotKey_Register(ttInst, HOTKEY_PUSHTOTALK_ID, &hotkey[0], INT32(hotkey.size()));
        m_bHotKey = m_xmlSettings.GetPushToTalk();
        m_szHotKey = CKeyCompDlg::GetHotkeyString(hotkey);
    }
    else
    {
        m_szHotKey.Empty();
    }

    //voice activation
    EnableVoiceActivation(m_xmlSettings.GetVoiceActivated());

    UpdateWindowTitle();

    if(m_xmlSettings.GetFirewallInstall(true))
    {
        FirewallInstall();
        m_xmlSettings.SetFirewallInstall(false);
    }

    //positioning in 3D
    TT_Enable3DSoundPositioning(ttInst, m_xmlSettings.GetAutoPositioning());

    EnableSpeech(m_xmlSettings.GetEventTTSEvents() != 0);

    //load fonts
    MyFont font;
    string szFaceName;
    int nSize;
    bool bBold, bUnderline, bItalic;
    if( m_xmlSettings.GetFont(szFaceName, nSize, bBold, bUnderline, bItalic) )
    {
        if(nSize>0)
        {
            font.szFaceName = STR_UTF8( szFaceName.c_str() );
            font.nSize = nSize;
            font.bBold = bBold;
            font.bUnderline = bUnderline;
            font.bItalic = bItalic;
            LOGFONT lfont;

            ConvertFont( font, lfont);
            m_Font.CreateFontIndirect(&lfont);
            SwitchFont();
        }
    }

    m_brush.CreateSolidBrush(RGB(244,244,244));

    BOOL b = m_tabChat.Create(IDD_TAB_CHAT, &m_wndTabCtrl);
    CString szChat = LoadText(IDS_CHAT, _T("Chat"));
    TRANSLATE_ITEM(IDS_CHAT, szChat);
    m_wndTabCtrl.AddTab(&m_tabChat, szChat, 0);
    b &= m_tabFiles.Create(IDD_TAB_FILES, &m_wndTabCtrl);
    CString szFiles = LoadText(IDS_FILES, _T("Files"));
    TRANSLATE_ITEM(IDS_FILES, szFiles);
    m_wndTabCtrl.AddTab(&m_tabFiles, szFiles, 0);
    m_tabChat.m_hAccel = m_hAccel;
    m_tabFiles.m_hAccel = m_hAccel;
    m_wndTabCtrl.SetOrientation(e_tabTop);

    //set splitter panes
    m_wndSplitter.Create(WS_CHILD | WS_DLGFRAME | WS_VISIBLE, CRect(0,0,0,0), this, IDC_VERT_SPLITTER);
    m_wndSplitter.SetPanes(&m_wndTree, &m_wndTabCtrl);

    m_bResizeReady = TRUE;

    //1 or 2 panes (2 panes is default)
    if(!m_xmlSettings.GetWindowExtended())
        OnChannelsViewchannelmessages();
    else
        OnSplitterMoved(0,0);

    //sizing
    int left,top,width,height;
    if(m_xmlSettings.GetWindowPlacement(left, top, width, height))
    {
        int nWidth = GetSystemMetrics(SM_CXSCREEN);
        int nHeight = GetSystemMetrics(SM_CYSCREEN);

        if(left > 0 && left < nWidth && top > 0 && top < nHeight)
            MoveWindow(left, top, width, height);
    }

    //always on top?
    if( m_xmlSettings.GetAlwaysOnTop() )
        SetWindowPos(&this->wndTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

    //show user count in treectrl
    m_wndTree.ShowUserCount(m_xmlSettings.GetShowUserCount());

    m_wndTree.SetSortOrder((SortOrder)m_xmlSettings.GetSortOrder());

    //show username instead of nickname
    bShowUsernames = m_xmlSettings.GetShowUsernames();

    //timestamp on messages?
    m_tabChat.m_wndRichEdit.m_bShowTimeStamp = m_xmlSettings.GetMessageTimeStamp();

    m_wndVUProgress.ShowWindow(m_xmlSettings.GetVuMeterUpdate()?SW_SHOW : SW_HIDE);

    RunAppUpdate();
    SetTimer(TIMER_APPUPDATE_ID, 24 * 60 * 60 * 1000, NULL);

    //register hotkeys
    UpdateHotKeys();

    if(m_xmlSettings.GetVideoCaptureEnabled(false))
        OnMeEnablevideotransmission();

    //parse command line arguments
    ParseArgs();

    //autoconnect to latest
    if(m_xmlSettings.GetAutoConnectToLastest() && 
        m_xmlSettings.GetLatestHostEntryCount()>0 &&
        m_szTTLink.GetLength() == 0    )//don't connect if a link has been provided
    {
        m_nLastRecvBytes = 0;
        m_nLastSentBytes = 0;
        HostEntry lasthost;
        m_xmlSettings.GetLatestHostEntry(m_xmlSettings.GetLatestHostEntryCount()-1, lasthost);
        m_host = lasthost;
        Connect( STR_UTF8( m_host.szAddress.c_str() ), m_host.nTcpPort,
                 m_host.nUdpPort, m_host.bEncrypted);
    }

    CString szCtrlName;
    szCtrlName.LoadString(IDS_MASTERVOL);
    TRANSLATE_ITEM(IDS_MASTERVOL, szCtrlName);
    SetAccessibleName(m_wndVolSlider, szCtrlName);
    szCtrlName.LoadString(IDS_MICGAIN);
    TRANSLATE_ITEM(IDS_MICGAIN, szCtrlName);
    SetAccessibleName(m_wndGainSlider, szCtrlName);
    szCtrlName.LoadString(IDS_VOICELEVEL);
    TRANSLATE_ITEM(IDS_VOICELEVEL, szCtrlName);
    SetAccessibleName(m_wndVUProgress, szCtrlName);
    szCtrlName.LoadString(IDS_VOICEACTLEVEL);
    TRANSLATE_ITEM(IDS_VOICEACTLEVEL, szCtrlName);
    SetAccessibleName(m_wndVoiceSlider, szCtrlName);

    return TRUE;  // return TRUE  unless you set the focus to a control
}

void CTeamTalkDlg::SwitchFont()
{
    m_wndTree.SetFont(&m_Font);
    m_tabChat.m_wndRichEdit.SetFont(&m_Font);
    m_tabChat.m_wndChanMessage.SetFont(&m_Font);
}

void CTeamTalkDlg::OnUpdateStats(CCmdUI *pCmdUI)
{
    pCmdUI->SetText(m_szStatusBar);
}

void CTeamTalkDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
    if(nID == SC_MINIMIZE)
    {
        GetWindowRect(&m_rectLast);
    }

    CDialogExx::OnSysCommand(nID, lParam);

    if(nID == SC_MINIMIZE)
    {
        //tray setup
        if(m_xmlSettings.GetMinimizeToTray())
        {
            m_pTray = new CSystemTray();
            CString wintitle;
            GetWindowText(wintitle);
            if(!m_pTray->Create(0, WM_TRAY_MSG, wintitle, GetIcon(TRUE), IDR_MENU_TRAY))
            {
                AfxMessageBox(LoadText(IDS_FAILEDTOCREATETRAYICON, _T("Failed to create tray icon")));
                delete m_pTray;
                m_pTray = NULL;
            }
            else
                ShowWindow(SW_HIDE);
        }
        m_bMinimized = TRUE;
    }
    else
        if(nID == SC_RESTORE)
        {
            if(m_pTray)
            {
                delete m_pTray;
                m_pTray = NULL;
                ShowWindow(SW_SHOW);
            }
            m_bMinimized = FALSE;
        }
}

void CTeamTalkDlg::OnWindowRestore()
{
    if(m_pTray)
    {
        delete m_pTray;
        m_pTray = NULL;
    }

    ShowWindow(SW_RESTORE);
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CTeamTalkDlg::OnPaint() 
{
    if (IsIconic())
    {
        CPaintDC dc(this); // device context for painting

        SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

        // Center icon in client rectangle
        int cxIcon = GetSystemMetrics(SM_CXICON);
        int cyIcon = GetSystemMetrics(SM_CYICON);
        CRect rect;
        GetClientRect(&rect);
        int x = (rect.Width() - cxIcon + 1) / 2;
        int y = (rect.Height() - cyIcon + 1) / 2;

        // Draw the icon
        dc.DrawIcon(x, y, m_hIcon);
    }
    else
    {
        CDialogExx::OnPaint();
    }
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CTeamTalkDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}

// Automation servers should not exit when a user closes the UI
//  if a controller still holds on to one of its objects.  These
//  message handlers make sure that if the proxy is still in use,
//  then the UI is hidden but the dialog remains around if it
//  is dismissed.

void CTeamTalkDlg::Exit()
{
	Disconnect();

	//////////////////////
	// Store all settings
	//////////////////////

	//save output volume
	m_xmlSettings.SetSoundOutputVolume(m_wndVolSlider.GetPos());
	m_xmlSettings.SetVoiceActivationLevel(m_wndVoiceSlider.GetPos());
	m_xmlSettings.SetVoiceActivated((TT_GetFlags(ttInst) & CLIENT_SNDINPUT_VOICEACTIVATED) != CLIENT_CLOSED);
	m_xmlSettings.SetVoiceGainLevel(m_wndGainSlider.GetPos());
	m_xmlSettings.SetPushToTalk(m_bHotKey);

	//erase tray of minimized
	if (m_pTray)
	{
		delete m_pTray;
		m_pTray = NULL;
		ShowWindow(SW_SHOW);
	}

	//store window position
	if (m_bMinimized)
		VERIFY(m_xmlSettings.SetWindowPlacement(m_rectLast.left, m_rectLast.top, m_rectLast.Width(), m_rectLast.Height()));
	else
	{
		CRect rect, rectSplit;
		GetWindowRect(&rect);
		//m_wndSplitter.GetWindowRect(&rectSplit);
		//int nWidth = m_bChanMessages? rectSplit.left - rect.left : rect.Width();
		VERIFY(m_xmlSettings.SetWindowPlacement(rect.left, rect.top, rect.Width()/*nWidth*/, rect.Height()));
	}

	m_xmlSettings.SetWindowExtended(m_bTwoPanes);

	CloseLogFile(m_logChan);

	//Close TeamTalk DLLs
	TT_CloseTeamTalk(ttInst);
#if defined(ENABLE_TOLK)
	if (Tolk_IsLoaded()) {
		Tolk_Unload();
	}
#endif
	m_xmlSettings.SaveFile();

	if (m_bResetSettings)
	{
		CString szCfgPath = STR_UTF8(m_xmlSettings.GetFileName());
		CString szDefPath = GetExecutableFolder() + _T("\\") + _T(SETTINGS_DEFAULT_FILE);
		if (!CopyFile(szDefPath, szCfgPath, FALSE))
		{
			CString szMsg;
			szMsg.Format(LoadText(IDS_FAILEDTOCOPY, _T("Failed to copy %s to %s")), szDefPath, szCfgPath);
			MessageBox(szMsg, LoadText(IDS_RESETSETTINGS, _T("Reset Settings")));
		}
	}

    m_pPlaySndThread->AddSoundEvent(NULL);
    m_pPlaySndThread->KillThread();

	CDialog::OnCancel();
}
	
void CTeamTalkDlg::OnClose()
{
	if (m_xmlSettings.GetMinimizeToTray())
	{
		SendMessage(WM_SYSCOMMAND, SC_MINIMIZE);
		m_bMinimized = TRUE;
	}
	else
	{
		Exit();
	}
}

void CTeamTalkDlg::OnOK()
{
    if(GetFocus() == &m_tabChat.m_wndChanMessage && TT_GetMyChannelID(ttInst)>0) 
    {
        CString s;
        m_tabChat.m_wndChanMessage.GetWindowText(s);
        m_tabChat.m_wndChanMessage.SetWindowText(_T(""));
        if(!s.IsEmpty())
        {
            TextMessage msg = {};
            msg.nMsgType = MSGTYPE_CHANNEL;
            msg.nFromUserID = TT_GetMyUserID(ttInst);
            msg.nChannelID = TT_GetMyChannelID(ttInst);
            msg.nToUserID = 0;
            COPYTTSTR(msg.szMessage, s);
            TT_DoTextMessage( ttInst, &msg );
            m_tabChat.m_wndChanMessage.AddLastMessage(s);
        }
    }
    else if (GetFocus() == &m_wndTree)
    {
        int nUserID = m_wndTree.GetSelectedUser();
        if(nUserID)
        {
            OnUsersMessages();
        }
        int nChannelID = m_wndTree.GetSelectedChannel(false);
        if (nChannelID)
        {
            OnChannelsJoinchannel();
        }
    }
    //    if (CanExit())
    //        CDialogExx::OnOK();
}

void CTeamTalkDlg::OnCancel() 
{
}

void CTeamTalkDlg::OnSize(UINT nType, int cx, int cy)
{
    CDialogExx::OnSize(nType, cx, cy);

    ResizeItems();
}

void CTeamTalkDlg::ResizeItems()
{
    if(m_bResizeReady)
    {
        const int W = 5; //from picture right -> slider
        const int M = 5; //middle to wnd
        const int FR = 10; //from window right to slider right
        const int FL = 10; //from window left to picture left
        int width, height = 0;

        CRect mainRect;
        GetWindowRect(&mainRect);
        ScreenToClient(&mainRect);

        //status bar
        CStatusBar* bar = CDialogExx::GetStatusBar();
        ASSERT(bar);
        CRect client;
        GetClientRect(&client);
        CRect rectStatusBar;
        bar->GetWindowRect(&rectStatusBar);
        bar->MoveWindow(0,client.Height()-rectStatusBar.Height(), client.Width(), rectStatusBar.Height());

        //resize splitter
        CRect rectSplitter;
        m_wndSplitter.GetWindowRect(&rectSplitter);
        ScreenToClient(&rectSplitter); //splitter set last in resizing

        int nRight;
        if(m_bTwoPanes)
            nRight = rectSplitter.right;
        else
            nRight = mainRect.right;

        CRect rmainRect(mainRect); //rmainRect will always be the "real" rect

        const int TFB = 70; //from tree bottom to window bottom
        const int TFT = 25; //from tree top to window top

        //fix MyTree control
        CRect rectTree;
        m_wndTree.GetWindowRect(&rectTree);
        ScreenToClient(&rectTree);
        rectTree.left = mainRect.left+FL;
        rectTree.right = nRight-FR;
        rectTree.top = TFT;
        rectTree.bottom = mainRect.bottom-TFB;
        m_wndTree.MoveWindow(rectTree.left, rectTree.top, rectTree.Width(), rectTree.Height());

        //fix right side tab ctrl
        CRect rectTab;
        m_wndTabCtrl.GetWindowRect(&rectTab);
        ScreenToClient(&rectTab);
        width = rectTab.Width();
        rectTab.left = rectSplitter.right + FL;
        if(m_bTwoPanes)
            rectTab.right = rmainRect.right - FR;
        else
            rectTab.right = rectTab.left + width;
        rectTab.top = rectTree.top;
        rectTab.bottom = mainRect.bottom - rectStatusBar.Height() - 10;
        m_wndTabCtrl.MoveWindow(rectTab.left, rectTab.top, rectTab.Width(), rectTab.Height());

        const int R1FT = 5; //from tree bottom to first row
        //volume picture
        CRect rectVol;
        m_wndVolPic.GetWindowRect(&rectVol);
        ScreenToClient(&rectVol);
        width = rectVol.Width();
        height = rectVol.Height();
        rectVol.left=mainRect.left+FL; rectVol.right=rectVol.left+width;
        rectVol.top = rectTree.bottom+R1FT; rectVol.bottom = rectVol.top + height;
        m_wndVolPic.MoveWindow(rectVol.left,rectVol.top,rectVol.Width(),rectVol.Height());

        //volume slider
        CRect rectVolSlider;
        m_wndVolSlider.GetWindowRect(&rectVolSlider);
        ScreenToClient(&rectVolSlider);
        height = rectVolSlider.Height();
        rectVolSlider.left = rectVol.right+W; rectVolSlider.right = nRight/2-W; 
        rectVolSlider.top = rectTree.bottom+R1FT-1; rectVolSlider.bottom = rectVolSlider.top+height;
        m_wndVolSlider.MoveWindow(rectVolSlider.left,rectVolSlider.top,rectVolSlider.Width(),rectVolSlider.Height());

        //VU static
        CRect rectVU;
        m_wndVU.GetWindowRect(rectVU);
        ScreenToClient(&rectVU);
        width = rectVU.Width();
        height = rectVU.Height();
        rectVU.left = nRight/2+M; rectVU.right = rectVU.left+width; 
        rectVU.top = rectTree.bottom+R1FT; rectVU.bottom = rectVU.top + height;
        m_wndVU.MoveWindow(rectVU.left,rectVU.top,rectVU.Width(),rectVU.Height());

        //VuMeter progress
        CRect rectVuMeter;
        m_wndVUProgress.GetWindowRect(&rectVuMeter);
        ScreenToClient(&rectVuMeter);
        height = rectVuMeter.Height();
        rectVuMeter.left = rectVU.right+W; rectVuMeter.right = rectTree.right; 
        rectVuMeter.top = rectTree.bottom+R1FT; rectVuMeter.bottom = rectVuMeter.top+height;
        m_wndVUProgress.MoveWindow(rectVuMeter.left,rectVuMeter.top,rectVuMeter.Width(),rectVuMeter.Height());

        const int R2FT = R1FT+18;

        //voice gain
        CRect rectGain;
        m_wndMikePic.GetWindowRect(&rectGain);
        ScreenToClient(&rectGain);
        width = rectGain.Width();
        height = rectGain.Height();
        rectGain.left = mainRect.left+FL; rectGain.right = rectGain.left+width;
        rectGain.top = rectTree.bottom+R2FT; rectGain.bottom = rectGain.top + height;
        m_wndMikePic.MoveWindow(rectGain.left,rectGain.top,rectGain.Width(),rectGain.Height());

        //voice gain
        CRect rectGainSlider;
        m_wndGainSlider.GetWindowRect(&rectGainSlider);
        ScreenToClient(&rectGainSlider);
        height = rectGainSlider.Height();
        rectGainSlider.left = rectGain.right+W; rectGainSlider.right = nRight/2-W; 
        rectGainSlider.top = rectTree.bottom+R2FT-1; rectGainSlider.bottom = rectGainSlider.top+height;
        m_wndGainSlider.MoveWindow(rectGainSlider.left,rectGainSlider.top,rectGainSlider.Width(),rectGainSlider.Height());

        //voice act picture
        CRect rectVoi;
        m_wndVoicePic.GetWindowRect(&rectVoi);
        ScreenToClient(&rectVoi);
        width = rectVoi.Width();
        height = rectVoi.Height();
        rectVoi.left = nRight/2+M; rectVoi.right = rectVoi.left+width;
        rectVoi.top = rectTree.bottom+R2FT; rectVoi.bottom = rectVoi.top + height;
        m_wndVoicePic.MoveWindow(rectVoi.left,rectVoi.top,rectVoi.Width(),rectVoi.Height());

        //voice act slider
        CRect rectVoiSlider;
        m_wndVoiceSlider.GetWindowRect(&rectVoiSlider);
        ScreenToClient(&rectVoiSlider);
        height = rectVoiSlider.Height();
        rectVoiSlider.left = rectVoi.right+W; rectVoiSlider.right = rectTree.right; 
        rectVoiSlider.top = rectTree.bottom+R2FT; rectVoiSlider.bottom = rectVoiSlider.top + height;
        m_wndVoiceSlider.MoveWindow(rectVoiSlider.left,rectVoiSlider.top,rectVoiSlider.Width(),rectVoiSlider.Height());

        //splitter resizing
        rectSplitter.top = rectTree.top;
        rectSplitter.bottom = rectTree.bottom;
        if(m_bTwoPanes)
            m_wndSplitter.MoveWindow(rectSplitter.left, rectSplitter.top, rectSplitter.Width(), rectSplitter.Height());
        else
            m_wndSplitter.MoveWindow(nRight - rectSplitter.Width(), rectSplitter.top, rectSplitter.Width(), rectSplitter.Height());            
    }
}

LRESULT CTeamTalkDlg::OnSplitterMoved(WPARAM wParam, LPARAM lParam)
{
    ResizeItems();
    return TRUE;
}

HBRUSH CTeamTalkDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
    HBRUSH hbr;
    ASSERT(pWnd->GetDlgCtrlID() != AFX_IDW_TOOLBAR);
    if(nCtlColor == CTLCOLOR_STATIC && pWnd)
    {
        switch(pWnd->GetDlgCtrlID())
        {
        case IDC_STATIC_VU :
            //make static labels transparent
            pDC->SetBkMode(TRANSPARENT);
            hbr = (HBRUSH)GetStockObject( NULL_BRUSH );
            break;
        default :
            hbr = CDialogExx::OnCtlColor(pDC, pWnd, nCtlColor);
            //hbr = m_brush;
        }
    }
    else
    {
        //hbr = m_brush;
        hbr = CDialogExx::OnCtlColor(pDC, pWnd, nCtlColor);
    }

    return hbr;
}

/////////////////////////////////////////////////////
/// Menu item events
/////////////////////////////////////////////////////

void CTeamTalkDlg::OnFileHostmanager()
{
    //clear join channels
    m_host.szChannel.clear();
    m_host.szChPasswd.clear();

    if( (TT_GetFlags(ttInst) & CLIENT_CONNECTION) == 0)
    {
        CHostManagerDlg dlg(&m_xmlSettings, this);
        for(int i=m_xmlSettings.GetLatestHostEntryCount()-1; i>=0; i--)
        {
            HostEntry entry;
            m_xmlSettings.GetLatestHostEntry(i,entry);
            dlg.m_vecHosts.push_back(entry);
        }
        if(m_xmlSettings.GetLatestHostEntryCount()==0)
        {
            dlg.m_nTcpPort = DEFAULT_TEAMTALK_TCPPORT;
            dlg.m_nUdpPort = DEFAULT_TEAMTALK_UDPPORT;
        }
        if(dlg.DoModal() == IDOK)
        {
            if(TT_GetFlags(ttInst) & CLIENT_CONNECTION)
                Disconnect();

            m_host.szAddress = STR_UTF8( dlg.m_szHostAddress.GetBuffer() );
            m_host.nTcpPort = dlg.m_nTcpPort;
            m_host.nUdpPort = dlg.m_nUdpPort;
            m_host.bEncrypted = dlg.m_bEncrypted;
            m_host.szUsername = STR_UTF8(dlg.m_szUsername.GetBuffer());
            m_host.szPassword = STR_UTF8(dlg.m_szPassword.GetBuffer());
            m_host.szChannel = STR_UTF8(dlg.m_szChannel.GetBuffer());
            m_host.szChPasswd = STR_UTF8(dlg.m_szChPassword.GetBuffer());

            m_xmlSettings.RemoveLatestHostEntry(m_host);
            m_xmlSettings.AddLatestHostEntry(m_host);

            for(size_t i=0;i<dlg.m_delHosts.size();i++)
                m_xmlSettings.RemoveLatestHostEntry(dlg.m_delHosts[i]);

            //remove lastly used
            if(m_xmlSettings.GetLatestHostEntryCount()>10)
            {
                HostEntry tmp = m_host;
                m_xmlSettings.GetLatestHostEntry(0, tmp);
                m_xmlSettings.RemoveLatestHostEntry(tmp);
            }

            m_xmlSettings.SaveFile();

            Connect(dlg.m_szHostAddress, dlg.m_nTcpPort, dlg.m_nUdpPort, dlg.m_bEncrypted);
        }
    }
    else
    {
        if(m_nReconnectTimerID)
            KillTimer(m_nReconnectTimerID);
        m_nReconnectTimerID = 0;
        Disconnect();
        CString s;
        s.Format(LoadText(IDS_DISCONNECTEDFROM, _T("Disconnected from %s TCP port %d UDP port %d")), 
            STR_UTF8(m_host.szAddress.c_str()), m_host.nTcpPort, 
            m_host.nUdpPort);
        AddStatusText(s);
    }
}

void CTeamTalkDlg::OnUpdateFileConnect(CCmdUI *pCmdUI)
{
    CString szText;
    if( TT_GetFlags(ttInst) & CLIENT_CONNECTION )
    {
        szText.LoadString(IDS_DISCONNECT);
        TRANSLATE_ITEM(IDS_DISCONNECT, szText);
        if(pCmdUI->m_nIndex == 0)
            pCmdUI->SetCheck(TRUE);
    }
    else
    {
        szText.LoadString(IDS_CONNECT);
        TRANSLATE_ITEM(IDS_CONNECT, szText);
        if(pCmdUI->m_nIndex == 0)//a little hack
            pCmdUI->SetCheck(FALSE);
    }
    pCmdUI->SetText(szText);

}

void CTeamTalkDlg::OnFileConnect()
{
    OnFileHostmanager();
}

void CTeamTalkDlg::OnFilePreferences()
{
    //OnUserVideoCaptureFrame needs to know this due to deletion of frames
    m_bPreferencesOpen = TRUE;

    CString szTitle;
    szTitle.LoadString(IDS_PREFERENCES);
    TRANSLATE_ITEM(IDS_PREFERENCES, szTitle);
    CMyPropertySheet sheet(szTitle, this);
    sheet.m_psh.dwFlags |= PSH_NOAPPLYNOW;

    CGeneralPage generalpage;
    CClientPage clientpage;
    CShortcutsPage shortcutspage;
    CSoundSysPage soundpage;
    CTextToSpeechPage ttspage;
    CWindowPage windowpage;
    CSoundEventsPage eventspage;
    CVideoCapturePage videopage;
    CAdvancedPage advancedpage;

    /// translate
    CString szGeneral, szConnection, szShortcuts, szSound, szWindow, szEvents, szTTS, szVideo, szAdvanced;
    TRANSLATE_ITEM(IDD_PROPPAGE_GENERALPAGE, szGeneral);
    TRANSLATE_ITEM(IDD_PROPPAGE_CLIENTPAGE, szConnection);
    TRANSLATE_ITEM(IDD_PROPPAGE_SHORTCUTS, szShortcuts);
    TRANSLATE_ITEM(IDD_PROPPAGE_SOUNDSYSPAGE, szSound);
    TRANSLATE_ITEM(IDD_PROPPAGE_WINDOWPAGE, szWindow);
    TRANSLATE_ITEM(IDD_PROPPAGE_SOUNDEVENTSPAGE, szEvents);
    TRANSLATE_ITEM(IDD_PROPPAGE_TEXTTOSPEECHPAGE, szTTS);
    TRANSLATE_ITEM(IDD_PROPPAGE_VIDEOCAPTURE, szVideo);
    TRANSLATE_ITEM(IDD_PROPPAGE_ADVANCEDPAGE, szAdvanced);

    generalpage.m_psp.pszTitle = szGeneral;
    windowpage.m_psp.pszTitle = szWindow;
    clientpage.m_psp.pszTitle = szConnection;
    soundpage.m_psp.pszTitle = szSound;
    shortcutspage.m_psp.pszTitle = szShortcuts;
    eventspage.m_psp.pszTitle = szEvents;
    ttspage.m_psp.pszTitle = szTTS;
    videopage.m_psp.pszTitle = szVideo;
    advancedpage.m_psp.pszTitle = szAdvanced;

    if(szGeneral.GetLength())
        generalpage.m_psp.dwFlags |= PSP_USETITLE;
    if(szWindow.GetLength())
        windowpage.m_psp.dwFlags |= PSP_USETITLE;
    if(szConnection.GetLength())
        clientpage.m_psp.dwFlags |= PSP_USETITLE;
    if(szSound.GetLength())
        soundpage.m_psp.dwFlags |= PSP_USETITLE;
    if(szShortcuts.GetLength())
        shortcutspage.m_psp.dwFlags |= PSP_USETITLE;
    if(szEvents.GetLength())
        eventspage.m_psp.dwFlags |= PSP_USETITLE;
    if(szTTS.GetLength())
        ttspage.m_psp.dwFlags |= PSP_USETITLE;
    if(szVideo.GetLength())
        videopage.m_psp.dwFlags |= PSP_USETITLE;
    if(szAdvanced.GetLength())
        advancedpage.m_psp.dwFlags |= PSP_USETITLE;

    /////////////////////
    // general page
    /////////////////////
    HotKey hook;
    m_xmlSettings.GetPushToTalkKey(hook);
    std::string szBearWareID, szToken;
    m_xmlSettings.GetBearWareLogin(szBearWareID, szToken);

    generalpage.m_sNickname = STR_UTF8( m_xmlSettings.GetNickname(STR_UTF8(DEFAULT_NICKNAME)).c_str() );
    generalpage.m_szBearWareID = STR_UTF8(szBearWareID);
    generalpage.m_szBearWareToken = STR_UTF8(szToken);
    generalpage.m_bFemale = m_xmlSettings.GetGender(DEFAULT_GENDER) == GENDER_FEMALE;
    generalpage.m_bVoiceAct = m_xmlSettings.GetVoiceActivated();
    generalpage.m_bPush = m_bHotKey;
    generalpage.m_Hotkey = hook;
    generalpage.m_nInactivity = m_xmlSettings.GetInactivityDelay();
    generalpage.m_bIdleVox = m_xmlSettings.GetDisableVadOnIdle();

    ///////////////////////
    // window page
    ///////////////////////
    windowpage.m_bTray = m_xmlSettings.GetMinimizeToTray();
    windowpage.m_bStartMinimized = m_xmlSettings.GetStartMinimized();
    windowpage.m_bPopMsg = m_xmlSettings.GetPopupOnMessage();
    memset(&windowpage.m_lf, 0, sizeof(LOGFONT));
    string szFontName; int nSize; bool bBold = false, bUnderline = false, bItalic = false;
    if( m_xmlSettings.GetFont(szFontName, nSize, bBold, bUnderline, bItalic) ) 
    {
        windowpage.m_Font.szFaceName = STR_UTF8( szFontName.c_str() );
        windowpage.m_Font.bBold = bBold;
        windowpage.m_Font.nSize = nSize;
        windowpage.m_Font.bUnderline = bUnderline;
        windowpage.m_Font.bItalic = bItalic;
    }
    windowpage.m_bAlwaysOnTop = m_xmlSettings.GetAlwaysOnTop();    
    windowpage.m_bShowUserCount = m_xmlSettings.GetShowUserCount();
    windowpage.m_bDBClickJoin = m_xmlSettings.GetJoinDoubleClick();
    windowpage.m_bQuitClearChannels = m_xmlSettings.GetQuitClearChannels();
    windowpage.m_bTimeStamp = m_xmlSettings.GetMessageTimeStamp();
    windowpage.m_szLanguage = STR_UTF8( m_xmlSettings.GetLanguageFile().c_str() );
    windowpage.m_bVuMeter = m_xmlSettings.GetVuMeterUpdate();
    windowpage.m_bCheckUpdates = m_xmlSettings.GetCheckApplicationUpdates();
    windowpage.m_nTextLen = m_xmlSettings.GetMaxTextLength(DEFAULT_MAX_STRING_LENGTH);
    windowpage.m_bShowUsername = m_xmlSettings.GetShowUsernames();
    windowpage.m_nSorting = m_xmlSettings.GetSortOrder();

    ///////////////////////
    // client settings
    ///////////////////////
    clientpage.m_bAutoConnect = m_xmlSettings.GetAutoConnectToLastest();
    clientpage.m_bReconnect = m_xmlSettings.GetReconnectOnDropped();
    clientpage.m_bAutoJoinRoot = m_xmlSettings.GetAutoJoinRootChannel();

    int nSub = m_xmlSettings.GetDefaultSubscriptions();
    clientpage.m_bSubUserMsg = (nSub & SUBSCRIBE_USER_MSG) != SUBSCRIBE_NONE;
    clientpage.m_bSubChanMsg = (nSub & SUBSCRIBE_CHANNEL_MSG) != SUBSCRIBE_NONE;
    clientpage.m_bSubBcastMsg = (nSub & SUBSCRIBE_BROADCAST_MSG) != SUBSCRIBE_NONE;
    clientpage.m_bSubVoice = (nSub & SUBSCRIBE_VOICE) != SUBSCRIBE_NONE;
    clientpage.m_bSubVideo = (nSub & SUBSCRIBE_VIDEOCAPTURE) != SUBSCRIBE_NONE;
    clientpage.m_bSubDesktop = (nSub & SUBSCRIBE_DESKTOP) != SUBSCRIBE_NONE;
    clientpage.m_bSubMediaFile = (nSub & SUBSCRIBE_MEDIAFILE) != SUBSCRIBE_NONE;

    clientpage.m_nClientTcpPort = m_xmlSettings.GetClientTcpPort(0);
    clientpage.m_nClientUdpPort = m_xmlSettings.GetClientUdpPort(0);

    ////////////////////
    // sound output page
    ///////////////////
    soundpage.m_nOutputDevice = m_xmlSettings.GetSoundOutputDevice(UNDEFINED);
    soundpage.m_szOutputDeviceID = STR_UTF8(m_xmlSettings.GetSoundOutputDevice());
    soundpage.m_nInputDevice = m_xmlSettings.GetSoundInputDevice(UNDEFINED);
    soundpage.m_szInputDeviceID = STR_UTF8(m_xmlSettings.GetSoundInputDevice());
    soundpage.m_bPositioning = m_xmlSettings.GetAutoPositioning();
    soundpage.m_bEchoCancel = m_xmlSettings.GetEchoCancel(DEFAULT_ECHO_ENABLE);
    soundpage.m_bAGC = m_xmlSettings.GetAGC(DEFAULT_AGC_ENABLE);
    soundpage.m_bDenoise = m_xmlSettings.GetDenoise(DEFAULT_DENOISE_ENABLE);
    soundpage.m_nMediaVsVoice = m_xmlSettings.GetMediaStreamVsVoice(DEFAULT_MEDIA_VS_VOICE);

    ///////////////////////
    // sound events
    ///////////////////////
    eventspage.m_uSoundEvents = m_xmlSettings.GetEventSoundsEnabled(SOUNDEVENT_DEFAULT);
    eventspage.m_SoundFiles[SOUNDEVENT_USER_JOIN] = STR_UTF8( m_xmlSettings.GetEventNewUser().c_str() );
    eventspage.m_SoundFiles[SOUNDEVENT_USER_LEFT] = STR_UTF8( m_xmlSettings.GetEventRemovedUser().c_str() );
    eventspage.m_SoundFiles[SOUNDEVENT_USER_TEXTMSG] = STR_UTF8( m_xmlSettings.GetEventNewMessage().c_str() );
    eventspage.m_SoundFiles[SOUNDEVENT_USER_CHANNEL_TEXTMSG] = STR_UTF8(m_xmlSettings.GetEventChannelMsg().c_str());
    eventspage.m_SoundFiles[SOUNDEVENT_USER_BROADCAST_TEXTMSG] = STR_UTF8( m_xmlSettings.GetEventBroadcastMsg().c_str() );
    eventspage.m_SoundFiles[SOUNDEVENT_USER_QUESTIONMODE] = STR_UTF8( m_xmlSettings.GetEventQuestionMode().c_str());
    eventspage.m_SoundFiles[SOUNDEVENT_USER_DESKTOP_ACCESS] = STR_UTF8( m_xmlSettings.GetEventDesktopAccessReq().c_str());
    eventspage.m_SoundFiles[SOUNDEVENT_USER_VIDEOSESSION_NEW] = STR_UTF8( m_xmlSettings.GetEventVideoSession().c_str());
    eventspage.m_SoundFiles[SOUNDEVENT_USER_DESKTOPSESSION_NEW] = STR_UTF8( m_xmlSettings.GetEventDesktopSession().c_str());
    eventspage.m_SoundFiles[SOUNDEVENT_CONNECTION_LOST] = STR_UTF8( m_xmlSettings.GetEventServerLost().c_str() );
    eventspage.m_SoundFiles[SOUNDEVENT_PUSHTOTALK] = STR_UTF8( m_xmlSettings.GetEventHotKey().c_str() );
    eventspage.m_SoundFiles[SOUNDEVENT_FILES_UPDATED] = STR_UTF8( m_xmlSettings.GetEventFilesUpd().c_str());
    eventspage.m_SoundFiles[SOUNDEVENT_FILETX_COMPLETE] = STR_UTF8( m_xmlSettings.GetEventTransferEnd().c_str());
    eventspage.m_SoundFiles[SOUNDEVENT_CHANNEL_SILENT] = STR_UTF8(m_xmlSettings.GetEventChannelSilent().c_str());
    eventspage.m_SoundFiles[SOUNDEVENT_VOICEACTIVATED] = STR_UTF8(m_xmlSettings.GetEventVoiceActivated().c_str());
    eventspage.m_SoundFiles[SOUNDEVENT_VOICEDEACTIVATED] = STR_UTF8( m_xmlSettings.GetEventVoiceDeactivated().c_str() );
    eventspage.m_SoundFiles[SOUNDEVENT_ENABLE_VOICEACTIVATION] = STR_UTF8(m_xmlSettings.GetEventEnableVoiceActivation().c_str());
    eventspage.m_SoundFiles[SOUNDEVENT_DISABLE_VOICEACTIVATION] = STR_UTF8(m_xmlSettings.GetEventDisableVoiceActivation().c_str());
    eventspage.m_SoundFiles[SOUNDEVENT_ME_ENABLE_VOICEACTIVATION] = STR_UTF8(m_xmlSettings.GetEventMeEnableVoiceActivation().c_str());
    eventspage.m_SoundFiles[SOUNDEVENT_ME_DISABLE_VOICEACTIVATION] = STR_UTF8(m_xmlSettings.GetEventMeDisableVoiceActivation().c_str());
    eventspage.m_SoundFiles[SOUNDEVENT_TRANSMITQUEUE_HEAD] = STR_UTF8( m_xmlSettings.GetEventTransmitQueueHead().c_str() );
    eventspage.m_SoundFiles[SOUNDEVENT_TRANSMITQUEUE_STOP] = STR_UTF8( m_xmlSettings.GetEventTransmitQueueStop().c_str() );

    ////////////////////////
    // Text to Speech
    ////////////////////////
    ttspage.m_uTTSEvents = m_xmlSettings.GetEventTTSEvents();

    ////////////////////////
    // ShortCuts
    ////////////////////////
    m_xmlSettings.GetHotKeyVoiceAct(shortcutspage.m_hkVoiceAct);
    m_xmlSettings.GetHotKeyVolumePlus(shortcutspage.m_hkVolumePlus);
    m_xmlSettings.GetHotKeyVolumeMinus(shortcutspage.m_hkVolumeMinus);
    m_xmlSettings.GetHotKeyMuteAll(shortcutspage.m_hkMuteAll);
    m_xmlSettings.GetHotKeyVoiceGainPlus(shortcutspage.m_hkGainPlus);
    m_xmlSettings.GetHotKeyVoiceGainMinus(shortcutspage.m_hkGainMinus);
    m_xmlSettings.GetHotKeyMinRestore(shortcutspage.m_hkMinRestore);

    ///////////////////////
    // Video Capture
    ////////////////////
    videopage.m_szVidDevID = STR_UTF8(m_xmlSettings.GetVideoCaptureDevice().c_str());

    m_xmlSettings.GetVideoCaptureFormat(videopage.m_capformat);
    videopage.m_nCapFormatIndex = m_xmlSettings.GetVideoCaptureFormat(-1);
    videopage.m_nVidCodecBitrate = m_xmlSettings.GetVideoCodecBitrate(DEFAULT_WEBM_VP8_BITRATE);

    ////////////////////
    // advanced
    ////////////////////

    sheet.AddPage(&generalpage);
    sheet.AddPage(&windowpage);
    sheet.AddPage(&clientpage);
    sheet.AddPage(&soundpage);
    sheet.AddPage(&eventspage);
    sheet.AddPage(&ttspage);
    sheet.AddPage(&shortcutspage);
    sheet.AddPage(&videopage);
    //sheet.AddPage(&advancedpage);

    if(sheet.DoModal() == IDOK)
    {
        //change the username if connected
        if( STR_UTF8( m_xmlSettings.GetNickname(STR_UTF8(DEFAULT_NICKNAME)).c_str() ) != generalpage.m_sNickname.GetBuffer())
        {
            if( TT_GetFlags(ttInst) & CLIENT_AUTHORIZED )
            {
                TT_DoChangeNickname(ttInst, generalpage.m_sNickname);
            }
        }

        m_xmlSettings.SetBearWareLogin(STR_UTF8(generalpage.m_szBearWareID),
                                       STR_UTF8(generalpage.m_szBearWareToken));

        int nGender = (generalpage.m_bFemale?GENDER_FEMALE:GENDER_MALE);
        if(m_xmlSettings.GetGender() != nGender)
        {
            if(nGender == GENDER_FEMALE)
                m_nStatusMode |= STATUSMODE_FEMALE;
            else
                m_nStatusMode &= ~STATUSMODE_FEMALE;

            if( TT_GetFlags(ttInst) & CLIENT_AUTHORIZED )
                TT_DoChangeStatus(ttInst, m_nStatusMode, m_szAwayMessage);
        }

        //////////////////////////////////////////////////
        //    write settings for General Page to ini file
        //////////////////////////////////////////////////
        m_xmlSettings.SetNickname( STR_UTF8( generalpage.m_sNickname.GetBuffer()));
        m_xmlSettings.SetGender(nGender);
        m_xmlSettings.SetPushToTalk(generalpage.m_bPush);
        m_bHotKey = generalpage.m_bPush;
        HotKey hotkey = generalpage.m_Hotkey;
        m_xmlSettings.SetPushToTalkKey(hotkey);

        m_xmlSettings.SetVoiceActivated(generalpage.m_bVoiceAct);
        EnableVoiceActivation(generalpage.m_bVoiceAct);
        m_xmlSettings.SetInactivityDelay(generalpage.m_nInactivity);
        m_xmlSettings.SetDisableVadOnIdle(generalpage.m_bIdleVox);

        // hook is installed in CGeneralPage so make sure that it's not installed twice
        if(generalpage.m_bPush && hotkey.size())
        {
            TT_HotKey_Register(ttInst, HOTKEY_PUSHTOTALK_ID, &hotkey[0], INT32(hotkey.size()));
            m_szHotKey = CKeyCompDlg::GetHotkeyString(hotkey);
        }
        else
        {
            TT_HotKey_Unregister(ttInst, HOTKEY_PUSHTOTALK_ID);
            m_szHotKey.Empty();
        }

        ////////////////////////////////////////
        // write settings for window page
        ////////////////////////////////////////
        m_xmlSettings.SetMinimizeToTray(windowpage.m_bTray);
        m_xmlSettings.SetStartMinimized(windowpage.m_bStartMinimized);
        m_xmlSettings.SetPopupOnMessage(windowpage.m_bPopMsg);
        if( windowpage.m_Font.nSize>0)
        {
            string facename = STR_UTF8( windowpage.m_Font.szFaceName.GetBuffer() );
            m_xmlSettings.SetFont(facename.c_str(), windowpage.m_Font.nSize, 
                windowpage.m_Font.bBold, windowpage.m_Font.bUnderline,
                windowpage.m_Font.bItalic);
            m_Font.DeleteObject();
            LOGFONT lfont;
            MyFont font;
            font.szFaceName = windowpage.m_Font.szFaceName;
            font.nSize = windowpage.m_Font.nSize;
            font.bBold = windowpage.m_Font.bBold;
            font.bItalic = windowpage.m_Font.bItalic;
            font.bUnderline = windowpage.m_Font.bUnderline;
            ConvertFont( font, lfont);
            m_Font.CreateFontIndirect(&lfont);

            SwitchFont();
        }
        m_xmlSettings.SetAlwaysOnTop(windowpage.m_bAlwaysOnTop);
        if( windowpage.m_bAlwaysOnTop )
            SetWindowPos(&this->wndTopMost,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE);
        else
        {
            CRect rect;
            GetWindowRect(&rect);
            SetWindowPos(&this->wndNoTopMost,rect.left,rect.top,rect.Width(),rect.Height(),SWP_NOMOVE|SWP_NOSIZE);
        }
        m_xmlSettings.SetShowUserCount(windowpage.m_bShowUserCount);
        m_wndTree.ShowUserCount(windowpage.m_bShowUserCount);
        m_xmlSettings.SetShowUsernames(windowpage.m_bShowUsername);
        m_xmlSettings.SetJoinDoubleClick(windowpage.m_bDBClickJoin);
        m_xmlSettings.SetQuitClearChannels(windowpage.m_bQuitClearChannels);
        m_xmlSettings.SetMessageTimeStamp(windowpage.m_bTimeStamp);
        m_tabChat.m_wndRichEdit.m_bShowTimeStamp = windowpage.m_bTimeStamp;
        m_xmlSettings.SetLanguageFile( STR_UTF8( windowpage.m_szLanguage.GetBuffer() ) );
        if(!windowpage.m_szLanguage.IsEmpty())
            Languages::Instance( windowpage.m_szLanguage );
        else
            Languages::Instance()->ClearLanguage();

        TRANSLATE(*this, IDD);
        ASSERT(GetMenu());
        Translate();

        m_xmlSettings.SetVuMeterUpdate(windowpage.m_bVuMeter);
        if(windowpage.m_bVuMeter)
            SetTimer(TIMER_VOICELEVEL_ID, VUMETER_UPDATE_TIMEOUT, NULL);
        else
            KillTimer(TIMER_VOICELEVEL_ID);
        m_wndVUProgress.ShowWindow(m_xmlSettings.GetVuMeterUpdate()?SW_SHOW : SW_HIDE);

        m_wndTree.SetSortOrder((SortOrder)windowpage.m_nSorting);
        m_xmlSettings.SetCheckApplicationUpdates(windowpage.m_bCheckUpdates);
        m_xmlSettings.SetMaxTextLength(windowpage.m_nTextLen);
        if(nTextLimit != windowpage.m_nTextLen || bShowUsernames != windowpage.m_bShowUsername||
            m_xmlSettings.GetSortOrder() != windowpage.m_nSorting)
        {
            nTextLimit = windowpage.m_nTextLen;
            bShowUsernames = windowpage.m_bShowUsername;

            users_t users = m_wndTree.GetUsers();
            channels_t chans = m_wndTree.GetChannels();
            for(users_t::const_iterator i=users.begin();i!=users.end();i++)
                m_wndTree.UpdateUser(i->second);
            for(channels_t::const_iterator i=chans.begin();i!=chans.end();i++)
                m_wndTree.UpdateChannel(i->second);
        }
        m_xmlSettings.SetSortOrder(windowpage.m_nSorting);

        //////////////////////////////////////////////////
        //    write settings for Client Page to ini file
        //////////////////////////////////////////////////
        m_xmlSettings.SetAutoConnectToLastest(clientpage.m_bAutoConnect);
        m_xmlSettings.SetReconnectOnDropped(clientpage.m_bReconnect);
        m_xmlSettings.SetClientTcpPort(clientpage.m_nClientTcpPort);
        m_xmlSettings.SetClientUdpPort(clientpage.m_nClientUdpPort);
        m_xmlSettings.SetAutoJoinRootChannel(clientpage.m_bAutoJoinRoot);

        nSub = SUBSCRIBE_NONE;
        if(clientpage.m_bSubUserMsg)
            nSub |= SUBSCRIBE_USER_MSG;
        if(clientpage.m_bSubChanMsg)
            nSub |= SUBSCRIBE_CHANNEL_MSG;
        if(clientpage.m_bSubBcastMsg)
            nSub |= SUBSCRIBE_BROADCAST_MSG;
        if(clientpage.m_bSubVoice)
            nSub |= SUBSCRIBE_VOICE;
        if(clientpage.m_bSubVideo)
            nSub |= SUBSCRIBE_VIDEOCAPTURE;
        if(clientpage.m_bSubDesktop)
            nSub |= SUBSCRIBE_DESKTOP;
        if(clientpage.m_bSubMediaFile)
            nSub |= SUBSCRIBE_MEDIAFILE;
        m_xmlSettings.SetDefaultSubscriptions(nSub);

        ///////////////////////////////////////////////////
        //    write settings for Sound System
        //////////////////////////////////////////////////
        BOOL bRestart = FALSE;
        bRestart |= m_xmlSettings.GetSoundOutputDevice(UNDEFINED) != soundpage.m_nOutputDevice;
        bRestart |= m_xmlSettings.GetSoundInputDevice(UNDEFINED) != soundpage.m_nInputDevice;
        bRestart |= m_xmlSettings.GetAGC(DEFAULT_AGC_ENABLE) != (bool)soundpage.m_bAGC;
        bRestart |= m_xmlSettings.GetEchoCancel(DEFAULT_ECHO_ENABLE) != (bool)soundpage.m_bEchoCancel;
        bRestart |= m_xmlSettings.GetDenoise(DEFAULT_DENOISE_ENABLE) != (bool)soundpage.m_bDenoise;
        bRestart = bRestart && (TT_GetFlags(ttInst) & CLIENT_CONNECTION);

        m_xmlSettings.SetSoundOutputDevice(soundpage.m_nOutputDevice);
        m_xmlSettings.SetSoundOutputDevice(STR_UTF8(soundpage.m_szOutputDeviceID));
        m_xmlSettings.SetSoundInputDevice(soundpage.m_nInputDevice);
        m_xmlSettings.SetSoundInputDevice(STR_UTF8(soundpage.m_szInputDeviceID));
        m_xmlSettings.SetAutoPositioning(soundpage.m_bPositioning);
        m_xmlSettings.SetEchoCancel(soundpage.m_bEchoCancel);
        m_xmlSettings.SetAGC(soundpage.m_bAGC);
        m_xmlSettings.SetDenoise(soundpage.m_bDenoise);
        if(m_xmlSettings.GetMediaStreamVsVoice(DEFAULT_MEDIA_VS_VOICE) != soundpage.m_nMediaVsVoice)
        {
            m_xmlSettings.SetMediaStreamVsVoice(soundpage.m_nMediaVsVoice);
            users_t users = m_wndTree.GetUsers();
            users_t::const_iterator i;
            double mediaVol = soundpage.m_nMediaVsVoice;
            mediaVol /= 100.0;
            for(i=users.begin();i!=users.end();i++)
            {
                TT_SetUserVolume(ttInst, i->first, STREAMTYPE_MEDIAFILE_AUDIO,
                                    INT32(RefVolume(DEFAULT_SOUND_OUTPUT_VOLUME) * mediaVol));
            }
        }

        if(bRestart && soundpage.m_nInputDevice != UNDEFINED && soundpage.m_nOutputDevice != UNDEFINED)
        {
            if (!InitSoundSystem(m_xmlSettings))
            {
                MessageBox(LoadText(IDS_FAILEDTOINITIALIZENEWSD, _T("Failed to initialize new sound devices.")),
                    LoadText(IDS_RESTARTSS, _T("Restart Sound System")), MB_OK);
            }
        }

        UpdateAudioConfig();

        ////////////////////////////////////////
        //    write settings for events
        ////////////////////////////////////////
        m_xmlSettings.SetEventSoundsEnabled(eventspage.m_uSoundEvents);
        m_xmlSettings.SetEventNewUser(STR_UTF8(eventspage.m_SoundFiles[SOUNDEVENT_USER_JOIN]));
        m_xmlSettings.SetEventRemovedUser(STR_UTF8(eventspage.m_SoundFiles[SOUNDEVENT_USER_LEFT]));
        m_xmlSettings.SetEventNewMessage(STR_UTF8(eventspage.m_SoundFiles[SOUNDEVENT_USER_TEXTMSG]));
        m_xmlSettings.SetEventServerLost(STR_UTF8(eventspage.m_SoundFiles[SOUNDEVENT_CONNECTION_LOST]));
        m_xmlSettings.SetEventHotKey(STR_UTF8(eventspage.m_SoundFiles[SOUNDEVENT_PUSHTOTALK]));
        m_xmlSettings.SetEventChannelMsg(STR_UTF8(eventspage.m_SoundFiles[SOUNDEVENT_USER_CHANNEL_TEXTMSG]));
        m_xmlSettings.SetEventBroadcastMsg(STR_UTF8(eventspage.m_SoundFiles[SOUNDEVENT_USER_BROADCAST_TEXTMSG]));
        m_xmlSettings.SetEventChannelSilent(STR_UTF8(eventspage.m_SoundFiles[SOUNDEVENT_CHANNEL_SILENT]));
        m_xmlSettings.SetEventFilesUpd(STR_UTF8(eventspage.m_SoundFiles[SOUNDEVENT_FILES_UPDATED]));
        m_xmlSettings.SetEventTransferEnd(STR_UTF8(eventspage.m_SoundFiles[SOUNDEVENT_FILETX_COMPLETE]));
        m_xmlSettings.SetEventVideoSession(STR_UTF8(eventspage.m_SoundFiles[SOUNDEVENT_USER_VIDEOSESSION_NEW]));
        m_xmlSettings.SetEventDesktopSession(STR_UTF8(eventspage.m_SoundFiles[SOUNDEVENT_USER_DESKTOPSESSION_NEW]));
        m_xmlSettings.SetEventQuestionMode(STR_UTF8(eventspage.m_SoundFiles[SOUNDEVENT_USER_QUESTIONMODE]));
        m_xmlSettings.SetEventDesktopAccessReq(STR_UTF8(eventspage.m_SoundFiles[SOUNDEVENT_USER_DESKTOP_ACCESS]));
        m_xmlSettings.SetEventVoiceActivated(STR_UTF8(eventspage.m_SoundFiles[SOUNDEVENT_VOICEACTIVATED]));
        m_xmlSettings.SetEventVoiceDeactivated(STR_UTF8(eventspage.m_SoundFiles[SOUNDEVENT_VOICEDEACTIVATED]));
        m_xmlSettings.SetEventEnableVoiceActivation(STR_UTF8(eventspage.m_SoundFiles[SOUNDEVENT_ENABLE_VOICEACTIVATION]));
        m_xmlSettings.SetEventDisableVoiceActivation(STR_UTF8(eventspage.m_SoundFiles[SOUNDEVENT_DISABLE_VOICEACTIVATION]));
        m_xmlSettings.SetEventMeEnableVoiceActivation(STR_UTF8(eventspage.m_SoundFiles[SOUNDEVENT_ME_ENABLE_VOICEACTIVATION]));
        m_xmlSettings.SetEventMeDisableVoiceActivation(STR_UTF8(eventspage.m_SoundFiles[SOUNDEVENT_ME_DISABLE_VOICEACTIVATION]));
        m_xmlSettings.SetEventTransmitQueueHead(STR_UTF8(eventspage.m_SoundFiles[SOUNDEVENT_TRANSMITQUEUE_HEAD]));
        m_xmlSettings.SetEventTransmitQueueStop(STR_UTF8(eventspage.m_SoundFiles[SOUNDEVENT_TRANSMITQUEUE_STOP]));

        ///////////////////////////////////////
        // write settings for Text to speech
        ///////////////////////////////////////
        m_xmlSettings.SetEventTTSEvents(ttspage.m_uTTSEvents);
        EnableSpeech(ttspage.m_uTTSEvents != 0);

        ///////////////////////////////////////
        // write settings for shortcuts
        ///////////////////////////////////////
        m_xmlSettings.SetHotKeyVoiceAct(shortcutspage.m_hkVoiceAct);
        m_xmlSettings.SetHotKeyVolumePlus(shortcutspage.m_hkVolumePlus);
        m_xmlSettings.SetHotKeyVolumeMinus(shortcutspage.m_hkVolumeMinus);
        m_xmlSettings.SetHotKeyMuteAll(shortcutspage.m_hkMuteAll);
        m_xmlSettings.SetHotKeyVoiceGainPlus(shortcutspage.m_hkGainPlus);
        m_xmlSettings.SetHotKeyVoiceGainMinus(shortcutspage.m_hkGainMinus);
        m_xmlSettings.SetHotKeyMinRestore(shortcutspage.m_hkMinRestore);

        UpdateHotKeys();

        ////////////////////////////////////
        // write settings video capture 
        ////////////////////////////////////
        m_xmlSettings.SetVideoCaptureDevice(STR_UTF8(videopage.m_szVidDevID));
        m_xmlSettings.SetVideoCaptureFormat(videopage.m_nCapFormatIndex);
        m_xmlSettings.SetVideoCodecBitrate(videopage.m_nVidCodecBitrate);
        m_xmlSettings.SetVideoCaptureFormat(videopage.m_capformat);

        /////////////////////////////////////////
        //   write settings for Advanced
        //////////////////////////////////////////

        m_xmlSettings.SaveFile();
    }
    m_bPreferencesOpen = FALSE;
}

void CTeamTalkDlg::OnFileExit()
{
    Exit();
}

void CTeamTalkDlg::OnUpdateMeChangenick(CCmdUI *pCmdUI)
{
    pCmdUI->Enable( (TT_GetFlags(ttInst) & CLIENT_AUTHORIZED) != CLIENT_CLOSED);
}

void CTeamTalkDlg::OnMeChangenick()
{
    CString szChNick = LoadText(IDS_CHANGENN, _T("Change nickname")), szNewNick = LoadText(IDS_NEWNN, _T("New nickname"));
    TRANSLATE_ITEM(IDS_CHANGENICKNAME, szChNick);
    TRANSLATE_ITEM(IDS_NEWNICKNAME, szNewNick);

    CInputDlg dlg(szChNick, szNewNick, STR_UTF8( m_xmlSettings.GetNickname(STR_UTF8(DEFAULT_NICKNAME)).c_str() ), this);
    if(dlg.DoModal() == IDOK)
    {
        TT_DoChangeNickname(ttInst, dlg.GetInputString());
        VERIFY(m_xmlSettings.SetNickname(STR_UTF8( dlg.GetInputString().GetBuffer() )));
    }
}

void CTeamTalkDlg::OnUpdateMeChangestatus(CCmdUI *pCmdUI)
{
    pCmdUI->Enable((TT_GetFlags(ttInst) & CLIENT_AUTHORIZED) != CLIENT_CLOSED);
}

void CTeamTalkDlg::OnMeChangestatus()
{
    CChangeStatusDlg dlg;
    dlg.m_nStatusMode = m_nStatusMode;
    dlg.m_szAwayMessage = m_szAwayMessage;
    if(dlg.DoModal() == IDOK)
    {
        m_nStatusMode = dlg.m_nStatusMode;
        m_szAwayMessage = dlg.m_szAwayMessage;
        TT_DoChangeStatus(ttInst, m_nStatusMode, m_szAwayMessage);
    }
}

void CTeamTalkDlg::OnMeEnablehotkey()
{
    m_szHotKey.Empty();
    if(m_bHotKey)
    {
        TT_HotKey_Unregister(ttInst, HOTKEY_PUSHTOTALK_ID);
        VERIFY(m_xmlSettings.SetPushToTalk(false));
    }
    else
    {
        HotKey hotkey;
        m_xmlSettings.GetPushToTalkKey(hotkey);
        if( hotkey.size())
        {
            TT_HotKey_Register(ttInst, HOTKEY_PUSHTOTALK_ID, &hotkey[0], INT32(hotkey.size()));
        }
        else
        {
            if( AfxMessageBox(    LoadText(IDS_NOCOMBINAISONKEYPTT, _T("No push-to-talk key combination is currently configured.\r\nWould you like to configure it now?")), MB_YESNO) == IDYES)
            {
                CKeyCompDlg dlg;
                dlg.DoModal();
                hotkey = dlg.m_Hotkey;

                if(dlg.m_Hotkey.size() )
                {
                    VERIFY(m_xmlSettings.SetPushToTalkKey(dlg.m_Hotkey));
                    TT_HotKey_Register(ttInst, HOTKEY_PUSHTOTALK_ID, &dlg.m_Hotkey[0], INT32(dlg.m_Hotkey.size()));
                    m_xmlSettings.SetPushToTalkKey(dlg.m_Hotkey);
                }
            }
        }

        VERIFY(m_xmlSettings.SetPushToTalk(true));
        m_szHotKey = CKeyCompDlg::GetHotkeyString(hotkey);
    }
    m_bHotKey = m_xmlSettings.GetPushToTalk();
}

void CTeamTalkDlg::OnUpdateMeEnablehotkey(CCmdUI *pCmdUI)
{
    pCmdUI->SetCheck(m_bHotKey);
}

void CTeamTalkDlg::OnUpdateMeEnablevoiceactivation(CCmdUI *pCmdUI)
{
    pCmdUI->SetCheck((TT_GetFlags(ttInst) & CLIENT_SNDINPUT_VOICEACTIVATED) != CLIENT_CLOSED);
}

void CTeamTalkDlg::OnMeEnablevoiceactivation()
{
    BOOL bCurState = (TT_GetFlags(ttInst) & CLIENT_SNDINPUT_VOICEACTIVATED);
    EnableVoiceActivation(!bCurState, SOUNDEVENT_ME_ENABLE_VOICEACTIVATION,
                          SOUNDEVENT_ME_DISABLE_VOICEACTIVATION);
    m_xmlSettings.SetVoiceActivated((TT_GetFlags(ttInst) & CLIENT_SNDINPUT_VOICEACTIVATED) != CLIENT_CLOSED);
    m_wndTree.SetUserTalking(TT_GetMyUserID(ttInst), IsMyselfTalking());
}

void CTeamTalkDlg::OnUpdateUsersViewinfo(CCmdUI *pCmdUI)
{
    if(m_wndTree.GetSelectedUser()>0)
        pCmdUI->Enable(TRUE);
    else
        pCmdUI->Enable(FALSE);
}

void CTeamTalkDlg::OnUsersViewinfo()
{
    int nUserID = m_wndTree.GetSelectedUser();
    User user = {};
    if( m_wndTree.GetUser(nUserID, user) )
    {
        CUserInfoDlg dlg;
        dlg.m_nUserID = user.nUserID;
        dlg.m_szNick = GetDisplayName(user);

        TT_GetUser(ttInst, nUserID, &user);

        dlg.m_szUsername = user.szUsername;
        dlg.m_szUserType = (user.uUserType & USERTYPE_ADMIN)? LoadText(IDS_ADMIN, _T("Admin")): LoadText(IDS_DEFAULT, _T("Default"));
        if(TT_GetMyUserRights(ttInst) & USERRIGHT_BAN_USERS)
            dlg.m_szIPAddr = user.szIPAddress;
        dlg.m_szClientName = user.szClientName;
        dlg.m_szVersion = GetVersion(user);

        dlg.DoModal();
    }
}

void CTeamTalkDlg::OnUpdateUsersMessages(CCmdUI *pCmdUI)
{
    pCmdUI->Enable(m_wndTree.GetSelectedUser()>0? TRUE : FALSE);
}

void CTeamTalkDlg::OnUsersMessages()
{
    int nUserID = m_wndTree.GetSelectedUser();
    OnUsersMessages(nUserID);
}

void CTeamTalkDlg::OnUsersMessages(int nUserID)
{
    User user = { 0 };
    if(m_wndTree.GetUser(nUserID, user))
    {
        CMessageDlg* pMsgDlg = GetUsersMessageSession(user.nUserID, TRUE);
        User myself;
        if(pMsgDlg && m_wndTree.GetUser(TT_GetMyUserID(ttInst), myself))
        {
            pMsgDlg->m_bShowTimeStamp = m_xmlSettings.GetMessageTimeStamp();
            pMsgDlg->ShowWindow(SW_SHOW);
            ::PostMessage(pMsgDlg->m_hWnd, WM_SETFOCUS, 0, 0);
        }
    }
}

void CTeamTalkDlg::OnUpdateUsersMuteVoice(CCmdUI *pCmdUI)
{
    int nUserID = m_wndTree.GetSelectedUser();
    User user;
    if( TT_GetUser(ttInst, nUserID, &user) )
    {
        pCmdUI->Enable(TRUE);
        pCmdUI->SetCheck(user.uUserState & USERSTATE_MUTE_VOICE);
    }
    else
    {
        pCmdUI->Enable(FALSE);
    }
}

void CTeamTalkDlg::OnUsersMuteVoice()
{
    int nUserID = m_wndTree.GetSelectedUser();
    User user;
    if( TT_GetUser(ttInst, nUserID, &user) )
    {
        TT_SetUserMute(ttInst, nUserID, STREAMTYPE_VOICE,
                       !(user.uUserState & USERSTATE_MUTE_VOICE));
    }
}

void CTeamTalkDlg::OnUpdateUsersMuteMediafile(CCmdUI *pCmdUI)
{
    int nUserID = m_wndTree.GetSelectedUser();
    User user;
    if( TT_GetUser(ttInst, nUserID, &user) )
    {
        pCmdUI->Enable(TRUE);
        pCmdUI->SetCheck(user.uUserState & USERSTATE_MUTE_MEDIAFILE);
    }
    else
    {
        pCmdUI->Enable(FALSE);
    }
}

void CTeamTalkDlg::OnUsersMuteMediafile()
{
    int nUserID = m_wndTree.GetSelectedUser();
    User user;
    if(TT_GetUser(ttInst, nUserID, &user))
    {
        TT_SetUserMute(ttInst, nUserID, STREAMTYPE_MEDIAFILE_AUDIO,
                       !(user.uUserState & USERSTATE_MUTE_MEDIAFILE));
    }
}

void CTeamTalkDlg::OnUpdateUsersVolume(CCmdUI *pCmdUI)
{
    pCmdUI->Enable(m_wndTree.GetSelectedUser()>0);
}

void CTeamTalkDlg::OnUsersVolume()
{
    int nUserID = m_wndTree.GetSelectedUser();
    User user;
    if(TT_GetUser(ttInst, nUserID, &user) )
    {
        CUserVolumeDlg dlg(user,this);
        dlg.DoModal();
    }
}

void CTeamTalkDlg::OnUpdateUsersKickchannel(CCmdUI *pCmdUI)
{
    pCmdUI->Enable(m_wndTree.GetSelectedUser()>0? TRUE : FALSE);
}

void CTeamTalkDlg::OnUsersKickFromChannel()
{
    std::vector<User> users = m_wndTree.GetSelectedUsers();
    for(auto i=users.begin();i!=users.end();++i)
    {
        TT_DoKickUser(ttInst, i->nUserID, i->nChannelID);
    }
}

void CTeamTalkDlg::OnUpdateKickKickandbanfromchannel(CCmdUI *pCmdUI)
{
    pCmdUI->Enable(m_wndTree.GetSelectedUser()>0 ? TRUE : FALSE);
}

void CTeamTalkDlg::OnKickKickandbanfromchannel()
{
    CBanTypeDlg dlg;
    if(dlg.DoModal() == IDOK)
    {
        TT_DoBanUserEx(ttInst, m_wndTree.GetSelectedUser(), BANTYPE_CHANNEL | dlg.m_uBanTypes);
        TT_DoKickUser(ttInst, m_wndTree.GetSelectedUser(), m_wndTree.GetSelectedChannel(true));
    }
}

void CTeamTalkDlg::OnUpdateUsersKickfromserver(CCmdUI *pCmdUI)
{
    pCmdUI->Enable(m_wndTree.GetSelectedUser()>0? TRUE : FALSE);
}

void CTeamTalkDlg::OnUsersKickfromserver()
{
    std::vector<User> users = m_wndTree.GetSelectedUsers();
    for(auto i=users.begin();i!=users.end();++i)
    {
        TT_DoKickUser(ttInst, i->nUserID, 0);
    }
}

void CTeamTalkDlg::OnUpdateUsersKickandban(CCmdUI *pCmdUI)
{
    pCmdUI->Enable(m_wndTree.GetSelectedUser()>0);
}

void CTeamTalkDlg::OnUsersKickandban()
{
    int nUserID = m_wndTree.GetSelectedUser();

    CBanTypeDlg dlg;
    if(dlg.DoModal() == IDOK)
    {
        TT_DoBanUserEx(ttInst, nUserID, dlg.m_uBanTypes);
        TT_DoKickUser(ttInst, nUserID, 0);
    }
}

void CTeamTalkDlg::OnUpdateUsersOp(CCmdUI *pCmdUI)
{
    pCmdUI->Enable(m_wndTree.GetSelectedUser()>0? TRUE : FALSE);
}

void CTeamTalkDlg::OnUsersOp()
{
    std::vector<User> users = m_wndTree.GetSelectedUsers();

    for(auto i=users.begin();i!=users.end();++i)
    {
        BOOL bNewState = !TT_IsChannelOperator(ttInst, i->nUserID, i->nChannelID);
        if(TT_GetMyUserRights(ttInst) & USERRIGHT_OPERATOR_ENABLE)
        {
            TT_DoChannelOp(ttInst, i->nUserID, i->nChannelID, bNewState);
        }
        else
        {
            CString szTitle = LoadText(IDS_OPPSW, _T("Operator password"));
            TRANSLATE_ITEM(IDC_STATIC_OPPASSWD, szTitle);
            CInputDlg dlg(szTitle, LoadText(IDS_ENTERPSW, _T("Enter password")), _T(""), this);
            if(dlg.DoModal() == IDOK)
                TT_DoChannelOpEx(ttInst, i->nUserID, i->nChannelID, dlg.m_szInput, bNewState);
        }
    }
}


void CTeamTalkDlg::OnUpdateUsersMuteVoiceall(CCmdUI *pCmdUI)
{
    pCmdUI->Enable((TT_GetFlags(ttInst) & CLIENT_SNDOUTPUT_READY) != CLIENT_CLOSED);
    pCmdUI->SetCheck((TT_GetFlags(ttInst) & CLIENT_SNDOUTPUT_MUTE) != CLIENT_CLOSED);
}

void CTeamTalkDlg::OnUsersMuteVoiceall()
{
    TT_SetSoundOutputMute(ttInst, !(TT_GetFlags(ttInst) & CLIENT_SNDOUTPUT_MUTE));
}

void CTeamTalkDlg::OnUpdateUsersPositionusers(CCmdUI *pCmdUI)
{
    pCmdUI->Enable((TT_GetFlags(ttInst) & CLIENT_CONNECTED) != CLIENT_CLOSED);
}

void CTeamTalkDlg::OnUsersPositionusers()
{
    if((TT_GetFlags(ttInst) & CLIENT_SNDINOUTPUT_DUPLEX))
    {
        MessageBox(LoadText(IDS_DUPLEXNOTSUPPORTPOS, _T("Positioning users is not supported in sound duplex mode")),
        LoadText(IDS_POSUSERS, _T("Position Users")), MB_OK);
        return;
    }

    SoundDevice dev;
    if(GetSoundOutputDevice(m_xmlSettings, &dev) == UNDEFINED || dev.nSoundSystem != SOUNDSYSTEM_DSOUND)
    {
        MessageBox(LoadText(IDS_POSONLYDS, _T("Positioning users is only support with DirectSound")),
        LoadText(IDS_POSUSERS, _T("Position Users")), MB_OK);
        return;
    }

    CPositionUsersDlg dlg(m_wndTree.GetUsers(TT_GetMyChannelID(ttInst)), this);
    dlg.m_bPositionUsers = m_xmlSettings.GetAutoPositioning();
    if(dlg.DoModal())
    {
        m_xmlSettings.SetAutoPositioning(dlg.m_bPositionUsers);
        TT_Enable3DSoundPositioning(ttInst, dlg.m_bPositionUsers);
    }
}

void CTeamTalkDlg::OnUpdateAdvancedIncvolumevoice(CCmdUI *pCmdUI)
{
    User user;
    if(TT_GetUser(ttInst, m_wndTree.GetSelectedUser(), &user))
        pCmdUI->Enable(user.nVolumeVoice<SOUND_VOLUME_MAX);
    else
        pCmdUI->Enable(FALSE);
}

void CTeamTalkDlg::OnAdvancedIncvolumevoice()
{
    auto users = m_wndTree.GetSelectedUsers();
    for(auto i=users.begin();i!=users.end();++i)
    {
        User user;
        if(TT_GetUser(ttInst, i->nUserID, &user))
        {
            int v = RefVolumeToPercent(user.nVolumeVoice);
            TT_SetUserVolume(ttInst, i->nUserID, STREAMTYPE_VOICE, RefVolume(v + 1));
        }
    }
}

void CTeamTalkDlg::OnUpdateAdvancedLowervolumevoice(CCmdUI *pCmdUI)
{
    User user;
    if(TT_GetUser(ttInst, m_wndTree.GetSelectedUser(), &user))
        pCmdUI->Enable(user.nVolumeVoice>SOUND_VOLUME_MIN);
    else
        pCmdUI->Enable(FALSE);
}

void CTeamTalkDlg::OnAdvancedLowervolumevoice()
{
    auto users = m_wndTree.GetSelectedUsers();
    for(auto i=users.begin();i!=users.end();++i)
    {
        User user;
        if(TT_GetUser(ttInst, i->nUserID, &user))
        {
            int v = RefVolumeToPercent(user.nVolumeVoice);
            TT_SetUserVolume(ttInst, i->nUserID, STREAMTYPE_VOICE,
                RefVolume(v - 1));
        }
    }
}

void CTeamTalkDlg::OnUpdateAdvancedIncvolumemediafile(CCmdUI *pCmdUI)
{
    User user;
    if(TT_GetUser(ttInst, m_wndTree.GetSelectedUser(), &user))
        pCmdUI->Enable(user.nVolumeMediaFile<SOUND_VOLUME_MAX);
    else
        pCmdUI->Enable(FALSE);
}

void CTeamTalkDlg::OnAdvancedIncvolumemediafile()
{
    auto users = m_wndTree.GetSelectedUsers();
    for(auto i=users.begin();i!=users.end();++i)
    {
        User user;
        if(TT_GetUser(ttInst, i->nUserID, &user))
        {
            int v = RefVolumeToPercent(user.nVolumeMediaFile);
            TT_SetUserVolume(ttInst, i->nUserID, STREAMTYPE_MEDIAFILE_AUDIO,
                RefVolume(v + 1));
        }
    }
}

void CTeamTalkDlg::OnUpdateAdvancedLowervolumemediafile(CCmdUI *pCmdUI)
{
    User user;
    if(TT_GetUser(ttInst, m_wndTree.GetSelectedUser(), &user))
        pCmdUI->Enable(user.nVolumeMediaFile>SOUND_VOLUME_MIN);
    else
        pCmdUI->Enable(FALSE);
}

void CTeamTalkDlg::OnAdvancedLowervolumemediafile()
{
    auto users = m_wndTree.GetSelectedUsers();
    for(auto i=users.begin();i!=users.end();++i)
    {
        User user;
        if(TT_GetUser(ttInst, i->nUserID, &user))
        {
            int v = RefVolumeToPercent(user.nVolumeMediaFile);
            TT_SetUserVolume(ttInst, i->nUserID, STREAMTYPE_MEDIAFILE_AUDIO,
                RefVolume(v - 1));
        }
    }
}

void CTeamTalkDlg::OnUpdateChannelsCreatechannel(CCmdUI *pCmdUI)
{
    pCmdUI->Enable((TT_GetFlags(ttInst) & CLIENT_AUTHORIZED) != CLIENT_CLOSED);
}

void CTeamTalkDlg::OnChannelsCreatechannel()
{
    CChannelDlg dlg(CChannelDlg::CREATE_CHANNEL);
    ServerProperties prop = {};
    TT_GetServerProperties(ttInst, &prop);
    dlg.m_nMaxUsers = prop.nMaxUsers;
    dlg.m_bEnableAGC = DEFAULT_CHANNEL_AUDIOCONFIG;
    dlg.m_nGainLevel = DEFAULT_AGC_GAINLEVEL/1000;
    if(dlg.DoModal() == IDOK)
    {
        int nParentID = TT_GetMyChannelID(ttInst);
        BOOL bEnableChan = (TT_GetMyUserRights(ttInst) & USERRIGHT_MODIFY_CHANNELS) != USERRIGHT_NONE;
        if(bEnableChan && m_wndTree.GetSelectedChannel())
            nParentID = m_wndTree.GetSelectedChannel();
        else if(nParentID<=0)
            nParentID = TT_GetRootChannelID(ttInst);

        Channel chan = {};
        chan.nParentID = nParentID;
        COPYTTSTR(chan.szName, dlg.m_szChannelname);
        COPYTTSTR(chan.szPassword, dlg.m_szChannelPassword);
        COPYTTSTR(chan.szTopic, dlg.m_szChannelTopic);
        COPYTTSTR(chan.szOpPassword, dlg.m_szOpPasswd);
        chan.nMaxUsers = dlg.m_nMaxUsers;
        chan.nDiskQuota = dlg.m_nDiskQuota * 1024;
        if(dlg.m_bSingleTxChannel)
            chan.uChannelType |= CHANNEL_SOLO_TRANSMIT;
        if(dlg.m_bStaticChannel)
            chan.uChannelType |= CHANNEL_PERMANENT;
        if(dlg.m_bClassRoom)
            chan.uChannelType |= CHANNEL_CLASSROOM;
        if(dlg.m_bOpRecvOnly)
            chan.uChannelType |= CHANNEL_OPERATOR_RECVONLY;
        if(dlg.m_bNoVoiceAct)
            chan.uChannelType |= CHANNEL_NO_VOICEACTIVATION;
        if(dlg.m_bNoRecord)
            chan.uChannelType |= CHANNEL_NO_RECORDING;

        chan.audiocodec = dlg.m_codec;
        chan.audiocfg.bEnableAGC = dlg.m_bEnableAGC;
        if(dlg.m_bEnableAGC)
            chan.audiocfg.nGainLevel = dlg.m_nGainLevel*1000;
        //store the channel in case the user's connection is dropped
        TTCHAR szParentPath[TT_STRLEN] = _T("");
        TT_GetChannelPath(ttInst, nParentID, szParentPath);
        CString szPath = szParentPath + dlg.m_szChannelname;
        m_host.szChannel = STR_UTF8(szPath);
        m_host.szChPasswd = STR_UTF8(dlg.m_szChannelPassword);

        chan.transmitUsers[0][TT_CLASSROOM_USERID_INDEX] = TT_CLASSROOM_FREEFORALL;
        chan.transmitUsers[0][TT_CLASSROOM_STREAMTYPE_INDEX] = STREAMTYPE_CLASSROOM_ALL;

        if(bEnableChan)
            TT_DoMakeChannel(ttInst, &chan);
        else
        {
            int nCmdID = TT_DoJoinChannel(ttInst, &chan);
            m_commands[nCmdID] = CMD_COMPLETE_JOIN;
        }
    }
}


void CTeamTalkDlg::OnUpdateChannelsUpdatechannel(CCmdUI *pCmdUI)
{
    pCmdUI->Enable(m_wndTree.GetSelectedChannel()>0);
}

void CTeamTalkDlg::OnChannelsUpdatechannel()
{
    Channel chan = {};
    if(!TT_GetChannel(ttInst, m_wndTree.GetSelectedChannel(), &chan))
        return;

    CChannelDlg dlg(CChannelDlg::UPDATE_CHANNEL);
    dlg.m_szChannelname = chan.szName;
    dlg.m_szChannelPassword = chan.szPassword;
    dlg.m_szChannelTopic = chan.szTopic;
    dlg.m_szOpPasswd = chan.szOpPassword;
    dlg.m_nDiskQuota = chan.nDiskQuota / 1024;
    dlg.m_nMaxUsers = chan.nMaxUsers;
    dlg.m_bSingleTxChannel = (chan.uChannelType & CHANNEL_SOLO_TRANSMIT) != CHANNEL_DEFAULT;
    dlg.m_bStaticChannel = (chan.uChannelType & CHANNEL_PERMANENT) != CHANNEL_DEFAULT;
    dlg.m_bClassRoom = (chan.uChannelType & CHANNEL_CLASSROOM) != CHANNEL_DEFAULT;
    dlg.m_bOpRecvOnly = (chan.uChannelType & CHANNEL_OPERATOR_RECVONLY) != CHANNEL_DEFAULT;
    dlg.m_bNoVoiceAct = (chan.uChannelType & CHANNEL_NO_VOICEACTIVATION) != CHANNEL_DEFAULT;
    dlg.m_bNoRecord = (chan.uChannelType & CHANNEL_NO_RECORDING) != CHANNEL_DEFAULT;

    dlg.m_codec = chan.audiocodec;
    dlg.m_bEnableAGC = chan.audiocfg.bEnableAGC;
    dlg.m_nGainLevel = chan.audiocfg.nGainLevel / 1000;
    if(dlg.DoModal() == IDOK)
    {
        COPYTTSTR(chan.szName, dlg.m_szChannelname);
        COPYTTSTR(chan.szPassword, dlg.m_szChannelPassword);
        COPYTTSTR(chan.szTopic, dlg.m_szChannelTopic);
        COPYTTSTR(chan.szOpPassword, dlg.m_szOpPasswd);
        chan.nDiskQuota = dlg.m_nDiskQuota * 1024;
        chan.nMaxUsers = dlg.m_nMaxUsers;
        if(dlg.m_bSingleTxChannel)
            chan.uChannelType |= CHANNEL_SOLO_TRANSMIT;
        else
            chan.uChannelType &= ~CHANNEL_SOLO_TRANSMIT;
        if(dlg.m_bStaticChannel)
            chan.uChannelType |= CHANNEL_PERMANENT;
        else
            chan.uChannelType &= ~CHANNEL_PERMANENT;
        if(dlg.m_bClassRoom)
            chan.uChannelType |= CHANNEL_CLASSROOM;
        else
            chan.uChannelType &= ~CHANNEL_CLASSROOM;
        if(dlg.m_bOpRecvOnly)
            chan.uChannelType |= CHANNEL_OPERATOR_RECVONLY;
        else
            chan.uChannelType &= ~CHANNEL_OPERATOR_RECVONLY;
        if(dlg.m_bNoVoiceAct)
            chan.uChannelType |= CHANNEL_NO_VOICEACTIVATION;
        else
            chan.uChannelType &= ~CHANNEL_NO_VOICEACTIVATION;
        if(dlg.m_bNoRecord)
            chan.uChannelType |= CHANNEL_NO_RECORDING;
        else
            chan.uChannelType &= ~CHANNEL_NO_RECORDING;
        chan.audiocodec = dlg.m_codec;
        
        chan.audiocfg.bEnableAGC = dlg.m_bEnableAGC;
        chan.audiocfg.nGainLevel = dlg.m_nGainLevel * 1000;

        TT_DoUpdateChannel(ttInst, &chan);
    }
}

void CTeamTalkDlg::OnUpdateChannelsDeletechannel(CCmdUI *pCmdUI)
{
    pCmdUI->Enable(m_wndTree.GetSelectedChannel()>0);
}

void CTeamTalkDlg::OnChannelsDeletechannel()
{
    int channelid = m_wndTree.GetSelectedChannel();
    TTCHAR path[TT_STRLEN] = {};
    TT_GetChannelPath(ttInst, channelid, path);
    CString s;
    s.Format(LoadText(IDS_SUREDELETECHAN, _T("Are you sure you want to delete channel %s")), path);
    if(MessageBox(s, LoadText(IDS_CHANDELETETITLE, _T("Delete Channel")), MB_YESNO) == IDYES)
        TT_DoRemoveChannel(ttInst, channelid);
}


void CTeamTalkDlg::OnUpdateChannelsJoinchannel(CCmdUI *pCmdUI)
{
    if(m_wndTree.GetSelectedChannel()>0 && m_wndTree.GetSelectedChannel() != TT_GetMyChannelID(ttInst))
        pCmdUI->Enable(TRUE);
    else
        pCmdUI->Enable(FALSE);
}

void CTeamTalkDlg::OnChannelsJoinchannel()
{
    int nChannelID = m_wndTree.GetSelectedChannel();
    Channel chan;
    if(m_wndTree.GetChannel(nChannelID, chan) && nChannelID != TT_GetMyChannelID(ttInst))
    {
        TTCHAR szChannelPath[TT_STRLEN] = _T("");
        TT_GetChannelPath(ttInst, nChannelID, szChannelPath);
        if(chan.bPassword)
        {
            CInputDlg dlg(LoadText(IDS_CHANPSW, _T("Channel password")), LoadText(IDS_ENTERPSW, _T("Enter password")), _T(""), this);
            dlg.m_szInput = m_channelPasswords[nChannelID];
            if(dlg.DoModal() == IDOK)
            {
                CString password = dlg.GetInputString();
                m_channelPasswords[nChannelID] = password;

                //store the channel in case the user's connection is dropped
                m_host.szChannel = STR_UTF8(szChannelPath);
                m_host.szChPasswd = STR_UTF8(password);

                
                int nCmdID = TT_DoJoinChannelByID(ttInst, nChannelID, password);
                m_commands[nCmdID] = CMD_COMPLETE_JOIN;
            }
        }
        else
        {
            //store the channel in case the user's connection is dropped
            m_host.szChannel = STR_UTF8(szChannelPath);
            m_host.szChPasswd.clear();

            int nCmdID = TT_DoJoinChannelByID(ttInst, nChannelID, _T(""));
            m_commands[nCmdID] = CMD_COMPLETE_JOIN;
            TRACE(_T("Joining %s\r\n"), szChannelPath);
        }
    }
}

void CTeamTalkDlg::OnUpdateChannelsViewchannelinfo(CCmdUI *pCmdUI)
{
    pCmdUI->Enable(m_wndTree.GetSelectedChannel()>0? TRUE : FALSE);
}

void CTeamTalkDlg::OnChannelsViewchannelinfo()
{
    int nChannelID = m_wndTree.GetSelectedChannel();
    Channel chan;
    if(m_wndTree.GetChannel(nChannelID, chan))
    {
        CChannelDlg dlg(CChannelDlg::READONLY_CHANNEL, this);
        dlg.m_szChannelname = chan.szName;
        dlg.m_szChannelTopic = chan.szTopic;
        dlg.m_szChannelPassword = chan.szPassword;
        dlg.m_nDiskQuota = chan.nDiskQuota/1024;
        dlg.m_nMaxUsers = chan.nMaxUsers;
        dlg.m_bStaticChannel = (chan.uChannelType & CHANNEL_PERMANENT)?TRUE:FALSE;
        dlg.m_bSingleTxChannel = (chan.uChannelType & CHANNEL_SOLO_TRANSMIT)?TRUE:FALSE;
        dlg.m_bClassRoom = (chan.uChannelType & CHANNEL_CLASSROOM)?TRUE:FALSE;
        dlg.m_bOpRecvOnly = (chan.uChannelType & CHANNEL_OPERATOR_RECVONLY)?TRUE:FALSE;
        dlg.m_bNoVoiceAct = (chan.uChannelType & CHANNEL_NO_VOICEACTIVATION)?TRUE:FALSE;
        dlg.m_bNoRecord = (chan.uChannelType & CHANNEL_NO_RECORDING)?TRUE:FALSE;

        dlg.m_codec = chan.audiocodec;
        dlg.m_bEnableAGC = chan.audiocfg.bEnableAGC;
        dlg.m_nGainLevel = chan.audiocfg.nGainLevel / 1000;
        dlg.DoModal();
    }
}

void CTeamTalkDlg::OnUpdateChannelsViewchannelmessages(CCmdUI *pCmdUI)
{
    pCmdUI->Enable(TRUE);
    pCmdUI->SetCheck(m_bTwoPanes);
}

void CTeamTalkDlg::OnChannelsViewchannelmessages()
{
    if(m_bTwoPanes)//now cut in half
    {
        //hide right side windows
        m_wndTabCtrl.ShowWindow(SW_HIDE);

        CRect rectTree, rectMain;
        m_wndTree.GetWindowRect(rectTree);
        GetWindowRect(rectMain);

        int nLeft = rectTree.left - rectMain.left;
        int nNewWidth = rectTree.Width()+( 2 * nLeft);    //tree ctrl determines the new width

        m_bTwoPanes = !m_bTwoPanes;

        m_bIgnoreResize = TRUE;
        MoveWindow(rectMain.left, rectMain.top, nNewWidth, rectMain.Height());

        m_bIgnoreResize = FALSE;
    }
    else
    {
        //now show the right side with tab-ctrl
        CRect rectTree, rectMemo, rectMain;
        m_wndTree.GetWindowRect(rectTree);
        m_wndTabCtrl.GetWindowRect(rectMemo);
        GetWindowRect(rectMain);

        int nLeft = rectTree.left - rectMain.left;
        int nNewWidth = (rectMemo.right + nLeft) - rectMain.left;    //tabctrl determines the new width

        m_bTwoPanes = !m_bTwoPanes;

        m_bIgnoreResize = TRUE;
        MoveWindow(rectMain.left, rectMain.top, nNewWidth, rectMain.Height());

        m_bIgnoreResize = FALSE;
        m_wndTabCtrl.ShowWindow(SW_SHOW);

        //remove MsgIcon (if set)
        if(TT_GetMyChannelID(ttInst)>0)
            m_wndTree.SetChannelMessage(TT_GetMyChannelID(ttInst), FALSE);
    }

    PostMessage(WM_SIZE);
}

void CTeamTalkDlg::OnHelpAbout()
{
    CAboutBox dlg;
    dlg.m_szCompiled = LoadText(IDS_COMPILEDON, _T("Compiled on ")) + _T( __DATE__ ) _T(" ") _T( __TIME__ ) _T(".\r\n");
#if defined(UNICODE)
    dlg.m_szCompiled += LoadText(IDS_UNICODEENABLED, _T("Unicode -> UTF-8 conversion enabled.\r\n"));
#else
    dlg.m_szCompiled += LoadText(IDS_LOCALECHARS, _T("Uses local character set.\r\n"));
#endif
    if(sizeof(void*) == 8)
        dlg.m_szCompiled += LoadText(IDS_TTDLLVER, _T("TeamTalk 64-bit DLL version: ")) + CString(TT_GetVersion());
    else
        dlg.m_szCompiled += LoadText(IDS_TTDLLVERS, _T("TeamTalk 32-bit DLL version: ")) + CString(TT_GetVersion());
    dlg.DoModal();
}

void CTeamTalkDlg::OnHelpWebsite()
{
    HINSTANCE i = ShellExecute(this->m_hWnd,LoadText(IDS_SHELLEXECOPEN, _T("open")),WEBSITE,_T(""),_T(""),SW_SHOW);
}

void CTeamTalkDlg::OnHelpManual()
{
    HINSTANCE i = ShellExecute(this->m_hWnd,LoadText(IDS_SHELLEXECOPEN, _T("open")),MANUALFILE,_T(""),NULL,SW_SHOW);
}

void CTeamTalkDlg::OnTimer(UINT_PTR nIDEvent)
{
    CDialogExx::OnTimer(nIDEvent);
    switch(nIDEvent)
    {
    case TIMER_ONESECOND_ID :
        {
            ClientStatistics stats = {};
            if( (TT_GetFlags(ttInst) & CLIENT_CONNECTED) &&
                 TT_GetClientStatistics(ttInst, &stats))
            {
                float rx, tx;
                rx = (stats.nUdpBytesRecv-m_nLastRecvBytes)/1024.0f;
                tx = (stats.nUdpBytesSent-m_nLastSentBytes)/1024.0f;
                m_nLastRecvBytes = stats.nUdpBytesRecv;
                m_nLastSentBytes = stats.nUdpBytesSent;

                //display RX/TX as default in the statusbar
                if(!m_nStatusTimerID)
                {
                    CString szHotKey;
                    if(m_bHotKey && !m_szHotKey.IsEmpty())
                    {
                        CString szFmt;
                        szFmt.LoadString(IDS_PUSHTOTALK);
                        TRANSLATE_ITEM(IDS_PUSHTOTALK, szFmt);
                        szHotKey.Format(szFmt, m_szHotKey);
                    }

                    if(stats.nUdpPingTimeMs != -1)
                        m_szStatusBar.Format(LoadText(IDS_RXTXPING, _T("RX: %.2fKB TX: %.2fKB PING: %u %s")), 
                        rx, tx, stats.nUdpPingTimeMs, szHotKey);
                    else
                        m_szStatusBar.Format(LoadText(IDS_RXTX, _T("RX: %.2fKB TX: %.2fKB %s")), 
                        rx, tx, szHotKey);
                }
            }

            //check inactivity
            int nUserID = TT_GetMyUserID(ttInst);
            User user;
            DWORD dwDelay = m_xmlSettings.GetInactivityDelay() * 1000;
            if(m_wndTree.GetUser(nUserID, user) && dwDelay > 0)
            {
                if((::GetTickCount() - GetLastInput()) >= dwDelay)
                {
                    if((user.nStatusMode & STATUSMODE_MASK) != STATUSMODE_AWAY)
                    {
                        TT_DoChangeStatus(ttInst, STATUSMODE_AWAY, LoadText(IDS_AWAY, _T("Away")));
                        if(m_xmlSettings.GetDisableVadOnIdle() && m_xmlSettings.GetVoiceActivated())
                            EnableVoiceActivation(FALSE);
                        m_bIdledOut = TRUE;
                    }
                }
                else if(m_bIdledOut)
                {
                    TT_DoChangeStatus(ttInst, STATUSMODE_AVAILABLE, _T(""));
                    m_bIdledOut = FALSE;
                    if(m_xmlSettings.GetDisableVadOnIdle() && m_xmlSettings.GetVoiceActivated())
                        EnableVoiceActivation(TRUE);
                }
            }
            break;
        }
    case TIMER_VOICELEVEL_ID :
        //VU-Meter update
        m_wndVUProgress.SetPos(TT_GetSoundInputLevel(ttInst));
        break;
    case TIMER_CONNECT_TIMEOUT_ID :
        if(TT_GetFlags(ttInst) & CLIENT_CONNECTED)
        {
            CString szHost;
            KillTimer(TIMER_CONNECT_TIMEOUT_ID);
            Disconnect();

            CString szErr;
            szErr.Format(LoadText(IDS_FAILEDTOCONNECTLP, _T("Failed to connect to %s port %d sound port %d.\r\nCheck that your host settings are correct.")), 
                STR_UTF8(m_host.szAddress.c_str()), m_host.nTcpPort, 
                m_host.nUdpPort);
            AfxMessageBox(szErr, MB_OK | MB_ICONSTOP);
        }
        break;
    case TIMER_STATUSMSG_ID :
        {
            static int interval = 0;

            if(m_qStatusMsgs.empty())
                interval++;

            if(m_qStatusMsgs.empty() && (interval % 6) == 0)//last message will remain for 6 * timer delay
            {
                KillTimer(TIMER_STATUSMSG_ID);
                m_nStatusTimerID = 0;
                interval = 0;
            }
            else if(m_qStatusMsgs.size())
            {
                m_szStatusBar = m_qStatusMsgs.front();
                m_qStatusMsgs.pop();
            }
            break;
        }
    case TIMER_RECONNECT_ID :
        if(TT_GetFlags(ttInst) & CLIENT_CONNECTED)
        {
            KillTimer(TIMER_RECONNECT_ID);
            m_nReconnectTimerID = 0;
            break;
        }
        if((TT_GetFlags(ttInst) & CLIENT_CONNECTION) == 0)
            Connect( STR_UTF8( m_host.szAddress.c_str() ), m_host.nTcpPort, 
                    m_host.nUdpPort, m_host.bEncrypted);
        break;
    case TIMER_DESKTOPSHARE_ID:
        if((TT_GetFlags(ttInst) & CLIENT_TX_DESKTOP) == 0)
        {
            //only update desktop if there's users in the channel
            //(save bandwidth)
            Channel my_channel = {};
            TT_GetChannel(ttInst, TT_GetMyChannelID(ttInst), &my_channel);
            int user_cnt = 0;
            if(TT_GetChannelUsers(ttInst, TT_GetMyChannelID(ttInst),
                NULL, &user_cnt) && (user_cnt > 1))
            {
                if(SendDesktopWindow())
                    m_bSendDesktopOnCompletion = FALSE;
            }
        }
        else
            m_bSendDesktopOnCompletion = TRUE;
        break;
    case TIMER_HTTPREQUEST_APPUPDATE_UPDATE_ID :
        ASSERT(m_httpUpdate.get());
        if(!m_httpUpdate.get())
        {
            KillTimer(TIMER_HTTPREQUEST_APPUPDATE_UPDATE_ID);
            break;
        }
        if(m_httpUpdate->SendReady())
            m_httpUpdate->Send(_T("<") _T( TT_XML_ROOTNAME ) _T("/>"));
        else if(m_httpUpdate->ResponseReady())
        {
            CString szResponse = m_httpUpdate->GetResponse();
            string xml = STR_UTF8(szResponse, szResponse.GetLength()*4);
            teamtalk::XMLDocument xmlDoc(TT_XML_ROOTNAME, TEAMTALK_XML_VERSION);
            if(xmlDoc.Parse(xml))
            {
                CString updname = STR_UTF8(xmlDoc.GetValue(false, "teamtalk/update/name", "").c_str());
                if(!updname.IsEmpty())
                {
                    CString str;
                    str.Format(LoadText(IDS_NEWUPDATEAVAILABLE, _T("New update available: %s")), updname);
                    AddStatusText(str);
                }
                CString szRegisterUrl = STR_UTF8(xmlDoc.GetValue(false, "teamtalk/bearware/register-url", "").c_str());
                if (!szRegisterUrl.IsEmpty())
                    CBearWareLoginDlg::szBearWareRegisterUrl = szRegisterUrl;
            }

            KillTimer(TIMER_HTTPREQUEST_APPUPDATE_UPDATE_ID);
            KillTimer(TIMER_HTTPREQUEST_APPUPDATE_TIMEOUT_ID);
            m_httpUpdate.reset();
        }
        break;
    case TIMER_HTTPREQUEST_APPUPDATE_TIMEOUT_ID :
        KillTimer(TIMER_HTTPREQUEST_APPUPDATE_UPDATE_ID);
        KillTimer(TIMER_HTTPREQUEST_APPUPDATE_TIMEOUT_ID);
        m_httpUpdate.reset();
        break;
    case TIMER_APPUPDATE_ID :
        RunAppUpdate();
        break;
    case TIMER_HTTPREQUEST_WEBLOGIN_ID :
        if(m_httpWebLogin->SendReady())
            m_httpWebLogin->Send(_T("<") _T(TT_XML_ROOTNAME) _T("/>"));
        else if (m_httpWebLogin->ResponseReady())
        {
            CString szResponse = m_httpWebLogin->GetResponse();
            string xml = STR_UTF8(szResponse, szResponse.GetLength() * 4);
            teamtalk::XMLDocument xmlDoc(TT_XML_ROOTNAME, TEAMTALK_XML_VERSION);
            if(xmlDoc.Parse(xml))
            {
                m_host.szUsername = xmlDoc.GetValue(false, "teamtalk/bearware/username", "");
                m_host.szPassword = "token=" + xmlDoc.GetValue(false, "teamtalk/bearware/servertoken", "");

                KillTimer(TIMER_HTTPREQUEST_WEBLOGIN_ID);
                KillTimer(TIMER_HTTPREQUEST_WEBLOGIN_TIMEOUT_ID);
                m_httpWebLogin.reset();

                Login();
            }
        }
        break;
    case TIMER_HTTPREQUEST_WEBLOGIN_TIMEOUT_ID :
        KillTimer(TIMER_HTTPREQUEST_WEBLOGIN_ID);
        KillTimer(TIMER_HTTPREQUEST_WEBLOGIN_TIMEOUT_ID);
        m_httpWebLogin.reset();
        break;
    }
}

void CTeamTalkDlg::OnNMCustomdrawSliderVolume(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
    UpdateMasterVolume(m_wndVolSlider.GetPos());
    *pResult = 0;
}

LRESULT CTeamTalkDlg::OnTrayMessage(WPARAM wParam, LPARAM lParam)
{
    if(m_pTray)
        return m_pTray->OnTrayNotification(wParam, lParam);
    return TRUE;
}

void CTeamTalkDlg::OnShowWindow(BOOL bShow, UINT nStatus)
{
    CDialogExx::OnShowWindow(bShow, nStatus);
    if(!m_pTray)
        if(m_xmlSettings.GetStartMinimized())
            PostMessage(WM_SYSCOMMAND, SC_MINIMIZE);
}

void CTeamTalkDlg::OnEndSession(BOOL bEnding)
{
    OnClose();
    CDialogExx::OnEndSession(bEnding);    
}

void CTeamTalkDlg::OnNMCustomdrawSliderVoiceact(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
    TT_SetVoiceActivationLevel(ttInst, m_wndVoiceSlider.GetPos());
    *pResult = 0;
}

BOOL CTeamTalkDlg::OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pData) 
{
    // if size doesn't match we don't know what this is

    if( pData->cbData == sizeof( MsgCmdLine ) )
    {
        MsgCmdLine msg;
        memcpy( &msg, pData->lpData, sizeof( MsgCmdLine ) );

        // process message
        m_cmdArgs.RemoveAll();
        CString args = msg.szPath;
        int i = 0;
        CString token = args.Tokenize(_T("¤"), i);
        while(!token.IsEmpty())
        {
            m_cmdArgs.AddTail(token);
            token = args.Tokenize(_T("¤"), i);
        }

        ParseArgs();

        return TRUE;
    }

    return CDialogExx::OnCopyData(pWnd, pData);
}

/// load XML file containing host settings (.tt file)
LRESULT CTeamTalkDlg::OnTeamTalkFile(WPARAM wParam, LPARAM lParam)
{
    HostEntry tthost;

    TTFile tt(TT_XML_ROOTNAME);
    if(tt.LoadFile(STR_LOCAL( m_szTTLink )) && 
       VersionSameOrLater(STR_UTF8(tt.GetFileVersion()), _T("5.0")) &&
       !tt.HasErrors() && tt.GetHostEntry(tthost, 0))
    {
        m_host = tthost;

        CString szText, szCaption;
        szText.Format(LoadText(IDS_CLIENTSETTINGS), m_szTTLink, APPTITLE_SHORT);
        szCaption.Format(LoadText(IDS_LOADFILECLIENTSET, _T("Load %s File")), _T(TTFILE_EXT));
        if(tt.HasClientSetup() && MessageBox(szText, szCaption, MB_YESNO) == IDYES)
        {
            //override nickname if set in .tt file and not set in settings
            if(m_host.szNickname.size())
            {
                m_xmlSettings.SetNickname(m_host.szNickname);
            }

            if(m_host.nGender != GENDER_NONE)
            {
                m_xmlSettings.SetGender(m_host.nGender);
            }

            //override PTT-key if set in .tt file and not set in settings
            teamtalk::HotKey tt_hotkey;
            m_xmlSettings.GetPushToTalkKey(tt_hotkey);
            if(m_host.hotkey.size())
            {
                m_bHotKey = FALSE;
                m_xmlSettings.SetPushToTalkKey(m_host.hotkey);
                OnMeEnablehotkey(); //now enable hotkey
            }

            //voice act
            if(m_host.nVoiceAct>=0)
            {
                EnableVoiceActivation(m_host.nVoiceAct>0);
                m_xmlSettings.SetVoiceActivated(m_host.nVoiceAct>0);
            }

            //video capture
            if(IsValid(m_host.capformat))
            {
                m_xmlSettings.SetVideoCaptureFormat(m_host.capformat);
                TT_CloseVideoCaptureDevice(ttInst);
            }

            //video codec
            if(m_host.vidcodec.nCodec == WEBM_VP8_CODEC)
            {
                m_xmlSettings.SetVideoCodecBitrate(m_host.vidcodec.webm_vp8.nRcTargetBitrate);
                TT_CloseVideoCaptureDevice(ttInst);
            }
        }

        if(TT_GetFlags(ttInst) & CLIENT_CONNECTION)
        {
            if(AfxMessageBox(LoadText(IDS_DISCONNECTCURRENT, _T("Disconnect from current host?")), MB_YESNO) == IDNO)
            {
                m_szTTLink.Empty();
                return TRUE;
            }
            else
                Disconnect();
        }

        if(!Connect(STR_UTF8( m_host.szAddress.c_str() ), m_host.nTcpPort,
                    m_host.nUdpPort, m_host.bEncrypted))
        {
            CString s;
            s.Format(LoadText(IDS_FAILEDTOCONNECTCHECKHOSTVALID, _T("Failed to connect to %s, port %d, sound port %d\r\nPlease check that that host is valid")), STR_UTF8(m_host.szAddress.c_str()), m_host.nTcpPort, m_host.nUdpPort);
            AfxMessageBox(s);
        }

        m_xmlSettings.RemoveLatestHostEntry(m_host);
        m_xmlSettings.AddLatestHostEntry(m_host);
    }
    else if(!VersionSameOrLater(STR_UTF8(tt.GetFileVersion()), _T("5.0")))
    {
        CString szError;
        szError.Format(LoadText(IDS_TTFILEINCOMPATIBLE, _T("The file \"%s\" is incompatible with %s")), m_szTTLink, APPTITLE);
        MessageBox(szError, LoadText(IDS_MSGBOXTTFILELOADERROR, _T("Load File")), MB_ICONERROR);
    }
    else
    {
        CString s;
        s.Format(LoadText(IDS_TTFILENOTVALIDHOST, _T("The file %s\r\ndoes not contain a valid %s host entry.\r\nError message: %s")), m_szTTLink, APPNAME, STR_UTF8(tt.GetError().c_str()));
        AfxMessageBox(s);
    }

    m_szTTLink.Empty();

    return TRUE;
}

LRESULT CTeamTalkDlg::OnTeamTalkLink(WPARAM wParam, LPARAM lParam)
{
    ASSERT(m_szTTLink.GetLength());
    //this is a tt://ipaddress.com
    if(StartsWith(m_szTTLink, TTURL, FALSE))
    {
        if(TT_GetFlags(ttInst) & CLIENT_CONNECTION)
            if(AfxMessageBox(LoadText(IDS_DISCONNECTCURRENT, _T("Disconnect from current host?")), MB_YESNO) == IDNO)
            {
                m_szTTLink.Empty();
                return TRUE;
            }
            else
                Disconnect();

        CString szHostStr = m_szTTLink.Right(m_szTTLink.GetLength() - int(_tcslen(TTURL)));
        HostEntry entry;
        entry.szAddress = STR_UTF8(szHostStr);
        entry.nTcpPort = DEFAULT_TEAMTALK_TCPPORT;
        entry.nUdpPort = DEFAULT_TEAMTALK_UDPPORT;

        int i = szHostStr.FindOneOf(_T("/?"));
        if(i != -1)
        {
            szHostStr = szHostStr.Left(i);
            entry.szAddress = STR_UTF8(szHostStr);
            while(i < m_szTTLink.GetLength() && m_szTTLink[i] != '?')
                i++;
            if(i+1 < m_szTTLink.GetLength() && m_szTTLink[i] == '?')
            {
                CString szParams = m_szTTLink.Mid(i+1, m_szTTLink.GetLength() - i);
                CString szToken;

                i=0;
                while(i<szParams.GetLength())
                {
                    szToken = szParams.Tokenize(_T("&"), i);
                    int j=0;
                    CString szSubToken = szToken.Tokenize(_T("="), j);

                    if(szSubToken.CompareNoCase(_T("tcpport")) == 0)
                        entry.nTcpPort = _ttoi(szToken.Tokenize(_T("="), j));
                    else if(szSubToken.CompareNoCase(_T("udpport")) == 0)
                        entry.nUdpPort = _ttoi(szToken.Tokenize(_T("="), j));
                    else if(szSubToken.CompareNoCase(_T("channel")) == 0)
                        entry.szChannel = STR_UTF8( szToken.Tokenize(_T("="), j) );
                    else if(szSubToken.CompareNoCase(_T("chanpasswd")) == 0)
                        entry.szChPasswd = STR_UTF8( szToken.Tokenize(_T("="), j) );
                    else if(szSubToken.CompareNoCase(_T("username")) == 0)
                        entry.szUsername = STR_UTF8( szToken.Tokenize(_T("="), j) );
                    else if(szSubToken.CompareNoCase(_T("password")) == 0)
                        entry.szPassword = STR_UTF8( szToken.Tokenize(_T("="), j) );
                    else if(szSubToken.CompareNoCase(_T("encrypted")) == 0)
                    {
                        CString szValue = szToken.Tokenize(_T("="), j);
                        entry.bEncrypted = (szValue.CompareNoCase(_T("true")) == 0 || szValue == _T("1"));
                    }
                }
            }
        }

        m_host = entry;
        m_xmlSettings.RemoveLatestHostEntry(m_host);
        m_xmlSettings.AddLatestHostEntry(m_host);

        Connect(szHostStr, m_host.nTcpPort, m_host.nUdpPort, m_host.bEncrypted);    
    }
    else
    {
        CString szErr; szErr = LoadText(IDS_UNABLETOPARSELINKADDR, _T("Unable to parse link address:\r\n")) + m_szTTLink;
        AfxMessageBox(szErr);
    }

    m_szTTLink.Empty();

    return TRUE;
}

void CTeamTalkDlg::OnDropFiles(HDROP hDropInfo)
{
    //accepting a TT link
    DragQueryFile(hDropInfo, 0, m_szTTLink.GetBufferSetLength(MAX_PATH), MAX_PATH);
    PostMessage(WM_TEAMTALKDLG_TTFILE);

    CDialogExx::OnDropFiles(hDropInfo);
}

void CTeamTalkDlg::OnNMDblclkTreeSession(NMHDR *pNMHDR, LRESULT *pResult)
{
    if( m_wndTree.GetSelectedUser()>0 )
        OnUsersMessages();
    if( m_wndTree.GetSelectedChannel()>0 && m_xmlSettings.GetJoinDoubleClick())
        OnChannelsJoinchannel();
    *pResult = 0;
}

void CTeamTalkDlg::OnUpdateMeUsespeechonevents(CCmdUI *pCmdUI)
{
    pCmdUI->SetCheck(m_bSpeech);
}

void CTeamTalkDlg::OnMeUsespeechonevents()
{
    EnableSpeech(!m_bSpeech);
}

void CTeamTalkDlg::OnHelpRunwizard()
{
    if(TT_GetFlags(ttInst) & CLIENT_CONNECTION)
        AfxMessageBox(LoadText(IDS_DISCOBEFORERUN, _T("Please disconnect before running this wizard")));
    else
        RunWizard();
}

void CTeamTalkDlg::Translate()
{
    //an ugly hack to translate pop-up menu but popup menus doesn't have IDs :(
    ASSERT(GetMenu());
    CMenu& menu = *GetMenu();
    ASSERT(menu.GetMenuItemCount() == 6);

    CString szFile, szMe, szUsers, szChannels, szServer, szHelp,
        szAdvanced, szMute, szKick, szSubscriptions, szUserInfo,
        szChanInfo;

    szFile.LoadString(ID_FILE);
    szMe.LoadString(ID_ME);
    szUsers.LoadString(ID_USERS);
    szChannels.LoadString(ID_CHANNELS);
    szServer.LoadString(ID_SERVER);
    szHelp.LoadString(ID_HELP);

    szUserInfo.LoadString(ID_USERS_USERINFO);
    szAdvanced.LoadString(ID_ADVANCED);
    szMute.LoadString(ID_MUTE);
    szKick.LoadString(ID_KICK);
    szSubscriptions.LoadString(ID_SUBSCRIPTIONS);
    szChanInfo.LoadString(ID_CHANNELS_CHANNELINFO);

    TRANSLATE_ITEM(ID_FILE, szFile);
    TRANSLATE_ITEM(ID_ME, szMe);
    TRANSLATE_ITEM(ID_USERS, szUsers);
    TRANSLATE_ITEM(ID_CHANNELS, szChannels);
    TRANSLATE_ITEM(ID_SERVER, szServer);
    TRANSLATE_ITEM(IDS_IDHELP, szHelp);

    TRANSLATE_ITEM(ID_ADVANCED, szAdvanced);
    TRANSLATE_ITEM(ID_SUBSCRIPTIONS, szSubscriptions);
    TRANSLATE_ITEM(ID_KICK, szKick);
    TRANSLATE_ITEM(ID_MUTE, szMute);
    TRANSLATE_ITEM(ID_USERS_USERINFO, szUserInfo);

    TRANSLATE_ITEM(ID_CHANNELS_CHANNELINFO, szChanInfo);

    menu.ModifyMenu(0, MF_BYPOSITION | MF_STRING, 0, szFile);
    menu.ModifyMenu(1, MF_BYPOSITION | MF_STRING, 0, szMe);
    menu.ModifyMenu(2, MF_BYPOSITION | MF_STRING, 0, szUsers);
    menu.ModifyMenu(3, MF_BYPOSITION | MF_STRING, 0, szChannels);
    menu.ModifyMenu(4, MF_BYPOSITION | MF_STRING, 0, szServer);
    menu.ModifyMenu(5, MF_BYPOSITION | MF_STRING, 0, szHelp);

    ASSERT(menu.GetSubMenu(2));
    CMenu* sub = menu.GetSubMenu(2);

    sub->ModifyMenu(0, MF_BYPOSITION | MF_STRING, 0, szUserInfo);
    sub->ModifyMenu(5, MF_BYPOSITION | MF_STRING, 0, szMute);
    sub->ModifyMenu(6, MF_BYPOSITION | MF_STRING, 0, szKick);
    sub->ModifyMenu(7, MF_BYPOSITION | MF_STRING, 0, szSubscriptions);
    sub->ModifyMenu(8, MF_BYPOSITION | MF_STRING, 0, szAdvanced);

    sub = menu.GetSubMenu(3);
    sub->ModifyMenu(0, MF_BYPOSITION | MF_STRING, 0, szChanInfo);

    //redraw
    DrawMenuBar();

    //Translate tab ctrl
    TCITEM item = { 0 };
    item.mask = TCIF_TEXT;
    m_wndTabCtrl.GetItem(0, &item);
    CString szChat = _T("Chat");
    TRANSLATE_ITEM(IDS_CHAT, szChat);
    item.pszText = szChat.GetBuffer();
    m_wndTabCtrl.SetItem(0, &item);
    m_wndTabCtrl.GetItem(1, &item);
    CString szFiles = _T("Files");
    TRANSLATE_ITEM(IDS_FILES, szFiles);
    item.pszText = szFiles.GetBuffer();
    m_wndTabCtrl.SetItem(1, &item);
    Invalidate();
}

void CTeamTalkDlg::ParseArgs()
{
    m_szTTLink.Empty();
    for(POSITION pos=m_cmdArgs.GetHeadPosition();pos != NULL;)
    {
        CString orgArg = m_cmdArgs.GetNext(pos);
        CString arg = orgArg;
        arg.MakeLower();

        if(arg == _T("voiceact"))
            EnableVoiceActivation(TRUE);
        else if(arg == _T("novoiceact"))
            EnableVoiceActivation(FALSE);
        else if(arg == _T("wizard"))
            continue;
        else if(m_cmdArgs.Find(_T("fwadd"))>0)
        {
            FirewallInstall();
        }
        else if(m_cmdArgs.Find(_T("fwremove"))>0)
        {
            CString szPath;
            GetModuleFileName(NULL, szPath.GetBufferSetLength(MAX_PATH), MAX_PATH);
            if(TT_Firewall_AppExceptionExists(szPath))
            {
                TT_Firewall_RemoveAppException(szPath);
            }
        }
        else if(m_cmdArgs.Find(_T("tone"))>0)
            TT_DBG_SetSoundInputTone(ttInst, STREAMTYPE_VOICE, 440);
        else if(StartsWith(arg, TTURL, FALSE))
            m_szTTLink = orgArg;
        else if(arg == _T("cfg"))
        {
            if(pos != NULL)
                m_cmdArgs.GetNext(pos); //skip next
        }
        else
            m_szTTLink = orgArg;
    }

    //check whether a TTLink has been passed upon startup
    if(m_szTTLink.GetLength()>0)
    {
        if(StartsWith(m_szTTLink, TTURL, FALSE))
            PostMessage(WM_TEAMTALKDLG_TTLINK);
        else
            PostMessage(WM_TEAMTALKDLG_TTFILE);
    }
}

void CTeamTalkDlg::DefaultUnsubscribe(int nUserID)
{
    ServerProperties srvprop;
    ZERO_STRUCT(srvprop);
    TT_GetServerProperties(ttInst, &srvprop);

    int nSub = m_xmlSettings.GetDefaultSubscriptions();
    Subscriptions nUnsub = 0;
    if((SUBSCRIBE_USER_MSG & nSub) == SUBSCRIBE_NONE)
        nUnsub |= SUBSCRIBE_USER_MSG;
    if((SUBSCRIBE_CHANNEL_MSG & nSub) == SUBSCRIBE_NONE)
        nUnsub |= SUBSCRIBE_CHANNEL_MSG;
    if((SUBSCRIBE_BROADCAST_MSG & nSub) == SUBSCRIBE_NONE)
        nUnsub |= SUBSCRIBE_BROADCAST_MSG;
    if((SUBSCRIBE_VOICE & nSub) == SUBSCRIBE_NONE)
        nUnsub |= SUBSCRIBE_VOICE;
    if((SUBSCRIBE_VIDEOCAPTURE & nSub) == SUBSCRIBE_NONE)
        nUnsub |= SUBSCRIBE_VIDEOCAPTURE;
    if((SUBSCRIBE_DESKTOP & nSub) == SUBSCRIBE_NONE)
        nUnsub |= SUBSCRIBE_DESKTOP;
    if((SUBSCRIBE_MEDIAFILE & nSub) == SUBSCRIBE_NONE)
        nUnsub |= SUBSCRIBE_MEDIAFILE;

    if(nUnsub)
    {
        int nCmdID = TT_DoUnsubscribe(ttInst, nUserID, nUnsub);
        if(nCmdID>0)
            m_commands[nCmdID] = CMD_COMPLETE_UNSUBSCRIBE;
    }
}

void CTeamTalkDlg::SubscribeToggle(int nUserID, Subscription sub)
{
    User user;
    if(TT_GetUser(ttInst, nUserID, &user))
    {
        if(user.uLocalSubscriptions & sub)
        {
            int nCmdID = TT_DoUnsubscribe(ttInst, nUserID, sub);
            if(nCmdID>0)
                m_commands[nCmdID] = CMD_COMPLETE_UNSUBSCRIBE;
        }
        else
            TT_DoSubscribe(ttInst, nUserID, sub);
    }
}

void CTeamTalkDlg::SubscribeCommon(int nUserID, Subscription sub, BOOL bEnable)
{
    int nCmdID;
    if(bEnable)
    {
        nCmdID = TT_DoSubscribe(ttInst, nUserID, sub);
        if(nCmdID>0)
            m_commands[nCmdID] = CMD_COMPLETE_SUBSCRIBE;
    }
    else
    {
        nCmdID = TT_DoUnsubscribe(ttInst, nUserID, sub);
        if(nCmdID>0)
            m_commands[nCmdID] = CMD_COMPLETE_UNSUBSCRIBE;
    }
}

void CTeamTalkDlg::FirewallInstall()
{
    CString szPath;
    GetModuleFileName(NULL, szPath.GetBufferSetLength(MAX_PATH), MAX_PATH);
    if(!TT_Firewall_AppExceptionExists(szPath))
    {
        CString szText;
        szText.Format(LoadText(IDS_ADDTOFWEXCEP, _T("Add %s to Windows Firewall exceptions?")), APPNAME);
        int nAnswer = MessageBox(szText, APPNAME, MB_YESNO);
        if(nAnswer == IDYES && !TT_Firewall_AddAppException(APPNAME, szPath))
            MessageBox(LoadText(IDS_FWFAILEDTOADD, _T("Failed to add application to Windows Firewall exceptions.")));
    }
}

void CTeamTalkDlg::UpdateAudioStorage(BOOL bEnable)
{
    UINT uStorageMode = m_xmlSettings.GetAudioLogStorageMode();
    CString szAudioFolder = STR_UTF8(m_xmlSettings.GetAudioLogStorage());
    AudioFileFormat aff = (AudioFileFormat)m_xmlSettings.GetAudioLogStorageFormat();

    userids_t::const_iterator ite = m_users.begin();
    while(ite != m_users.end())
    {
        if(bEnable && (uStorageMode & AUDIOSTORAGE_SEPARATEFILES))
            TT_SetUserMediaStorageDir(ttInst, *ite, szAudioFolder, NULL, aff);
        else
            TT_SetUserMediaStorageDir(ttInst, *ite, _T(""), NULL, aff);
        ite++;
    }

    //Also include 'self'
    if(bEnable && (uStorageMode & AUDIOSTORAGE_SEPARATEFILES))
        TT_SetUserMediaStorageDir(ttInst, AUDIOSTORAGE_LOCAL_USERID, szAudioFolder, NULL, aff);
    else
        TT_SetUserMediaStorageDir(ttInst, AUDIOSTORAGE_LOCAL_USERID, _T(""), NULL, aff);

    TT_StopRecordingMuxedAudioFile(ttInst);
    if(bEnable && (uStorageMode & AUDIOSTORAGE_SINGLEFILE))
    {
        CString szAudioFile;
        CTime tm = CTime::GetCurrentTime();
        szAudioFile.Format(_T("%s\\%4d%02d%02d-%02d%02d%02d Conference"),
            szAudioFolder, tm.GetYear(), tm.GetMonth(), tm.GetDay(), 
            tm.GetHour(), tm.GetMinute(), tm.GetSecond());
        switch(aff)
        {
        case AFF_WAVE_FORMAT :
            szAudioFile += _T(".wav");
            break;
        case AFF_CHANNELCODEC_FORMAT :
            szAudioFile += _T(".ogg");
            break;
        case AFF_MP3_16KBIT_FORMAT :
        case AFF_MP3_32KBIT_FORMAT :
        case AFF_MP3_64KBIT_FORMAT :
        case AFF_MP3_128KBIT_FORMAT :
        case AFF_MP3_256KBIT_FORMAT :
            szAudioFile += _T(".mp3");
            break;
        }

        Channel chan;
        if(TT_GetChannel(ttInst, TT_GetMyChannelID(ttInst), &chan))
        {
            if(!TT_StartRecordingMuxedAudioFile(ttInst, &chan.audiocodec, 
                szAudioFile, aff))
            {
                MessageBox(LoadText(IDS_RECSTARTFAILED, _T("Failed to start recording")), LoadText(IDS_ERR, _T("Error")));
                return;
            }
            else
                AddStatusText(LoadText(IDS_RECORDINGFILE, _T("Recording to file: ")) + szAudioFile);
        }
    }
}

void CTeamTalkDlg::UpdateMasterVolume(int nVol)
{
    if(m_wndVolSlider.GetPos() != nVol)
        m_wndVolSlider.SetPos(nVol);
    TT_SetSoundOutputVolume(ttInst, RefVolume(nVol));
}

void CTeamTalkDlg::UpdateGainLevel(int nGain)
{
    if(m_wndGainSlider.GetPos() != nGain)
        m_wndGainSlider.SetPos(nGain);

    // m_wndGainSlider windows is disabled in AudioConfig-mode
    SpeexDSP spxdsp =  {};
    if(TT_GetSoundInputPreprocess(ttInst, &spxdsp) && spxdsp.bEnableAGC)
    {
        double percent = nGain;
        percent /= 100.;
        spxdsp.nGainLevel = INT32(SOUND_GAIN_MAX * percent);
        TT_SetSoundInputPreprocess(ttInst, &spxdsp);
        TT_SetSoundInputGainLevel(ttInst, SOUND_GAIN_DEFAULT);
    }
    else
    {
        int gain = RefGain(nGain);
        TT_SetSoundInputGainLevel(ttInst, gain);
    }
}

void CTeamTalkDlg::UpdateAudioConfig()
{
    SpeexDSP spxdsp = {};
    spxdsp.bEnableAGC = m_xmlSettings.GetAGC(DEFAULT_AGC_ENABLE);
    spxdsp.nGainLevel = DEFAULT_AGC_GAINLEVEL;
    spxdsp.nMaxIncDBSec = DEFAULT_AGC_INC_MAXDB;
    spxdsp.nMaxDecDBSec = DEFAULT_AGC_DEC_MAXDB;
    spxdsp.nMaxGainDB = DEFAULT_AGC_GAINMAXDB;
    spxdsp.bEnableDenoise = m_xmlSettings.GetDenoise(DEFAULT_DENOISE_ENABLE);
    spxdsp.nMaxNoiseSuppressDB = DEFAULT_DENOISE_SUPPRESS;
    spxdsp.bEnableEchoCancellation = m_xmlSettings.GetEchoCancel(DEFAULT_ECHO_ENABLE);
    spxdsp.nEchoSuppress = DEFAULT_ECHO_SUPPRESS;
    spxdsp.nEchoSuppressActive = DEFAULT_ECHO_SUPPRESSACTIVE;

    //if in a channel then let AGC settings override local settings
    Channel chan;
    if(m_wndTree.GetChannel(m_wndTree.GetMyChannelID(), chan) &&
       chan.audiocfg.bEnableAGC)
    {
        spxdsp.bEnableAGC = chan.audiocfg.bEnableAGC;
        spxdsp.nGainLevel = chan.audiocfg.nGainLevel;
        TT_SetSoundInputGainLevel(ttInst, SOUND_GAIN_DEFAULT);
        TT_SetSoundInputPreprocess(ttInst, &spxdsp);
    }
    else
    {
        // enable SpeexDSP if no AGC/AEC etc is enabled from sound device.
        SoundDeviceEffects effects = {};
        TT_GetSoundDeviceEffects(ttInst, &effects);
        if (!effects.bEnableAGC && !effects.bEnableDenoise && !effects.bEnableEchoCancellation)
            TT_SetSoundInputPreprocess(ttInst, &spxdsp);

        UpdateGainLevel(m_wndGainSlider.GetPos());
    }

    m_wndGainSlider.EnableWindow(!chan.audiocfg.bEnableAGC);

    TT_Enable3DSoundPositioning(ttInst, m_xmlSettings.GetAutoPositioning());
}

void CTeamTalkDlg::UpdateChannelLog()
{
    Channel chan;
    ZERO_STRUCT(chan);

    CloseLogFile(m_logChan);

    if(!m_wndTree.GetChannel(TT_GetMyChannelID(ttInst), chan))
        return;

    CString szPath = STR_UTF8(m_xmlSettings.GetChanTextLogStorage());
    if(szPath.GetLength())
    {
        if(!OpenLogFile(m_logChan, szPath, CString(chan.szName) + _T(".clog"), szPath))
        {
            CString szFormat;
            szFormat.Format(LoadText(IDS_FILEOPENFAILED, _T("Failed to open file: %s")), szPath);
            m_tabChat.m_wndRichEdit.AddLogMesage(szFormat);
        }
    }
}

HWND CTeamTalkDlg::GetSharedDesktopWindowHWND()
{
    HWND hWnd = NULL;

    switch(m_xmlSettings.GetDesktopShareMode())
    {
    default :
    case DESKTOPSHARE_DESKTOP :
        hWnd = TT_Windows_GetDesktopHWND();
        break;
    case DESKTOPSHARE_ACTIVE_WINDOW :
        hWnd = TT_Windows_GetDesktopActiveHWND();
        break;
    case DESKTOPSHARE_SPECIFIC_WINDOW :
        hWnd = m_hShareWnd;
        break;
    }
    return hWnd;
}

BOOL CTeamTalkDlg::SendDesktopWindow()
{
    HWND hWnd = GetSharedDesktopWindowHWND();

    BitmapFormat bmp_mode;
    if(m_xmlSettings.GetDesktopShareRgbMode() != UNDEFINED)
        bmp_mode = (BitmapFormat)m_xmlSettings.GetDesktopShareRgbMode();
    else
        bmp_mode = BMP_RGB16_555;

    int ret = TT_SendDesktopWindowFromHWND(ttInst, hWnd, bmp_mode, DESKTOPPROTOCOL_ZLIB_1);
    return ret >= 0;
}

void CTeamTalkDlg::RestartSendDesktopWindowTimer()
{
    KillTimer(TIMER_DESKTOPSHARE_ID);
    int nTimeout = m_xmlSettings.GetDesktopShareUpdateInterval();
    if(nTimeout == UNDEFINED)
        nTimeout = DEFAULT_SENDDESKTOPWINDOW_TIMEOUT;

    SetTimer(TIMER_DESKTOPSHARE_ID, nTimeout, NULL);
}

LRESULT CTeamTalkDlg::OnFileTransferDlgClosed(WPARAM wParam, LPARAM lParam)
{
    mtransferdlg_t::iterator ite = m_mTransfers.find(INT32(wParam));
    ASSERT(ite != m_mTransfers.end());
    if(ite != m_mTransfers.end())
        m_xmlSettings.SetCloseTransferDialog(ite->second->m_bAutoClose);
    m_mTransfers.erase(INT32(wParam));

    return TRUE;
}

void CTeamTalkDlg::OnUpdateChannelsUploadfile(CCmdUI *pCmdUI)
{
    pCmdUI->Enable(TT_GetMyChannelID(ttInst)>0);
}

void CTeamTalkDlg::OnChannelsUploadfile()
{
    int nChannelID = TT_GetMyChannelID(ttInst);
    if(nChannelID>0)
    {
        CString szWorkDir;
        GetCurrentDirectory(MAX_PATH, szWorkDir.GetBufferSetLength(MAX_PATH));

        CString filetypes = LoadText(IDS_ALLFILESTYPE, _T("All files (*.*)|*.*|"));
        CFileDialog dlg(TRUE, 0,0,OFN_FILEMUSTEXIST| OFN_HIDEREADONLY,filetypes, this);
        if(dlg.DoModal() == IDOK)
        {
            if(!TT_DoSendFile(ttInst, TT_GetMyChannelID(ttInst), dlg.GetPathName()))
                AfxMessageBox(LoadText(IDS_SENDFILEFAILED, _T("Failed to send file.")));
        }
        SetCurrentDirectory(szWorkDir);
    }
}

void CTeamTalkDlg::OnUpdateChannelsDownloadfile(CCmdUI *pCmdUI)
{
    pCmdUI->Enable(m_tabFiles.GetSelectedFiles().size()>0);
}

void CTeamTalkDlg::OnChannelsDownloadfile()
{
    int nChannelID = TT_GetMyChannelID(ttInst);
    std::vector<int> fileids = m_tabFiles.GetSelectedFiles();
    int nCount = 0;
    TT_GetChannelFiles(ttInst, nChannelID, NULL, &nCount);
    if(nChannelID>0 && fileids.size() && nCount>0)
    {
        std::vector<RemoteFile> remotefiles;
        remotefiles.resize(nCount);
        TT_GetChannelFiles(ttInst, nChannelID, &remotefiles[0], &nCount);
        for(size_t i=0;i<fileids.size();i++)
        {
            size_t j;
            for(j=0;j<remotefiles.size();j++)
            {
                if(remotefiles[j].nFileID == fileids[i])
                    break;
            }
            if(j==remotefiles.size())continue;

            CString szWorkDir;
            GetCurrentDirectory(MAX_PATH, szWorkDir.GetBufferSetLength(MAX_PATH));

            CString szFilters = LoadText(IDS_ALLFILESTYPE, _T("All Files (*.*)|*.*||"));
            CFileDialog fileDlg(FALSE, NULL, remotefiles[j].szFileName, OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY, szFilters, this);
            if(fileDlg.DoModal() == IDOK)
            {
                if(!TT_DoRecvFile(ttInst, TT_GetMyChannelID(ttInst), fileids[i], fileDlg.GetPathName()))
                    AfxMessageBox(LoadText(IDS_DLFILEFAILED, _T("Failed to download file.")));
            }
            SetCurrentDirectory(szWorkDir);
        }
    }
}

void CTeamTalkDlg::OnUpdateChannelsDeletefile(CCmdUI *pCmdUI)
{
    pCmdUI->Enable(m_tabFiles.GetSelectedFiles().size()>0);
}

void CTeamTalkDlg::OnChannelsDeletefile()
{
    int nChannelID = m_wndTree.GetMyChannelID();
    std::vector<int> fileids = m_tabFiles.GetSelectedFiles();
    for(size_t i=0;i<fileids.size();i++)
        TT_DoDeleteFile(ttInst, TT_GetMyChannelID(ttInst), fileids[i]);
}

LRESULT CTeamTalkDlg::OnFilesDropped(WPARAM wParam, LPARAM lParam)
{
    if(m_tabFiles.m_wndFiles.m_Files.GetCount())
    {
        CString szFileName = m_tabFiles.m_wndFiles.m_Files.GetHead();
        int nChannelID = TT_GetMyChannelID(ttInst);
        if(nChannelID>0)
        {
            if(!TT_DoSendFile(ttInst, TT_GetMyChannelID(ttInst), szFileName))
                AfxMessageBox(LoadText(IDS_SENDFILEFAILED, _T("Failed to send file.")));
        }
    }
    return TRUE;
}

LRESULT CTeamTalkDlg::OnMoveUser(WPARAM wParam, LPARAM lParam)
{
    TT_DoMoveUser(ttInst, INT32(wParam), INT32(lParam));
    return TRUE;
}

LRESULT CTeamTalkDlg::OnVideoDlgClosed(WPARAM wParam, LPARAM lParam)
{
    ASSERT(wParam & VIDEOTYPE_MASK);
    int nUserID = INT32(wParam);
    //userid = 0 is local video (used in Testing video device)
    if(nUserID>0)
    {
        CloseVideoSession(nUserID);
        Subscriptions subs = SUBSCRIBE_NONE;
        if(nUserID & VIDEOTYPE_CAPTURE)
            subs |= SUBSCRIBE_VIDEOCAPTURE;
        if(nUserID & VIDEOTYPE_MEDIAFILE)
            subs |= SUBSCRIBE_MEDIAFILE;

        int nCmdID = TT_DoUnsubscribe(ttInst, nUserID & VIDEOTYPE_USERMASK, subs);
        if(nCmdID>0)
            m_commands[nCmdID] = CMD_COMPLETE_UNSUBSCRIBE;
        m_videoignore.insert(nUserID);
    }

    return TRUE;
}

LRESULT CTeamTalkDlg::OnVideoDlgEnded(WPARAM wParam, LPARAM lParam)
{
    if(wParam>0)
    {
        CloseVideoSession(INT32(wParam));
    }

    return TRUE;
}

LRESULT CTeamTalkDlg::OnDesktopDlgClosed(WPARAM wParam, LPARAM lParam)
{
    CloseDesktopSession(INT32(wParam));
    ServerProperties prop = {};

    TT_GetServerProperties(ttInst, &prop);

    int nCmdID = TT_DoUnsubscribe(ttInst, INT32(wParam), SUBSCRIBE_DESKTOP);
    if(nCmdID>0)
        m_commands[nCmdID] = CMD_COMPLETE_UNSUBSCRIBE;

    m_desktopignore.insert(INT32(wParam));

    return TRUE;
}

LRESULT CTeamTalkDlg::OnDesktopDlgEnded(WPARAM wParam, LPARAM lParam)
{
    CloseDesktopSession(INT32(wParam));
    return TRUE;
}

LRESULT CTeamTalkDlg::OnOnlineUsersDlgClosed(WPARAM wParam, LPARAM lParam)
{
    m_pOnlineUsersDlg = nullptr;
    return TRUE;
}

void CTeamTalkDlg::OnUpdateChannelsLeavechannel(CCmdUI *pCmdUI)
{
    pCmdUI->Enable(TT_GetMyChannelID(ttInst)>0);
}

void CTeamTalkDlg::OnChannelsLeavechannel()
{
    int nChannelID = m_wndTree.GetSelectedChannel();
    TT_DoLeaveChannel(ttInst);
}

void CTeamTalkDlg::OnUpdateChannelsStreamMediaFileToChannel(CCmdUI *pCmdUI)
{
    ClientFlags flags = TT_GetFlags(ttInst);
    BOOL bChecked = flags & (CLIENT_STREAM_AUDIO | CLIENT_STREAM_VIDEO);
    pCmdUI->SetCheck(bChecked?BST_CHECKED:BST_UNCHECKED);
}

void CTeamTalkDlg::OnChannelsStreamMediaFileToChannel()
{
    ClientFlags flags = TT_GetFlags(ttInst);
    if(flags & (CLIENT_STREAM_AUDIO | CLIENT_STREAM_VIDEO))
        StopMediaStream();
    else
    {
        m_pStreamMediaDlg.reset(new CStreamMediaDlg(m_xmlSettings, this));
        auto files = m_xmlSettings.GetLastMediaFiles();
        for (auto a : files)
            m_pStreamMediaDlg->m_fileList.AddTail(STR_UTF8(a));

        if (m_pStreamMediaDlg->DoModal() == IDOK)
        {
            files.clear();
            for(POSITION pos=m_pStreamMediaDlg->m_fileList.GetHeadPosition();pos != nullptr;)
                files.push_back(STR_UTF8(m_pStreamMediaDlg->m_fileList.GetNext(pos)));
            m_xmlSettings.SetLastMediaFiles(files);

            StartMediaStream();
        }
        m_pStreamMediaDlg.reset();
    }
}

void CTeamTalkDlg::OnUpdateServerServerproperties(CCmdUI *pCmdUI)
{
    pCmdUI->Enable((TT_GetFlags(ttInst) & CLIENT_AUTHORIZED) != CLIENT_CLOSED);
}

void CTeamTalkDlg::OnServerServerproperties()
{
    ServerProperties prop = {};
    if(!TT_GetServerProperties(ttInst, &prop))
        return;

    BOOL bReadOnly = (TT_GetMyUserRights(ttInst) & USERRIGHT_UPDATE_SERVERPROPERTIES) == 0;
    CServerPropertiesDlg dlg(bReadOnly, this);
    dlg.m_szSrvName = prop.szServerName;
    dlg.m_szMOTD = prop.szMOTD;
    dlg.m_szMOTDRaw = prop.szMOTDRaw;
    dlg.m_bAutoSave = prop.bAutoSave;
    dlg.m_nMaxUsers = prop.nMaxUsers;
    dlg.m_nTcpPort = prop.nTcpPort;
    dlg.m_nUdpPort = prop.nUdpPort;
    dlg.m_nUserTimeout = prop.nUserTimeout;
    dlg.m_nAudioTx = prop.nMaxVoiceTxPerSecond / 1024;
    dlg.m_nVideoTx = prop.nMaxVideoCaptureTxPerSecond / 1024;
    dlg.m_nTotalTx = prop.nMaxTotalTxPerSecond / 1024;
    dlg.m_nMediaFileTx = prop.nMaxMediaFileTxPerSecond / 1024;
    dlg.m_nDesktopTxMax = prop.nMaxDesktopTxPerSecond / 1024;
    dlg.m_nLoginsBan = prop.nMaxLoginAttempts;
    dlg.m_nMaxIPLogins = prop.nMaxLoginsPerIPAddress;
    dlg.m_szVersion = prop.szServerVersion;
    if(dlg.DoModal() == IDOK && !bReadOnly)
    {
        COPYTTSTR(prop.szServerName, dlg.m_szSrvName);
        //if(dlg.m_szMOTD != prop.szMOTDRaw &&
        //   MessageBox(_T("Update message of the day?"),
        //              _T("Message of the day"), MB_YESNO) == IDYES)
        COPYTTSTR(prop.szMOTDRaw, dlg.m_szMOTD);
        prop.nMaxUsers = dlg.m_nMaxUsers;
        prop.nTcpPort = dlg.m_nTcpPort;
        prop.nUdpPort = dlg.m_nUdpPort;
        prop.nUserTimeout = dlg.m_nUserTimeout;
        prop.bAutoSave = dlg.m_bAutoSave;
        prop.nMaxVoiceTxPerSecond = dlg.m_nAudioTx*1024;
        prop.nMaxVideoCaptureTxPerSecond = dlg.m_nVideoTx*1024;
        prop.nMaxMediaFileTxPerSecond = dlg.m_nMediaFileTx*1024;
        prop.nMaxDesktopTxPerSecond = dlg.m_nDesktopTxMax*1024;
        prop.nMaxTotalTxPerSecond = dlg.m_nTotalTx*1024;
        prop.nMaxLoginAttempts = dlg.m_nLoginsBan;
        prop.nMaxLoginsPerIPAddress = dlg.m_nMaxIPLogins;

        TT_DoUpdateServer(ttInst, &prop);
    }
}

void CTeamTalkDlg::OnUpdateServerListuseraccounts(CCmdUI *pCmdUI)
{
    pCmdUI->Enable((TT_GetFlags(ttInst) & CLIENT_AUTHORIZED) != CLIENT_CLOSED);
}

void CTeamTalkDlg::OnServerListuseraccounts()
{
    if(TT_GetMyUserType(ttInst) & USERTYPE_ADMIN)
    {
        int id = TT_DoListUserAccounts(ttInst, 0, 100000);
        if(id>0)
            m_commands[id] = CMD_COMPLETE_LISTACCOUNTS;
    }
    else
    {
        CUserAccountsDlg dlg(this, UAD_READONLY);

        UserAccount ua;
        ZERO_STRUCT(ua);
        TT_GetMyUserAccount(ttInst, &ua);
        dlg.m_accounts.push_back(ua);
        dlg.DoModal();
    }
}

void CTeamTalkDlg::OnUpdateServerOnlineusers(CCmdUI *pCmdUI)
{
    pCmdUI->Enable((TT_GetFlags(ttInst) & CLIENT_AUTHORIZED) != CLIENT_CLOSED);
}

void CTeamTalkDlg::OnServerOnlineusers()
{
    if (!m_pOnlineUsersDlg)
    {
        m_pOnlineUsersDlg = new COnlineUsersDlg(this);
        m_pOnlineUsersDlg->Create(COnlineUsersDlg::IDD, GetDesktopWindow());

        auto users = m_wndTree.GetUsers();
        for(auto u : users)
        {
            if (m_users.find(u.first) != m_users.end())
                m_pOnlineUsersDlg->AddUser(u.second);
        }
    }
    else
        m_pOnlineUsersDlg->SetForegroundWindow();

    m_pOnlineUsersDlg->ShowWindow(SW_SHOW);
}

void CTeamTalkDlg::OnUpdateServerSaveconfiguration(CCmdUI *pCmdUI)
{
    pCmdUI->Enable((TT_GetMyUserType(ttInst) & USERTYPE_ADMIN) != CLIENT_CLOSED);
}

void CTeamTalkDlg::OnServerSaveconfiguration()
{
    TT_DoSaveConfig(ttInst);
}

void CTeamTalkDlg::OnUpdateAdvancedStoreformove(CCmdUI *pCmdUI)
{
    pCmdUI->Enable(m_wndTree.GetSelectedUser()>0);
}

void CTeamTalkDlg::OnAdvancedStoreformove()
{
    int nMoveUserID = m_wndTree.GetSelectedUser();
    if (nMoveUserID)
        m_moveusers.insert(nMoveUserID);

}

void CTeamTalkDlg::OnUpdateAdvancedMoveuser(CCmdUI *pCmdUI)
{
    pCmdUI->Enable(m_moveusers.size() && m_wndTree.GetSelectedChannel(true));
}

void CTeamTalkDlg::OnAdvancedMoveuser()
{
    int nChanID = m_wndTree.GetSelectedChannel(true);
    std::for_each(m_moveusers.begin(), m_moveusers.end(),
        [nChanID](int nUserID)
    {
        TT_DoMoveUser(ttInst, nUserID, nChanID);
    });
    m_moveusers.clear();
}

void CTeamTalkDlg::OnUpdateAdvancedMoveuserdialog(CCmdUI *pCmdUI)
{
    pCmdUI->Enable(m_wndTree.GetSelectedUser()>0 || m_moveusers.size());
}

void CTeamTalkDlg::OnAdvancedMoveuserdialog()
{    
    TTCHAR szChan[TT_STRLEN];
    CMoveToChannelDlg dlg;
    const channels_t& channels = m_wndTree.GetChannels();
    for(channels_t::const_iterator ite=channels.begin();
        ite!=channels.end();ite++)
    {
        if(TT_GetChannelPath(ttInst, ite->second.nChannelID, szChan))
            dlg.m_Channels.AddTail(szChan);
    }

    TT_GetChannelPath(ttInst, m_nLastMoveChannel, szChan);
    dlg.m_szChannel = szChan;
    if(dlg.DoModal() == IDOK &&
       (m_nLastMoveChannel = TT_GetChannelIDFromPath(ttInst, dlg.m_szChannel)) )
    {
        int userid = m_wndTree.GetSelectedUser();
        if(userid)
            m_moveusers.insert(userid);

        int nChannelID = m_nLastMoveChannel;
        std::for_each(m_moveusers.begin(), m_moveusers.end(),
            [nChannelID](int nUserID)
        {
            TT_DoMoveUser(ttInst, nUserID, nChannelID);
        });
        m_moveusers.clear();
    }
}

void CTeamTalkDlg::OnUpdateServerListbannedusers(CCmdUI *pCmdUI)
{
    BOOL bEnable = (TT_GetMyUserRights(ttInst) & USERRIGHT_BAN_USERS) != USERRIGHT_NONE;
    pCmdUI->Enable(bEnable);
}

void CTeamTalkDlg::OnServerListbannedusers()
{
    int cmdid = TT_DoListBans(ttInst, 0, 0, 1000000);
    if(cmdid>0)
        m_commands[cmdid] = CMD_COMPLETE_LIST_SERVERBANS;
}

void CTeamTalkDlg::OnUpdateChannelsBannedusersinchannel(CCmdUI *pCmdUI)
{
    pCmdUI->Enable(TRUE);
}

void CTeamTalkDlg::OnChannelsBannedusersinchannel()
{
    int nChannelID = m_wndTree.GetSelectedChannel(TRUE);
    int cmdid = TT_DoListBans(ttInst, nChannelID, 0, 1000000);
    if(cmdid>0)
        m_commands[cmdid] = CMD_COMPLETE_LIST_CHANNELBANS;
}

void CTeamTalkDlg::OnUpdateUsersStoreconversationstodisk(CCmdUI *pCmdUI)
{
    pCmdUI->SetCheck(m_xmlSettings.GetAudioLogStorageMode() != AUDIOSTORAGE_NONE);
}

void CTeamTalkDlg::OnUsersStoreconversationstodisk()
{
    UINT uStorageMode = m_xmlSettings.GetAudioLogStorageMode();

    CMediaStorageDlg dlg;
    dlg.m_szAudioDir = STR_UTF8(m_xmlSettings.GetAudioLogStorage());
    dlg.m_uAFF = m_xmlSettings.GetAudioLogStorageFormat();
    dlg.m_bSingleFile = (uStorageMode & AUDIOSTORAGE_SINGLEFILE)?TRUE:FALSE;
    dlg.m_bSeparateFiles = (uStorageMode & AUDIOSTORAGE_SEPARATEFILES)?TRUE:FALSE;

    dlg.m_szChanLogDir = STR_UTF8(m_xmlSettings.GetChanTextLogStorage());
    dlg.m_szUserTxtDir = STR_UTF8(m_xmlSettings.GetUserTextLogStorage());

    switch (dlg.DoModal())
    {
    case IDOK:
        m_xmlSettings.SetAudioLogStorage(STR_UTF8(dlg.m_szAudioDir));
        m_xmlSettings.SetAudioLogStorageFormat(dlg.m_uAFF);
        uStorageMode = AUDIOSTORAGE_NONE;
        if(dlg.m_bSingleFile)
            uStorageMode |= AUDIOSTORAGE_SINGLEFILE;
        if(dlg.m_bSeparateFiles)
            uStorageMode |= AUDIOSTORAGE_SEPARATEFILES;
        m_xmlSettings.SetAudioLogStorageMode(uStorageMode);
        m_xmlSettings.SetChanTextLogStorage(STR_UTF8(dlg.m_szChanLogDir));
        m_xmlSettings.SetUserTextLogStorage(STR_UTF8(dlg.m_szUserTxtDir));

        UpdateChannelLog();

        UpdateAudioStorage(TRUE);
        break;
    case IDCLOSE :
        if(uStorageMode != AUDIOSTORAGE_NONE)
        {
            UpdateAudioStorage(FALSE);
            m_xmlSettings.SetAudioLogStorageMode(AUDIOSTORAGE_NONE);
            return;
        }
        break;
    }
}

void CTeamTalkDlg::OnUpdateSubscriptionsUsermessages(CCmdUI *pCmdUI)
{
    User user;
    if(TT_GetUser(ttInst, m_wndTree.GetSelectedUser(), &user))
    {
        pCmdUI->SetCheck(user.uLocalSubscriptions & SUBSCRIBE_USER_MSG);
        pCmdUI->Enable(TRUE);
    }
    else
    {
        pCmdUI->SetCheck(FALSE);
        pCmdUI->Enable(FALSE);
    }
}

void CTeamTalkDlg::OnSubscriptionsUsermessages()
{
    SubscribeToggle(m_wndTree.GetSelectedUser(), SUBSCRIBE_USER_MSG);
}

void CTeamTalkDlg::OnUpdateSubscriptionsChannelmessages(CCmdUI *pCmdUI)
{
    User user;
    if(TT_GetUser(ttInst, m_wndTree.GetSelectedUser(), &user))
    {
        pCmdUI->SetCheck(user.uLocalSubscriptions & SUBSCRIBE_CHANNEL_MSG);
        pCmdUI->Enable(TRUE);
    }
    else
    {
        pCmdUI->SetCheck(FALSE);
        pCmdUI->Enable(FALSE);
    }
}

void CTeamTalkDlg::OnSubscriptionsChannelmessages()
{
    SubscribeToggle(m_wndTree.GetSelectedUser(), SUBSCRIBE_CHANNEL_MSG);
}

void CTeamTalkDlg::OnUpdateSubscriptionsBroadcastmessages(CCmdUI *pCmdUI)
{
    User user;
    if(TT_GetUser(ttInst, m_wndTree.GetSelectedUser(), &user))
    {
        pCmdUI->SetCheck(user.uLocalSubscriptions & SUBSCRIBE_BROADCAST_MSG);
        pCmdUI->Enable(TRUE);
    }
    else
    {
        pCmdUI->SetCheck(FALSE);
        pCmdUI->Enable(FALSE);
    }
}

void CTeamTalkDlg::OnSubscriptionsBroadcastmessages()
{
    SubscribeToggle(m_wndTree.GetSelectedUser(), SUBSCRIBE_BROADCAST_MSG);
}

void CTeamTalkDlg::OnUpdateSubscriptionsAudio(CCmdUI *pCmdUI)
{
    User user;
    if(TT_GetUser(ttInst, m_wndTree.GetSelectedUser(), &user))
    {
        pCmdUI->SetCheck(user.uLocalSubscriptions & SUBSCRIBE_VOICE);
        pCmdUI->Enable(TRUE);
    }
    else
    {
        pCmdUI->SetCheck(FALSE);
        pCmdUI->Enable(FALSE);
    }
}

void CTeamTalkDlg::OnSubscriptionsAudio()
{
    SubscribeToggle(m_wndTree.GetSelectedUser(), SUBSCRIBE_VOICE);
}

void CTeamTalkDlg::OnUpdateSubscriptionsVideo(CCmdUI *pCmdUI)
{
    User user;
    if(TT_GetUser(ttInst, m_wndTree.GetSelectedUser(), &user))
    {
        pCmdUI->SetCheck(user.uLocalSubscriptions & SUBSCRIBE_VIDEOCAPTURE);
        pCmdUI->Enable(TRUE);
    }
    else
    {
        pCmdUI->SetCheck(FALSE);
        pCmdUI->Enable(FALSE);
    }
}

void CTeamTalkDlg::OnSubscriptionsVideo()
{
    SubscribeToggle(m_wndTree.GetSelectedUser(), SUBSCRIBE_VIDEOCAPTURE);
    m_videoignore.erase(m_wndTree.GetSelectedUser() | VIDEOTYPE_CAPTURE);
}

void CTeamTalkDlg::OnUpdateSubscriptionsDesktop(CCmdUI *pCmdUI)
{
    User user;
    if(TT_GetUser(ttInst, m_wndTree.GetSelectedUser(), &user))
    {
        pCmdUI->SetCheck(user.uLocalSubscriptions & SUBSCRIBE_DESKTOP);
        pCmdUI->Enable(TRUE);
    }
    else
    {
        pCmdUI->SetCheck(FALSE);
        pCmdUI->Enable(FALSE);
    }
}

void CTeamTalkDlg::OnSubscriptionsDesktop()
{
    SubscribeToggle(m_wndTree.GetSelectedUser(), SUBSCRIBE_DESKTOP);
    m_desktopignore.erase(m_wndTree.GetSelectedUser());
}

void CTeamTalkDlg::OnUpdateSubscriptionsInterceptusermessages(CCmdUI *pCmdUI)
{
    User user;
    if(TT_GetUser(ttInst, m_wndTree.GetSelectedUser(), &user))
    {
        pCmdUI->SetCheck(user.uLocalSubscriptions & SUBSCRIBE_INTERCEPT_USER_MSG);
        pCmdUI->Enable(TRUE);
    }
    else
    {
        pCmdUI->SetCheck(FALSE);
        pCmdUI->Enable(FALSE);
    }
}

void CTeamTalkDlg::OnSubscriptionsInterceptusermessages()
{
    SubscribeToggle(m_wndTree.GetSelectedUser(), SUBSCRIBE_INTERCEPT_USER_MSG);
}

void CTeamTalkDlg::OnUpdateSubscriptionsInterceptchannelmessages(CCmdUI *pCmdUI)
{
    User user;
    if(TT_GetUser(ttInst, m_wndTree.GetSelectedUser(), &user))
    {
        pCmdUI->SetCheck(user.uLocalSubscriptions & SUBSCRIBE_INTERCEPT_CHANNEL_MSG);
        pCmdUI->Enable(TRUE);
    }
    else
    {
        pCmdUI->SetCheck(FALSE);
        pCmdUI->Enable(FALSE);
    }
}

void CTeamTalkDlg::OnSubscriptionsInterceptchannelmessages()
{
    SubscribeToggle(m_wndTree.GetSelectedUser(), SUBSCRIBE_INTERCEPT_CHANNEL_MSG);
}

void CTeamTalkDlg::OnUpdateSubscriptionsInterceptaudio(CCmdUI *pCmdUI)
{
    User user;
    if(TT_GetUser(ttInst, m_wndTree.GetSelectedUser(), &user))
    {
        pCmdUI->SetCheck(user.uLocalSubscriptions & SUBSCRIBE_INTERCEPT_VOICE);
        pCmdUI->Enable(TRUE);
    }
    else
    {
        pCmdUI->SetCheck(FALSE);
        pCmdUI->Enable(FALSE);
    }
}

void CTeamTalkDlg::OnSubscriptionsInterceptaudio()
{
    SubscribeToggle(m_wndTree.GetSelectedUser(), SUBSCRIBE_INTERCEPT_VOICE);
}


void CTeamTalkDlg::OnUpdateSubscriptionsInterceptvideo(CCmdUI *pCmdUI)
{
    User user;
    if(TT_GetUser(ttInst, m_wndTree.GetSelectedUser(), &user))
    {
        pCmdUI->SetCheck(user.uLocalSubscriptions & SUBSCRIBE_INTERCEPT_VIDEOCAPTURE);
        pCmdUI->Enable(TRUE);
    }
    else
    {
        pCmdUI->SetCheck(FALSE);
        pCmdUI->Enable(FALSE);
    }
}

void CTeamTalkDlg::OnSubscriptionsInterceptvideo()
{
    SubscribeToggle(m_wndTree.GetSelectedUser(), SUBSCRIBE_INTERCEPT_VIDEOCAPTURE);
}

void CTeamTalkDlg::OnUpdateSubscriptionsInterceptdesktop(CCmdUI *pCmdUI)
{
    User user;
    if(TT_GetUser(ttInst, m_wndTree.GetSelectedUser(), &user))
    {
        pCmdUI->SetCheck(user.uLocalSubscriptions & SUBSCRIBE_INTERCEPT_DESKTOP);
        pCmdUI->Enable(TRUE);
    }
    else
    {
        pCmdUI->SetCheck(FALSE);
        pCmdUI->Enable(FALSE);
    }
}

void CTeamTalkDlg::OnSubscriptionsInterceptdesktop()
{
    SubscribeToggle(m_wndTree.GetSelectedUser(), SUBSCRIBE_INTERCEPT_DESKTOP);
}



void CTeamTalkDlg::OnUpdateSubscriptionsMediafilestream(CCmdUI *pCmdUI)
{
    User user;
    if(TT_GetUser(ttInst, m_wndTree.GetSelectedUser(), &user))
    {
        pCmdUI->SetCheck(user.uLocalSubscriptions & SUBSCRIBE_MEDIAFILE);
        pCmdUI->Enable(TRUE);
    }
    else
    {
        pCmdUI->SetCheck(FALSE);
        pCmdUI->Enable(FALSE);
    }
}


void CTeamTalkDlg::OnSubscriptionsMediafilestream()
{
    SubscribeToggle(m_wndTree.GetSelectedUser(), SUBSCRIBE_MEDIAFILE);
}


void CTeamTalkDlg::OnUpdateSubscriptionsInterceptmediafilestream(CCmdUI *pCmdUI)
{
    User user;
    if(TT_GetUser(ttInst, m_wndTree.GetSelectedUser(), &user))
    {
        pCmdUI->SetCheck(user.uLocalSubscriptions & SUBSCRIBE_INTERCEPT_MEDIAFILE);
        pCmdUI->Enable(TRUE);
    }
    else
    {
        pCmdUI->SetCheck(FALSE);
        pCmdUI->Enable(FALSE);
    }
}


void CTeamTalkDlg::OnSubscriptionsInterceptmediafilestream()
{
    SubscribeToggle(m_wndTree.GetSelectedUser(), SUBSCRIBE_INTERCEPT_MEDIAFILE);
}

void CTeamTalkDlg::ToggleTransmitUsers(int nUserID, StreamTypes uStreamTypes)
{
    Channel chan;
    if(!TT_GetChannel(ttInst, m_wndTree.GetSelectedChannel(true), &chan))
        return;

    if(!ToggleTransmitUser(chan, nUserID, uStreamTypes))
    {
        CString szCaption = LoadText(IDS_CLASSROOMCHANNEL),
            szFmt = LoadText(IDS_MAX_TX_USERS), szText;
        szText.Format(szFmt, TT_TRANSMITUSERS_MAX);
        MessageBox(szText, szCaption, MB_OK);
        return;
    }

    TT_DoUpdateChannel(ttInst, &chan);
}

void CTeamTalkDlg::PlaySoundEvent(SoundEvent event)
{
    SoundEvents events = m_xmlSettings.GetEventSoundsEnabled(SOUNDEVENT_ALL);
    CString szFilename;
    switch(events & event)
    {
    case SOUNDEVENT_USER_JOIN :
        szFilename = STR_UTF8(m_xmlSettings.GetEventNewUser());
        break;
    case SOUNDEVENT_USER_LEFT :
        szFilename = STR_UTF8(m_xmlSettings.GetEventRemovedUser());
        break;
    case SOUNDEVENT_USER_TEXTMSG :
        szFilename = STR_UTF8(m_xmlSettings.GetEventNewMessage());
        break;
    case SOUNDEVENT_USER_CHANNEL_TEXTMSG :
        szFilename = STR_UTF8(m_xmlSettings.GetEventChannelMsg());
        break;
    case SOUNDEVENT_USER_BROADCAST_TEXTMSG:
        szFilename = STR_UTF8(m_xmlSettings.GetEventBroadcastMsg());
        break;
    case SOUNDEVENT_USER_QUESTIONMODE :
        szFilename = STR_UTF8(m_xmlSettings.GetEventQuestionMode());
        break;
    case SOUNDEVENT_USER_DESKTOP_ACCESS :
        szFilename = STR_UTF8(m_xmlSettings.GetEventDesktopAccessReq());
        break;
    case SOUNDEVENT_USER_VIDEOSESSION_NEW :
        szFilename = STR_UTF8(m_xmlSettings.GetEventVideoSession());
        break;
    case SOUNDEVENT_USER_DESKTOPSESSION_NEW :
        szFilename = STR_UTF8(m_xmlSettings.GetEventDesktopSession());
        break;
    case SOUNDEVENT_CONNECTION_LOST :
        szFilename = STR_UTF8(m_xmlSettings.GetEventServerLost());
        break;
    case SOUNDEVENT_PUSHTOTALK :
        szFilename = STR_UTF8(m_xmlSettings.GetEventHotKey());
        break;
    case SOUNDEVENT_FILES_UPDATED :
        szFilename = STR_UTF8(m_xmlSettings.GetEventFilesUpd());
        break;
    case SOUNDEVENT_FILETX_COMPLETE :
        szFilename = STR_UTF8(m_xmlSettings.GetEventTransferEnd());
        break;
    case SOUNDEVENT_CHANNEL_SILENT :
        szFilename = STR_UTF8(m_xmlSettings.GetEventChannelSilent());
        break;
    case SOUNDEVENT_VOICEACTIVATED :
        szFilename = STR_UTF8(m_xmlSettings.GetEventVoiceActivated());
        break;
    case SOUNDEVENT_VOICEDEACTIVATED :
        szFilename = STR_UTF8(m_xmlSettings.GetEventVoiceDeactivated());
        break;
    case SOUNDEVENT_ENABLE_VOICEACTIVATION:
        szFilename = STR_UTF8(m_xmlSettings.GetEventEnableVoiceActivation());
        break;
    case SOUNDEVENT_DISABLE_VOICEACTIVATION:
        szFilename = STR_UTF8(m_xmlSettings.GetEventDisableVoiceActivation());
        break;
    case SOUNDEVENT_ME_ENABLE_VOICEACTIVATION:
        szFilename = STR_UTF8(m_xmlSettings.GetEventMeEnableVoiceActivation());
        break;
    case SOUNDEVENT_ME_DISABLE_VOICEACTIVATION:
        szFilename = STR_UTF8(m_xmlSettings.GetEventMeDisableVoiceActivation());
        break;
    case SOUNDEVENT_TRANSMITQUEUE_HEAD :
        szFilename = STR_UTF8(m_xmlSettings.GetEventTransmitQueueHead());
        break;
    case SOUNDEVENT_TRANSMITQUEUE_STOP :
        szFilename = STR_UTF8(m_xmlSettings.GetEventTransmitQueueStop());
        break;
    }

    if(szFilename.GetLength())
        m_pPlaySndThread->AddSoundEvent(szFilename);
}

void CTeamTalkDlg::RunAppUpdate()
{
    if(m_xmlSettings.GetCheckApplicationUpdates())
    {
        m_httpUpdate.reset(new CHttpRequest(URL_APPUPDATE));
        SetTimer(TIMER_HTTPREQUEST_APPUPDATE_UPDATE_ID, 500, NULL);
        SetTimer(TIMER_HTTPREQUEST_APPUPDATE_TIMEOUT_ID, 5000, NULL);
    }
}

void CTeamTalkDlg::OnUpdateAdvancedAllowvoicetransmission(CCmdUI *pCmdUI)
{
    UpdateAllowTransmitMenuItem(m_wndTree.GetSelectedUser(), 
                                m_wndTree.GetSelChannel(),
                                STREAMTYPE_VOICE, pCmdUI);
}

void CTeamTalkDlg::OnAdvancedAllowvoicetransmission()
{
    ToggleTransmitUsers(m_wndTree.GetSelectedUser(), STREAMTYPE_VOICE);
}

void CTeamTalkDlg::OnUpdateAdvancedAllowvideotransmission(CCmdUI *pCmdUI)
{
    UpdateAllowTransmitMenuItem(m_wndTree.GetSelectedUser(),
                                m_wndTree.GetSelChannel(),
                                STREAMTYPE_VIDEOCAPTURE, pCmdUI);
}

void CTeamTalkDlg::OnAdvancedAllowvideotransmission()
{
    ToggleTransmitUsers(m_wndTree.GetSelectedUser(), STREAMTYPE_VIDEOCAPTURE);
}

void CTeamTalkDlg::OnUpdateAdvancedAllowdesktoptransmission(CCmdUI *pCmdUI)
{
    UpdateAllowTransmitMenuItem(m_wndTree.GetSelectedUser(),
                                m_wndTree.GetSelChannel(),
                                STREAMTYPE_DESKTOP, pCmdUI);
}

void CTeamTalkDlg::OnAdvancedAllowdesktoptransmission()
{
    ToggleTransmitUsers(m_wndTree.GetSelectedUser(), STREAMTYPE_DESKTOP);
}

void CTeamTalkDlg::OnUpdateAdvancedAllowmediafiletransmission(CCmdUI *pCmdUI)
{
    UpdateAllowTransmitMenuItem(m_wndTree.GetSelectedUser(),
                                m_wndTree.GetSelChannel(),
        STREAMTYPE_MEDIAFILE_AUDIO | STREAMTYPE_MEDIAFILE_VIDEO, pCmdUI);
}

void CTeamTalkDlg::OnAdvancedAllowmediafiletransmission()
{
    ToggleTransmitUsers(m_wndTree.GetSelectedUser(), STREAMTYPE_MEDIAFILE_AUDIO | STREAMTYPE_MEDIAFILE_VIDEO);
}

void CTeamTalkDlg::OnUpdateAdvancedAllowallvoicetransmission(CCmdUI *pCmdUI)
{
    UpdateAllowTransmitMenuItem(TT_CLASSROOM_FREEFORALL, 
                                m_wndTree.GetSelChannel(),
                                STREAMTYPE_VOICE, pCmdUI);
}

void CTeamTalkDlg::OnAdvancedAllowallvoicetransmission()
{
    ToggleTransmitUsers(TT_CLASSROOM_FREEFORALL, STREAMTYPE_VOICE);
}

void CTeamTalkDlg::OnUpdateAdvancedAllowallvideotransmission(CCmdUI *pCmdUI)
{
    UpdateAllowTransmitMenuItem(TT_CLASSROOM_FREEFORALL, 
                                m_wndTree.GetSelChannel(),
                                STREAMTYPE_VIDEOCAPTURE, pCmdUI);
}

void CTeamTalkDlg::OnAdvancedAllowallvideotransmission()
{
    ToggleTransmitUsers(TT_CLASSROOM_FREEFORALL, STREAMTYPE_VIDEOCAPTURE);
}

void CTeamTalkDlg::OnUpdateAdvancedAllowallmediafiletransmission(CCmdUI *pCmdUI)
{
    UpdateAllowTransmitMenuItem(TT_CLASSROOM_FREEFORALL, 
                                m_wndTree.GetSelChannel(),
                                STREAMTYPE_MEDIAFILE, pCmdUI);
}

void CTeamTalkDlg::OnAdvancedAllowallmediafiletransmission()
{
    ToggleTransmitUsers(TT_CLASSROOM_FREEFORALL, STREAMTYPE_MEDIAFILE);
}

void CTeamTalkDlg::OnUpdateAdvancedAllowalldesktoptransmission(CCmdUI *pCmdUI)
{
    UpdateAllowTransmitMenuItem(TT_CLASSROOM_FREEFORALL, 
                                m_wndTree.GetSelChannel(),
                                STREAMTYPE_DESKTOP, pCmdUI);
}

void CTeamTalkDlg::OnAdvancedAllowalldesktoptransmission()
{
    ToggleTransmitUsers(TT_CLASSROOM_FREEFORALL, STREAMTYPE_DESKTOP);
}

void CTeamTalkDlg::OnUpdateServerServerstatistics(CCmdUI *pCmdUI)
{
    pCmdUI->Enable((TT_GetMyUserType(ttInst) & USERTYPE_ADMIN) != USERTYPE_NONE);
}

void CTeamTalkDlg::OnServerServerstatistics()
{
    int nCmdID = TT_DoQueryServerStats(ttInst);
    if(nCmdID>0)
        m_commands[nCmdID] = CMD_COMPLETE_SERVERSTATS;
    else
        MessageBox(LoadText(IDS_QUERYSERVSTATFAILED, _T("Failed to query server statistics")), LoadText(IDS_SERVSTAT, _T("Server Statistics")), MB_OK);
}

void CTeamTalkDlg::OnNMCustomdrawSliderGainlevel(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
    UpdateGainLevel(m_wndGainSlider.GetPos());
    *pResult = 0;
}

void CTeamTalkDlg::OnUpdateServerBroadcastmessage(CCmdUI *pCmdUI)
{
    BOOL bEnable = (TT_GetMyUserRights(ttInst) & USERRIGHT_TEXTMESSAGE_BROADCAST) != USERRIGHT_NONE;
    pCmdUI->Enable(bEnable);
}

void CTeamTalkDlg::OnServerBroadcastmessage()
{
    CInputDlg dlg(LoadText(IDS_BROADMSG, _T("Broadcast Message")), LoadText(IDS_MSGTOBROAD, _T("Message to broadcast")), 0, this);
    if(dlg.DoModal() == IDOK)
    {
        TextMessage msg = {};
        msg.nMsgType = MSGTYPE_BROADCAST;
        COPYTTSTR(msg.szMessage, dlg.m_szInput);
        TT_DoTextMessage(ttInst, &msg);
    }
}

void CTeamTalkDlg::OnUpdateMeEnablevideotransmission(CCmdUI *pCmdUI)
{
    ClientFlags uFlags = TT_GetFlags(ttInst);
    pCmdUI->SetCheck((uFlags & CLIENT_TX_VIDEOCAPTURE)?BST_CHECKED:BST_UNCHECKED);
}

void CTeamTalkDlg::OnMeEnablevideotransmission()
{
    if(TT_GetFlags(ttInst) & CLIENT_VIDEOCAPTURE_READY)
    {
        TT_StopVideoCaptureTransmission(ttInst);
        TT_CloseVideoCaptureDevice(ttInst);

        m_nStatusMode &= ~STATUSMODE_VIDEOTX;
        TT_DoChangeStatus(ttInst, m_nStatusMode, m_szAwayMessage);
        m_xmlSettings.SetVideoCaptureEnabled(false);
        return;
    }

    CString szCaption = LoadText(IDS_ENABLEVT, _T("Enable Video Transmission"));
    CString szDeviceID = STR_UTF8(m_xmlSettings.GetVideoCaptureDevice());
    vector<VideoCaptureDevice> viddevs;
    int count = 0;
    TT_GetVideoCaptureDevices(NULL, &count);
    if(count)
    {
        viddevs.resize(count);
        BOOL bOk = TT_GetVideoCaptureDevices(&viddevs[0], &count);
    }

    size_t i;
    for(i=0;i<viddevs.size();i++)
    {
        if(viddevs[i].szDeviceID == szDeviceID)
            break;
    }

    VideoFormat capformat;
    ZERO_STRUCT(capformat);

    if(i == viddevs.size())
    {
        //just select video dev 0 as primary
        if(viddevs.size())
        {
            szDeviceID = viddevs[0].szDeviceID;
        }
        else
        {
            MessageBox(LoadText(IDS_NOVIDEODEVICE, _T("No video devices detected.\r\nPress Client -> Preferences -> Video Capture to reconfigure.")), 
                       szCaption, MB_OK);
            return;
        }
    }

    m_xmlSettings.GetVideoCaptureFormat(capformat);
    
    if(!IsValid(capformat))
    {
        capformat.nWidth = DEFAULT_VIDEO_WIDTH;
        capformat.nHeight = DEFAULT_VIDEO_HEIGHT;
        capformat.nFPS_Numerator = DEFAULT_VIDEO_FPS;
        capformat.nFPS_Denominator = 1;
        capformat.picFourCC = DEFAULT_VIDEO_FOURCC;
    }

    VideoCodec codec;
    ZERO_STRUCT(codec);
    codec.nCodec = DEFAULT_VIDEOCODEC;
    switch(codec.nCodec)
    {
    case WEBM_VP8_CODEC :
        codec.webm_vp8.nRcTargetBitrate = m_xmlSettings.GetVideoCodecBitrate(DEFAULT_WEBM_VP8_BITRATE);
        codec.webm_vp8.nEncodeDeadline = DEFAULT_WEBMVP8_DEADLINE;
        break;
    }

    if(!TT_InitVideoCaptureDevice(ttInst, szDeviceID, &capformat))
    {
        MessageBox(LoadText(IDS_STARTVIDEODEVICEFAILED, _T("Failed to start video capture device.")),
                   szCaption, MB_OK);
        return;
    }

    if(!TT_StartVideoCaptureTransmission(ttInst, &codec))
    {
        MessageBox(LoadText(IDS_INITIALIZEVIDEOCODECFAILED, _T("Failed to initiate video codec.")),
                   szCaption, MB_OK);
        TT_CloseVideoCaptureDevice(ttInst);
        return;
    }

    m_nStatusMode |= STATUSMODE_VIDEOTX;
    TT_DoChangeStatus(ttInst, m_nStatusMode, m_szAwayMessage);

    m_xmlSettings.SetVideoCaptureEnabled(true);
}

void CTeamTalkDlg::OnUpdateMeEnabledesktopsharing(CCmdUI *pCmdUI)
{
    pCmdUI->Enable(TT_GetMyChannelID(ttInst)>0);
    pCmdUI->SetCheck((TT_GetFlags(ttInst) & CLIENT_DESKTOP_ACTIVE) != CLIENT_CLOSED);
}

void CTeamTalkDlg::OnMeEnabledesktopsharing()
{
    if(TT_GetFlags(ttInst) & CLIENT_DESKTOP_ACTIVE)
    {
        TT_CloseDesktopWindow(ttInst);
        KillTimer(TIMER_DESKTOPSHARE_ID);

        m_nStatusMode &= ~STATUSMODE_DESKTOP;
        TT_DoChangeStatus(ttInst, m_nStatusMode, m_szAwayMessage);
        return;
    }

    CDesktopShareDlg dlg(this);
    switch(m_xmlSettings.GetDesktopShareMode())
    {
    default:
    case DESKTOPSHARE_DESKTOP :
        dlg.m_bShareDesktop = TRUE;
        break;
    case DESKTOPSHARE_ACTIVE_WINDOW :
        dlg.m_bShareActive = TRUE;
        break;
    case DESKTOPSHARE_SPECIFIC_WINDOW :
        dlg.m_bShareTitle = TRUE;
        break;
    }

    if(m_xmlSettings.GetDesktopShareUpdateInterval() != UNDEFINED)
        dlg.m_nUpdateInterval = m_xmlSettings.GetDesktopShareUpdateInterval();

    if(m_xmlSettings.GetDesktopShareRgbMode() != UNDEFINED)
        dlg.m_nRGBMode = (BitmapFormat)m_xmlSettings.GetDesktopShareRgbMode();

    dlg.m_hShareWnd = m_hShareWnd;

    if(dlg.DoModal() != IDOK)
        return;

    if(dlg.m_bShareDesktop)
        m_xmlSettings.SetDesktopShareMode(DESKTOPSHARE_DESKTOP);
    if(dlg.m_bShareActive)
        m_xmlSettings.SetDesktopShareMode(DESKTOPSHARE_ACTIVE_WINDOW);
    if(dlg.m_bShareTitle)
    {
        m_xmlSettings.SetDesktopShareMode(DESKTOPSHARE_SPECIFIC_WINDOW);
        m_hShareWnd = dlg.m_hShareWnd;
    }
    m_xmlSettings.SetDesktopShareUpdateInterval(dlg.m_nUpdateInterval);
    m_xmlSettings.SetDesktopShareRgbMode(dlg.m_nRGBMode);

    SendDesktopWindow();

    if(dlg.m_bUpdateInterval)
        RestartSendDesktopWindowTimer();

    m_nStatusMode |= STATUSMODE_DESKTOP;
    TT_DoChangeStatus(ttInst, m_nStatusMode, m_szAwayMessage);
}


void CTeamTalkDlg::OnUpdateSubscriptionsDesktopacces(CCmdUI *pCmdUI)
{
    User user;
    if(TT_GetUser(ttInst, m_wndTree.GetSelectedUser(), &user))
    {
        pCmdUI->SetCheck(user.uLocalSubscriptions & SUBSCRIBE_DESKTOPINPUT);
        pCmdUI->Enable(TRUE);
    }
    else
    {
        pCmdUI->SetCheck(FALSE);
        pCmdUI->Enable(FALSE);
    }
}


void CTeamTalkDlg::OnSubscriptionsDesktopacces()
{
    SubscribeToggle(m_wndTree.GetSelectedUser(), SUBSCRIBE_DESKTOPINPUT);
}


void CTeamTalkDlg::OnUpdateUsersAllowdesktopaccess(CCmdUI *pCmdUI)
{
    OnUpdateSubscriptionsDesktopacces(pCmdUI);
}


void CTeamTalkDlg::OnUsersAllowdesktopaccess()
{
    OnSubscriptionsDesktopacces();
}


void CTeamTalkDlg::OnUpdateUserinfoSpeakuserinfo(CCmdUI *pCmdUI)
{
    pCmdUI->Enable(m_bSpeech &&
        (m_wndTree.GetSelectedUser()>0 || m_wndTree.GetSelectedChannel()>0));
}


void CTeamTalkDlg::OnUserinfoSpeakuserinfo()
{
    CStringList szSpeakList;

    int nID = m_wndTree.GetSelectedUser();
    if(nID>0)
    {
        User user;
        if(!m_wndTree.GetUser(nID, user))
            return;

        CString szUser, szVoice, szMute, szMediaFile, szMuteMediaFile,
            szVideoCapture, szDesktop, szChanOp = LoadText(IDS_CHANOP, _T("Channel Operator"));

        szUser.LoadString(IDS_USER);
        TRANSLATE_ITEM(IDD_TAB_CHANNELOP, szChanOp);
        szVoice.LoadString(IDS_TALKING);
        szMute.LoadString(IDS_MUTE);
        szMediaFile.LoadString(IDS_STREAMING_MEDIAFILE);
        szMuteMediaFile.LoadString(IDS_MUTE_MEDIAFILE);
        szVideoCapture.LoadString(IDS_VIDEOCAPTURE);
        szDesktop.LoadString(IDS_DESKTOP);

        TRANSLATE_ITEM(IDS_USER, szUser);
        TRANSLATE_ITEM(IDS_TALKING, szVoice);
        TRANSLATE_ITEM(IDS_MUTE, szMute);
        TRANSLATE_ITEM(IDS_STREAMING_MEDIAFILE, szMediaFile);
        TRANSLATE_ITEM(IDS_MUTE_MEDIAFILE, szMuteMediaFile);
        TRANSLATE_ITEM(IDS_VIDEOCAPTURE, szVideoCapture);
        TRANSLATE_ITEM(IDS_DESKTOP, szDesktop);

        szSpeakList.AddTail(szUser);

        CString szStatus;

        if(TT_IsChannelOperator(ttInst, user.nUserID, user.nChannelID))
            szSpeakList.AddTail(szChanOp);

        if(user.uUserState & USERSTATE_VOICE)
            szSpeakList.AddTail(szVoice);
        switch(user.nStatusMode & STATUSMODE_MASK)
        {
        case STATUSMODE_AVAILABLE :
            szStatus.LoadString(IDC_RADIO_ONLINE);
            TRANSLATE_ITEM(IDC_RADIO_ONLINE, szStatus);
            break;
        case STATUSMODE_AWAY :
            szStatus.LoadString(IDC_RADIO_AWAY);
            TRANSLATE_ITEM(IDC_RADIO_AWAY, szStatus);
            break;
        case STATUSMODE_QUESTION :
            szStatus.LoadString(IDC_RADIO_QUESTION);
            TRANSLATE_ITEM(IDC_RADIO_QUESTION, szStatus);
            break;
        }
        szStatus = StripAmpersand(szStatus);
        if(szStatus.GetLength())
            szSpeakList.AddTail(szStatus);
        if(user.uUserState & USERSTATE_MUTE_VOICE)
            szSpeakList.AddTail(szMute);
        if((user.uUserState & USERSTATE_MEDIAFILE) ||
           (user.nStatusMode & STATUSMODE_STREAM_MEDIAFILE))
            szSpeakList.AddTail(szMediaFile);
        if(user.uUserState & USERSTATE_MUTE_MEDIAFILE)
            szSpeakList.AddTail(szMuteMediaFile);
        if((user.uUserState & USERSTATE_VIDEOCAPTURE) ||
           (user.nStatusMode & STATUSMODE_VIDEOTX))
            szSpeakList.AddTail(szVideoCapture);
        if((user.uUserState & USERSTATE_DESKTOP) ||
           (user.nStatusMode & STATUSMODE_DESKTOP))
            szSpeakList.AddTail(szDesktop);
    }
    else if((nID = m_wndTree.GetSelectedChannel())>0)
    {
        Channel chan;
        if(!m_wndTree.GetChannel(nID, chan))
            return;

        CString szChannel, szPasswd, szClassroom;
        szChannel.LoadString(IDS_CHANNEL);
        szPasswd.LoadString(IDS_PASSWORD_PROTECTED);
        szClassroom.LoadString(IDS_CLASSROOMCHANNEL);

        TRANSLATE_ITEM(IDS_CHANNEL, szChannel);
        TRANSLATE_ITEM(IDS_PASSWORD_PROTECTED, szPasswd);
        TRANSLATE_ITEM(IDS_CLASSROOMCHANNEL, szClassroom);

        szSpeakList.AddTail(szChannel);
        if(chan.uChannelType & CHANNEL_CLASSROOM)
            szSpeakList.AddTail(szClassroom);
        if(chan.bPassword)
            szSpeakList.AddTail(szPasswd);
    }

    CString szSpeak;
    for(POSITION pos=szSpeakList.GetHeadPosition();pos!=NULL;)
    {
        szSpeak += szSpeakList.GetNext(pos);
        if(pos != NULL)
            szSpeak += _T(" ");
    }
    if(szSpeak.GetLength())
        AddVoiceMessage(szSpeak);
}

void CTeamTalkDlg::OnUpdateChannelinfoSpeakchannelinfo(CCmdUI *pCmdUI)
{
    pCmdUI->Enable(m_bSpeech && m_wndTree.GetSelectedChannel()>0);
}

void CTeamTalkDlg::OnChannelinfoSpeakchannelinfo()
{
    OnUserinfoSpeakuserinfo();
}

void CTeamTalkDlg::OnUpdateChannelinfoSpeakchannelstate(CCmdUI *pCmdUI)
{
    pCmdUI->Enable(m_wndTree.GetMyChannelID()>0);
}

void CTeamTalkDlg::OnChannelinfoSpeakchannelstate()
{
    std::set<int> voice, mediafile, vidcap, desktop;
    int nMyChannel = m_wndTree.GetMyChannelID();
    users_t users = m_wndTree.GetUsers(nMyChannel);
    for(auto i=users.begin();i!=users.end();++i)
    {
        if (i->second.uUserState & USERSTATE_VOICE)
            voice.insert(i->first);
        if (i->second.uUserState & USERSTATE_MEDIAFILE)
            mediafile.insert(i->first);
        if (i->second.uUserState & USERSTATE_VIDEOCAPTURE)
            vidcap.insert(i->first);
        if (i->second.uUserState & USERSTATE_DESKTOP)
            desktop.insert(i->first);
    }

    CString szVoice, szMediaFile, szVideoCapture, szDesktop;

    if (voice.size())
    {
        szVoice.LoadString(IDS_TALKING);
        TRANSLATE_ITEM(IDS_TALKING, szVoice);
        szVoice += _T(". ");
        for(auto i=voice.begin();i!=voice.end();++i)    
            szVoice += GetDisplayName(users.find(*i)->second) + _T(", ");
        AddVoiceMessage(szVoice);
    }
    if (mediafile.size())
    {
        szMediaFile.LoadString(IDS_STREAMING_MEDIAFILE);
        TRANSLATE_ITEM(IDS_STREAMING_MEDIAFILE, szMediaFile);
        szMediaFile += _T(". ");
        for(auto i=mediafile.begin();i!=mediafile.end();++i)    
            szMediaFile += GetDisplayName(users.find(*i)->second) + _T(", ");
        AddVoiceMessage(szMediaFile);
    }
    if (vidcap.size())
    {
        szVideoCapture.LoadString(IDS_VIDEOCAPTURE);
        TRANSLATE_ITEM(IDS_VIDEOCAPTURE, szVideoCapture);
        szVideoCapture += _T(". ");
        for(auto i=vidcap.begin();i!=vidcap.end();++i)    
            szVideoCapture += GetDisplayName(users.find(*i)->second) + _T(", ");
        AddVoiceMessage(szVideoCapture);
    }
    if (desktop.size())
    {
        szDesktop.LoadString(IDS_DESKTOP);
        TRANSLATE_ITEM(IDS_DESKTOP, szDesktop);
        szDesktop += _T(". ");
        for(auto i=desktop.begin();i!=desktop.end();++i)    
            szDesktop += GetDisplayName(users.find(*i)->second) + _T(", ");
        AddVoiceMessage(szDesktop);
    }
}

void CTeamTalkDlg::OnHelpResetpreferencestodefault()
{
    CString szTitle, szMsg;
    szMsg.LoadString(IDS_RESETPREFERENCES);
    TRANSLATE_ITEM(IDS_RESETPREFERENCES, szMsg);

    szTitle = ExtractMenuText(ID_HELP_RESETPREFERENCESTODEFAULT,
                              LoadText(IDS_RESETPREF, _T("Reset Preferences")));
    if(MessageBox(szMsg, szTitle, MB_YESNO) == IDYES)
    {
        m_bResetSettings = TRUE;
        OnClose();

        szMsg.LoadString(IDS_RESTARTAPPLICATION);
        TRANSLATE_ITEM(IDS_RESTARTAPPLICATION, szMsg);
        MessageBox(szMsg, szTitle, MB_OK);
    }
}

void CTeamTalkDlg::OnClientNewclientinstance()
{
    std::string filename = m_xmlSettings.GetFileName();
    CString szIniPath = STR_UTF8(filename);

    // check if we are creating a new profile from a profile
    if(m_xmlSettings.GetProfileName().size())
    {
        std::smatch sm;
        if(std::regex_search(filename, sm, std::regex("(.\\d{1,2})$")) && sm.size())
        {
            szIniPath = szIniPath.Left(szIniPath.GetLength() - int(sm[1].str().size()));;
        }
    }

    // load existing profiles
    std::map<CString, CString> profiles;
    CStringList profilenames;
    const int MAX_PROFILES = 16;
    int freeno = -1;
    for(int i = 1; i <= MAX_PROFILES; i++)
    {
        CString szIniFile;
        szIniFile.Format(_T("%s.%d"), szIniPath, i);
        ClientXML settings(TT_XML_ROOTNAME);
        if(settings.LoadFile(STR_LOCAL(szIniFile)))
        {
            CString szName = STR_UTF8(settings.GetProfileName());
            profilenames.AddTail(szName);
            profiles[szName] = szIniFile;
        }
        else if(freeno < 0)
            freeno = i;
    }

    const CString szNewProfile = LoadText(IDS_NEWPROFILE, _T("New Profile")),
        szDelProfile = LoadText(IDS_DELETEPROFILE, _T("Delete Profile")),
        szTitle = ExtractMenuText(ID_CLIENT_NEWCLIENTINSTANCE,
                                  LoadText(IDS_NEWCLIENTINSTAN, _T("New Client Instance"))),
        szSelect = LoadText(IDS_SELECTPROFILE, _T("Select profile")),
        szProfileName = LoadText(IDS_PROFILENAME, _T("Profile name"));

    if(profiles.size() < MAX_PROFILES)
        profilenames.AddTail(szNewProfile);
    if(profiles.size() > 0)
        profilenames.AddTail(szDelProfile);

    BOOL bOk = FALSE;
    CInputDlg profileDlg(szTitle, szSelect);
    profileDlg.m_inputList.AddHead(&profilenames);
    
    if (profileDlg.DoModal() != IDOK)
        return;

    if(profileDlg.m_szInput == szDelProfile)
    {
        RemoveString(profilenames, szNewProfile);
        RemoveString(profilenames, szDelProfile);

        CInputDlg delDlg(szTitle, szDelProfile);
        delDlg.m_inputList.AddHead(&profilenames);
        if (delDlg.DoModal() != IDOK)
            return;

        if (STR_UTF8(m_xmlSettings.GetFileName()) != profiles[delDlg.m_szInput])
        {
            DeleteFile(profiles[delDlg.m_szInput]);
        }
        return;
    }
    else if(profileDlg.m_szInput == szNewProfile)
    {
        CString szNewProfileName;
        szNewProfileName.Format(_T("%s %d"), szNewProfile, freeno);

        CInputDlg newDlg(szNewProfile, szProfileName, szNewProfileName);
        if(newDlg.DoModal() != IDOK)
            return;

        if(newDlg.m_szInput.GetLength())
        {
            CString szOldCfg = szIniPath;
            szIniPath.Format(_T("%s.%d"), szOldCfg, freeno);
            CopyFile(szOldCfg, szIniPath, FALSE);
            ClientXML settings(TT_XML_ROOTNAME);
            if(settings.LoadFile(STR_LOCAL(szIniPath)))
            {
                settings.SetProfileName(STR_UTF8(newDlg.m_szInput));
                settings.SaveFile();
            }
        }
        else return;
    }
    else
    {
        szIniPath = profiles[profileDlg.m_szInput];
    }

    TCHAR szBuff[MAX_PATH] = _T("");
    GetModuleFileName(NULL, szBuff, MAX_PATH);
    CString szCmdLine;
    szCmdLine.Format(_T(" -cfg \"%s\""), szIniPath);
    STARTUPINFO info;
    ZERO_STRUCT(info);
    PROCESS_INFORMATION processInfo;
    ZERO_STRUCT(processInfo);

    if (!CreateProcess(szBuff, szCmdLine.GetBuffer(), NULL, NULL, FALSE, CREATE_DEFAULT_ERROR_MODE, NULL, NULL, &info, &processInfo))
        MessageBox(szTitle, LoadText(IDS_EXECUTEFAILED, _T("Failed to execute: ")) + szCmdLine);
}

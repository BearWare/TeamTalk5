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

// UserAccountsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "../Resource.h"
#include "UserAccountsDlg.h"
#include "AppInfo.h"
#include <vector>

using namespace std;

extern TTInstance* ttInst;

// CUserAccountsDlg dialog

IMPLEMENT_DYNAMIC(CUserAccountsDlg, CDialog)

CUserAccountsDlg::CUserAccountsDlg(CWnd* pParent /*=NULL*/, UserAccountsDisplay uad /*= UAD_READWRITE*/)
	: CDialog(CUserAccountsDlg::IDD, pParent)
    , m_uad(uad)
    , m_bResizeReady(FALSE)
{

}

CUserAccountsDlg::~CUserAccountsDlg()
{
}

void CUserAccountsDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_BUTTON_NEW, m_btnNew);
    DDX_Control(pDX, IDC_BUTTON_ADD, m_btnAdd);
    DDX_Control(pDX, IDC_BUTTON_DEL, m_btnDel);
    DDX_Control(pDX, IDC_LIST_ACCOUNTS, m_wndAccounts);
    DDX_Control(pDX, IDC_EDIT_USERNAME, m_wndUsername);
    DDX_Control(pDX, IDC_EDIT_PASSWORD, m_wndPassword);
    DDX_Control(pDX, IDC_RADIO_ADMIN, m_wndAdminUser);
    DDX_Control(pDX, IDC_RADIO_DEFAULTUSER, m_wndDefaultUser);
    DDX_Control(pDX, IDC_COMBO_INITCHANNEL, m_wndInitChannel);
    DDX_Control(pDX, IDC_EDIT_NOTE, m_wndNote);
    DDX_Control(pDX, IDC_CHECK_DOUBLELOGIN, m_wndDoubleLogin);
    DDX_Control(pDX, IDC_CHECK_CHANNELSTEMP, m_wndTempChannels);
    DDX_Control(pDX, IDC_CHECK_VIEWALLUSERS, m_wndViewAllUsers);
    DDX_Control(pDX, IDC_CHECK_USERBCAST, m_wndUserBcast);
    DDX_Control(pDX, IDC_CHECK_TRANSMITVOICE, m_wndTransmitVoice);
    DDX_Control(pDX, IDC_CHECK_TRANSMITVIDEOCAPTURE, m_wndTransmitVideo);
    DDX_Control(pDX, IDC_CHECK_TRANSMITDESKTOP, m_wndTransmitDesktops);
    DDX_Control(pDX, IDC_CHECK_TRANSMITDESKTOPINPUT, m_wndTransmitDesktopInput);
    DDX_Control(pDX, IDC_CHECK_CHANNELSPERMANENT, m_wndPermChannels);
    DDX_Control(pDX, IDC_CHECK_KICKUSERS, m_wndKickUsers);
    DDX_Control(pDX, IDC_CHECK_BANUSERS, m_wndBanUsers);
    DDX_Control(pDX, IDC_CHECK_MOVEUSERS, m_wndMoveUsers);
    DDX_Control(pDX, IDC_CHECK_CHANNELOP, m_wndChannelOp);
    DDX_Control(pDX, IDC_CHECK_UPLOADFILES, m_wndUploadFiles);
    DDX_Control(pDX, IDC_CHECK_DOWNLOADFILES, m_wndDownloadFiles);
    DDX_Control(pDX, IDC_CHECK_UPDATESERVER, m_wndSrvProp);
    DDX_Control(pDX, IDC_CHECK_TRANSMITAUDIOFILE, m_wndTransmitAudFiles);
    DDX_Control(pDX, IDC_CHECK_TRANSMITVIDEOFILE, m_wndTransmitVidFiles);
    DDX_Control(pDX, IDC_TAB_USERACCOUNT, m_wndTabCtrl);
    DDX_Control(pDX, IDC_CHECK_CHANGENICKNAME, m_wndChangeNickname);
    DDX_Control(pDX, IDC_CHECK_RECORDVOICE, m_wndRecordVoice);
}



BOOL CUserAccountsDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    m_wndTabCtrl.SetOrientation(e_tabTop);
    VERIFY(m_wndChanOpTab.Create(IDD_TAB_CHANNELOP, &m_wndTabCtrl));
    VERIFY(m_wndCodecTab.Create(IDD_TAB_CODECLIMIT, &m_wndTabCtrl));
    VERIFY(m_wndAbuseTab.Create(IDD_TAB_ABUSE, &m_wndTabCtrl));

    CString szTabTitle = _T("Channel Operator");
    TRANSLATE_ITEM(IDD_TAB_CHANNELOP, szTabTitle);
    m_wndTabCtrl.AddTab(&m_wndChanOpTab, szTabTitle, 0);
    szTabTitle = _T("Codec Limitations");
    TRANSLATE_ITEM(IDD_TAB_CODECLIMIT, szTabTitle);
    m_wndTabCtrl.AddTab(&m_wndCodecTab, szTabTitle, 0);
    szTabTitle = _T("Abuse Prevention");
    TRANSLATE_ITEM(IDD_TAB_ABUSE, szTabTitle);
    m_wndTabCtrl.AddTab(&m_wndAbuseTab, szTabTitle, 0);

    TRANSLATE(*this, IDD);

    m_wndInitChannel.AddString(_T(""));

    int nCount = 0;
    TT_GetServerChannels(ttInst, NULL, &nCount);
    if(nCount)
    {
        vector<Channel> channels;
        channels.resize(nCount);
        TT_GetServerChannels(ttInst, &channels[0], &nCount);
        for(int i=0;i<nCount;i++)
        {
            TTCHAR szPath[TT_STRLEN];
            if((channels[i].uChannelType & CHANNEL_PERMANENT) &&
               TT_GetChannelPath(ttInst, channels[i].nChannelID, szPath))
            {
                int index = m_wndChanOpTab.m_wndAvailChannels.AddString(szPath);
                m_wndChanOpTab.m_wndAvailChannels.SetItemData(index, channels[i].nChannelID);
                index = m_wndInitChannel.AddString(szPath);
                m_wndInitChannel.SetItemData(index, channels[i].nChannelID);
            }
        }
    }

    ListAccounts();

    if(m_uad == UAD_READONLY && m_accounts.size() == 1)
    {
        m_wndAccounts.SetCurSel(0);
        size_t i = m_wndAccounts.GetItemData(0);
        if (i != CB_ERR)
            ShowUserAccount(m_accounts[i]);
    }
    else
        OnBnClickedButtonNew();

    m_bResizeReady = TRUE;

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

BEGIN_MESSAGE_MAP(CUserAccountsDlg, CDialog)
    ON_BN_CLICKED(IDC_BUTTON_NEW, &CUserAccountsDlg::OnBnClickedButtonNew)
    ON_BN_CLICKED(IDC_BUTTON_ADD, &CUserAccountsDlg::OnBnClickedButtonAdd)
    ON_BN_CLICKED(IDC_BUTTON_DEL, &CUserAccountsDlg::OnBnClickedButtonDel)
    ON_LBN_SELCHANGE(IDC_LIST_ACCOUNTS, &CUserAccountsDlg::OnLbnSelchangeListAccounts)
    ON_EN_CHANGE(IDC_EDIT_USERNAME, &CUserAccountsDlg::OnEnChangeEditUsername)
    ON_EN_CHANGE(IDC_EDIT_PASSWORD, &CUserAccountsDlg::OnEnChangeEditPassword)
    ON_BN_CLICKED(IDC_RADIO_DEFAULTUSER, &CUserAccountsDlg::OnBnClickedRadioDefaultuser)
    ON_BN_CLICKED(IDC_RADIO_ADMIN, &CUserAccountsDlg::OnBnClickedRadioAdmin)
    ON_WM_SIZE()
END_MESSAGE_MAP()


// CUserAccountsDlg message handlers


void CUserAccountsDlg::OnBnClickedButtonNew()
{
    UserAccount useraccount;
    ZERO_STRUCT(useraccount);
    useraccount.uUserType = USERTYPE_DEFAULT;
    useraccount.uUserRights = USERRIGHT_DEFAULT;
    useraccount.nAudioCodecBpsLimit = DEFAULT_AUDIOCODEC_BPS_LIMIT;
    ShowUserAccount(useraccount);
}

void CUserAccountsDlg::OnBnClickedButtonAdd()
{
    CString szUsername, szChannel;
    m_wndUsername.GetWindowText(szUsername);
    szUsername.Trim();
    m_wndUsername.SetWindowText(szUsername);
    
    UserAccount account = {};
    m_wndUsername.GetWindowText(account.szUsername, TT_STRLEN);
    m_wndPassword.GetWindowText(account.szPassword, TT_STRLEN);

    if(_tcslen(account.szUsername) == 0 &&
       MessageBox(LoadText(IDS_USERACCOUNTCREATEANONYMOUS, _T("Create anonymous user account?")), 
                  LoadText(IDS_USERACCOUNTADDUPDATE, _T("Add/Update User Account")), MB_YESNO) != IDYES)
        return;

    account.uUserType = (m_wndAdminUser.GetCheck() == BST_CHECKED)? USERTYPE_ADMIN : USERTYPE_DEFAULT;
    if(m_wndDoubleLogin.GetCheck() == BST_CHECKED)
        account.uUserRights |= USERRIGHT_MULTI_LOGIN;
    if(m_wndChangeNickname.GetCheck() == BST_UNCHECKED)
        account.uUserRights |= USERRIGHT_LOCKED_NICKNAME;
    if(m_wndViewAllUsers.GetCheck() == BST_CHECKED)
        account.uUserRights |= USERRIGHT_VIEW_ALL_USERS;
    if(m_wndPermChannels.GetCheck() == BST_CHECKED)
        account.uUserRights |= USERRIGHT_MODIFY_CHANNELS;
    if(m_wndTempChannels.GetCheck() == BST_CHECKED)
        account.uUserRights |= USERRIGHT_CREATE_TEMPORARY_CHANNEL;
    if(m_wndUserBcast.GetCheck() == BST_CHECKED)
        account.uUserRights |= USERRIGHT_TEXTMESSAGE_BROADCAST;
    if(m_wndKickUsers.GetCheck() == BST_CHECKED)
        account.uUserRights |= USERRIGHT_KICK_USERS;
    if(m_wndBanUsers.GetCheck() == BST_CHECKED)
        account.uUserRights |= USERRIGHT_BAN_USERS;
    if(m_wndMoveUsers.GetCheck() == BST_CHECKED)
        account.uUserRights |= USERRIGHT_MOVE_USERS;
    if(m_wndChannelOp.GetCheck() == BST_CHECKED)
        account.uUserRights |= USERRIGHT_OPERATOR_ENABLE;
    if(m_wndUploadFiles.GetCheck() == BST_CHECKED)
        account.uUserRights |= USERRIGHT_UPLOAD_FILES;
    if(m_wndDownloadFiles.GetCheck() == BST_CHECKED)
        account.uUserRights |= USERRIGHT_DOWNLOAD_FILES;
    if(m_wndSrvProp.GetCheck() == BST_CHECKED)
        account.uUserRights |= USERRIGHT_UPDATE_SERVERPROPERTIES;
    if(m_wndTransmitVoice.GetCheck() == BST_CHECKED)
        account.uUserRights |= USERRIGHT_TRANSMIT_VOICE;
    if(m_wndTransmitVideo.GetCheck() == BST_CHECKED)
        account.uUserRights |= USERRIGHT_TRANSMIT_VIDEOCAPTURE;
    if(m_wndTransmitAudFiles.GetCheck() == BST_CHECKED)
        account.uUserRights |= USERRIGHT_TRANSMIT_MEDIAFILE_AUDIO;
    if(m_wndTransmitVidFiles.GetCheck() == BST_CHECKED)
        account.uUserRights |= USERRIGHT_TRANSMIT_MEDIAFILE_VIDEO;
    if(m_wndTransmitDesktops.GetCheck() == BST_CHECKED)
        account.uUserRights |= USERRIGHT_TRANSMIT_DESKTOP;
    if(m_wndTransmitDesktopInput.GetCheck() == BST_CHECKED)
        account.uUserRights |= USERRIGHT_TRANSMIT_DESKTOPINPUT;
    if(m_wndRecordVoice.GetCheck() == BST_CHECKED)
        account.uUserRights |= USERRIGHT_RECORD_VOICE;

    m_wndInitChannel.GetWindowText(szChannel);
    szChannel.Trim();
    m_wndInitChannel.SetWindowText(szChannel);
    m_wndInitChannel.GetWindowText(account.szInitChannel, TT_STRLEN);
    
    m_wndNote.GetWindowText(account.szNote, TT_STRLEN);

    for(int i=0;i<TT_CHANNELS_OPERATOR_MAX;i++)
    {
        if(i<m_wndChanOpTab.m_wndSelChannels.GetCount())
        {
            ASSERT(m_wndChanOpTab.m_wndSelChannels.GetItemData(i));
            account.autoOperatorChannels[i] = INT32(m_wndChanOpTab.m_wndSelChannels.GetItemData(i));
        }
        else
            account.autoOperatorChannels[i] = 0;
    }

    account.nAudioCodecBpsLimit = GetWindowNumber(m_wndCodecTab.m_wndBitrate) * 1000;

    account.abusePrevent = m_wndAbuseTab.m_abuse;

    TT_DoNewUserAccount(ttInst, &account);

    for(size_t i=0;i<m_accounts.size();i++)
    {
        if(CString(m_accounts[i].szUsername) == account.szUsername)
        {
            m_accounts.erase(m_accounts.begin()+i);
            break;
        }
    }
    m_accounts.push_back(account);

    OnBnClickedButtonNew();

    ListAccounts();
    UpdateControls();
}

void CUserAccountsDlg::OnBnClickedButtonDel()
{
    int i = m_wndAccounts.GetCurSel();
    CString szUsername;
    m_wndAccounts.GetText(i, szUsername);
    m_wndUsername.GetWindowText(szUsername);

    TT_DoDeleteUserAccount(ttInst, szUsername);

    for(size_t i=0;i<m_accounts.size();i++)
        if(szUsername == m_accounts[i].szUsername)
        {
            m_accounts.erase(m_accounts.begin()+i);
            break;
        }
    ListAccounts();
    UpdateControls();
}

void CUserAccountsDlg::OnLbnSelchangeListAccounts()
{
    int index = m_wndAccounts.GetCurSel();
    if(index == LB_ERR)
        return;

    size_t i = GetItemData(m_wndAccounts, 0);
    ShowUserAccount(m_accounts[i]);
}

void CUserAccountsDlg::OnEnChangeEditUsername()
{
    UpdateControls();
}

void CUserAccountsDlg::OnEnChangeEditPassword()
{
    UpdateControls();
}

void CUserAccountsDlg::UpdateControls()
{
    CString s;
    m_wndUsername.GetWindowText(s);
    m_wndPassword.GetWindowText(s);

    CString szUsername;
    m_wndUsername.GetWindowText(szUsername);

    BOOL bWrite = m_uad == UAD_READWRITE;
    BOOL bCheck = m_wndDefaultUser.GetCheck() == BST_CHECKED && bWrite;
    m_wndDoubleLogin.EnableWindow(bCheck);
    m_wndChangeNickname.EnableWindow(bCheck);
    m_wndViewAllUsers.EnableWindow(bCheck);
    m_wndPermChannels.EnableWindow(bCheck);
    m_wndTempChannels.EnableWindow(bCheck);
    m_wndUserBcast.EnableWindow(bCheck);
    m_wndKickUsers.EnableWindow(bCheck);
    m_wndBanUsers.EnableWindow(bCheck);
    m_wndMoveUsers.EnableWindow(bCheck);
    m_wndChannelOp.EnableWindow(bCheck);
    m_wndUploadFiles.EnableWindow(bCheck);
    m_wndDownloadFiles.EnableWindow(bCheck);
    m_wndSrvProp.EnableWindow(bCheck);
    m_wndTransmitVoice.EnableWindow(bCheck);
    m_wndTransmitVideo.EnableWindow(bCheck);
    m_wndTransmitAudFiles.EnableWindow(bCheck);
    m_wndTransmitVidFiles.EnableWindow(bCheck);
    m_wndTransmitDesktops.EnableWindow(bCheck);
    m_wndTransmitDesktopInput.EnableWindow(bCheck);
    m_wndRecordVoice.EnableWindow(bCheck);

    m_wndUsername.SetReadOnly(!bWrite);
    m_wndPassword.SetReadOnly(!bWrite || szUsername == WEBLOGIN_FACEBOOK_USERNAME || EndsWith(szUsername, WEBLOGIN_FACEBOOK_USERNAMEPOSTFIX));
    m_wndNote.SetReadOnly(!bWrite);
    m_wndAdminUser.EnableWindow(bWrite);
    m_wndDefaultUser.EnableWindow(bWrite);
    m_wndInitChannel.EnableWindow(bWrite);
    m_wndChanOpTab.m_btnAddChan.EnableWindow(bWrite);
    m_wndChanOpTab.m_btnRmChan.EnableWindow(bWrite);
    m_wndCodecTab.m_wndBitrate.EnableWindow(bWrite);
    m_wndAbuseTab.m_wndCmdLimit.EnableWindow(bWrite);

    m_btnNew.EnableWindow(bWrite);
    m_btnAdd.EnableWindow(bWrite);
    m_btnDel.EnableWindow(m_wndAccounts.GetCurSel() >= 0 && bWrite);
}

void CUserAccountsDlg::ShowUserAccount(const UserAccount& useraccount)
{
    m_wndUsername.SetWindowText(useraccount.szUsername);
    m_wndPassword.SetWindowText(useraccount.szPassword);
    m_wndAdminUser.SetCheck((useraccount.uUserType & USERTYPE_ADMIN)?BST_CHECKED:BST_UNCHECKED);
    m_wndDefaultUser.SetCheck((useraccount.uUserType & USERTYPE_DEFAULT)?BST_CHECKED:BST_UNCHECKED);

    m_wndDoubleLogin.SetCheck((useraccount.uUserRights & USERRIGHT_MULTI_LOGIN)?BST_CHECKED:BST_UNCHECKED);
    m_wndChangeNickname.SetCheck((useraccount.uUserRights & USERRIGHT_LOCKED_NICKNAME) ? BST_UNCHECKED : BST_CHECKED);
    m_wndViewAllUsers.SetCheck((useraccount.uUserRights & USERRIGHT_VIEW_ALL_USERS)?BST_CHECKED:BST_UNCHECKED);
    m_wndPermChannels.SetCheck((useraccount.uUserRights & USERRIGHT_MODIFY_CHANNELS)?BST_CHECKED:BST_UNCHECKED);
    m_wndTempChannels.SetCheck((useraccount.uUserRights & USERRIGHT_CREATE_TEMPORARY_CHANNEL)?BST_CHECKED:BST_UNCHECKED);
    m_wndUserBcast.SetCheck((useraccount.uUserRights & USERRIGHT_TEXTMESSAGE_BROADCAST)?BST_CHECKED:BST_UNCHECKED);
    m_wndKickUsers.SetCheck((useraccount.uUserRights & USERRIGHT_KICK_USERS)?BST_CHECKED:BST_UNCHECKED);
    m_wndBanUsers.SetCheck((useraccount.uUserRights & USERRIGHT_BAN_USERS)?BST_CHECKED:BST_UNCHECKED);
    m_wndMoveUsers.SetCheck((useraccount.uUserRights & USERRIGHT_MOVE_USERS)?BST_CHECKED:BST_UNCHECKED);
    m_wndChannelOp.SetCheck((useraccount.uUserRights & USERRIGHT_OPERATOR_ENABLE)?BST_CHECKED:BST_UNCHECKED);
    m_wndUploadFiles.SetCheck((useraccount.uUserRights & USERRIGHT_UPLOAD_FILES)?BST_CHECKED:BST_UNCHECKED);
    m_wndDownloadFiles.SetCheck((useraccount.uUserRights & USERRIGHT_DOWNLOAD_FILES)?BST_CHECKED:BST_UNCHECKED);
    m_wndSrvProp.SetCheck((useraccount.uUserRights & USERRIGHT_UPDATE_SERVERPROPERTIES)?BST_CHECKED:BST_UNCHECKED);
    m_wndTransmitVoice.SetCheck((useraccount.uUserRights & USERRIGHT_TRANSMIT_VOICE)?BST_CHECKED:BST_UNCHECKED);
    m_wndTransmitVideo.SetCheck((useraccount.uUserRights & USERRIGHT_TRANSMIT_VIDEOCAPTURE)?BST_CHECKED:BST_UNCHECKED);
    m_wndTransmitAudFiles.SetCheck((useraccount.uUserRights & USERRIGHT_TRANSMIT_MEDIAFILE_AUDIO)?BST_CHECKED:BST_UNCHECKED);
    m_wndTransmitVidFiles.SetCheck((useraccount.uUserRights & USERRIGHT_TRANSMIT_MEDIAFILE_VIDEO)?BST_CHECKED:BST_UNCHECKED);
    m_wndTransmitDesktops.SetCheck((useraccount.uUserRights & USERRIGHT_TRANSMIT_DESKTOP)?BST_CHECKED:BST_UNCHECKED);
    m_wndTransmitDesktopInput.SetCheck((useraccount.uUserRights & USERRIGHT_TRANSMIT_DESKTOPINPUT) ? BST_CHECKED : BST_UNCHECKED);
    m_wndRecordVoice.SetCheck((useraccount.uUserRights & USERRIGHT_RECORD_VOICE)?BST_CHECKED:BST_UNCHECKED);

    m_wndNote.SetWindowText(useraccount.szNote);
    int nChan = m_wndInitChannel.FindString(-1, useraccount.szInitChannel);
    if(nChan>=0)
        m_wndInitChannel.SetCurSel(nChan);
    else
        m_wndInitChannel.SetCurSel(0);

    // Channel Operator - tab control
    m_wndChanOpTab.m_wndSelChannels.ResetContent();
    TTCHAR szChannel[TT_STRLEN];
    for(int i=0;i<TT_CHANNELS_OPERATOR_MAX;i++)
    {
        if(TT_GetChannelPath(ttInst,
                             useraccount.autoOperatorChannels[i], 
                             szChannel))
        {
             int ii = m_wndChanOpTab.m_wndSelChannels.AddString(szChannel);
             m_wndChanOpTab.m_wndSelChannels.SetItemData(ii, useraccount.autoOperatorChannels[i]);
        }
    }

    // Audio Codec - tab control
    SetWindowNumber(m_wndCodecTab.m_wndBitrate, useraccount.nAudioCodecBpsLimit / 1000);

    // Abuse - tab control
    m_wndAbuseTab.m_abuse = useraccount.abusePrevent;
    m_wndAbuseTab.ShowAbuseInfo();

    UpdateControls();
}

void CUserAccountsDlg::ListAccounts()
{
    CString szAnonymous; szAnonymous.LoadString(IDS_ANONYMOUS);
    TRANSLATE_ITEM(IDS_ANONYMOUS, szAnonymous);
    CString szFmt; szFmt.Format(_T("<%s>"), szAnonymous);
    m_wndAccounts.ResetContent();
    for(size_t i=0;i<m_accounts.size();i++)
    {
        int pos;
        if(_tcslen(m_accounts[i].szUsername) == 0)
        {
            pos = m_wndAccounts.AddString(szFmt);
        }
        else
        {
            pos = m_wndAccounts.AddString(m_accounts[i].szUsername);
        }
        m_wndAccounts.SetItemData(pos, i);
    }
}

void CUserAccountsDlg::OnBnClickedRadioDefaultuser()
{
    UpdateControls();
}


void CUserAccountsDlg::OnBnClickedRadioAdmin()
{
    UpdateControls();
}


void CUserAccountsDlg::OnSize(UINT nType, int cx, int cy)
{
    CDialog::OnSize(nType, cx, cy);
}

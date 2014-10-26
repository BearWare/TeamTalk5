// UserAccountsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "../Resource.h"
#include "UserAccountsDlg.h"
#include <vector>

using namespace std;

extern TTInstance* ttInst;

// CUserAccountsDlg dialog

IMPLEMENT_DYNAMIC(CUserAccountsDlg, CDialog)

CUserAccountsDlg::CUserAccountsDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CUserAccountsDlg::IDD, pParent)
    , m_nBitrate(DEFAULT_AUDIOCODEC_BPS_LIMIT)
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
    DDX_Control(pDX, IDC_LIST_AVAILCHANNELS, m_wndAvailChannels);
    DDX_Control(pDX, IDC_LIST_SELECTEDCHANNELS, m_wndSelChannels);
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
    DDX_Control(pDX, IDC_EDIT_BITRATE, m_wndBitrate);
    DDX_Text(pDX, IDC_EDIT_BITRATE, m_nBitrate);
}


BOOL CUserAccountsDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    TRANSLATE(*this, IDD);

    m_wndInitChannel.AddString(_T(""));

    int nCount = 0;
    TT_GetServerChannels(ttInst, NULL, &nCount);
    if(nCount)
    {
        vector<Channel> channels;
        channels.resize(nCount);
        TT_GetServerChannels(ttInst, &channels[0], &nCount);
        for(size_t i=0;i<nCount;i++)
        {
            TTCHAR szPath[TT_STRLEN];
            if((channels[i].uChannelType & CHANNEL_PERMANENT) &&
               TT_GetChannelPath(ttInst, channels[i].nChannelID, szPath))
            {
                int index = m_wndAvailChannels.AddString(szPath);
                m_wndAvailChannels.SetItemData(index, channels[i].nChannelID);
                index = m_wndInitChannel.AddString(szPath);
                m_wndInitChannel.SetItemData(index, channels[i].nChannelID);
            }
        }
    }
    
    ListAccounts();

    OnBnClickedButtonNew();

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
    ON_BN_CLICKED(IDC_BUTTON_ADDCHANNEL, &CUserAccountsDlg::OnBnClickedButtonAddchannel)
    ON_BN_CLICKED(IDC_BUTTON_DELCHANNEL, &CUserAccountsDlg::OnBnClickedButtonDelchannel)
    ON_BN_CLICKED(IDC_RADIO_DEFAULTUSER, &CUserAccountsDlg::OnBnClickedRadioDefaultuser)
    ON_BN_CLICKED(IDC_RADIO_ADMIN, &CUserAccountsDlg::OnBnClickedRadioAdmin)
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
    UserAccount account = {0};
    m_wndUsername.GetWindowText(account.szUsername, TT_STRLEN);
    m_wndPassword.GetWindowText(account.szPassword, TT_STRLEN);

    if(_tcslen(account.szUsername) == 0 &&
       MessageBox(_T("Create anonymous user account?"), 
                  _T("Add/Update User Account"), MB_YESNO) != IDYES)
        return;

    account.uUserType = (m_wndAdminUser.GetCheck() == BST_CHECKED)? USERTYPE_ADMIN : USERTYPE_DEFAULT;
    if(m_wndDoubleLogin.GetCheck() == BST_CHECKED)
        account.uUserRights |= USERRIGHT_MULTI_LOGIN;
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

    m_wndInitChannel.GetWindowText(account.szInitChannel, TT_STRLEN);
    m_wndNote.GetWindowText(account.szNote, TT_STRLEN);

    for(int i=0;i<TT_CHANNELS_OPERATOR_MAX;i++)
    {
        if(i<m_wndSelChannels.GetCount())
        {
            ASSERT(m_wndSelChannels.GetItemData(i));
            account.autoOperatorChannels[i] = m_wndSelChannels.GetItemData(i);
        }
        else
            account.autoOperatorChannels[i] = 0;
    }

    account.nAudioCodecBpsLimit = GetWindowNumber(m_wndBitrate) * 1000;

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

    m_wndUsername.SetWindowText(_T(""));
    m_wndPassword.SetWindowText(_T(""));
    m_wndDefaultUser.SetCheck(BST_CHECKED);

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

    const UserAccount& account = m_accounts[index];
    ShowUserAccount(account);
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
    BOOL bEnabled = TRUE;
    m_wndUsername.GetWindowText(s);
    m_wndPassword.GetWindowText(s);

    CString szUsername;
    m_wndUsername.GetWindowText(szUsername);

    m_wndDoubleLogin.EnableWindow(m_wndDefaultUser.GetCheck() == BST_CHECKED);
    m_wndViewAllUsers.EnableWindow(m_wndDefaultUser.GetCheck() == BST_CHECKED);
    m_wndPermChannels.EnableWindow(m_wndDefaultUser.GetCheck() == BST_CHECKED);
    m_wndTempChannels.EnableWindow(m_wndDefaultUser.GetCheck() == BST_CHECKED);
    m_wndUserBcast.EnableWindow(m_wndDefaultUser.GetCheck() == BST_CHECKED);
    m_wndKickUsers.EnableWindow(m_wndDefaultUser.GetCheck() == BST_CHECKED);
    m_wndBanUsers.EnableWindow(m_wndDefaultUser.GetCheck() == BST_CHECKED);
    m_wndMoveUsers.EnableWindow(m_wndDefaultUser.GetCheck() == BST_CHECKED);
    m_wndChannelOp.EnableWindow(m_wndDefaultUser.GetCheck() == BST_CHECKED);
    m_wndUploadFiles.EnableWindow(m_wndDefaultUser.GetCheck() == BST_CHECKED);
    m_wndDownloadFiles.EnableWindow(m_wndDefaultUser.GetCheck() == BST_CHECKED);
    m_wndSrvProp.EnableWindow(m_wndDefaultUser.GetCheck() == BST_CHECKED);
    m_wndTransmitVoice.EnableWindow(m_wndDefaultUser.GetCheck() == BST_CHECKED);
    m_wndTransmitVideo.EnableWindow(m_wndDefaultUser.GetCheck() == BST_CHECKED);
    m_wndTransmitAudFiles.EnableWindow(m_wndDefaultUser.GetCheck() == BST_CHECKED);
    m_wndTransmitVidFiles.EnableWindow(m_wndDefaultUser.GetCheck() == BST_CHECKED);
    m_wndTransmitDesktops.EnableWindow(m_wndDefaultUser.GetCheck() == BST_CHECKED);
    m_wndTransmitDesktopInput.EnableWindow(m_wndDefaultUser.GetCheck() == BST_CHECKED);

    m_btnAdd.EnableWindow(bEnabled);
    m_btnDel.EnableWindow(m_wndAccounts.GetCurSel()>=0);
}

void CUserAccountsDlg::ShowUserAccount(const UserAccount& useraccount)
{
    m_wndUsername.SetWindowText(useraccount.szUsername);
    m_wndPassword.SetWindowText(useraccount.szPassword);
    m_wndAdminUser.SetCheck((useraccount.uUserType & USERTYPE_ADMIN)?BST_CHECKED:BST_UNCHECKED);
    m_wndDefaultUser.SetCheck((useraccount.uUserType & USERTYPE_DEFAULT)?BST_CHECKED:BST_UNCHECKED);

    m_wndDoubleLogin.SetCheck((useraccount.uUserRights & USERRIGHT_MULTI_LOGIN)?BST_CHECKED:BST_UNCHECKED);
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
    m_wndTransmitDesktopInput.SetCheck((useraccount.uUserRights & USERRIGHT_TRANSMIT_DESKTOPINPUT)?BST_CHECKED:BST_UNCHECKED);

    m_wndNote.SetWindowText(useraccount.szNote);
    SetWindowNumber(m_wndBitrate, useraccount.nAudioCodecBpsLimit / 1000);
    int nChan = m_wndInitChannel.FindString(-1, useraccount.szInitChannel);
    if(nChan>=0)
        m_wndInitChannel.SetCurSel(nChan);
    else
        m_wndInitChannel.SetCurSel(0);
    m_wndSelChannels.ResetContent();
    TTCHAR szChannel[TT_STRLEN];
    for(int i=0;i<TT_CHANNELS_OPERATOR_MAX;i++)
    {
        if(TT_GetChannelPath(ttInst,
                             useraccount.autoOperatorChannels[i], 
                             szChannel))
        {
             int ii = m_wndSelChannels.AddString(szChannel);
             m_wndSelChannels.SetItemData(ii, useraccount.autoOperatorChannels[i]);
        }
    }
    UpdateControls();
}

void CUserAccountsDlg::ListAccounts()
{
    m_wndAccounts.ResetContent();
    for(size_t i=0;i<m_accounts.size();i++)
    {
        m_wndAccounts.AddString(m_accounts[i].szUsername);
    }
}

void CUserAccountsDlg::OnBnClickedButtonAddchannel()
{
    int i = m_wndAvailChannels.GetCurSel();
    if(i == LB_ERR)
        return;
    int nItemData = m_wndAvailChannels.GetItemData(i);
    CString szChan;
    m_wndAvailChannels.GetText(i, szChan);
    i = m_wndSelChannels.AddString(szChan);
    m_wndSelChannels.SetItemData(i, nItemData);
}

void CUserAccountsDlg::OnBnClickedButtonDelchannel()
{
    int i = m_wndSelChannels.GetCurSel();
    if(i == LB_ERR)
        return;
    m_wndSelChannels.DeleteString(i);
}


void CUserAccountsDlg::OnBnClickedRadioDefaultuser()
{
    UpdateControls();
}


void CUserAccountsDlg::OnBnClickedRadioAdmin()
{
    UpdateControls();
}

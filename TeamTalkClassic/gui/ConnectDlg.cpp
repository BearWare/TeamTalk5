// ConnectDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "ConnectDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CConnectDlg dialog


CConnectDlg::CConnectDlg(CWnd* pParent /*=NULL*/)
    : CDialog(CConnectDlg::IDD, pParent)
    , m_nTcpPort(DEFAULT_TEAMTALK_TCPPORT)
    , m_nUdpPort(DEFAULT_TEAMTALK_UDPPORT)
    , m_szPassword(_T(""))
    , m_szHostAddress(_T(""))
    , m_szChPasswd(_T(""))
    , m_szChannel(_T(""))
    , m_bEncrypted(FALSE)
{
    //{{AFX_DATA_INIT(CConnectDlg)
    //}}AFX_DATA_INIT
}


void CConnectDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_STATIC_GRPCONNECT, m_wndGroupBox);
    DDX_Control(pDX, IDC_HOSTSOUNDPORT_EDIT, m_wndHostSoundPort);
    DDX_Control(pDX, IDC_STATIC_DEFHOST, m_wndDefHostText);
    DDX_Control(pDX, IDC_STATIC_DEFSOUND, m_wndDefSoundText);
    DDX_Control(pDX, IDC_STATIC_HOSTPORT, m_wndHostPortText);
    DDX_Control(pDX, IDC_HOSTPORT_EDIT, m_wndHostPort);
    DDX_Control(pDX, IDC_STATIC_SOUNDPORT, m_wndHostSoundPortText);
    DDX_Control(pDX, IDOK, m_btnOK);
    DDX_Control(pDX, IDCANCEL, m_btnCancel);
    DDX_Text(pDX, IDC_HOSTPORT_EDIT, m_nTcpPort);
    DDX_Text(pDX, IDC_HOSTSOUNDPORT_EDIT, m_nUdpPort);
    DDX_Control(pDX, IDC_COMBO_HOSTADDRESS, m_wndHostAddress);
    DDX_CBString(pDX, IDC_COMBO_HOSTADDRESS, m_szHostAddress);
    DDV_MinMaxUInt(pDX, m_nTcpPort, 1, 65535);
    DDV_MinMaxUInt(pDX, m_nUdpPort, 1, 65535);
    DDX_Control(pDX, IDC_STATIC_GRPJOINCHAN, m_wndGrpJoinChan);
    DDX_Text(pDX, IDC_EDIT_USERNAME, m_szUsername);
    DDX_Text(pDX, IDC_EDIT_PASSWORD, m_szPassword);
    DDX_Control(pDX, IDC_EDIT_USERNAME, m_wndUsername);
    DDX_Control(pDX, IDC_EDIT_PASSWORD, m_wndPassword);
    DDX_Text(pDX, IDC_EDIT_CHPASSWD, m_szChPasswd);
    DDX_Text(pDX, IDC_EDIT_CHANNEL, m_szChannel);
    DDX_Control(pDX, IDC_EDIT_CHANNEL, m_wndChannel);
    DDX_Control(pDX, IDC_EDIT_CHPASSWD, m_wndChPasswd);
    DDX_Check(pDX, IDC_CHECK_ENCRYPTED, m_bEncrypted);
    DDX_Control(pDX, IDC_CHECK_ENCRYPTED, m_wndEncrypted);
}

void CConnectDlg::DisplayHosts()
{
    m_wndHostAddress.ResetContent();

    for(int i=0;i<m_vecHosts.size();i++)
        m_wndHostAddress.AddString( STR_UTF8(m_vecHosts[i].szAddress.c_str()));

    if(m_vecHosts.size()>0)
    {
        m_wndHostAddress.SetCurSel(0);
        OnCbnSelchangeComboHostaddress();
        /*
        m_wndSrvPasswd.SetWindowText( STR_UTF8(m_vecHosts[0].szSrvPasswd.c_str()) );
        CString s;s.Format(_T("%d"),m_vecHosts[0].nTcpPort);
        m_nTcpPort = m_vecHosts[0].nTcpPort;
        m_wndHostPort.SetWindowText(s);
        s.Format(_T("%d"),m_vecHosts[0].nUdpPort);
        m_nUdpPort = m_vecHosts[0].nUdpPort;
        m_wndHostSoundPort.SetWindowText(s);
        m_wndChannel.SetWindowText(STR_UTF8(m_vecHosts[0].szChannel.c_str()));
        m_wndChPasswd.SetWindowText(STR_UTF8(m_vecHosts[0].szChPasswd.c_str()));
        */
    }
}

BEGIN_MESSAGE_MAP(CConnectDlg, CDialog)
    //{{AFX_MSG_MAP(CConnectDlg)
        // NOTE: the ClassWizard will add message map macros here
    //}}AFX_MSG_MAP
    ON_CBN_SELCHANGE(IDC_COMBO_HOSTADDRESS, OnCbnSelchangeComboHostaddress)
    ON_STN_CLICKED(IDC_STATIC_DEFSOUND, OnStnClickedStaticDefsound)
    ON_BN_CLICKED(IDOK, OnBnClickedOk)
    ON_CBN_EDITUPDATE(IDC_COMBO_HOSTADDRESS, OnCbnEditupdateComboHostaddress)
    ON_BN_CLICKED(IDC_BUTTON_DELENTRY, &CConnectDlg::OnBnClickedButtonDelentry)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CConnectDlg message handlers

BOOL CConnectDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    TRANSLATE(*this, IDD);
#ifndef ENABLE_ENCRYPTION
    m_wndEncrypted.ShowWindow(SW_HIDE);
#endif

    DisplayHosts();

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

void CConnectDlg::OnCbnSelchangeComboHostaddress()
{
    int index = m_wndHostAddress.GetCurSel();
    if(index != CB_ERR && index<m_vecHosts.size())
    {
        CString s;s.Format(_T("%d"),m_vecHosts[index].nTcpPort);
        m_wndHostPort.SetWindowText(s);
        s.Format(_T("%d"),m_vecHosts[index].nUdpPort);
        m_wndHostSoundPort.SetWindowText(s);
        m_wndEncrypted.SetCheck(m_vecHosts[index].bEncrypted?BST_CHECKED:BST_UNCHECKED);
        m_wndUsername.SetWindowText(STR_UTF8(m_vecHosts[index].szUsername.c_str()));
        m_wndPassword.SetWindowText(STR_UTF8(m_vecHosts[index].szPassword.c_str()));
        m_wndChannel.SetWindowText(STR_UTF8(m_vecHosts[index].szChannel.c_str()));
        m_wndChPasswd.SetWindowText(STR_UTF8(m_vecHosts[index].szChPasswd.c_str()));
    }
}

void CConnectDlg::OnStnClickedStaticDefsound()
{
}

void CConnectDlg::OnBnClickedOk()
{
    CString s;
    m_wndHostAddress.GetWindowText(s);
    if(s.GetLength()>0)
        OnOK();
    else
        AfxMessageBox(_T("Please provide the host's IP-address"));
}

void CConnectDlg::OnCbnEditupdateComboHostaddress()
{
    //m_wndPassword.SetWindowText(_T(""));
    //CString s;s.Format("%d",DEFAULT_TEAMTALK_TCPPORT);
    //m_nTcpPort = DEFAULT_TEAMTALK_TCPPORT;
    //m_wndHostPort.SetWindowText(s);
    //s.Format("%d",DEFAULT_TEAMTALK_UDPPORT);
    //m_nUdpPort = DEFAULT_TEAMTALK_UDPPORT;
    //m_wndHostSoundPort.SetWindowText(s);
}

void CConnectDlg::OnBnClickedButtonDelentry()
{
    int index = m_wndHostAddress.GetCurSel();
    if(index == CB_ERR)
        return;
    if(index < m_vecHosts.size())
    {
        m_delHosts.push_back(m_vecHosts[index]);
        m_vecHosts.erase(m_vecHosts.begin()+index);
    }
    DisplayHosts();
}

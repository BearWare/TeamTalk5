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

// HostManagerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "HostManagerDlg.h"
#include "TeamTalkDlg.h"
#include "GenerateTTFileDlg.h"
#include "BearWareLoginDlg.h"
#include "AppInfo.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace teamtalk;
using namespace std;

enum
{
    TIMER_HTTPREQUEST_SERVERLIST_UPDATE_ID,
    TIMER_HTTPREQUEST_SERVERLIST_TIMEOUT_ID,

    TIMER_HTTPREQUEST_BEARWARE_LOGIN_UPDATE_ID,
    TIMER_HTTPREQUEST_BEARWARE_LOGIN_TIMEOUT_ID,
};

#define PUBSERVER_ITEMDATA 0x80000000

/////////////////////////////////////////////////////////////////////////////
// CHostManagerDlg dialog

CHostManagerDlg::CHostManagerDlg(ClientXML* xmlSettings, CWnd* pParent /*=NULL*/)
    : CDialog(CHostManagerDlg::IDD, pParent)
    , m_szChannel(_T(""))
    , m_szChPassword(_T(""))
    , m_bPubServers(FALSE)
    , m_bEncrypted(FALSE)
{
    //{{AFX_DATA_INIT(CHostManagerDlg)
    m_szEntryName = _T("");
    m_nTcpPort = 0;
    m_nUdpPort = 0;
    //}}AFX_DATA_INIT
    m_pSettings = xmlSettings;
    
    m_bPubServers = m_pSettings->GetShowPublicServers();
}

CHostManagerDlg::~CHostManagerDlg()
{
    m_pSettings->SetShowPublicServers(m_bPubServers);
}

void CHostManagerDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CHostManagerDlg)
    DDX_Control(pDX, IDC_BUTTON_DELETE, m_wndDelete);
    DDX_Control(pDX, IDC_LIST_HOSTS, m_wndHosts);
    DDX_Control(pDX, IDC_EDIT_HOSTUDPPORT, m_wndHostUdpPort);
    DDX_Control(pDX, IDC_EDIT_HOSTTCPPORT, m_wndHostPort);
    DDX_Control(pDX, IDC_EDIT_ENTRYNAME, m_wndEntryName);
    DDX_Text(pDX, IDC_EDIT_ENTRYNAME, m_szEntryName);
    DDX_Text(pDX, IDC_EDIT_HOSTTCPPORT, m_nTcpPort);
    DDV_MinMaxUInt(pDX, m_nTcpPort, 1, 65535);
    DDX_Text(pDX, IDC_EDIT_HOSTUDPPORT, m_nUdpPort);
    DDV_MinMaxUInt(pDX, m_nUdpPort, 1, 65535);
    //}}AFX_DATA_MAP
    DDX_Text(pDX, IDC_EDIT_CHANNEL, m_szChannel);
    DDX_Text(pDX, IDC_EDIT_CHPASSWD, m_szChPassword);
    DDX_Control(pDX, IDC_EDIT_CHANNEL, m_wndChannel);
    DDX_Control(pDX, IDC_EDIT_CHPASSWD, m_wndChPasswd);
    DDX_Text(pDX, IDC_COMBO_USERNAME, m_szUsername);
    DDX_Text(pDX, IDC_EDIT_PASSWORD, m_szPassword);
    DDX_Control(pDX, IDC_COMBO_USERNAME, m_wndUsername);
    DDX_Control(pDX, IDC_EDIT_PASSWORD, m_wndPassword);
    DDX_Control(pDX, IDC_CHECK_PUBLICSERVERS, m_btnPubServers);
    DDX_Check(pDX, IDC_CHECK_PUBLICSERVERS, m_bPubServers);
    DDX_Control(pDX, IDC_CHECK_ENCRYPTED, m_wndEncrypted);
    DDX_Check(pDX, IDC_CHECK_ENCRYPTED, m_bEncrypted);
    DDX_Control(pDX, IDC_COMBO_HOSTADDRESS, m_wndHostAddress);
    DDX_CBString(pDX, IDC_COMBO_HOSTADDRESS, m_szHostAddress);
    DDX_Control(pDX, IDC_BUTTON_IMPORTTTILE, m_wndImportBtn);
}


BEGIN_MESSAGE_MAP(CHostManagerDlg, CDialog)
    //{{AFX_MSG_MAP(CHostManagerDlg)
    ON_BN_CLICKED(IDC_BUTTON_NEW, OnButtonNew)
    ON_BN_CLICKED(IDC_BUTTON_ADDUPDATE, OnButtonAdd)
    ON_BN_CLICKED(IDC_BUTTON_DELETE, OnButtonDelete)
    ON_LBN_SELCHANGE(IDC_LIST_HOSTS, OnSelchangeListHosts)
    //}}AFX_MSG_MAP
    ON_BN_CLICKED(IDC_BUTTON_CONNECT, OnBnClickedOk)
    ON_WM_TIMER()
    ON_BN_CLICKED(IDC_CHECK_PUBLICSERVERS, &CHostManagerDlg::OnBnClickedCheckPublicservers)
    ON_LBN_DBLCLK(IDC_LIST_HOSTS, &CHostManagerDlg::OnLbnDblclkListHosts)
    ON_BN_CLICKED(IDC_BUTTON_GENTT, &CHostManagerDlg::OnBnClickedButtonGentt)
    ON_BN_CLICKED(IDC_BUTTON_DELENTRY, &CHostManagerDlg::OnBnClickedButtonDelentry)
    ON_CBN_SELCHANGE(IDC_COMBO_HOSTADDRESS, &CHostManagerDlg::OnCbnSelchangeComboHostaddress)
    ON_BN_CLICKED(IDC_BUTTON_IMPORTTTILE, &CHostManagerDlg::OnBnClickedButtonImportttile)
    ON_CBN_EDITCHANGE(IDC_COMBO_USERNAME, &CHostManagerDlg::OnCbnEditchangeComboUsername)
//    ON_CBN_SELCHANGE(IDC_COMBO_USERNAME, &CHostManagerDlg::OnCbnSelchangeComboUsername)
//    ON_CBN_EDITUPDATE(IDC_COMBO_USERNAME, &CHostManagerDlg::OnCbnEditupdateComboUsername)
    ON_CBN_KILLFOCUS(IDC_COMBO_USERNAME, &CHostManagerDlg::OnCbnKillfocusComboUsername)
    ON_EN_SETFOCUS(IDC_EDIT_ENTRYNAME, &CHostManagerDlg::OnEnSetfocusEditEntryname)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CHostManagerDlg message handlers

BOOL CHostManagerDlg::OnInitDialog() 
{
    CDialog::OnInitDialog();

    TRANSLATE(*this, IDD);

    m_wndDelete.EnableWindow(FALSE);

    m_wndUsername.AddString(WEBLOGIN_FACEBOOK_USERNAME);

    if(m_bPubServers)
        ShowPublicServers();

    DisplayHosts();

    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}

void CHostManagerDlg::DisplayHosts()
{
    m_wndHostAddress.ResetContent();
    m_wndHosts.ResetContent();

    for(int i=0;i<m_pSettings->GetHostManagerEntryCount();i++)
    {
        HostEntry entry;
        m_pSettings->GetHostManagerEntry(i, entry);
        m_wndHosts.AddString( STR_UTF8( entry.szEntryName.c_str() ));
    }
    
    if(m_wndHosts.GetCount()==0)
        OnButtonNew();

    for(size_t i=0;i<m_vecHosts.size();i++)
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
        m_wndHostUdpPort.SetWindowText(s);
        m_wndChannel.SetWindowText(STR_UTF8(m_vecHosts[0].szChannel.c_str()));
        m_wndChPasswd.SetWindowText(STR_UTF8(m_vecHosts[0].szChPasswd.c_str()));
        */
    }
}

void CHostManagerDlg::ShowFieldError()
{
    AfxMessageBox(LoadText(IDS_HOSTMANAGERFILLALLFIELDS, _T("Please fill in all the required fields.\r\n- Entry name\r\n- Host address\r\n- TCP port\r\n- UDP port\r\n")));
}

BOOL CHostManagerDlg::GetHostEntry(teamtalk::HostEntry& entry)
{
    CString szName, szAddress, szSrvPasswd, szUsername, szPassword, szChannel, szChPasswd;
    m_wndEntryName.GetWindowText(szName);
    entry.szEntryName = STR_UTF8( szName );
    m_wndHostAddress.GetWindowText(szAddress);
    entry.szAddress = STR_UTF8( szAddress );
    entry.bEncrypted = m_wndEncrypted.GetCheck() == BST_CHECKED;
    m_wndUsername.GetWindowText(szUsername);
    entry.szUsername = STR_UTF8(szUsername);
    m_wndPassword.GetWindowText(szPassword);
    entry.szPassword = STR_UTF8(szPassword);
    m_wndChannel.GetWindowText(szChannel);
    entry.szChannel = STR_UTF8(szChannel);
    m_wndChPasswd.GetWindowText(szChPasswd);
    entry.szChPasswd = STR_UTF8(szChPasswd);

    CString tmp;
    m_wndHostPort.GetWindowText(tmp);
    entry.nTcpPort = _ttoi(tmp);
    m_wndHostUdpPort.GetWindowText(tmp);
    entry.nUdpPort = _ttoi(tmp);
    if(entry.szEntryName.size()==0 ||
        entry.szAddress.size()==0 ||
        (entry.nTcpPort<=0 || entry.nTcpPort>65535) ||
        (entry.nUdpPort<=0 || entry.nUdpPort>65535))
    {
        ShowFieldError();
        return FALSE;
    }
    return TRUE;
}

void CHostManagerDlg::OnButtonNew() 
{
    m_wndEntryName.SetWindowText(_T(""));
    m_wndHostAddress.SetWindowText(_T(""));
    CString s;
    s.Format(_T("%d"),DEFAULT_TEAMTALK_UDPPORT);
    m_wndHostUdpPort.SetWindowText(s);
    s.Format(_T("%d"),DEFAULT_TEAMTALK_TCPPORT);
    m_wndHostPort.SetWindowText(s);
    m_wndEncrypted.SetCheck(BST_UNCHECKED);
    m_wndUsername.SetWindowText(_T(""));
    OnCbnEditchangeComboUsername();
    m_wndPassword.SetWindowText(_T(""));
    m_wndChannel.SetWindowText(_T(""));
    m_wndChPasswd.SetWindowText(_T(""));
}

void CHostManagerDlg::OnButtonAdd() 
{
    BOOL found = FALSE;

    HostEntry entry;
    if(!GetHostEntry(entry))
        return;

    for(int i=0;i<m_wndHosts.GetCount();i++)
    {
        CString szListEntry;
        CString szEntry;
        m_wndHosts.GetText(i, szListEntry);
        m_wndEntryName.GetWindowText(szEntry);
        if(szListEntry == szEntry)
        {
            const TCHAR* t = szEntry;
            m_pSettings->RemoveHostManagerEntry( STR_UTF8( t ) );
            found = TRUE;
            break;
        }
    }

    m_pSettings->AddHostManagerEntry(entry);
    if(!found)
    {
        int index = 0;
        while((m_wndHosts.GetItemData(index) & PUBSERVER_ITEMDATA) == 0)index++;
        m_wndHosts.InsertString(index, STR_UTF8( entry.szEntryName.c_str() ) );
    }
}

void CHostManagerDlg::OnButtonDelete() 
{
    int index = m_wndHosts.GetCurSel();
    if(m_wndHosts.GetItemData(index) & PUBSERVER_ITEMDATA)
    {
        MessageBox(LoadText(IDS_HOSTMANAGERCANNOTDELPUBSERV, _T("Cannot delete public servers")), LoadText(IDS_HOSTMANAGERDELHOST, _T("Delete Host")));
        return;
    }

    for(int i=0;i<m_wndHosts.GetCount();i++)
    {
        CString szListEntry;
        CString szEntry;
        m_wndHosts.GetText(i, szListEntry);
        m_wndEntryName.GetWindowText(szEntry);
        if(szListEntry == szEntry)
        {
            const TCHAR* t = szListEntry;
            m_pSettings->RemoveHostManagerEntry( STR_UTF8( t ));
            m_wndHosts.DeleteString(index);
            break;
        }
    }
    if(m_wndHosts.GetCount()>0)
    {
        m_wndHosts.SetCurSel(index<m_wndHosts.GetCount()? index : m_wndHosts.GetCount()-1);
        OnSelchangeListHosts();
    }
}

void CHostManagerDlg::OnSelchangeListHosts() 
{
    int index = m_wndHosts.GetCurSel();
    if(index != LB_ERR)
    {
        BOOL ok = FALSE;
        CString entryname;
        m_wndHosts.GetText(index, entryname);
        HostEntry entry;
        const TCHAR* t = entryname;
        if(m_wndHosts.GetItemData(index) & PUBSERVER_ITEMDATA)
        {
            int first = 0;
            while((m_wndHosts.GetItemData(first) & PUBSERVER_ITEMDATA) == 0)first++;
            index = index - first;
            ASSERT(index < int(m_pubservers.size()));
            if(index < int(m_pubservers.size()))
            {
                entry = m_pubservers[index];
                ok = TRUE;
            }
            m_wndDelete.EnableWindow(FALSE);
        }
        else if(m_pSettings->GetHostManagerEntry( STR_UTF8( t ), entry))
        {
            ok = TRUE;
            m_wndDelete.EnableWindow();
        }
        else ASSERT(0);
        if(ok)
        {
            m_wndEntryName.SetWindowText( STR_UTF8( entry.szEntryName.c_str() ));
            m_wndHostAddress.SetWindowText( STR_UTF8( entry.szAddress.c_str() ));
            CString s;s.Format(_T("%d"),entry.nTcpPort);
            m_wndHostPort.SetWindowText(s);
            s.Format(_T("%d"),entry.nUdpPort);
            m_wndHostUdpPort.SetWindowText(s);
            m_wndEncrypted.SetCheck(entry.bEncrypted?BST_CHECKED:BST_UNCHECKED);
            m_wndUsername.SetWindowText(STR_UTF8(entry.szUsername.c_str()));
            OnCbnEditchangeComboUsername();
            m_wndPassword.SetWindowText(STR_UTF8(entry.szPassword.c_str()));
            m_wndChannel.SetWindowText(STR_UTF8(entry.szChannel.c_str()));
            m_wndChPasswd.SetWindowText(STR_UTF8(entry.szChPasswd.c_str()));
        }
    }
    else
        m_wndDelete.EnableWindow(FALSE);
}

void CHostManagerDlg::OnBnClickedOk()
{
    if(m_wndHostAddress.GetWindowTextLength() == 0 ||
        m_wndHostPort.GetWindowTextLength() == 0 ||
        m_wndHostUdpPort.GetWindowTextLength() == 0)
    {
        ShowFieldError();
    }
    else
    {
        CString szUsername;
        m_wndUsername.GetWindowText(szUsername);

        if (szUsername == WEBLOGIN_BEARWARE_USERNAME)
        {
            std::string username, token;
            m_pSettings->GetBearWareLogin(username, token);

            if (username.empty())
            {
                CBearWareLoginDlg dlg;
                if (dlg.DoModal() == IDOK)
                {
                    szUsername = dlg.m_szUsername;
                    m_pSettings->SetBearWareLogin(STR_UTF8(dlg.m_szUsername), STR_UTF8(dlg.m_szToken));
                }
                else
                    return;
            }
            szUsername = STR_UTF8(username);
            m_wndUsername.SetWindowText(szUsername);
            m_wndPassword.SetWindowText(_T(""));
        }

        OnOK();
    }
}

void CHostManagerDlg::OnTimer(UINT_PTR nIDEvent)
{
    CDialog::OnTimer(nIDEvent);

    switch(nIDEvent)
    {
    case TIMER_HTTPREQUEST_SERVERLIST_UPDATE_ID :
        ASSERT(m_HttpPubServers);
        if(!m_HttpPubServers)
        {
            KillTimer(TIMER_HTTPREQUEST_SERVERLIST_UPDATE_ID);
            break;
        }
        if(m_HttpPubServers->SendReady())
            m_HttpPubServers->Send(_T("<") _T( TT_XML_ROOTNAME ) _T("/>"));
        else if(m_HttpPubServers->ResponseReady())
        {
            CString szResponse = m_HttpPubServers->GetResponse();

            string xml = STR_UTF8(szResponse, szResponse.GetLength()*4);
            TTFile ttfile(TT_XML_ROOTNAME);
            if(ttfile.Parse(xml))
            {
                int i=0;
                HostEntry entry;
                while(ttfile.GetHostEntry(entry, i++))
                {
                    m_pubservers.push_back(entry);
                    int newindex = m_wndHosts.AddString(CString(LoadText(IDS_HOSTMANAGERPUBLIC, _T("Public: "))) 
                        + STR_UTF8(entry.szEntryName.c_str()));
                    m_wndHosts.SetItemData(newindex, PUBSERVER_ITEMDATA);
                }
            }

            KillTimer(TIMER_HTTPREQUEST_SERVERLIST_UPDATE_ID);
            KillTimer(TIMER_HTTPREQUEST_SERVERLIST_TIMEOUT_ID);
            m_HttpPubServers.reset();
        }
        break;
    case TIMER_HTTPREQUEST_SERVERLIST_TIMEOUT_ID :
        KillTimer(TIMER_HTTPREQUEST_SERVERLIST_UPDATE_ID);
        KillTimer(TIMER_HTTPREQUEST_SERVERLIST_TIMEOUT_ID);
        m_HttpPubServers.reset();
        break;
    }
}

void CHostManagerDlg::ShowPublicServers()
{
    KillTimer(TIMER_HTTPREQUEST_SERVERLIST_UPDATE_ID);
    KillTimer(TIMER_HTTPREQUEST_SERVERLIST_TIMEOUT_ID);
    m_HttpPubServers.reset(new CHttpRequest(URL_PUBLICSERVER));
    SetTimer(TIMER_HTTPREQUEST_SERVERLIST_UPDATE_ID, 500, NULL);
    SetTimer(TIMER_HTTPREQUEST_SERVERLIST_TIMEOUT_ID, 5000, NULL);
}

void CHostManagerDlg::OnBnClickedCheckPublicservers()
{
    if(m_btnPubServers.GetCheck() == BST_CHECKED)
    {
        ShowPublicServers();
    }
    else
    {
        for(int i=0;i<m_wndHosts.GetCount();)
        {
            if(m_wndHosts.GetItemData(i) & PUBSERVER_ITEMDATA)
                m_wndHosts.DeleteString(i);
            else i++;
        }
    }
}

void CHostManagerDlg::OnLbnDblclkListHosts()
{
    OnBnClickedOk();
}

void CHostManagerDlg::OnBnClickedButtonGentt()
{
    HostEntry entry;
    if(!GetHostEntry(entry))
        return;

    CGenerateTTFileDlg dlg(this);
    dlg.m_hostentry = entry;
    dlg.DoModal();
}

void CHostManagerDlg::OnBnClickedButtonDelentry()
{
    int index = m_wndHostAddress.GetCurSel();
    if(index == CB_ERR)
        return;
    if(index < int(m_vecHosts.size()))
    {
        m_delHosts.push_back(m_vecHosts[index]);
        m_vecHosts.erase(m_vecHosts.begin()+index);
    }
    DisplayHosts();
}

void CHostManagerDlg::OnCbnSelchangeComboHostaddress()
{
    int index = m_wndHostAddress.GetCurSel();
    if(index != CB_ERR && index < int(m_vecHosts.size()))
    {
        CString s;s.Format(_T("%d"),m_vecHosts[index].nTcpPort);
        m_wndHostPort.SetWindowText(s);
        s.Format(_T("%d"),m_vecHosts[index].nUdpPort);
        m_wndHostUdpPort.SetWindowText(s);
        m_wndEncrypted.SetCheck(m_vecHosts[index].bEncrypted?BST_CHECKED:BST_UNCHECKED);
        m_wndUsername.SetWindowText(STR_UTF8(m_vecHosts[index].szUsername.c_str()));
        OnCbnEditchangeComboUsername();
        m_wndPassword.SetWindowText(STR_UTF8(m_vecHosts[index].szPassword.c_str()));
        m_wndChannel.SetWindowText(STR_UTF8(m_vecHosts[index].szChannel.c_str()));
        m_wndChPasswd.SetWindowText(STR_UTF8(m_vecHosts[index].szChPasswd.c_str()));
    }
}

void CHostManagerDlg::OnBnClickedButtonImportttile()
{
    CString szFileTypes;
    szFileTypes.Format(LoadText(IDS_HOSTMANAGERHOSTFILES, _T("Host files (*%s)|*%s|All files (*.*)|*.*|")), _T(TTFILE_EXT), _T(TTFILE_EXT));

    CFileDialog dlg(TRUE, 0,0,OFN_FILEMUSTEXIST| OFN_HIDEREADONLY, szFileTypes, this);
    if(dlg.DoModal() == IDOK)
    {
        HostEntry tthost;

        TTFile tt(TT_XML_ROOTNAME);
        if(tt.LoadFile(STR_LOCAL( dlg.GetPathName() )) && 
            VersionSameOrLater(STR_UTF8(tt.GetFileVersion()), _T("5.0")) &&
            !tt.HasErrors() && tt.GetHostEntry(tthost, 0))
        {
            m_pSettings->AddHostManagerEntry(tthost);
            int i = m_wndHosts.AddString(STR_UTF8( tthost.szEntryName.c_str()));
            m_wndHosts.SetCurSel(i);
            OnSelchangeListHosts();
        }
        else
        {
            CString s, szCaption;
            s.Format(LoadText(IDS_HOSTMANAGERFILENOTCONTAINVALIDENTRY, _T("The file %s\r\ndoes not contain a valid %s host entry.\r\nError message: %s")), dlg.GetPathName(), APPNAME, STR_UTF8(tt.GetError().c_str()));
            m_wndImportBtn.GetWindowText(szCaption);
            MessageBox(s, StripAmpersand(szCaption), MB_ICONERROR);
        }
    }
}

void CHostManagerDlg::OnCbnEditchangeComboUsername()
{
    CString szUsername;
    m_wndUsername.GetWindowText(szUsername);
    m_wndPassword.EnableWindow(szUsername != WEBLOGIN_FACEBOOK_USERNAME);
    if(szUsername == WEBLOGIN_FACEBOOK_USERNAME)
        m_wndPassword.SetWindowText(_T(""));
}

void CHostManagerDlg::OnCbnKillfocusComboUsername()
{
    OnCbnEditchangeComboUsername();
}


void CHostManagerDlg::OnEnSetfocusEditEntryname()
{
    CString szName, szHost, szUsername, szPort;
    m_wndEntryName.GetWindowText(szName);
    m_wndHostAddress.GetWindowText(szHost);
    m_wndUsername.GetWindowText(szUsername);
    m_wndHostPort.GetWindowText(szPort);
    if (szName.IsEmpty() && szHost.GetLength() && szUsername.GetLength() && szPort.GetLength())
    {
        szName.Format(_T("%s@%s:%s"), szUsername, szHost, szPort);
        m_wndEntryName.SetWindowText(szName);
    }
}

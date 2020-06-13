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

// OnlineUsersDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "OnlineUsersDlg.h"
#include "TeamTalkDlg.h"
#include <vector>

extern TTInstance* ttInst;

// COnlineUsersDlg dialog

#define DISCONNECTED_USERID 0

IMPLEMENT_DYNAMIC(COnlineUsersDlg, CDialog)

COnlineUsersDlg::COnlineUsersDlg(CTeamTalkDlg* pParent /*=NULL*/)
	: CDialog(COnlineUsersDlg::IDD, pParent)
    , m_pParent(pParent)
{
#ifndef _WIN32_WCE
	EnableActiveAccessibility();
#endif

}

COnlineUsersDlg::~COnlineUsersDlg()
{
}

void COnlineUsersDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST_ONLINEUSERS, m_wndUsers);
    DDX_Control(pDX, IDC_CHECK_SHOWDISCONNECTED, m_wndDisconnectUsers);
}


BEGIN_MESSAGE_MAP(COnlineUsersDlg, CDialog)
    ON_WM_CONTEXTMENU()
    ON_COMMAND(ID_POPUP_KICK, &COnlineUsersDlg::OnPopupKick)
    ON_COMMAND(ID_POPUP_KICKANDBAN, &COnlineUsersDlg::OnPopupKickandban)
    ON_COMMAND(ID_POPUP_OP, &COnlineUsersDlg::OnPopupOp)
    ON_COMMAND(ID_POPUP_COPYUSERINFORMATION, &COnlineUsersDlg::OnPopupCopyuserinformation)
    ON_WM_SIZE()
    ON_COMMAND(ID_POPUP_MESSAGES, &COnlineUsersDlg::OnPopupMessages)
    ON_COMMAND(ID_POPUP_STOREFORMOVE, &COnlineUsersDlg::OnPopupStoreformove)
    ON_BN_CLICKED(IDC_CHECK_SHOWDISCONNECTED, &COnlineUsersDlg::OnBnClickedCheckShowdisconnected)
END_MESSAGE_MAP()


// COnlineUsersDlg message handlers

BOOL COnlineUsersDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    TRANSLATE(*this, IDD);

    static CResizer::CBorderInfo s_bi[] = {

        { IDC_LIST_ONLINEUSERS,
        { CResizer::eFixed, IDC_MAIN, CResizer::eLeft },
        { CResizer::eFixed, IDC_MAIN, CResizer::eTop },
        { CResizer::eFixed, IDC_MAIN, CResizer::eRight },
        { CResizer::eFixed, IDC_MAIN, CResizer::eBottom } },

        { IDC_STATIC_CURUSERS,
        { CResizer::eFixed, IDC_MAIN, CResizer::eLeft },
        { CResizer::eFixed, IDC_MAIN, CResizer::eTop },
        { CResizer::eFixed, IDC_MAIN, CResizer::eRight },
        { CResizer::eFixed, IDC_MAIN, CResizer::eBottom } },

        { IDC_CHECK_SHOWDISCONNECTED,
        { CResizer::eFixed, IDC_MAIN, CResizer::eLeft },
        { CResizer::eFixed, IDC_MAIN, CResizer::eBottom },
        { CResizer::eFixed, IDC_MAIN, CResizer::eRight },
        { CResizer::eFixed, IDC_MAIN, CResizer::eBottom } },
    };

    const int nSize = sizeof(s_bi) / sizeof(s_bi[0]);
    m_resizer.Init(m_hWnd, NULL, s_bi, nSize);

    //load accelerators
    m_hAccel = ::LoadAccelerators(AfxGetResourceHandle(), (LPCTSTR)IDR_ACCELERATOR3);
    if (!m_hAccel)
        MessageBox(LoadText(IDS_ACCELERATORNOTLOADDED, _T("The accelerator table was not loaded")));

    m_wndUsers.SetExtendedStyle(m_wndUsers.GetExtendedStyle() | LVS_EX_FULLROWSELECT);

    m_wndUsers.InsertColumn(COLUMN_USERID, LoadText(IDS_ONLINEDLGID, _T("ID")));
    m_wndUsers.SetColumnWidth(COLUMN_USERID, 45);
    m_wndUsers.InsertColumn(COLUMN_NICKNAME, LoadText(IDS_ONLINEDLGNICKNAME, _T("Nickname")));
    m_wndUsers.SetColumnWidth(COLUMN_NICKNAME, 100);
    m_wndUsers.InsertColumn(COLUMN_STATUSMSG, LoadText(IDS_ONLINEDLGSM, _T("Status message")));
    m_wndUsers.SetColumnWidth(COLUMN_STATUSMSG, 100);
    m_wndUsers.InsertColumn(COLUMN_USERNAME, LoadText(IDS_ONLINEDLGUSERNAME, _T("Username")));
    m_wndUsers.SetColumnWidth(COLUMN_USERNAME, 65);
    m_wndUsers.InsertColumn(COLUMN_CHANNEL, LoadText(IDS_ONLINEDLGCHAN, _T("Channel")));
    m_wndUsers.SetColumnWidth(COLUMN_CHANNEL, 100);
    m_wndUsers.InsertColumn(COLUMN_IPADDRESS, LoadText(IDS_ONLINEDLGIPADDR, _T("IP-address")));
    m_wndUsers.SetColumnWidth(COLUMN_IPADDRESS, 100);
    m_wndUsers.InsertColumn(COLUMN_VERSION_, LoadText(IDS_ONLINEDLGVERS, _T("Version")));
    m_wndUsers.SetColumnWidth(COLUMN_VERSION_, 100);

    m_wndUsers.SetFocus();

    return FALSE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

void COnlineUsersDlg::AddUser(const User& user)
{
    int iIndex = 0;
    auto i = std::find_if(m_users.begin(), m_users.end(), [user](User u) { return u.nUserID == user.nUserID; });
    if (i != m_users.end())
    {
        i = m_users.insert(i, user);
        i = m_users.erase(i + 1);
        iIndex = int(i - m_users.begin());
    }
    else
    {
        CString s;
        s.Format(_T("%d"), user.nUserID);
        iIndex = m_wndUsers.InsertItem(int(m_users.size()), s, 0);
        m_users.push_back(user);
    }
    m_wndUsers.SetItemText(iIndex, COLUMN_NICKNAME, user.szNickname);
    m_wndUsers.SetItemText(iIndex, COLUMN_STATUSMSG, user.szStatusMsg);
    m_wndUsers.SetItemText(iIndex, COLUMN_USERNAME, user.szUsername);
    TTCHAR szChannel[TT_STRLEN] = _T("");
    TT_GetChannelPath(ttInst, user.nChannelID, szChannel);
    m_wndUsers.SetItemText(iIndex, COLUMN_CHANNEL, szChannel);
    m_wndUsers.SetItemText(iIndex, COLUMN_IPADDRESS, user.szIPAddress);
    m_wndUsers.SetItemText(iIndex, COLUMN_VERSION_, GetVersion(user));
    m_wndUsers.SetItemData(iIndex, user.nUserID);
}

void COnlineUsersDlg::RemoveUser(int nUserID)
{
    BOOL bEraseItem = m_wndDisconnectUsers.GetCheck() != BST_CHECKED;

    auto ite = std::find_if(m_users.begin(), m_users.end(), [nUserID] (User u) { return u.nUserID == nUserID; });
    if (ite != m_users.end())
    {
        ite->nUserID = DISCONNECTED_USERID;
        if (bEraseItem)
            m_users.erase(ite);
    }

    for(int i=0;i<m_wndUsers.GetItemCount();i++)
    {
        if (m_wndUsers.GetItemData(i) == nUserID)
        {
            if (bEraseItem)
            {
                m_wndUsers.DeleteItem(i);
            }
            else
            {
                m_wndUsers.SetItemText(i, 0, _T("0"));
                m_wndUsers.SetItemData(i, DISCONNECTED_USERID);
                break;
            }
        }
    }
}

void COnlineUsersDlg::ResetUsers()
{
    m_users.clear();
    m_wndUsers.DeleteAllItems();
}

void COnlineUsersDlg::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
    CMenu menu;
    menu.LoadMenu(IDR_MENU_ONLINEUSERS);
    TRANSLATE(menu);
    CMenu* pop = menu.GetSubMenu(0);
    if(!pop)
        return;

    UINT uCmd = pop->TrackPopupMenu(TPM_RETURNCMD | TPM_LEFTALIGN |
        TPM_RIGHTBUTTON, point.x, point.y,
        this, NULL );
    MenuCommand(uCmd);
}

BOOL COnlineUsersDlg::PreTranslateMessage(MSG* pMsg)
{
    if (WM_KEYFIRST <= pMsg->message && pMsg->message <= WM_KEYLAST)
        if (m_hAccel && ::TranslateAccelerator(m_hWnd, m_hAccel, pMsg))
            return TRUE;

    return CDialog::PreTranslateMessage(pMsg);
}

void COnlineUsersDlg::MenuCommand(UINT uCmd)
{
    int i = 0;
    int count = m_wndUsers.GetItemCount();
    for (i=0;i<count;i++)
    {
        if(m_wndUsers.GetItemState(i, LVIS_SELECTED) == LVIS_SELECTED)
        {
            break;
        }
    }

    if (i == count)
        return;

    User user = m_users[i];
    Channel chan = {};
    TT_GetChannel(ttInst, user.nChannelID, &chan);

    switch(uCmd)
    {
    case ID_POPUP_KICKANDBAN :
        if (user.nUserID == 0) // logged out user
        {
            if (_tcslen(user.szIPAddress))
            {
                BannedUser ban = {};
                COPYTTSTR(ban.szIPAddress, user.szIPAddress);
                ban.uBanTypes = BANTYPE_IPADDR;
                TT_DoBan(ttInst, &ban);
            }
        }
        else
        {
            TT_DoBanUser(ttInst, user.nUserID, 0);
            TT_DoKickUser(ttInst, user.nUserID, 0);
        }
        break;
    case ID_POPUP_KICK :
        TT_DoKickUser(ttInst, user.nUserID, user.nChannelID);
        break;
    case ID_POPUP_OP :
        TT_DoChannelOpEx(ttInst, user.nUserID, user.nChannelID, chan.szOpPassword,
                         !TT_IsChannelOperator(ttInst, user.nUserID, user.nChannelID));
        break;
    case ID_POPUP_COPYUSERINFORMATION :
    {
        CString szText;
        CString szUserID;
        szUserID.Format(_T("%d"), user.nUserID);
        TTCHAR szChannel[TT_STRLEN] = _T("");
        TT_GetChannelPath(ttInst, user.nChannelID, szChannel);
        szText = szUserID;
        szText += _T("\t");
        szText += GetDisplayName(user);
        szText += _T("\t");
        szText += user.szStatusMsg;
        szText += _T("\t");
        szText += user.szUsername;
        szText += _T("\t");
        szText += szChannel;
        szText += _T("\t");
        szText += user.szIPAddress;
        szText += _T("\t");
        szText += GetVersion(user);

        OpenClipboard();
        EmptyClipboard();
        HGLOBAL hglbCopy;
        hglbCopy = GlobalAlloc(GMEM_MOVEABLE,
            (szText.GetLength() + 1) * sizeof(TCHAR));
        if(hglbCopy)
        {
            LPVOID szStr = GlobalLock(hglbCopy);
            memcpy(szStr, szText.GetBuffer(), (szText.GetLength() + 1) * sizeof(TCHAR));
            GlobalUnlock(hglbCopy);
#if defined(UNICODE) || defined(_UNICODE)
            SetClipboardData(CF_UNICODETEXT, hglbCopy);
#else
            SetClipboardData(CF_TEXT, hglbCopy);
#endif
        }
        CloseClipboard();
    }
    break;
    case ID_POPUP_MESSAGES :
        if(m_pParent)
            m_pParent->OnUsersMessages(user.nUserID);
        break;
    case ID_POPUP_STOREFORMOVE :
        if (user.nUserID != 0)
            m_pParent->m_moveusers.insert(user.nUserID);
        break;
    }
}

void COnlineUsersDlg::OnPopupKick()
{
    MenuCommand(ID_POPUP_KICK);
}

void COnlineUsersDlg::OnPopupKickandban()
{
    MenuCommand(ID_POPUP_KICKANDBAN);
}

void COnlineUsersDlg::OnPopupOp()
{
    MenuCommand(ID_POPUP_OP);
}

void COnlineUsersDlg::OnPopupCopyuserinformation()
{
    MenuCommand(ID_POPUP_COPYUSERINFORMATION);
}

void COnlineUsersDlg::OnPopupMessages()
{
    MenuCommand(ID_POPUP_MESSAGES);
}

void COnlineUsersDlg::OnPopupStoreformove()
{
    MenuCommand(ID_POPUP_STOREFORMOVE);
}

void COnlineUsersDlg::OnSize(UINT nType, int cx, int cy)
{
    // TODO: Group box overlaps listbox for some reason...
    CDialog::OnSize(nType, cx, cy);
    m_resizer.Move();
}

void COnlineUsersDlg::PostNcDestroy()
{
    CDialog::PostNcDestroy();
    m_pParent->SendMessage(WM_ONLINEUSERSDLG_CLOSED);
    delete this;
}

void COnlineUsersDlg::OnCancel()
{
    CDialog::OnCancel();
    DestroyWindow();
}


void COnlineUsersDlg::OnBnClickedCheckShowdisconnected()
{
    BOOL bEraseItem = m_wndDisconnectUsers.GetCheck() != BST_CHECKED;

    auto ite = m_users.begin();
    while ((ite = std::find_if(m_users.begin(), m_users.end(), [](User u) { return u.nUserID == DISCONNECTED_USERID; })) != m_users.end())
    {
        m_users.erase(ite);
    }

    for (int i = 0; i<m_wndUsers.GetItemCount(); )
    {
        if(m_wndUsers.GetItemData(i) == DISCONNECTED_USERID)
        {
            m_wndUsers.DeleteItem(i);
        }
        else i++;
    }

}

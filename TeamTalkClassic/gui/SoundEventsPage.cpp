/*
 * Copyright (c) 2005-2017, BearWare.dk
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
 * This source code is part of the TeamTalk 5 SDK owned by
 * BearWare.dk. All copyright statements may not be removed 
 * or altered from any source distribution. If you use this
 * software in a product, an acknowledgment in the product 
 * documentation is required.
 *
 */

#include "stdafx.h"
#include "Resource.h"
#include "SoundEventsPage.h"

#if defined(ENABLE_TOLK)
#include <Tolk.h>
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// CSoundEventsPage dialog

IMPLEMENT_DYNAMIC(CSoundEventsPage, CPropertyPage)
CSoundEventsPage::CSoundEventsPage()
    : CPropertyPage(CSoundEventsPage::IDD)
  , m_szDesktopAccessReq(_T(""))
{
}

CSoundEventsPage::~CSoundEventsPage()
{
}

void CSoundEventsPage::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_EDIT_EVENTS_NEWMESSAGE, m_szNewMessagePath);
    DDX_Text(pDX, IDC_EDIT_EVENTS_NEWUSER, m_szNewUserPath);
    DDX_Text(pDX, IDC_EDIT_EVENTS_USERREMOVED, m_szUserRemovedPath);
    DDX_Control(pDX, IDC_EDIT_EVENTS_NEWMESSAGE, m_wndNewMessage);
    DDX_Control(pDX, IDC_EDIT_EVENTS_NEWUSER, m_wndNewUser);
    DDX_Control(pDX, IDC_EDIT_EVENTS_SERVERLOST, m_wndServerLost);
    DDX_Control(pDX, IDC_EDIT_EVENTS_USERREMOVED, m_wndUserRemoved);
    DDX_Text(pDX, IDC_EDIT_EVENTS_SERVERLOST, m_szServerLostPath);
    DDX_Control(pDX, IDC_EDIT_EVENTS_HOTKEY, m_wndHotKey);
    DDX_Text(pDX, IDC_EDIT_EVENTS_HOTKEY, m_szHotKeyPath);
    DDX_Control(pDX, IDC_EDIT_EVENTS_CHANMESSAGE, m_wndChanMsgEdit);
    DDX_Text(pDX, IDC_EDIT_EVENTS_CHANMESSAGE, m_szChanMsg);
    DDX_Control(pDX, IDC_EDIT_EVENTS_STOPPEDTALK, m_wndStopTalkEdit);
    DDX_Text(pDX, IDC_EDIT_EVENTS_STOPPEDTALK, m_szStopTalk);
    DDX_Control(pDX, IDC_EDIT_EVENTS_FILEUPD, m_wndFilesUpd);
    DDX_Text(pDX, IDC_EDIT_EVENTS_FILEUPD, m_szFilesUpd);
    DDX_Control(pDX, IDC_EDIT_EVENTS_TRANSFERSTART, m_wndTransferEnd);
    DDX_Text(pDX, IDC_EDIT_EVENTS_TRANSFERSTART, m_szTransferEnd);
    DDX_Control(pDX, IDC_EDIT_EVENTS_VIDEOSESSION, m_wndVideoSession);
    DDX_Control(pDX, IDC_EDIT_EVENTS_DESKTOPSESSION, m_wndDesktopSession);
    DDX_Control(pDX, IDC_EDIT_EVENTS_QUESTIONMODE, m_wndQuestionMode);
    DDX_Text(pDX, IDC_EDIT_EVENTS_VIDEOSESSION, m_szNewVideoSession);
    DDX_Text(pDX, IDC_EDIT_EVENTS_DESKTOPSESSION, m_szNewDesktopSession);
    DDX_Text(pDX, IDC_EDIT_EVENTS_QUESTIONMODE, m_szQuestionMode);
    DDX_Text(pDX, IDC_EDIT_EVENTS_DESKTOPACCESSREQ, m_szDesktopAccessReq);
    DDX_Control(pDX, IDC_EDIT_EVENTS_DESKTOPACCESSREQ, m_wndDesktopAccessReq);
}

CString CSoundEventsPage::GetEventFilePath()
{
    CString filetypes = _T("Wave files (*.wav)|*.wav|All files (*.*)|*.*|");
    CFileDialog dlg(TRUE, 0,0,OFN_FILEMUSTEXIST| OFN_HIDEREADONLY,filetypes, this);
    TCHAR s[MAX_PATH];
    CString szFilePath;
    GetCurrentDirectory(MAX_PATH, s);
    if(dlg.DoModal() == IDOK)
        szFilePath = dlg.GetPathName();
    SetCurrentDirectory(s);

    return szFilePath;
}

BEGIN_MESSAGE_MAP(CSoundEventsPage, CPropertyPage)
    ON_BN_CLICKED(IDC_BUTTON_EVENTS_NEWUSER, OnBnClickedButtonEventsNewuser)
    ON_BN_CLICKED(IDC_BUTTON_EVENTS_USERREMOVED, OnBnClickedButtonEventsUserremoved)
    ON_BN_CLICKED(IDC_BUTTON_EVENTS_SERVERLOST, OnBnClickedButtonEventsServerlost)
    ON_BN_CLICKED(IDC_BUTTON_EVENTS_NEWMESSAGE, OnBnClickedButtonEventsNewmessage)
    ON_BN_CLICKED(IDC_BUTTON_EVENTS_HOTKEY, OnBnClickedButtonEventsHotkey)
    ON_BN_CLICKED(IDC_BUTTON_EVENTS_CHANMESSAGE, OnBnClickedButtonEventsChanmessage)
  ON_BN_CLICKED(IDC_BUTTON_EVENTS_STOPPEDTALK, OnBnClickedButtonEventsStoppedtalk)
    ON_BN_CLICKED(IDC_BUTTON_EVENTS_FILESUPD, OnBnClickedButtonEventsFilesupd)
    ON_BN_CLICKED(IDC_BUTTON_EVENTS_TRANSFEREND, OnBnClickedButtonEventsTransferend)
    ON_BN_CLICKED(IDC_BUTTON_EVENTS_VIDEOSESSION, &CSoundEventsPage::OnBnClickedButtonEventsVideosession)
    ON_BN_CLICKED(IDC_BUTTON_EVENTS_DESKTOPSESSION, &CSoundEventsPage::OnBnClickedButtonEventsDesktopsession)
    ON_BN_CLICKED(IDC_BUTTON_EVENTS_QUESTIONMODE, &CSoundEventsPage::OnBnClickedButtonEventsQuestionmode)
    ON_BN_CLICKED(IDC_BUTTON_EVENTS_DESKTOPACCESSREQ, &CSoundEventsPage::OnBnClickedButtonEventsDesktopaccessreq)
END_MESSAGE_MAP()


// CSoundEventsPage message handlers

void CSoundEventsPage::OnBnClickedButtonEventsNewuser()
{
    CString szFilePath = GetEventFilePath();
    if(szFilePath.GetLength())
        m_wndNewUser.SetWindowText(szFilePath);
}

void CSoundEventsPage::OnBnClickedButtonEventsUserremoved()
{
    CString szFilePath = GetEventFilePath();
    if(szFilePath.GetLength())
        m_wndUserRemoved.SetWindowText(szFilePath);
}

void CSoundEventsPage::OnBnClickedButtonEventsServerlost()
{
    CString szFilePath = GetEventFilePath();
    if(szFilePath.GetLength())
        m_wndServerLost.SetWindowText(szFilePath);
}

void CSoundEventsPage::OnBnClickedButtonEventsNewmessage()
{
    CString szFilePath = GetEventFilePath();
    if(szFilePath.GetLength())
        m_wndNewMessage.SetWindowText(szFilePath);
}


void CSoundEventsPage::OnBnClickedButtonEventsHotkey()
{
    CString szFilePath = GetEventFilePath();
    if(szFilePath.GetLength())
        m_wndHotKey.SetWindowText(szFilePath);
}

void CSoundEventsPage::OnBnClickedButtonEventsChanmessage()
{
    CString szFilePath = GetEventFilePath();
    if(szFilePath.GetLength())
        m_wndChanMsgEdit.SetWindowText(szFilePath);
}

void CSoundEventsPage::OnBnClickedButtonEventsStoppedtalk()
{
    CString szFilePath = GetEventFilePath();
    if(szFilePath.GetLength())
        m_wndStopTalkEdit.SetWindowText(szFilePath);
}

void CSoundEventsPage::OnBnClickedButtonEventsFilesupd()
{
    CString szFilePath = GetEventFilePath();
    if(szFilePath.GetLength())
        m_wndFilesUpd.SetWindowText(szFilePath);
}

void CSoundEventsPage::OnBnClickedButtonEventsTransferend()
{
    CString szFilePath = GetEventFilePath();
    if(szFilePath.GetLength())
        m_wndTransferEnd.SetWindowText(szFilePath);
}

BOOL CSoundEventsPage::OnInitDialog()
{
    CPropertyPage::OnInitDialog();

    TRANSLATE(*this, IDD);

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

void CSoundEventsPage::OnBnClickedButtonEventsVideosession()
{
    CString szFilePath = GetEventFilePath();
    if(szFilePath.GetLength())
        m_wndVideoSession.SetWindowText(szFilePath);
}

void CSoundEventsPage::OnBnClickedButtonEventsDesktopsession()
{
    CString szFilePath = GetEventFilePath();
    if(szFilePath.GetLength())
        m_wndDesktopSession.SetWindowText(szFilePath);
}

void CSoundEventsPage::OnBnClickedButtonEventsQuestionmode()
{
    CString szFilePath = GetEventFilePath();
    if(szFilePath.GetLength())
        m_wndQuestionMode.SetWindowText(szFilePath);
}


void CSoundEventsPage::OnBnClickedButtonEventsDesktopaccessreq()
{
    CString szFilePath = GetEventFilePath();
    if(szFilePath.GetLength())
        m_wndDesktopAccessReq.SetWindowText(szFilePath);
}

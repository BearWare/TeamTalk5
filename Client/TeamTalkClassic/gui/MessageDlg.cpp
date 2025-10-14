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

// SendMessageDlg.cpp : implementation file
//

#include "StdAfx.h"
#include "Resource.h"
#include "MessageDlg.h"
#include "../TeamTalkDlg.h"
#include "Helper.h"

#include <string>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern TTInstance* ttInst;

/////////////////////////////////////////////////////////////////////////////
// CSendMessageDlg dialog

CMessageDlg::CMessageDlg(teamtalk::ClientXML& xmlSettings, CTeamTalkDlg* pParent, const User& myself, const User& user,
                         LPCTSTR szLogFolder/* = NULL*/)
: CDialog(CMessageDlg::IDD, pParent)
, m_xmlSettings(xmlSettings)
, m_myself(myself)
, m_user(user)
, m_bUserAlive(TRUE)
, m_pParent(pParent)
{
    if(szLogFolder && _tcslen(szLogFolder))
        OpenLogFile(m_logFile, szLogFolder, GetDisplayName(user) + _T(".ulog"));
}


void CMessageDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CSendMessageDlg)
    DDX_Control(pDX, IDC_BUTTON_SEND, m_btnSend);
    DDX_Control(pDX, IDC_RICHEDIT_MESSAGE, m_richMessage);
    DDX_Control(pDX, IDC_RICHEDIT_HISTORY, m_richHistory);
    //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMessageDlg, CDialog)
    //{{AFX_MSG_MAP(CSendMessageDlg)
    ON_BN_CLICKED(IDC_BUTTON_SEND, OnButtonSend)
    //}}AFX_MSG_MAP
    ON_WM_SIZE()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSendMessageDlg message handlers

void CMessageDlg::OnCancel() 
{
    CDialog::OnCancel();
    CloseLogFile(m_logFile);
    DestroyWindow();
}

BOOL CMessageDlg::OnInitDialog() 
{
    CDialog::OnInitDialog();

    TRANSLATE(*this, IDD);

    //load accelerators
    m_hAccel = ::LoadAccelerators(AfxGetResourceHandle(), (LPCTSTR)IDR_ACCELERATOR1);
    if (!m_hAccel)
        MessageBox(LoadText(IDS_ACCELERATORNOTLOADDED, _T("The accelerator table was not loaded")));

    m_richHistory.GetSelectionCharFormat(m_defaultCF);
    m_richHistory.m_bShowTimeStamp = m_xmlSettings.GetMessageTimeStamp();

    if(IsAlive())
    {
        for(size_t i=0;i<m_messages.size();i++)
            AppendMessage(m_messages[i], FALSE);

        CString s;
        GetWindowText(s);
        SetWindowText(s + _T(" - ") + GetDisplayName(m_user));
    }

    m_richHistory.LimitText(MAX_HISTORY_LENGTH);

    SetIcon(AfxGetApp()->LoadIcon(IDR_MAINFRAME),TRUE);

    static CResizer::CBorderInfo s_bi[] = {

        {IDC_RICHEDIT_HISTORY,    
        {CResizer::eFixed, IDC_MAIN, CResizer::eLeft}, 
        {CResizer::eFixed, IDC_MAIN, CResizer::eTop},  
        {CResizer::eFixed, IDC_MAIN, CResizer::eRight},
        {CResizer::eFixed, IDC_MAIN, CResizer::eBottom}}, 

        {IDC_STATIC_NEWMESSAGE,    
        {CResizer::eFixed, IDC_MAIN, CResizer::eLeft}, 
        {CResizer::eFixed, IDC_MAIN, CResizer::eBottom},  
        {CResizer::eFixed, IDC_MAIN, CResizer::eRight},
        {CResizer::eFixed, IDC_MAIN, CResizer::eBottom}}, 

        {IDC_RICHEDIT_MESSAGE,    
        {CResizer::eFixed, IDC_MAIN, CResizer::eLeft}, 
        {CResizer::eFixed, IDC_MAIN, CResizer::eBottom},  
        {CResizer::eFixed, IDC_MAIN, CResizer::eRight},
        {CResizer::eFixed, IDC_MAIN, CResizer::eBottom}}, 

        {IDCANCEL,
        {CResizer::eFixed, IDC_MAIN, CResizer::eLeft}, 
        {CResizer::eFixed, IDC_MAIN, CResizer::eBottom},  
        {CResizer::eFixed, IDC_MAIN, CResizer::eLeft},
        {CResizer::eFixed, IDC_MAIN, CResizer::eBottom}}, 
        {IDC_BUTTON_SEND,       
        {CResizer::eFixed, IDC_MAIN, CResizer::eRight}, 
        {CResizer::eFixed, IDC_MAIN, CResizer::eBottom},  
        {CResizer::eFixed, IDC_MAIN, CResizer::eRight},
        {CResizer::eFixed, IDC_MAIN, CResizer::eBottom}}, 

    };
    const int nSize = sizeof(s_bi)/sizeof(s_bi[0]);
    m_resizer.Init(m_hWnd, NULL, s_bi, nSize);

    if(m_lf.lfHeight>0)
    {
        m_Font.CreateFontIndirect(&m_lf);
        m_richHistory.SetFont(&m_Font);
        m_richMessage.SetFont(&m_Font);
    }

    m_richMessage.SetFocus();

    return FALSE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

void CMessageDlg::OnButtonSend()
{
    CString msg;
    m_richMessage.GetWindowText(msg);

    if (IsAlive() && msg.GetLength() > 0)
    {
        MyTextMessage usermsg;
        usermsg.nMsgType = MSGTYPE_USER;
        usermsg.nFromUserID = m_myself.nUserID;
        usermsg.nToUserID = m_user.nUserID;
        _tcsncpy(usermsg.szMessage, msg.GetBuffer(), TT_STRLEN - 1);

        int utf8_len = WideCharToMultiByte(CP_UTF8, 0, msg, -1, NULL, 0, NULL, NULL);
        if (utf8_len < TT_STRLEN) {
            if (TT_DoTextMessage(ttInst, &usermsg) > 0) {
                m_richMessage.SetWindowText(_T(""));
                AppendMessage(usermsg, TRUE);
                m_pParent->PlaySoundEvent(SOUNDEVENT_USER_TEXTMSGSENT);
                if (m_xmlSettings.GetEventTTSEvents() & TTS_SUBSCRIPTIONS_TEXTMSG_PRIVATE) {
                    CString szMsg;
                    szMsg.Format(LoadText(IDS_MPSEND, _T("Private message sent: %s")), usermsg.szMessage);
                    AddTextToSpeechMessage(szMsg);
                }
            }
            else {
                AfxMessageBox(LoadText(IDS_MSGDLGFAILEDTOSEND, _T("Failed to send message!")));
            }
        }
        else {
            CString szError;
                szError.Format(LoadText(IDS_MSGCHARSLIMIT, _T("Your message has exceeded the limit by %d characters. Please reduce it and try again.")), utf8_len - TT_STRLEN + 1);
                MessageBox(szError, LoadText(IDS_MSGCHARSLIMITTITLE, _T("Character limit exceeded")), MB_OK);
        }

    }
}

BOOL CMessageDlg::IsAlive()
{
    return m_bUserAlive;
}

void CMessageDlg::PostNcDestroy() 
{
    CDialog::PostNcDestroy();
    m_pParent->SendMessage(WM_MESSAGEDLG_CLOSED, m_user.nUserID);
    delete this;
}


void CMessageDlg::SetAlive(BOOL state)
{
    m_bUserAlive = state;
    m_btnSend.EnableWindow(state);
}

void CMessageDlg::AppendMessage(const MyTextMessage& msg, BOOL bStore/* = TRUE*/)
{
    //append to message list
    if(bStore)
        m_messages.push_back(msg);

    // Insert Name
    CString szName = (msg.nFromUserID == m_myself.nUserID) ? GetDisplayName(m_myself) : GetDisplayName(m_user);

    m_richHistory.AddMessage(szName, msg);

    if(bStore)
        WriteLogMsg(m_logFile, szName + msg.szMessage + _T("\r\n"));
}

void CMessageDlg::OnSize(UINT nType, int cx, int cy)
{
    m_resizer.Move();
    CDialog::OnSize(nType, cx, cy);
}


BOOL CMessageDlg::PreTranslateMessage(MSG* pMsg)
{
    if (WM_KEYFIRST <= pMsg->message && pMsg->message <= WM_KEYLAST)
        if (m_hAccel && ::TranslateAccelerator(m_hWnd, m_hAccel, pMsg))
            return TRUE;

    return CDialog::PreTranslateMessage(pMsg);
}

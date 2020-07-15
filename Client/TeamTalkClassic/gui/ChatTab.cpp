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

// ChatTab.cpp : implementation file
//
#include "stdafx.h"
#include "Resource.h"
#include "ChatTab.h"
#include "RichEditThemed.h"
#include ".\chattab.h"


// CChatTab dialog

IMPLEMENT_DYNAMIC(CChatTab, CMyTab)
CChatTab::CChatTab(CWnd* pParent /*=NULL*/)
    : CMyTab(CChatTab::IDD, pParent)
    , m_hAccel(0)
{
}

CChatTab::~CChatTab()
{
}

void CChatTab::DoDataExchange(CDataExchange* pDX)
{
    CMyTab::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_RICHEDIT_CHANMESSAGES, m_wndRichEdit);
    DDX_Control(pDX, IDC_EDIT_CHANMESSAGE, m_wndChanMessage);
}


BEGIN_MESSAGE_MAP(CChatTab, CMyTab)
    ON_WM_SIZE()
END_MESSAGE_MAP()


// CChatTab message handlers

BOOL CChatTab::OnInitDialog()
{
    CMyTab::OnInitDialog();

    static CResizer::CBorderInfo s_bi[] = {

        {IDC_RICHEDIT_CHANMESSAGES,    
        {CResizer::eFixed, IDC_MAIN, CResizer::eLeft}, 
        {CResizer::eFixed, IDC_MAIN, CResizer::eTop},  
        {CResizer::eFixed, IDC_MAIN, CResizer::eRight},
        {CResizer::eFixed, IDC_MAIN, CResizer::eBottom}}, 

        {IDC_EDIT_CHANMESSAGE,    
        {CResizer::eFixed, IDC_MAIN, CResizer::eLeft}, 
        {CResizer::eFixed, IDC_MAIN, CResizer::eBottom},  
        {CResizer::eFixed, IDC_MAIN, CResizer::eRight},
        {CResizer::eFixed, IDC_MAIN, CResizer::eBottom}}, 
    };
    const int nSize = sizeof(s_bi)/sizeof(s_bi[0]);
    m_resizer.Init(m_hWnd, NULL, s_bi, nSize);

    //enable link detection in CRichEdit
#if !defined(_WIN64)
    HWND hRichEdit = ::GetDlgItem(m_hWnd, IDC_RICHEDIT_CHANMESSAGES);
    CRichEditThemed::Attach(hRichEdit);    //xp look
#endif
    m_wndRichEdit.SendMessage(EM_AUTOURLDETECT, TRUE, 0);
    m_wndRichEdit.SetEventMask(m_wndRichEdit.GetEventMask() | ENM_LINK) ;

    TRANSLATE(*this, IDD);
    SetAccessibleName(m_wndRichEdit, LoadText(IDS_CHANHISTORYLAB, _T("History")));
    SetAccessibleName(m_wndChanMessage, LoadText(IDS_CHANMESSAGELAB, _T("Message")));

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

void CChatTab::OnSize(UINT nType, int cx, int cy)
{
    CMyTab::OnSize(nType, cx, cy);

    m_resizer.Move();
}

void CChatTab::OnOK()
{
    CMyTab::OnOK();
    HWND hWnd = AfxGetMainWnd()->m_hWnd;
    AfxGetMainWnd()->SendMessage(WM_COMMAND, IDOK, (LPARAM)hWnd);
}

BOOL CChatTab::PreTranslateMessage(MSG* pMsg)
{
    if (WM_KEYFIRST <= pMsg->message && pMsg->message <= WM_KEYLAST)
        if (m_hAccel && ::TranslateAccelerator(AfxGetMainWnd()->m_hWnd, m_hAccel, pMsg))
            return TRUE;

    return CMyTab::PreTranslateMessage(pMsg);
}

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
#include "Resource.h"
#include "ChangeStatusDlg.h"

//need status mode information

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// CChangeStatus dialog

IMPLEMENT_DYNAMIC(CChangeStatusDlg, CDialog)
CChangeStatusDlg::CChangeStatusDlg(CWnd* pParent /*=NULL*/)
    : CDialog(CChangeStatusDlg::IDD, pParent)
    , m_szAwayMessage(_T("")),
    m_nStatusMode(STATUSMODE_AVAILABLE)
{
}

CChangeStatusDlg::~CChangeStatusDlg()
{
}

void CChangeStatusDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_EDIT_AWAYMESSAGE, m_szAwayMessage);
    DDX_Control(pDX, IDC_EDIT_AWAYMESSAGE, m_wndMessage);
    DDX_Control(pDX, IDC_RADIO_AWAY, m_wndAway);
    DDX_Control(pDX, IDC_RADIO_ONLINE, m_wndOnline);
    DDX_Control(pDX, IDC_RADIO_QUESTION, m_wndQuestion);
}


BEGIN_MESSAGE_MAP(CChangeStatusDlg, CDialog)
    ON_BN_CLICKED(IDC_RADIO_ONLINE, OnBnClickedRadioOnline)
    ON_BN_CLICKED(IDC_RADIO_AWAY, OnBnClickedRadioAway)
    ON_BN_CLICKED(IDC_RADIO_QUESTION, &CChangeStatusDlg::OnBnClickedRadioQuestion)
END_MESSAGE_MAP()


// CChangeStatus message handlers

void CChangeStatusDlg::OnBnClickedRadioOnline()
{
    m_nStatusMode &= ~STATUSMODE_MASK;
    m_nStatusMode |= STATUSMODE_AVAILABLE;
}

void CChangeStatusDlg::OnBnClickedRadioAway()
{
    m_nStatusMode &= ~STATUSMODE_MASK;
    m_nStatusMode |= STATUSMODE_AWAY;
}

void CChangeStatusDlg::OnBnClickedRadioQuestion()
{
    m_nStatusMode &= ~STATUSMODE_MASK;
    m_nStatusMode |= STATUSMODE_QUESTION;
}

BOOL CChangeStatusDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    TRANSLATE(*this, IDD);

    switch(m_nStatusMode & STATUSMODE_MASK)
    {
    case STATUSMODE_AVAILABLE : 
        m_wndOnline.SetCheck(TRUE);
        OnBnClickedRadioOnline();
        break;
    case STATUSMODE_AWAY : 
        m_wndAway.SetCheck(TRUE);
        OnBnClickedRadioAway();
        break;
    case STATUSMODE_QUESTION :
        m_wndQuestion.SetCheck(TRUE);
        OnBnClickedRadioQuestion();
        break;
    }

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

void CChangeStatusDlg::OnOK()
{
    CDialog::OnOK();
}

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
#include "GeneralPage.h"
#include "TeamTalkDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
extern TTInstance* ttInst;

// CGeneralPage dialog

IMPLEMENT_DYNAMIC(CGeneralPage, CPropertyPage)
CGeneralPage::CGeneralPage()
: CPropertyPage(CGeneralPage::IDD)
, m_nInactivity(0)
, m_bIdleVox(FALSE)
, m_bFemale(FALSE)
{
    m_bPush = FALSE;
    m_sNickname = _T("");
    m_bVoiceAct = FALSE;
}

CGeneralPage::~CGeneralPage()
{
}

void CGeneralPage::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_EDIT_KEYCOMB, m_KeyEdit);
    DDX_Control(pDX, IDC_CHECK_PUSHTOTALK, m_btnPush);
    DDX_Control(pDX, IDC_CHECK_VOICEACT, m_btnVoice);
    DDX_Check(pDX, IDC_CHECK_PUSHTOTALK, m_bPush);
    DDX_Text(pDX, IDC_EDIT_NICKNAME, m_sNickname);
    DDX_Check(pDX, IDC_CHECK_VOICEACT, m_bVoiceAct);
    DDX_Control(pDX, IDC_BUTTON_SETUPKEYS, m_wndKeysBtn);
    DDX_Control(pDX, IDC_STATIC_KEYCOMP, m_wndKeyComp);
    DDX_Text(pDX, IDC_EDIT_INACTIVITY, m_nInactivity);
    DDX_Control(pDX, IDC_EDIT_INACTIVITY, m_wndInactivity);
    DDX_Control(pDX, IDC_CHECK_IDLEVOX, m_wndIdleVox);
    DDX_Check(pDX, IDC_CHECK_IDLEVOX, m_bIdleVox);
    BOOL bMale = !m_bFemale;
    DDX_Check(pDX, IDC_RADIO_MALE, bMale);
    DDX_Check(pDX, IDC_RADIO_FEMALE, m_bFemale);
}


BEGIN_MESSAGE_MAP(CGeneralPage, CPropertyPage)
    ON_BN_CLICKED(IDC_CHECK_PUSHTOTALK, OnBnClickedCheckPushtotalk)
    ON_BN_CLICKED(IDC_BUTTON_SETUPKEYS, OnBnClickedSetupKeys)
    ON_EN_CHANGE(IDC_EDIT_INACTIVITY, OnEnChangeEditInactivity)
END_MESSAGE_MAP()


// CGeneralPage message handlers

BOOL CGeneralPage::OnInitDialog()
{
    CPropertyPage::OnInitDialog();

    TRANSLATE(*this, IDD);

    m_KeyEdit.SetWindowText(CKeyCompDlg::GetHotkeyString(m_Hotkey));

    OnBnClickedCheckPushtotalk();

    OnEnChangeEditInactivity();

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

void CGeneralPage::OnBnClickedCheckPushtotalk()
{
    if(m_btnPush.GetCheck())
    {
        m_wndKeysBtn.EnableWindow(TRUE);
        m_wndKeyComp.EnableWindow(TRUE);
        m_KeyEdit.EnableWindow(TRUE);
    }
    else
    {
        m_wndKeysBtn.EnableWindow(FALSE);
        m_wndKeyComp.EnableWindow(FALSE);
        m_KeyEdit.EnableWindow(FALSE);
    }
}

void CGeneralPage::OnBnClickedSetupKeys()
{
    CKeyCompDlg dlg;
    dlg.DoModal();
    m_Hotkey = dlg.m_Hotkey;

    m_KeyEdit.SetWindowText(CKeyCompDlg::GetHotkeyString(m_Hotkey));

    if(m_bPush && m_Hotkey.size())
    {
        TT_HotKey_Unregister(ttInst, HOTKEY_PUSHTOTALK_ID);
        TT_HotKey_Register(ttInst, HOTKEY_PUSHTOTALK_ID, &m_Hotkey[0], INT32(m_Hotkey.size()));
    }
}

void CGeneralPage::OnEnChangeEditInactivity()
{
    CString szIdle;
    m_wndInactivity.GetWindowText(szIdle);
    int nIdle = _ttoi(szIdle);
    if(nIdle != 0)
    {
        m_wndIdleVox.EnableWindow(TRUE);
    }
    else
    {
        m_wndIdleVox.EnableWindow(FALSE);
        m_wndIdleVox.SetCheck(BST_UNCHECKED);
    }
}

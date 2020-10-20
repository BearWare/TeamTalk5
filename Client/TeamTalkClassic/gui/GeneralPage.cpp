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
#include "GeneralPage.h"
#include "TeamTalkDlg.h"
#include "BearWareLoginDlg.h"

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
, m_szBearWareID(_T(""))
, m_bRestoreUser(FALSE)
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
    DDX_Text(pDX, IDC_EDIT_BEARWAREID, m_szBearWareID);
    DDX_Control(pDX, IDC_EDIT_BEARWAREID, m_wndBearWareID);
    DDX_Control(pDX, IDC_BUTTON_SETUPBEARWARE, m_wndSetupBearWare);
    DDX_Check(pDX, IDC_CHECK_WEBLOGIN, m_bRestoreUser);
}


BEGIN_MESSAGE_MAP(CGeneralPage, CPropertyPage)
    ON_BN_CLICKED(IDC_CHECK_PUSHTOTALK, OnBnClickedCheckPushtotalk)
    ON_BN_CLICKED(IDC_BUTTON_SETUPKEYS, OnBnClickedSetupKeys)
    ON_EN_CHANGE(IDC_EDIT_INACTIVITY, OnEnChangeEditInactivity)
    ON_BN_CLICKED(IDC_BUTTON_SETUPBEARWARE, &CGeneralPage::OnBnClickedButtonSetupbearware)
END_MESSAGE_MAP()


// CGeneralPage message handlers

BOOL CGeneralPage::OnInitDialog()
{
    CPropertyPage::OnInitDialog();

    TRANSLATE(*this, IDD);
    CString Lab1;
    Lab1.Format(LoadText(IDS_INACTDELAYLAB, _T("Define away status after %d seconds")), m_nInactivity);
    SetAccessibleName(m_wndInactivity, Lab1);

    m_KeyEdit.SetWindowText(CKeyCompDlg::GetHotkeyString(m_Hotkey));

    OnBnClickedCheckPushtotalk();

    OnEnChangeEditInactivity();

    if (!m_szBearWareID.IsEmpty())
        TRANSLATE(m_wndSetupBearWare, IDS_RESET, _T("Reset"));

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

void CGeneralPage::OnBnClickedButtonSetupbearware()
{
    CString szUsername;
    m_wndBearWareID.GetWindowText(szUsername);

    if (szUsername.IsEmpty())
    {
        CBearWareLoginDlg dlg(this);
        if (dlg.DoModal() == IDOK)
        {
            m_wndBearWareID.SetWindowText(dlg.m_szUsername);
            m_szBearWareToken = dlg.m_szToken;
            TRANSLATE(m_wndSetupBearWare, IDS_RESET, _T("Reset"));
        }
    }
    else
    {
        m_wndBearWareID.SetWindowText(_T(""));
        m_szBearWareToken.Empty();
        TRANSLATE(m_wndSetupBearWare, IDC_BUTTON_SETUPBEARWARE, _T("&Activate"));
    }
}

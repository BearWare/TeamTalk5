// wizard\WizGeneralPage.cpp : implementation file
//

#include "stdafx.h"
#include "TeamTalkApp.h"
#include "WizGeneralPage.h"

// CWizGeneralPage dialog
IMPLEMENT_DYNAMIC(CWizGeneralPage, CNGWizardPage)
    CWizGeneralPage::CWizGeneralPage()
    : CNGWizardPage(CWizGeneralPage::IDD)
{
    m_bPush = FALSE;
    m_sNickname = _T("");
    m_bVoiceAct = FALSE;
    m_nInactivity = 0;
}

CWizGeneralPage::~CWizGeneralPage()
{
}

void CWizGeneralPage::DoDataExchange(CDataExchange* pDX)
{
    CNGWizardPage::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_EDIT_KEYCOMB, m_KeyEdit);
    DDX_Control(pDX, IDC_CHECK_PUSHTOTALK, m_btnPush);
    DDX_Control(pDX, IDC_CHECK_VOICEACT, m_btnVoice);
    DDX_Check(pDX, IDC_CHECK_PUSHTOTALK, m_bPush);
    DDX_Text(pDX, IDC_EDIT_NICKNAME, m_sNickname);
    DDX_Check(pDX, IDC_CHECK_VOICEACT, m_bVoiceAct);
    DDX_Control(pDX, IDC_BUTTON_SETUPKEYS, m_wndKeysBtn);
    DDX_Control(pDX, IDC_STATIC_KEYCOMP, m_wndKeyComp);
    DDX_Text(pDX, IDC_EDIT_INACTIVITY, m_nInactivity);
}


BEGIN_MESSAGE_MAP(CWizGeneralPage, CNGWizardPage)
    ON_BN_CLICKED(IDC_CHECK_PUSHTOTALK, OnBnClickedCheckPushtotalk)
    ON_BN_CLICKED(IDC_BUTTON_SETUPKEYS, OnBnClickedButtonSetupkeys)
END_MESSAGE_MAP()


// CWizGeneralPage message handlers

BOOL CWizGeneralPage::OnSetActive()
{
    CPropertySheetEx* pSheet = (CPropertySheetEx*)GetParent();
    ASSERT_KINDOF(CPropertySheetEx, pSheet);
    pSheet->SetWizardButtons(PSWIZB_BACK | PSWIZB_NEXT);

    return CNGWizardPage::OnSetActive();
}

BOOL CWizGeneralPage::OnInitDialog()
{
    CNGWizardPage::OnInitDialog();

    m_KeyEdit.SetWindowText(CKeyCompDlg::GetHotkeyString(m_Hotkey));

    OnBnClickedCheckPushtotalk();

    TRANSLATE(*this, IDD);

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

void CWizGeneralPage::OnBnClickedCheckPushtotalk()
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

void CWizGeneralPage::OnBnClickedButtonSetupkeys()
{
    CKeyCompDlg dlg;
    dlg.DoModal();
    m_Hotkey = dlg.m_Hotkey;

    m_KeyEdit.SetWindowText(CKeyCompDlg::GetHotkeyString(m_Hotkey));
}

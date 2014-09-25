// wizard\WizCompletionPage.cpp : implementation file
//

#include "stdafx.h"
#include "TeamTalkApp.h"
#include "TeamTalkDlg.h"
#include "WizCompletionPage.h"


// CWizCompletionPage dialog

IMPLEMENT_DYNAMIC(CWizCompletionPage, CNGWizardPage)
    CWizCompletionPage::CWizCompletionPage()
    : CNGWizardPage(CWizCompletionPage::IDD)
    , m_bManual(FALSE)
    , m_bWebsite(FALSE)
{
}

CWizCompletionPage::~CWizCompletionPage()
{
}

void CWizCompletionPage::DoDataExchange(CDataExchange* pDX)
{
    CNGWizardPage::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_STATIC_WIZCOMPLETED, m_wndTitle);
    DDX_Check(pDX, IDC_CHECK_MANUAL, m_bManual);
    DDX_Check(pDX, IDC_CHECK_WEBSITE, m_bWebsite);
    DDX_Control(pDX, IDC_EDIT_WIZCOMPLETED, m_wndComplete);
}


BEGIN_MESSAGE_MAP(CWizCompletionPage, CNGWizardPage)
END_MESSAGE_MAP()


// CWizCompletionPage message handlers

BOOL CWizCompletionPage::OnInitDialog()
{
    CNGWizardPage::OnInitDialog();

    VERIFY(font1.CreateFont(
        24,                        // nHeight
        0,                         // nWidth
        0,                         // nEscapement
        0,                         // nOrientation
        FW_BOLD,                 // nWeight
        FALSE,                     // bItalic
        FALSE,                     // bUnderline
        0,                         // cStrikeOut
        ANSI_CHARSET,              // nCharSet
        OUT_DEFAULT_PRECIS,        // nOutPrecision
        CLIP_DEFAULT_PRECIS,       // nClipPrecision
        ANTIALIASED_QUALITY,           // nQuality
        DEFAULT_PITCH | FF_SWISS,  // nPitchAndFamily
        _T("Arial")));                 // lpszFacename

    m_wndTitle.SetFont(&font1);

    CString s = _T("You have now completed the ") APPNAME _T(" setup wizard. ")
        _T("If you wish to check out the manual on how to use ") APPNAME _T(" ")
        _T("you can click the first checkbox below. You can also check out ")
        _T("the ") WEBSITE _T(" website to look for servers to connect to by ")
        _T("clicking the second checkbox.");

    m_wndComplete.SetWindowText(s);

    TRANSLATE(*this, IDD);

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CWizCompletionPage::OnSetActive()
{
    CPropertySheetEx* pSheet = (CPropertySheetEx*)GetParent();
    ASSERT_KINDOF(CPropertySheetEx, pSheet);
    pSheet->SetWizardButtons(PSWIZB_BACK | PSWIZB_NEXT | PSWIZB_FINISH);

    return CNGWizardPage::OnSetActive();
}

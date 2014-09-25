// WizWelcomePage.cpp : implementation file
//

#include "stdafx.h"
#include "TeamTalkApp.h"
#include "WizWelcomePage.h"
#include ".\wizwelcomepage.h"


// CWizWelcomePage dialog
IMPLEMENT_DYNAMIC(CWizWelcomePage, CNGWizardPage)
    CWizWelcomePage::CWizWelcomePage()
    : CNGWizardPage(CWizWelcomePage::IDD)
    , m_bLanguage(FALSE)
    , m_szLanguage(_T(""))
{
}

CWizWelcomePage::~CWizWelcomePage()
{
}

void CWizWelcomePage::DoDataExchange(CDataExchange* pDX)
{
    CNGWizardPage::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_STATIC_INTRO, m_wndIntro);
    DDX_Control(pDX, IDC_STATIC_WIZWELCOME, m_wndTitle);
    DDX_Check(pDX, IDC_CHECK_UILANGUAGE, m_bLanguage);
    DDX_Control(pDX, IDC_COMBO_LANGUAGE, m_wndLanguage);
    DDX_CBString(pDX, IDC_COMBO_LANGUAGE, m_szLanguage);
    DDX_Control(pDX, IDC_CHECK_UILANGUAGE, m_wndCheckLanguage);
}


BEGIN_MESSAGE_MAP(CWizWelcomePage, CNGWizardPage)
    ON_CBN_SELCHANGE(IDC_COMBO_LANGUAGE, OnCbnSelchangeComboLanguage)
    ON_BN_CLICKED(IDC_CHECK_UILANGUAGE, OnBnClickedCheckUilanguage)
END_MESSAGE_MAP()


// CWizWelcomePage message handlers

BOOL CWizWelcomePage::OnInitDialog()
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

    CStringList languages;
    GetLanguageFiles(languages);

    for(POSITION pos=languages.GetHeadPosition();pos!=NULL;)
        m_wndLanguage.AddString(languages.GetNext(pos));

    if(!m_szLanguage.IsEmpty())
        m_wndLanguage.SelectString(-1, m_szLanguage);
    else
        m_wndLanguage.SetCurSel(0);

    m_wndLanguage.EnableWindow(m_bLanguage);

    TRANSLATE(*this, IDD);

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CWizWelcomePage::OnSetActive()
{
    CPropertySheetEx* pSheet = (CPropertySheetEx*)GetParent();
    ASSERT_KINDOF(CPropertySheetEx, pSheet);
    pSheet->SetWizardButtons(PSWIZB_NEXT);

    return CNGWizardPage::OnSetActive();
}

void CWizWelcomePage::OnCbnSelchangeComboLanguage()
{
    CString szLanguage;
    m_wndLanguage.GetWindowText(szLanguage);
    if(!szLanguage.IsEmpty())
    {
        Languages::Instance(szLanguage);
        TRANSLATE(*this, IDD);
    }
}

void CWizWelcomePage::OnBnClickedCheckUilanguage()
{
    BOOL bEnable = m_wndCheckLanguage.GetCheck() == BST_CHECKED;
    m_wndLanguage.EnableWindow(    bEnable );

    CString szLanguage;
    m_wndLanguage.GetWindowText(szLanguage);

    if(bEnable)
    {
        if(szLanguage != Languages::Instance()->GetCurLanguage())
            OnCbnSelchangeComboLanguage();
    }
    else
    {
        Languages::Instance()->ClearLanguage();
    }
}

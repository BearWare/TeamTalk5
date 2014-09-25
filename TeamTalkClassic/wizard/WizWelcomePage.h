#pragma once

#include "NGWizardPage.h"
#include "afxwin.h"

// CWizWelcomePage dialog

class CWizWelcomePage : public CNGWizardPage
{
    DECLARE_DYNAMIC(CWizWelcomePage)
public:
    CWizWelcomePage();   // standard constructor
    virtual ~CWizWelcomePage();

    virtual BOOL OnSetActive();
    // Dialog Data
    enum { IDD = IDD_WIZARD_WELCOMEPAGE };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    DECLARE_MESSAGE_MAP()

    CStatic m_wndIntro;
    CStatic m_wndTitle;
    CFont font1;
public:
    virtual BOOL OnInitDialog();
    BOOL m_bLanguage;
    CComboBox m_wndLanguage;
    CString m_szLanguage;
    afx_msg void OnCbnSelchangeComboLanguage();
    afx_msg void OnBnClickedCheckUilanguage();
    CButton m_wndCheckLanguage;
};

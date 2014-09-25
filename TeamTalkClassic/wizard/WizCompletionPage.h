#pragma once

#include "NGWizardPage.h"
#include "afxwin.h"

// CWizCompletionPage dialog

class CWizCompletionPage : public CNGWizardPage
{
    DECLARE_DYNAMIC(CWizCompletionPage)

public:
    CWizCompletionPage();
    virtual ~CWizCompletionPage();

    // Dialog Data
    enum { IDD = IDD_WIZARD_COMPLETIONPAGE };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    DECLARE_MESSAGE_MAP()
    CStatic m_wndTitle;
    CFont font1;

public:
    virtual BOOL OnInitDialog();
    virtual BOOL OnSetActive();
    BOOL m_bManual;
    BOOL m_bWebsite;
    CEdit m_wndComplete;
};

#pragma once

#include "NGWizardPage.h"
#include "afxwin.h"

#include "../gui/KeyCompDlg.h"

// CWizGeneralPage dialog

class CWizGeneralPage : public CNGWizardPage
{
    DECLARE_DYNAMIC(CWizGeneralPage)
public:
    CWizGeneralPage();   // standard constructor
    virtual ~CWizGeneralPage();

    teamtalk::HotKey m_Hotkey;
    BOOL    m_bPush;
    BOOL    m_bVoiceAct;
    CString    m_sNickname;
    int m_nInactivity;

    CEdit    m_KeyEdit;
    CButton    m_btnPush;
    CButton    m_btnVoice;
    CButton m_wndKeysBtn;
    CStatic m_wndKeyComp;

    virtual BOOL OnSetActive();

    // Dialog Data
    enum { IDD = IDD_WIZARD_GENERALPAGE };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    DECLARE_MESSAGE_MAP()
public:
    virtual BOOL OnInitDialog();
    afx_msg void OnBnClickedCheckPushtotalk();
    afx_msg void OnBnClickedButtonSetupkeys();
};

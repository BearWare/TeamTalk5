#pragma once

#include "Resizer.h"

#include "ChatRichEditCtrl.h"
#include "MyEdit.h"
#include "afxcmn.h"

#include "MyTab.h"

// CChatTab dialog

class CChatTab : public CMyTab
{
    DECLARE_DYNAMIC(CChatTab)

public:
    CChatTab(CWnd* pParent = NULL);   // standard constructor
    virtual ~CChatTab();

// Dialog Data
    enum { IDD = IDD_TAB_CHAT };

    HACCEL m_hAccel; // accelerator table

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    DECLARE_MESSAGE_MAP()

    CResizer m_resizer;

public:
    virtual BOOL OnInitDialog();
    afx_msg void OnSize(UINT nType, int cx, int cy);
    CChatRichEditCtrl m_wndRichEdit;
    CMyEdit m_wndChanMessage;
protected:
    virtual void OnOK();
public:
    virtual BOOL PreTranslateMessage(MSG* pMsg);
};

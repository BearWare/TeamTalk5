#pragma once

#include "MyTab.h"
#include "afxwin.h"

// CAbuseTab dialog

class CAbuseTab : public CMyTab
{
	DECLARE_DYNAMIC(CAbuseTab)

public:
	CAbuseTab(CWnd* pParent = NULL);   // standard constructor
	virtual ~CAbuseTab();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_TAB_ABUSE };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
    CComboBox m_wndCmdLimit;
};

#pragma once
#include "MyTab.h"
#include "Resizer.h"


// CChannelOpTab dialog

class CChannelOpTab : public CMyTab
{
	DECLARE_DYNAMIC(CChannelOpTab)

public:
	CChannelOpTab(CWnd* pParent = NULL);   // standard constructor
	virtual ~CChannelOpTab();

// Dialog Data
	enum { IDD = IDD_TAB_CHANNELOP };

    CListBox m_wndAvailChannels;
    CListBox m_wndSelChannels;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

    CResizer m_resizer;

public:
    virtual BOOL OnInitDialog();
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnBnClickedButtonAddchannel();
    afx_msg void OnBnClickedButtonDelchannel();

};

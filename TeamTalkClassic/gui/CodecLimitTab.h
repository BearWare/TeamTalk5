#pragma once

#include "MyTab.h"
#include "Resizer.h"

// CCodecLimitTab dialog

class CCodecLimitTab : public CMyTab
{
	DECLARE_DYNAMIC(CCodecLimitTab)

public:
	CCodecLimitTab(CWnd* pParent = NULL);   // standard constructor
	virtual ~CCodecLimitTab();

    CEdit m_wndBitrate;
    int m_nBitrate;

// Dialog Data
	enum { IDD = IDD_TAB_CODECLIMIT };
    CResizer m_resizer;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
    virtual BOOL OnInitDialog();
    afx_msg void OnSize(UINT nType, int cx, int cy);
};

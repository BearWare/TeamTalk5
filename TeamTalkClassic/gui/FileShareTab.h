#pragma once

#include "Resizer.h"
#include "afxcmn.h"

// CFileShareTab dialog

class CFileShareTab : public CDialog
{
	DECLARE_DYNAMIC(CFileShareTab)

public:
	CFileShareTab(CWnd* pParent = NULL);   // standard constructor
	virtual ~CFileShareTab();
	void SetNotifier(CWnd* pWnd);

// Dialog Data
	enum { IDD = IDD_TAB_FILESHARE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

	CResizer m_resizer;
	CWnd* m_pNotifyWnd;
public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
protected:
	virtual void OnOK();
public:
	virtual BOOL OnInitDialog();
protected:
	virtual void OnCancel();
public:
	CListCtrl m_wndFileList;
};

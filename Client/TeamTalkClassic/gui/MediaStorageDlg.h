/*
 * Copyright (c) 2005-2018, BearWare.dk
 * 
 * Contact Information:
 *
 * Bjoern D. Rasmussen
 * Kirketoften 5
 * DK-8260 Viby J
 * Denmark
 * Email: contact@bearware.dk
 * Phone: +45 20 20 54 59
 * Web: http://www.bearware.dk
 *
 * This source code is part of the TeamTalk SDK owned by
 * BearWare.dk. Use of this file, or its compiled unit, requires a
 * TeamTalk SDK License Key issued by BearWare.dk.
 *
 * The TeamTalk SDK License Agreement along with its Terms and
 * Conditions are outlined in the file License.txt included with the
 * TeamTalk SDK distribution.
 *
 */

#pragma once
#include "afxwin.h"


// CMediaStorageDlg dialog

class CMediaStorageDlg : public CDialog
{
	DECLARE_DYNAMIC(CMediaStorageDlg)

public:
	CMediaStorageDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CMediaStorageDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_STORECONVERSATIONS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
    CComboBox m_wndAFF;
    CEdit m_wndAudioDir;
    CString m_szAudioDir;
    BOOL m_bSingleFile;
    BOOL m_bSeparateFiles;
    UINT m_uAFF;
    afx_msg void OnBnClickedButtonBrowse();
    virtual BOOL OnInitDialog();
    afx_msg void OnBnClickedOk();
    afx_msg void OnBnClickedButtonChanlogbrowse();
    afx_msg void OnBnClickedButtonUser2userbrowse();
    CEdit m_wndChanLogDir;
    CString m_szChanLogDir;
    CEdit m_wndUserTxtDir;
    CString m_szUserTxtDir;
};

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
#include "afxcmn.h"


// CDesktopShareDlg dialog

class CDesktopShareDlg : public CDialog
{
	DECLARE_DYNAMIC(CDesktopShareDlg)

public:
	CDesktopShareDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDesktopShareDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_DESKTOPSHARE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

    void UpdateControls();
public:
    afx_msg void OnBnClickedRadioSharespecific();
    afx_msg void OnBnClickedCheckInterval();
    afx_msg void OnBnClickedRadioShareactive();
    afx_msg void OnBnClickedRadioSharedesktop();
    virtual BOOL OnInitDialog();
    CComboBox m_wndWindowTitle;
    CComboBox m_wndColorMode;
    BitmapFormat m_nRGBMode;
    int m_nUpdateInterval;
    HWND m_hShareWnd;
    CButton m_wndInterval;
    CButton m_wndShareDesktop;
    CButton m_wndShareActive;
    CButton m_wndShareTitle;
    BOOL m_bShareDesktop;
    BOOL m_bShareActive;
    BOOL m_bShareTitle;
    CEdit m_wndIntervalValue;
    CSpinButtonCtrl m_wndSpinUpdInterval;
    afx_msg void OnDeltaposSpinUpdateinterval(NMHDR *pNMHDR, LRESULT *pResult);
protected:
    virtual void OnOK();
public:
    BOOL m_bUpdateInterval;
};

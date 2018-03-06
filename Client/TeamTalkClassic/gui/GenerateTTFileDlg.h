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


// CGenerateTTFileDlg dialog

class CGenerateTTFileDlg : public CDialog
{
	DECLARE_DYNAMIC(CGenerateTTFileDlg)

public:
	CGenerateTTFileDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CGenerateTTFileDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_GENERATETTFILE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

    void LoadVideoFormats();

public:
    virtual BOOL OnInitDialog();
    teamtalk::HostEntry m_hostentry;
    CEdit m_wndUsername;
    CEdit m_wndPassword;
    CEdit m_wndNickname;
    CButton m_wndMale;
    CButton m_wndFemale;
    CButton m_wndPttChkBox;
    CButton m_wndSetupKeys;
    CEdit m_wndKeyComb;
    teamtalk::HotKey m_Hotkey;
    afx_msg void OnBnClickedCheckClientoverride();
    afx_msg void OnBnClickedCheckPushtotalk();
    afx_msg void OnBnClickedButtonSetupkeys();
    afx_msg void OnBnClickedButtonSavettfile();
    CButton m_wndOverrideClient;
    CComboBox m_wndVidCodec;
    CEdit m_wndVidBitrate;
    CSpinButtonCtrl m_wndVidBitrateSpinCtrl;
    afx_msg void OnCbnSelchangeComboVidcodec();
    CButton m_wndVox;
    CComboBox m_wndCapfmt;
    std::vector<VideoFormat> m_vidcap_fmts;
};

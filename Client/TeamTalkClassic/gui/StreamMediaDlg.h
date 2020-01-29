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

#include "Helper.h"

// CStreamMediaDlg dialog

class CStreamMediaDlg : public CDialog
{
	DECLARE_DYNAMIC(CStreamMediaDlg)

public:
	CStreamMediaDlg(teamtalk::ClientXML& xmlSettings, CWnd* pParent = NULL);   // standard constructor
	virtual ~CStreamMediaDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_STREAMMEDIA };

    void ProcessTTMessage(const TTMessage& msg);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

    void UpdateMediaFile(const CString szFileName);
    void UpdateControls();

    teamtalk::ClientXML& m_xmlSettings;

    int m_nPlaybackID = 0;
    MediaFilePlayback m_mfp = {};
    MediaFileInfo m_mfi = {};

    void UpdateOffset();

public:
    afx_msg void OnBnClickedButtonBrowse();
    CComboBox m_wndVidCodec;
    int m_nVidCodecBitrate;
    CSpinButtonCtrl m_wndVidBitrateSpinCtrl;
    virtual BOOL OnInitDialog();
    CStatic m_wndAudioFormat;
    CStatic m_wndVideoFormat;
    CEdit m_wndVideoBitrate;
    CStringList m_fileList;
    CComboBox m_wndFilename;
    virtual void OnOK();
    CComboBox m_wndAudioPreprocessor;
    CButton m_wndAudioSetup;
    CSliderCtrl m_wndOffset;
    CStatic m_wndTimeOffset;
    CButton m_wndStopPlayback;
    CButton m_wndStartPlayback;
    CStatic m_wndDuration;
    afx_msg void OnBnClickedButtonAudiosetup();
    afx_msg void OnNMCustomdrawSliderOffset(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnBnClickedButtonStop();
    afx_msg void OnBnClickedButtonPlay();
    afx_msg void OnNMReleasedcaptureSliderOffset(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnTRBNThumbPosChangingSliderOffset(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnCbnSelchangeComboAudiopreprocessor();
    afx_msg void OnCbnSelchangeComboFilename();
    CButton m_wndRepeat;
};

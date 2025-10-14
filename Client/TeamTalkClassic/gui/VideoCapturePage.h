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


// CVideoCapturePage dialog

class CVideoCapturePage : public CPropertyPage
{
	DECLARE_DYNAMIC(CVideoCapturePage)

public:
	CVideoCapturePage();
	virtual ~CVideoCapturePage();

// Dialog Data
	enum { IDD = IDD_PROPPAGE_VIDEOCAPTURE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	DECLARE_MESSAGE_MAP()
    
    int m_nVidDev;
public:
    CString m_szVidDevID;
    int m_nCapFormatIndex;
    VideoFormat m_capformat;

    CComboBox m_wndVidDev;
    CComboBox m_wndVidRes;
    CButton m_wndRgb32;
    CButton m_wndI420;
    CButton m_wndYUY2;
    BOOL m_bRGB32;
    BOOL m_bI420;
    BOOL m_bYUY2;
    CComboBox m_wndVidCodec;
    virtual BOOL OnInitDialog();
    afx_msg void OnBnClickedButtonVidtest();

private:
    std::vector<VideoCaptureDevice> m_videodevices;
public:
    CButton m_wndVidTest;
    afx_msg void OnCbnSelchangeComboViddev();
    afx_msg void OnBnClickedRadioVidrgb32();
    afx_msg void OnBnClickedRadioVidi420();
    afx_msg void OnBnClickedRadioVidyuy2();
    CSpinButtonCtrl m_wndVidBitrateSpinCtrl;
    int m_nVidCodecBitrate;
    virtual void OnOK();
    afx_msg void OnCbnSelchangeComboVidres();
};

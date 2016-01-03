/*
 * Copyright (c) 2005-2016, BearWare.dk
 * 
 * Contact Information:
 *
 * Bjoern D. Rasmussen
 * Skanderborgvej 40 4-2
 * DK-8000 Aarhus C
 * Denmark
 * Email: contact@bearware.dk
 * Phone: +45 20 20 54 59
 * Web: http://www.bearware.dk
 *
 * This source code is part of the TeamTalk 5 SDK owned by
 * BearWare.dk. All copyright statements may not be removed 
 * or altered from any source distribution. If you use this
 * software in a product, an acknowledgment in the product 
 * documentation is required.
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

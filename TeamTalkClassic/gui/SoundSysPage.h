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

#include "afxcmn.h"
#include "afxwin.h"

#include <vector>
#include <queue>

// CSoundSysPage dialog

#ifndef UNDEFINED
#define UNDEFINED -1
#endif

class CSoundSysPage
    : public CPropertyPage
{
    DECLARE_DYNAMIC(CSoundSysPage)

public:
    CSoundSysPage();
    virtual ~CSoundSysPage();
    void ShowDrivers(SoundSystem nSoundSystem);
    int m_nInputDevice;
    CString m_szInputDeviceID;
    int m_nOutputDevice;
    CString m_szOutputDeviceID;

    CComboBox m_InputDriversCombo;
    CComboBox m_OutputDriversCombo;
    CButton m_WinButton;
    CButton m_DxButton;
    CButton m_WasApiButton;
    BOOL m_bDenoise;
    CButton m_btnDenoiseBtn;
    CButton m_wndDefaultBtn;

    // Dialog Data
    enum { IDD = IDD_PROPPAGE_SOUNDSYSPAGE };

protected:

    CButton m_wndTestBtn;
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    DECLARE_MESSAGE_MAP()

    void RefreshSoundDevices();
    void UpdateSoundControls();

    BOOL m_bTesting;
    TTSoundLoop* m_SndLoopBack;
    BOOL StartTest();
    void StopTest();

    typedef std::map<int, SoundDevice> sounddevs_t;
    sounddevs_t m_SoundDevices;

public:
    afx_msg void OnBnClickedRadioWinaudio();
    virtual BOOL OnInitDialog();
    afx_msg void OnBnClickedRadioDirectsound();
    afx_msg void OnCbnSelchangeComboInputdriver();
    afx_msg void OnCbnSelchangeComboOutputdriver();
    afx_msg void OnBnClickedButtonTest();
    afx_msg void OnBnClickedDefault();
    CStatic m_wndInputText;
    CStatic m_wndOutputText;
    BOOL m_bPositioning;
    virtual BOOL OnKillActive();
    virtual void OnCancel();
    CButton m_wndPositionBtn;
    CButton m_btnDuplexMode;
    BOOL m_bDuplexMode;
    CButton m_btnEchoCancel;
    BOOL m_bEchoCancel;
    afx_msg void OnBnClickedCheckDuplexmode();
    afx_msg void OnBnClickedCheckEchochannel();
    afx_msg void OnBnClickedCheckDenoise();
    CStatic m_wndInputSampleRates;
    CStatic m_wndOutputSampleRates;
    afx_msg void OnBnClickedRadioWasapi();
    BOOL m_bAGC;
    CButton m_wndAGC;
    afx_msg void OnBnClickedButtonRefreshsnd();
    CButton m_wndRefreshDevs;
    int m_nMediaVsVoice;
    CSliderCtrl m_wndMediaVsVoice;
};

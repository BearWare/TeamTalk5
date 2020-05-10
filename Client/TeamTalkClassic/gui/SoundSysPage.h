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
    CButton m_btnEchoCancel;
    BOOL m_bEchoCancel;
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
    afx_msg void OnBnClickedCheckAgc();
};

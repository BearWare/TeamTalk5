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

#include "NGWizardPage.h"

#include "afxwin.h"
#include "../gui/SoundSysPage.h"

#include <vector>

// CWizSoundSysPage dialog

class CWizSoundSysPage : public CNGWizardPage
{
    DECLARE_DYNAMIC(CWizSoundSysPage)
public:
    CWizSoundSysPage();   // standard constructor
    virtual ~CWizSoundSysPage();

    void ShowDrivers(SoundSystem nSoundSystem);
    int m_nInputDevice;
    int m_nOutputDevice;

    CComboBox    m_InputDriversCombo;
    CComboBox    m_OutputDriversCombo;
    CButton    m_WasAPIButton;
    CButton    m_DxButton;
    CButton m_wndTestBtn;

    virtual BOOL OnSetActive();

    // Dialog Data
    enum { IDD = IDD_WIZARD_SOUNDSYSPAGE };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    void Close();

    DECLARE_MESSAGE_MAP()

    BOOL m_bTesting;
    TTSoundLoop* m_SndLoopBack;
    std::vector<SoundDevice> m_devices;

    BOOL StartTest();
    void StopTest();

public:
    virtual BOOL OnInitDialog();
    afx_msg void OnBnClickedRadioWasAPI();
    afx_msg void OnBnClickedRadioDirectsound();
    afx_msg void OnCbnSelchangeComboInputdriver();
    afx_msg void OnCbnSelchangeComboOutputdriver();
    afx_msg void OnBnClickedButtonTest();
};

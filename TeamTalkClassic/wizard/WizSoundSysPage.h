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
    SoundSystem m_nSoundSystem;

    CComboBox    m_InputDriversCombo;
    CComboBox    m_OutputDriversCombo;
    CButton    m_WinButton;
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

    BOOL StartTest();
    void StopTest();

public:
    virtual BOOL OnInitDialog();
    afx_msg void OnBnClickedRadioWinaudio();
    afx_msg void OnBnClickedRadioDirectsound();
    afx_msg void OnCbnSelchangeComboInputdriver();
    afx_msg void OnCbnSelchangeComboOutputdriver();
    afx_msg void OnBnClickedButtonTest();
    CComboBox m_wndMixerCombo;
    afx_msg void OnCbnSelchangeComboMixer();
    int m_nMixerInput;
};

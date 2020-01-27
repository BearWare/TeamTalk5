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

#include "stdafx.h"
#include "Resource.h"
#include "SoundSysPage.h"
#include <set>


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace std;
extern TTInstance* ttInst;

// CSoundSysPage dialog

IMPLEMENT_DYNAMIC(CSoundSysPage, CPropertyPage)
CSoundSysPage::CSoundSysPage()
: CPropertyPage(CSoundSysPage::IDD)
, m_nInputDevice(UNDEFINED)
, m_nOutputDevice(UNDEFINED)
, m_bPositioning(FALSE)
, m_bTesting(FALSE)
, m_bDenoise(FALSE)
, m_bDuplexMode(DEFAULT_SOUND_DUPLEXMODE)
, m_bEchoCancel(DEFAULT_ECHO_ENABLE)
, m_SndLoopBack(NULL)
, m_bAGC(DEFAULT_AGC_ENABLE)
, m_nMediaVsVoice(DEFAULT_MEDIA_VS_VOICE)
{
    TT_GetDefaultSoundDevices(&m_nInputDevice, &m_nOutputDevice);
}

CSoundSysPage::~CSoundSysPage()
{
}

void CSoundSysPage::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_COMBO_INPUTDRIVER, m_InputDriversCombo);
    DDX_Control(pDX, IDC_COMBO_OUTPUTDRIVER, m_OutputDriversCombo);
    DDX_Control(pDX, IDC_RADIO_WASAPI, m_WasApiButton);
    DDX_Control(pDX, IDC_RADIO_WINAUDIO, m_WinButton);
    DDX_Control(pDX, IDC_RADIO_DIRECTSOUND, m_DxButton);
    DDX_Control(pDX, IDC_BUTTON_TEST, m_wndTestBtn);
    DDX_Control(pDX, IDC_STATIC_INPUT, m_wndInputText);
    DDX_Control(pDX, IDC_STATIC_OUTPUT, m_wndOutputText);
    DDX_Check(pDX, IDC_CHECK_POSITIONING, m_bPositioning);
    DDX_Control(pDX, IDC_CHECK_POSITIONING, m_wndPositionBtn);
    DDX_Check(pDX, IDC_CHECK_DENOISE, m_bDenoise);
    DDX_Control(pDX, IDC_CHECK_DENOISE, m_btnDenoiseBtn);
    DDX_Control(pDX, IDC_BUTTON_DEFAULT, m_wndDefaultBtn);
    DDX_Control(pDX, IDC_CHECK_DUPLEXMODE, m_btnDuplexMode);
    DDX_Check(pDX, IDC_CHECK_DUPLEXMODE, m_bDuplexMode);
    DDX_Control(pDX, IDC_CHECK_ECHOCANCEL, m_btnEchoCancel);
    DDX_Check(pDX, IDC_CHECK_ECHOCANCEL, m_bEchoCancel);
    DDX_Control(pDX, IDC_STATIC_INPUT_SAMPLERATES, m_wndInputSampleRates);
    DDX_Control(pDX, IDC_STATIC_OUTPUT_SAMPLERATES, m_wndOutputSampleRates);
    DDX_Check(pDX, IDC_CHECK_AGC, m_bAGC);
    DDX_Control(pDX, IDC_CHECK_AGC, m_wndAGC);
    DDX_Control(pDX, IDC_BUTTON_REFRESHSND, m_wndRefreshDevs);
    DDV_MinMaxInt(pDX, m_nMediaVsVoice, 0, 200);
    DDX_Slider(pDX, IDC_SLIDER_MEDIASTREAM_VOL, m_nMediaVsVoice);
    DDX_Control(pDX, IDC_SLIDER_MEDIASTREAM_VOL, m_wndMediaVsVoice);
}


BEGIN_MESSAGE_MAP(CSoundSysPage, CPropertyPage)
    ON_BN_CLICKED(IDC_RADIO_WINAUDIO, OnBnClickedRadioWinaudio)
    ON_BN_CLICKED(IDC_RADIO_DIRECTSOUND, OnBnClickedRadioDirectsound)
    ON_CBN_SELCHANGE(IDC_COMBO_INPUTDRIVER, OnCbnSelchangeComboInputdriver)
    ON_CBN_SELCHANGE(IDC_COMBO_OUTPUTDRIVER, OnCbnSelchangeComboOutputdriver)
    ON_BN_CLICKED(IDC_BUTTON_TEST, OnBnClickedButtonTest)
    ON_BN_CLICKED(IDC_BUTTON_DEFAULT, OnBnClickedDefault)
    ON_BN_CLICKED(IDC_CHECK_DUPLEXMODE, &CSoundSysPage::OnBnClickedCheckDuplexmode)
    ON_BN_CLICKED(IDC_CHECK_ECHOCANCEL, &CSoundSysPage::OnBnClickedCheckEchochannel)
    ON_BN_CLICKED(IDC_CHECK_DENOISE, &CSoundSysPage::OnBnClickedCheckDenoise)
    ON_BN_CLICKED(IDC_RADIO_WASAPI, &CSoundSysPage::OnBnClickedRadioWasapi)
    ON_BN_CLICKED(IDC_BUTTON_REFRESHSND, &CSoundSysPage::OnBnClickedButtonRefreshsnd)
END_MESSAGE_MAP()


// CSoundSysPage message handlers
BOOL CSoundSysPage::OnInitDialog()
{
    CPropertyPage::OnInitDialog();

    TRANSLATE(*this, IDD);

    ClientFlags uFlags = TT_GetFlags(ttInst);
    m_wndRefreshDevs.EnableWindow((uFlags &
        (CLIENT_SNDINPUT_READY | CLIENT_SNDOUTPUT_READY | CLIENT_SNDINOUTPUT_DUPLEX)) == 0);

    RefreshSoundDevices();

    m_wndMediaVsVoice.SetRange(0, 200, TRUE);
    m_wndMediaVsVoice.SetPos(m_nMediaVsVoice);
    
    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

void CSoundSysPage::OnBnClickedRadioWinaudio()
{
    ShowDrivers(SOUNDSYSTEM_WINMM);
}

void CSoundSysPage::OnBnClickedRadioDirectsound()
{
    ShowDrivers(SOUNDSYSTEM_DSOUND);
}

void CSoundSysPage::OnBnClickedRadioWasapi()
{
    ShowDrivers(SOUNDSYSTEM_WASAPI);
}

void CSoundSysPage::ShowDrivers(SoundSystem nSoundSystem)
{
    m_OutputDriversCombo.ResetContent();
    BOOL bFoundNoDev = FALSE;
    sounddevs_t::const_iterator ii = m_SoundDevices.begin();
    for(;ii != m_SoundDevices.end();ii++)
    {
        bFoundNoDev |= ii->second.nDeviceID == TT_SOUNDDEVICE_ID_TEAMTALK_VIRTUAL;
        if(ii->second.nSoundSystem != nSoundSystem ||
           ii->second.nMaxOutputChannels == 0)
            continue;
        int index = m_OutputDriversCombo.AddString(ii->second.szDeviceName);
        m_OutputDriversCombo.SetItemData(index, ii->second.nDeviceID);
    }

    CString szNoDev = _T("No Sound Device");
    TRANSLATE_ITEM(IDS_NOSOUNDDEVICE, szNoDev);
    if(bFoundNoDev)
    {
        int index = m_OutputDriversCombo.AddString(szNoDev);
        m_OutputDriversCombo.SetItemData(index, TT_SOUNDDEVICE_ID_TEAMTALK_VIRTUAL);
    }

    bool bFound = false;
    for(int i=0;i<m_OutputDriversCombo.GetCount() && m_szOutputDeviceID.GetLength() && !bFound;++i)
    {
        if(m_szOutputDeviceID == m_SoundDevices[int(m_OutputDriversCombo.GetItemData(i))].szDeviceID)
        {
            m_OutputDriversCombo.SetCurSel(i);
            bFound = true;
        }
    }

    for(int i=0;i<m_OutputDriversCombo.GetCount() && !bFound;i++)
    {
        if(m_OutputDriversCombo.GetItemData(i) == m_nOutputDevice)
        {
            m_OutputDriversCombo.SetCurSel(i);
            bFound = true;
            break;
        }
    }

    if(!bFound && m_OutputDriversCombo.GetCount())
    {
        m_OutputDriversCombo.SetCurSel(0);
        m_nOutputDevice = int(m_OutputDriversCombo.GetItemData(0));
    }

    //fill input
    m_InputDriversCombo.ResetContent();
    ii = m_SoundDevices.begin();
    for(;ii != m_SoundDevices.end();ii++)
    {
        if(ii->second.nSoundSystem != nSoundSystem ||
           ii->second.nMaxInputChannels == 0)
            continue;
        int index = m_InputDriversCombo.AddString( ii->second.szDeviceName );
        m_InputDriversCombo.SetItemData(index, ii->second.nDeviceID);
    }

    if(bFoundNoDev)
    {
        int index = m_InputDriversCombo.AddString(szNoDev);
        m_InputDriversCombo.SetItemData(index, TT_SOUNDDEVICE_ID_TEAMTALK_VIRTUAL);
    }

    bFound = false;
    for(int i=0;i<m_InputDriversCombo.GetCount() && m_szInputDeviceID.GetLength() && !bFound;++i)
    {
        if(m_szInputDeviceID == m_SoundDevices[int(m_InputDriversCombo.GetItemData(i))].szDeviceID)
        {
            m_InputDriversCombo.SetCurSel(i);
            bFound = true;
        }
    }

    for(int i=0;i<m_InputDriversCombo.GetCount() && !bFound;i++)
    {
        if(m_InputDriversCombo.GetItemData(i) == m_nInputDevice)
        {
            m_InputDriversCombo.SetCurSel(i);
            bFound = true;
        }
    }
    if(!bFound && m_InputDriversCombo.GetCount())
    {
        m_InputDriversCombo.SetCurSel(0);
        m_nInputDevice = int(m_InputDriversCombo.GetItemData(0));
    }

    m_DxButton.SetCheck(nSoundSystem == SOUNDSYSTEM_DSOUND? BST_CHECKED : BST_UNCHECKED);
    m_WinButton.SetCheck(nSoundSystem == SOUNDSYSTEM_WINMM? BST_CHECKED : BST_UNCHECKED);
    m_WasApiButton.SetCheck(nSoundSystem == SOUNDSYSTEM_WASAPI? BST_CHECKED : BST_UNCHECKED);

    OnCbnSelchangeComboOutputdriver();
    OnCbnSelchangeComboInputdriver();
}

void CSoundSysPage::OnCbnSelchangeComboInputdriver()
{
    m_nInputDevice = int(m_InputDriversCombo.GetItemData(m_InputDriversCombo.GetCurSel()));
    if(m_SoundDevices.find(m_nInputDevice) != m_SoundDevices.end())
        m_szInputDeviceID = m_SoundDevices[m_nInputDevice].szDeviceID;
    else
        m_szInputDeviceID.Empty();

    UpdateSoundControls();
}

void CSoundSysPage::OnCbnSelchangeComboOutputdriver()
{
    m_nOutputDevice = int(m_OutputDriversCombo.GetItemData(m_OutputDriversCombo.GetCurSel()));
    if(m_SoundDevices.find(m_nOutputDevice) != m_SoundDevices.end())
        m_szOutputDeviceID = m_SoundDevices[m_nOutputDevice].szDeviceID;
    else
        m_szOutputDeviceID.Empty();

    UpdateSoundControls();
}

void CSoundSysPage::OnBnClickedButtonTest()
{
    if(m_nInputDevice == UNDEFINED)
    {
        AfxMessageBox(_T("No input device selected"));
        return;
    }
    if(m_nOutputDevice == UNDEFINED)
    {
        AfxMessageBox(_T("No output device selected"));
        return;
    }

    CString szTest;
    if(!m_bTesting)
    {
        m_bTesting = StartTest();
        if(m_bTesting)
        {
            szTest.LoadString(IDS_STOP);
            TRANSLATE_ITEM(IDS_STOP, szTest);
            m_wndTestBtn.SetWindowText(szTest);
            m_DxButton.EnableWindow(FALSE);
            m_WinButton.EnableWindow(FALSE);
            m_WasApiButton.EnableWindow(FALSE);
            m_OutputDriversCombo.EnableWindow(FALSE);
            m_InputDriversCombo.EnableWindow(FALSE);
        }
    }
    else
    {
        StopTest();
        szTest.LoadString(IDS_TESTSELECTED);
        TRANSLATE_ITEM(IDS_TESTSELECTED, szTest);
        m_wndTestBtn.SetWindowText(szTest);
        m_DxButton.EnableWindow(TRUE);
        m_WinButton.EnableWindow(TRUE);
        m_WasApiButton.EnableWindow(TRUE);
        m_OutputDriversCombo.EnableWindow(TRUE);
        m_InputDriversCombo.EnableWindow(TRUE);
        m_bTesting = FALSE;
    }
}

BOOL CSoundSysPage::OnKillActive()
{
    if(m_bTesting)
        StopTest();

    TRACE(_T("Input device ID: %s\n"), m_szInputDeviceID);
    TRACE(_T("Output device ID: %s\n"), m_szOutputDeviceID);

    return CPropertyPage::OnKillActive();
}

void CSoundSysPage::OnCancel()
{
    if(m_bTesting)
        StopTest();

    CPropertyPage::OnCancel();
}

BOOL CSoundSysPage::StartTest()
{
    if(m_bTesting)
        return FALSE;

    sounddevs_t::const_iterator ii = m_SoundDevices.find(m_nInputDevice);
    if(ii == m_SoundDevices.end())
        return FALSE;

    int nChannels = 1;
    const SoundDevice& in_dev = ii->second;

    SpeexDSP spxdsp = {};
    spxdsp.bEnableAGC = DEFAULT_AGC_ENABLE;
    spxdsp.nGainLevel = DEFAULT_AGC_GAINLEVEL;
    spxdsp.nMaxIncDBSec = DEFAULT_AGC_INC_MAXDB;
    spxdsp.nMaxDecDBSec = DEFAULT_AGC_DEC_MAXDB;
    spxdsp.nMaxGainDB = DEFAULT_AGC_GAINMAXDB;

    spxdsp.bEnableDenoise = m_bDenoise;
    spxdsp.nMaxNoiseSuppressDB = DEFAULT_DENOISE_SUPPRESS;

    spxdsp.bEnableEchoCancellation = m_bEchoCancel;
    spxdsp.nEchoSuppress = DEFAULT_ECHO_SUPPRESS;
    spxdsp.nEchoSuppressActive = DEFAULT_ECHO_SUPPRESSACTIVE;

    m_SndLoopBack = TT_StartSoundLoopbackTest(m_nInputDevice, 
                                              m_nOutputDevice, 
                                              in_dev.nDefaultSampleRate, 
                                              nChannels,
                                              m_btnDuplexMode.GetCheck() == BST_CHECKED,
                                              &spxdsp);
    if(!m_SndLoopBack)
    {
        AfxMessageBox(_T("Failed to initialize sound devices. Check your selected input and output devices."));
        return FALSE;
    }

    return TRUE;
}

void CSoundSysPage::StopTest()
{
    TT_CloseSoundLoopbackTest(m_SndLoopBack);
}

void CSoundSysPage::RefreshSoundDevices()
{
    m_SoundDevices.clear();

    int count = 0;
    std::vector<SoundDevice> devices;
    TT_GetSoundDevices(NULL, &count);
    if(count)
    {
        devices.resize(count);
        TT_GetSoundDevices(&devices[0], &count);
    }
    for(int i=0; i<count; i++)
        m_SoundDevices[devices[i].nDeviceID] = devices[i];

    sounddevs_t::const_iterator ii = m_SoundDevices.find(m_nOutputDevice);
    if(ii != m_SoundDevices.end())
    {
        ShowDrivers(ii->second.nSoundSystem != SOUNDSYSTEM_NONE?
            ii->second.nSoundSystem : SOUNDSYSTEM_WASAPI);
    }
    UpdateSoundControls();
}

void CSoundSysPage::UpdateSoundControls()
{
    BOOL bDuplexOk = m_nInputDevice>=0 && m_nOutputDevice>=0;

    sounddevs_t::const_iterator ii;
    CString szInputSampleRates, szOutputSampleRates;
    SoundDevice in_dev = {};
    ii = m_SoundDevices.find(m_nInputDevice);
    if(ii != m_SoundDevices.end())
    {
        in_dev = ii->second;
        szInputSampleRates.Format(_T("%d"), in_dev.inputSampleRates[0]);
        for(int i=1;i<TT_SAMPLERATES_MAX && in_dev.inputSampleRates[i]>0;i++)
        {
            CString s;
            s.Format(_T(", %d"), in_dev.inputSampleRates[i]);
            szInputSampleRates += s;
        }
    }

    CString sound3d;
    SoundDevice out_dev = {};
    ii = m_SoundDevices.find(m_nOutputDevice);
    if(ii != m_SoundDevices.end())
    {
        out_dev = ii->second;
        if(out_dev.bSupports3D)
        {
            sound3d.LoadString(IDS_SUPPORTS3D);
            TRANSLATE_ITEM(IDS_SUPPORTS3D, sound3d);
            m_wndPositionBtn.EnableWindow(TRUE);
        }
        else
        {
            sound3d.LoadString(IDS_NOSUPPORTS3D);
            TRANSLATE_ITEM(IDS_NOSUPPORTS3D, sound3d);
            m_wndPositionBtn.EnableWindow(TRUE);
        }
        szOutputSampleRates.Format(_T("%d"), out_dev.outputSampleRates[0]);
        for(int i=1;i<TT_SAMPLERATES_MAX && out_dev.outputSampleRates[i]>0;i++)
        {
            CString s;
            s.Format(_T(", %d"), out_dev.outputSampleRates[i]);
            szOutputSampleRates += s;
        }
    }
    
    //
    std::set<int> output_rates(out_dev.outputSampleRates,
                               out_dev.outputSampleRates + TT_SAMPLERATES_MAX);

    bDuplexOk &= (BOOL)in_dev.nDefaultSampleRate>0 && (output_rates.find(in_dev.nDefaultSampleRate) != output_rates.end());

    m_wndInputSampleRates.SetWindowText(szInputSampleRates);
    m_wndOutputSampleRates.SetWindowText(szOutputSampleRates);

    m_bDuplexMode &= bDuplexOk;
    m_bEchoCancel &= (m_bDuplexMode && bDuplexOk);
    m_btnDuplexMode.EnableWindow(bDuplexOk);
    m_btnDuplexMode.SetCheck(bDuplexOk && m_bDuplexMode?BST_CHECKED:BST_UNCHECKED);
    m_btnEchoCancel.EnableWindow(bDuplexOk && m_bDuplexMode);
    m_btnEchoCancel.SetCheck(m_bDuplexMode && m_bEchoCancel?BST_CHECKED:BST_UNCHECKED);
    m_wndPositionBtn.EnableWindow(!m_bDuplexMode && out_dev.nSoundSystem == SOUNDSYSTEM_DSOUND);
    m_wndPositionBtn.SetCheck(m_wndPositionBtn.GetCheck() == BST_CHECKED && !m_bDuplexMode);
}

void CSoundSysPage::OnBnClickedDefault()
{
    TT_GetDefaultSoundDevices(&m_nInputDevice, &m_nOutputDevice);
    
    sounddevs_t::const_iterator ii = m_SoundDevices.find(m_nOutputDevice);
    if(ii != m_SoundDevices.end())
        ShowDrivers(ii->second.nSoundSystem);

    m_btnDuplexMode.SetCheck(DEFAULT_SOUND_DUPLEXMODE? BST_CHECKED : BST_UNCHECKED);
    m_btnDenoiseBtn.SetCheck(DEFAULT_DENOISE_ENABLE? BST_CHECKED : BST_UNCHECKED);
    m_btnEchoCancel.SetCheck(DEFAULT_ECHO_ENABLE? BST_CHECKED : BST_UNCHECKED);
    m_wndAGC.SetCheck(DEFAULT_AGC_ENABLE? BST_CHECKED : BST_UNCHECKED);
    m_wndMediaVsVoice.SetPos(DEFAULT_MEDIA_VS_VOICE);
}

void CSoundSysPage::OnBnClickedCheckDuplexmode()
{
    m_bDuplexMode = m_btnDuplexMode.GetCheck() == BST_CHECKED;
    UpdateSoundControls();
}

void CSoundSysPage::OnBnClickedCheckEchochannel()
{
    m_bEchoCancel = m_btnEchoCancel.GetCheck() == BST_CHECKED;
    UpdateSoundControls();
}

void CSoundSysPage::OnBnClickedCheckDenoise()
{
    m_bDenoise = m_btnDenoiseBtn.GetCheck() == BST_CHECKED;
    UpdateSoundControls();
}


void CSoundSysPage::OnBnClickedButtonRefreshsnd()
{
    TT_RestartSoundSystem();
    RefreshSoundDevices();
}

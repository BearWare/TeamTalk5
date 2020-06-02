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

// wizard\WizSoundSysPage.cpp : implementation file
//

#include "stdafx.h"
#include "TeamTalkApp.h"
#include "WizSoundSysPage.h"
extern TTInstance* ttInst;

// CWizSoundSysPage dialog
IMPLEMENT_DYNAMIC(CWizSoundSysPage, CNGWizardPage)
    CWizSoundSysPage::CWizSoundSysPage()
    : CNGWizardPage(CWizSoundSysPage::IDD)
    , m_nInputDevice(UNDEFINED)
    , m_nOutputDevice(UNDEFINED)
    , m_bTesting(FALSE)
    , m_SndLoopBack(NULL)
{
}

CWizSoundSysPage::~CWizSoundSysPage()
{
    Close();
}

void CWizSoundSysPage::DoDataExchange(CDataExchange* pDX)
{
    CNGWizardPage::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_COMBO_INPUTDRIVER, m_InputDriversCombo);
    DDX_Control(pDX, IDC_COMBO_OUTPUTDRIVER, m_OutputDriversCombo);
    DDX_Control(pDX, IDC_RADIO_WASAPI, m_WasAPIButton);
    DDX_Control(pDX, IDC_RADIO_DIRECTSOUND, m_DxButton);
    DDX_Control(pDX, IDC_BUTTON_TEST, m_wndTestBtn);
}

void CWizSoundSysPage::Close()
{
    if(m_bTesting)
        StopTest();
}

BEGIN_MESSAGE_MAP(CWizSoundSysPage, CNGWizardPage)
    ON_BN_CLICKED(IDC_RADIO_WASAPI, OnBnClickedRadioWasAPI)
    ON_BN_CLICKED(IDC_RADIO_DIRECTSOUND, OnBnClickedRadioDirectsound)
    ON_CBN_SELCHANGE(IDC_COMBO_INPUTDRIVER, OnCbnSelchangeComboInputdriver)
    ON_CBN_SELCHANGE(IDC_COMBO_OUTPUTDRIVER, OnCbnSelchangeComboOutputdriver)
    ON_BN_CLICKED(IDC_BUTTON_TEST, OnBnClickedButtonTest)
END_MESSAGE_MAP()


// CWizSoundSysPage message handlers

BOOL CWizSoundSysPage::OnSetActive()
{
    CPropertySheetEx* pSheet = (CPropertySheetEx*)GetParent();
    ASSERT_KINDOF(CPropertySheetEx, pSheet);
    pSheet->SetWizardButtons(PSWIZB_BACK | PSWIZB_NEXT);

    return CNGWizardPage::OnSetActive();
}

BOOL CWizSoundSysPage::OnInitDialog()
{
    CNGWizardPage::OnInitDialog();

    TRANSLATE(*this, IDD);

    int count = 0;
    TT_GetSoundDevices(NULL, &count);
    if (count)
    {
        m_devices.resize(count);
        TT_GetSoundDevices(&m_devices[0], &count);
    }

    // The default radiobutton is automatically triggered so we cannot select
    // current selection (seems like a bug in the NGWizard)
/*
    for (size_t i=0;i<m_devices.size();++i)
    {
        if (m_devices[i].nDeviceID == m_nOutputDevice)
        {
            switch (m_devices[i].nSoundSystem)
            {
            case SOUNDSYSTEM_WASAPI :
                m_WasAPIButton.SetCheck(BST_CHECKED);
                OnBnClickedRadioWasAPI();
                break;
            case SOUNDSYSTEM_DSOUND :
                m_DxButton.SetCheck(BST_CHECKED);
                OnBnClickedRadioDirectsound();
                break;
            }
            break;
        }
    }
*/
    m_WasAPIButton.SetCheck(BST_CHECKED);
    OnBnClickedRadioWasAPI();

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

void CWizSoundSysPage::OnBnClickedRadioWasAPI()
{
    TT_GetDefaultSoundDevicesEx(SOUNDSYSTEM_WASAPI, &m_nInputDevice, &m_nOutputDevice);
    ShowDrivers(SOUNDSYSTEM_WASAPI);
}

void CWizSoundSysPage::OnBnClickedRadioDirectsound()
{
    TT_GetDefaultSoundDevicesEx(SOUNDSYSTEM_DSOUND, &m_nInputDevice, &m_nOutputDevice);
    ShowDrivers(SOUNDSYSTEM_DSOUND);
}

void CWizSoundSysPage::OnCbnSelchangeComboInputdriver()
{
    m_nInputDevice = GetItemData(m_InputDriversCombo, m_nInputDevice);
}

void CWizSoundSysPage::OnCbnSelchangeComboOutputdriver()
{
    m_nOutputDevice = GetItemData(m_OutputDriversCombo, m_nOutputDevice);
}

void CWizSoundSysPage::ShowDrivers(SoundSystem nSoundSystem)
{
    //fill output
    m_OutputDriversCombo.ResetContent();
    for(size_t i=0; i<m_devices.size(); i++)
    {
        if(m_devices[i].nSoundSystem != nSoundSystem ||
            m_devices[i].nMaxOutputChannels == 0)
            continue;
        int index = m_OutputDriversCombo.AddString(CString(m_devices[i].szDeviceName));
        m_OutputDriversCombo.SetItemData(index, m_devices[i].nDeviceID);
    }

    bool bFound = false;
    for(int i=0;i<m_OutputDriversCombo.GetCount();i++)
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
    }

    //fill input
    m_InputDriversCombo.ResetContent();
    for(size_t j=0;j<m_devices.size();j++)
    {
        if(m_devices[j].nSoundSystem != nSoundSystem ||
            m_devices[j].nMaxInputChannels == 0)
            continue;
        int index = m_InputDriversCombo.AddString( CString(m_devices[j].szDeviceName) );
        m_InputDriversCombo.SetItemData(index, m_devices[j].nDeviceID);
    }

    bFound = false;
    for(int i=0;i<m_InputDriversCombo.GetCount();i++)
    {
        if(m_InputDriversCombo.GetItemData(i) == m_nInputDevice)
        {
            m_InputDriversCombo.SetCurSel(i);
            bFound = true;
            break;
        }
    }

    if(!bFound && m_InputDriversCombo.GetCount())
    {
        m_InputDriversCombo.SetCurSel(0);
    }

    OnCbnSelchangeComboOutputdriver();
    OnCbnSelchangeComboInputdriver();
}

void CWizSoundSysPage::OnBnClickedButtonTest()
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
            m_wndTestBtn.SetWindowText(szTest);
            TRANSLATE(m_wndTestBtn, IDS_STOP, _T("Stop"));
            m_DxButton.EnableWindow(FALSE);
            m_WasAPIButton.EnableWindow(FALSE);
            m_OutputDriversCombo.EnableWindow(FALSE);
            m_InputDriversCombo.EnableWindow(FALSE);
        }
    }
    else
    {
        StopTest();
        szTest.LoadString(IDS_TESTSELECTED);
        m_wndTestBtn.SetWindowText(szTest);
        TRANSLATE(m_wndTestBtn, IDS_TESTSELECTED, _T("Test selected"));
        m_DxButton.EnableWindow(TRUE);
        m_WasAPIButton.EnableWindow(TRUE);
        m_OutputDriversCombo.EnableWindow(TRUE);
        m_InputDriversCombo.EnableWindow(TRUE);
        m_bTesting = FALSE;
    }
}

BOOL CWizSoundSysPage::StartTest()
{
    ASSERT(!m_bTesting);
    BOOL bRet = FALSE;

    if(!m_bTesting)
    {
        m_SndLoopBack = TT_StartSoundLoopbackTest(m_nInputDevice, m_nOutputDevice, 
            16000, 1, FALSE, NULL);
        if(!m_SndLoopBack)
        {
            AfxMessageBox(_T("Failed to initialize sound devices. Check your selected input and output devices."));
            bRet = FALSE;
        }
        else
            bRet = TRUE;
    }

    return bRet;
}

void CWizSoundSysPage::StopTest()
{
    TT_CloseSoundLoopbackTest(m_SndLoopBack);
}

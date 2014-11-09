/*
 * Copyright (c) 2005-2014, BearWare.dk
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


#include "stdafx.h"
#include "Resource.h"
#include "VideoCapturePage.h"
#include "UserVideoDlg.h"
#include <Helper.h>

extern TTInstance* ttInst;

// CVideoCapturePage dialog

IMPLEMENT_DYNAMIC(CVideoCapturePage, CPropertyPage)

CVideoCapturePage::CVideoCapturePage()
	: CPropertyPage(CVideoCapturePage::IDD)
    , m_bRGB32(TRUE)
    , m_bI420(FALSE)
    , m_bYUY2(FALSE)
    , m_nVidDev(-1)
    , m_nCapFormatIndex(-1)
    , m_nVidCodecBitrate(DEFAULT_WEBM_VP8_BITRATE)
    , m_capformat()
{

}

CVideoCapturePage::~CVideoCapturePage()
{
}

void CVideoCapturePage::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_COMBO_VIDDEV, m_wndVidDev);
    DDX_Control(pDX, IDC_COMBO_VIDRES, m_wndVidRes);
    DDX_Control(pDX, IDC_RADIO_VIDRGB32, m_wndRgb32);
    DDX_Control(pDX, IDC_RADIO_VIDI420, m_wndI420);
    DDX_Control(pDX, IDC_RADIO_VIDYUY2, m_wndYUY2);
    DDX_Control(pDX, IDC_COMBO_VIDCODEC, m_wndVidCodec);
    DDX_Control(pDX, IDC_BUTTON_VIDTEST, m_wndVidTest);
    DDX_Control(pDX, IDC_SPIN_VIDBITRATE, m_wndVidBitrateSpinCtrl);
    DDX_Text(pDX, IDC_EDIT_VIDBITRATE, m_nVidCodecBitrate);
}


BEGIN_MESSAGE_MAP(CVideoCapturePage, CPropertyPage)
    ON_BN_CLICKED(IDC_BUTTON_VIDTEST, &CVideoCapturePage::OnBnClickedButtonVidtest)
    ON_CBN_SELCHANGE(IDC_COMBO_VIDDEV, &CVideoCapturePage::OnCbnSelchangeComboViddev)
    ON_BN_CLICKED(IDC_RADIO_VIDRGB32, &CVideoCapturePage::OnBnClickedRadioVidrgb32)
    ON_BN_CLICKED(IDC_RADIO_VIDI420, &CVideoCapturePage::OnBnClickedRadioVidi420)
    ON_BN_CLICKED(IDC_RADIO_VIDYUY2, &CVideoCapturePage::OnBnClickedRadioVidyuy2)
    ON_CBN_SELCHANGE(IDC_COMBO_VIDRES, &CVideoCapturePage::OnCbnSelchangeComboVidres)
END_MESSAGE_MAP()


// CVideoCapturePage message handlers

BOOL CVideoCapturePage::OnInitDialog()
{
    CPropertyPage::OnInitDialog();
    TRANSLATE(*this, IDD);

    AddString(m_wndVidCodec, _T("WebM VP8"), WEBM_VP8_CODEC);
    SetCurSelItemData(m_wndVidCodec, DEFAULT_VIDEOCODEC);
    m_wndVidBitrateSpinCtrl.SetRange(0, 1000);

    int count = 0;
    TT_GetVideoCaptureDevices(NULL, &count);
    m_videodevices.resize(count);

    if(count == 0)
    {
        //No devices... disable everything
        m_wndVidDev.EnableWindow(FALSE);
        m_wndVidRes.EnableWindow(FALSE);
        m_wndVidTest.EnableWindow(FALSE);
        m_wndRgb32.EnableWindow(FALSE);
        m_wndI420.EnableWindow(FALSE);
        m_wndYUY2.EnableWindow(FALSE);
        m_wndVidCodec.EnableWindow(FALSE);
    }
    else
        TT_GetVideoCaptureDevices(&m_videodevices[0], &count);

    for(size_t i=0;i<m_videodevices.size();i++)
    {
        int iDev = m_wndVidDev.AddString(m_videodevices[i].szDeviceName);
        if(m_videodevices[i].szDeviceID == m_szVidDevID)
        {
            m_wndVidDev.SetCurSel(iDev);
            m_nVidDev = i;
        }
    }

    if(m_nVidDev < 0)
        m_wndVidDev.SetCurSel(0);
    
    if(m_nVidDev >= 0 && m_nCapFormatIndex >= 0)
    {
        m_wndVidRes.SetCurSel(m_nCapFormatIndex);
        switch(m_videodevices[m_nVidDev].videoFormats[m_nCapFormatIndex].picFourCC)
        {
        case FOURCC_RGB32 :
            m_wndRgb32.SetCheck(BST_CHECKED);
            break;
        case FOURCC_I420 :
            m_wndI420.SetCheck(BST_CHECKED);
            break;
        case FOURCC_YUY2 :
            m_wndYUY2.SetCheck(BST_CHECKED);
            break;
        }
    }
    else
        m_wndRgb32.SetCheck(BST_CHECKED);

    OnCbnSelchangeComboViddev();

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

void CVideoCapturePage::OnBnClickedButtonVidtest()
{
    TT_CloseVideoCaptureDevice(ttInst);

    int iDev = m_wndVidDev.GetCurSel();
    int iFormat = m_wndVidRes.GetItemData(m_wndVidRes.GetCurSel());
    if(TT_InitVideoCaptureDevice(ttInst, m_videodevices[iDev].szDeviceID,
       &m_videodevices[iDev].videoFormats[iFormat]))
    {
        int fps = 30;

        if(m_videodevices[iDev].videoFormats[iFormat].nFPS_Denominator)
            fps = m_videodevices[iDev].videoFormats[iFormat].nFPS_Numerator / m_videodevices[iDev].videoFormats[iFormat].nFPS_Denominator;

        int msec = 1000 / fps;
        CUserVideoDlg dlg(0 | VIDEOTYPE_CAPTURE, msec, this);
        dlg.DoModal();
        TT_CloseVideoCaptureDevice(ttInst);
    }
    else
        MessageBox(_T("Failed to start selected device"), _T("Test Selected"), MB_OK);
}

void CVideoCapturePage::OnCbnSelchangeComboViddev()
{
    m_wndVidRes.ResetContent();
    if(m_videodevices.empty())
        return;

    BOOL bRgb32 = FALSE, bI420 = FALSE, bYUY2 = FALSE;
    FourCC fourcc = FOURCC_NONE;
    if(m_wndRgb32.GetCheck() == BST_CHECKED)
        fourcc = FOURCC_RGB32;
    if(m_wndI420.GetCheck() == BST_CHECKED)
        fourcc = FOURCC_I420;
    if(m_wndYUY2.GetCheck() == BST_CHECKED)
        fourcc = FOURCC_YUY2;

    int iDev = m_wndVidDev.GetCurSel();
    for(int i=0;i<m_videodevices[iDev].nVideoFormatsCount;i++)
    {
        bRgb32 |= m_videodevices[iDev].videoFormats[i].picFourCC == FOURCC_RGB32;
        bI420 |= m_videodevices[iDev].videoFormats[i].picFourCC == FOURCC_I420;
        bYUY2 |=  m_videodevices[iDev].videoFormats[i].picFourCC == FOURCC_YUY2;

        if(m_videodevices[iDev].videoFormats[i].picFourCC != fourcc)
            continue;

        int fps = m_videodevices[iDev].videoFormats[i].nFPS_Numerator /
            m_videodevices[iDev].videoFormats[i].nFPS_Denominator;

        CString szCapFormat;
        szCapFormat.Format(_T("%dx%d, FPS %d"),
            m_videodevices[iDev].videoFormats[i].nWidth,
            m_videodevices[iDev].videoFormats[i].nHeight,
            fps);
        int iCap = m_wndVidRes.AddString(szCapFormat);
        m_wndVidRes.SetItemData(iCap, i);
        if(m_nVidDev == iDev && i == m_nCapFormatIndex)
            m_wndVidRes.SetCurSel(i);
    }
    if(m_wndVidRes.GetCurSel()<0)
        m_wndVidRes.SetCurSel(0);

    m_wndRgb32.EnableWindow(bRgb32);
    m_wndI420.EnableWindow(bI420);
    m_wndYUY2.EnableWindow(bYUY2);
}

void CVideoCapturePage::OnBnClickedRadioVidrgb32()
{
    OnCbnSelchangeComboViddev();
}

void CVideoCapturePage::OnBnClickedRadioVidi420()
{
    OnCbnSelchangeComboViddev();
}

void CVideoCapturePage::OnBnClickedRadioVidyuy2()
{
    OnCbnSelchangeComboViddev();
}

void CVideoCapturePage::OnCbnSelchangeComboVidres()
{
}

void CVideoCapturePage::OnOK()
{
    int iDev = m_wndVidDev.GetCurSel();
    if(m_videodevices.size())
    {
        m_szVidDevID = m_videodevices[iDev].szDeviceID;
        m_nCapFormatIndex = m_wndVidRes.GetItemData(m_wndVidRes.GetCurSel());
        m_capformat = m_videodevices[iDev].videoFormats[m_nCapFormatIndex];
    }
    CPropertyPage::OnOK();
}


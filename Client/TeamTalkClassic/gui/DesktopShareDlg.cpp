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
#include "DesktopShareDlg.h"

IMPLEMENT_DYNAMIC(CDesktopShareDlg, CDialog)

CDesktopShareDlg::CDesktopShareDlg(CWnd* pParent /*= NULL*/)
    : CDialog(CDesktopShareDlg::IDD, pParent)
    , m_nRGBMode(BMP_RGB16_555)
    , m_nUpdateInterval(DEFAULT_SENDDESKTOPWINDOW_TIMEOUT)
    , m_bShareDesktop(FALSE)
    , m_bShareActive(FALSE)
    , m_bShareTitle(FALSE)
    , m_hShareWnd(NULL)
    , m_bUpdateInterval(TRUE)
{
}

CDesktopShareDlg::~CDesktopShareDlg()
{
}

void CDesktopShareDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_COMBO_WINDOWTITLE, m_wndWindowTitle);
    DDX_Control(pDX, IDC_COMBO_COLORMODE, m_wndColorMode);
    DDX_Text(pDX, IDC_EDIT_INTERVAL, m_nUpdateInterval);
    DDV_MinMaxInt(pDX, m_nUpdateInterval, 100, 60000);
    DDX_Control(pDX, IDC_CHECK_INTERVAL, m_wndInterval);
    DDX_Control(pDX, IDC_RADIO_SHAREDESKTOP, m_wndShareDesktop);
    DDX_Control(pDX, IDC_RADIO_SHAREACTIVE, m_wndShareActive);
    DDX_Control(pDX, IDC_RADIO_SHARESPECIFIC, m_wndShareTitle);
    DDX_Check(pDX, IDC_RADIO_SHAREDESKTOP, m_bShareDesktop);
    DDX_Check(pDX, IDC_RADIO_SHAREACTIVE, m_bShareActive);
    DDX_Check(pDX, IDC_RADIO_SHARESPECIFIC, m_bShareTitle);
    DDX_Control(pDX, IDC_EDIT_INTERVAL, m_wndIntervalValue);
    DDX_Control(pDX, IDC_SPIN_UPDATEINTERVAL, m_wndSpinUpdInterval);
    DDX_Check(pDX, IDC_CHECK_INTERVAL, m_bUpdateInterval);
}

void CDesktopShareDlg::UpdateControls()
{
    m_wndWindowTitle.EnableWindow(m_wndShareTitle.GetCheck() == BST_CHECKED);
    m_wndIntervalValue.EnableWindow(m_wndInterval.GetCheck() == BST_CHECKED);
}

BEGIN_MESSAGE_MAP(CDesktopShareDlg, CDialog)
    ON_BN_CLICKED(IDC_RADIO_SHARESPECIFIC, &CDesktopShareDlg::OnBnClickedRadioSharespecific)
    ON_BN_CLICKED(IDC_CHECK_INTERVAL, &CDesktopShareDlg::OnBnClickedCheckInterval)
    ON_BN_CLICKED(IDC_RADIO_SHAREACTIVE, &CDesktopShareDlg::OnBnClickedRadioShareactive)
    ON_BN_CLICKED(IDC_RADIO_SHAREDESKTOP, &CDesktopShareDlg::OnBnClickedRadioSharedesktop)
    ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_UPDATEINTERVAL, &CDesktopShareDlg::OnDeltaposSpinUpdateinterval)
END_MESSAGE_MAP()

void CDesktopShareDlg::OnBnClickedRadioSharespecific()
{
    UpdateControls();
}

void CDesktopShareDlg::OnBnClickedCheckInterval()
{
    UpdateControls();
}

void CDesktopShareDlg::OnBnClickedRadioShareactive()
{
    UpdateControls();
}

void CDesktopShareDlg::OnBnClickedRadioSharedesktop()
{
    UpdateControls();
}

BOOL CDesktopShareDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    TRANSLATE(*this, IDD);

    UpdateControls();

    m_wndSpinUpdInterval.SetRange32(100, 60000);

    int i;
    i = m_wndColorMode.AddString(LoadText(IDS_DESKTOPDLGLOW, _T("Low (8-bit colors)")));
    m_wndColorMode.SetItemData(i, BMP_RGB8_PALETTE);
    i = m_wndColorMode.AddString(LoadText(IDS_DESKTOPDLGMEDIUM, _T("Medium (16-bit colors)")));
    m_wndColorMode.SetItemData(i, BMP_RGB16_555);
    i = m_wndColorMode.AddString(LoadText(IDS_DESKTOPDLGHIGH, _T("High (24-bit colors)")));
    m_wndColorMode.SetItemData(i, BMP_RGB24);
    i = m_wndColorMode.AddString(LoadText(IDS_DESKTOPDLGMAXIMUM, _T("Maximum (32-bit colors)")));
    m_wndColorMode.SetItemData(i, BMP_RGB32);
    m_wndColorMode.SetCurSel(0);

    HWND hWnd = NULL;
    i = 0;
    int sel = -1;
    while(TT_Windows_GetDesktopWindowHWND(i, &hWnd))
    {
        ShareWindow wnd;
        if(TT_Windows_GetWindow(hWnd, &wnd) && _tcslen(wnd.szWindowTitle))
        {
            int x = m_wndWindowTitle.AddString(wnd.szWindowTitle);
            m_wndWindowTitle.SetItemData(x, (DWORD_PTR)hWnd);
            if(hWnd == m_hShareWnd)
                sel = x;
        }
        i++;
    }

    if(sel>=0)
        m_wndWindowTitle.SetCurSel(sel);
    else if(i)
        m_wndWindowTitle.SetCurSel(1);

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

void CDesktopShareDlg::OnDeltaposSpinUpdateinterval(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
    *pResult = 0;

    int n = GetWindowNumber(m_wndIntervalValue);
    if(pNMUpDown->iDelta>0)
        n += 100;
    else
        n -= 100;
    SetWindowNumber(m_wndIntervalValue, n);
}

void CDesktopShareDlg::OnOK()
{
    CDialog::OnOK();
    m_hShareWnd = (HWND)m_wndWindowTitle.GetItemData(m_wndWindowTitle.GetCurSel());
    m_nRGBMode = (BitmapFormat)m_wndColorMode.GetItemData(m_wndColorMode.GetCurSel());
}

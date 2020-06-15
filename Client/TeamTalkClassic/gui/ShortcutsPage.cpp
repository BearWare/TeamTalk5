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
#include "ShortcutsPage.h"
#include ".\shortcutspage.h"


// CShortcutsPage dialog

IMPLEMENT_DYNAMIC(CShortcutsPage, CPropertyPage)
CShortcutsPage::CShortcutsPage()
: CPropertyPage(CShortcutsPage::IDD)
{
}

CShortcutsPage::~CShortcutsPage()
{
}

void CShortcutsPage::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_EDIT_HKVA, m_wndHKVA);
    DDX_Control(pDX, IDC_EDIT_HKIV, m_wndHKIV);
    DDX_Control(pDX, IDC_EDIT_HKLV, m_wndHKLV);
    DDX_Control(pDX, IDC_EDIT_HKMA, m_wndHKMA);
    DDX_Control(pDX, IDC_EDIT_HKIVG, m_wndHKIVG);
    DDX_Control(pDX, IDC_EDIT_HKLVG, m_wndHKLVG);
    DDX_Control(pDX, IDC_BUTTON_HKVA, m_btnHKVA);
    DDX_Control(pDX, IDC_BUTTON_HKIV, m_btnHKIV);
    DDX_Control(pDX, IDC_BUTTON_HKLV, m_btnHKLV);
    DDX_Control(pDX, IDC_BUTTON_HKMA, m_btnHKMA);
    DDX_Control(pDX, IDC_BUTTON_HKIVG, m_btnHKIVG);
    DDX_Control(pDX, IDC_BUTTON_HKLVG, m_btnHKLVG);
    DDX_Control(pDX, IDC_BUTTON_HKMIN, m_btnHKMin);
    DDX_Control(pDX, IDC_EDIT_HKMIN, m_wndHKMin);
}


BEGIN_MESSAGE_MAP(CShortcutsPage, CPropertyPage)
    ON_BN_CLICKED(IDC_BUTTON_HKVA, OnBnClickedButtonHkva)
    ON_BN_CLICKED(IDC_BUTTON_HKIV, OnBnClickedButtonHkiv)
    ON_BN_CLICKED(IDC_BUTTON_HKLV, OnBnClickedButtonHklv)
    ON_BN_CLICKED(IDC_BUTTON_HKMA, OnBnClickedButtonHkma)
    ON_BN_CLICKED(IDC_BUTTON_HKIVG, OnBnClickedButtonHkivg)
    ON_BN_CLICKED(IDC_BUTTON_HKLVG, OnBnClickedButtonHklvg)
    ON_BN_CLICKED(IDC_BUTTON_HKMIN, &CShortcutsPage::OnBnClickedButtonHkmin)
END_MESSAGE_MAP()


// CShortcutsPage message handlers

BOOL CShortcutsPage::OnInitDialog()
{
    CPropertyPage::OnInitDialog();

    TRANSLATE(*this, IDD);

    UpdateHotKeys();

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

void CShortcutsPage::UpdateHotKeys()
{
    m_wndHKVA.SetWindowText(CKeyCompDlg::GetHotkeyString(m_hkVoiceAct));
    m_wndHKIV.SetWindowText(CKeyCompDlg::GetHotkeyString(m_hkVolumePlus));
    m_wndHKLV.SetWindowText(CKeyCompDlg::GetHotkeyString(m_hkVolumeMinus));
    m_wndHKMA.SetWindowText(CKeyCompDlg::GetHotkeyString(m_hkMuteAll));
    m_wndHKIVG.SetWindowText(CKeyCompDlg::GetHotkeyString(m_hkGainPlus));
    m_wndHKLVG.SetWindowText(CKeyCompDlg::GetHotkeyString(m_hkGainMinus));
    m_wndHKMin.SetWindowText(CKeyCompDlg::GetHotkeyString(m_hkMinRestore));

    if(m_hkVoiceAct.size())
        m_btnHKVA.SetWindowText(LoadText(IDS_SHORTCUTPAGECLEAR, _T("Clear")));
    else
        TRANSLATE(m_btnHKVA, IDC_BUTTON_SETUPKEYS, _T("Setup keys"));
    if(m_hkVolumePlus.size())
        m_btnHKIV.SetWindowText(LoadText(IDS_SHORTCUTPAGECLEAR, _T("Clear")));
    else
        TRANSLATE(m_btnHKIV, IDC_BUTTON_SETUPKEYS, _T("Setup keys"));
    if(m_hkVolumeMinus.size())
        m_btnHKLV.SetWindowText(LoadText(IDS_SHORTCUTPAGECLEAR, _T("Clear")));
    else
        TRANSLATE(m_btnHKLV, IDC_BUTTON_SETUPKEYS, _T("Setup keys"));
    if(m_hkMuteAll.size())
        m_btnHKMA.SetWindowText(LoadText(IDS_SHORTCUTPAGECLEAR, _T("Clear")));
    else
        TRANSLATE(m_btnHKMA, IDC_BUTTON_SETUPKEYS, _T("Setup keys"));
    if(m_hkGainPlus.size())
        m_btnHKIVG.SetWindowText(LoadText(IDS_SHORTCUTPAGECLEAR, _T("Clear")));
    else
        TRANSLATE(m_btnHKIVG, IDC_BUTTON_SETUPKEYS, _T("Setup keys"));
    if(m_hkGainMinus.size())
        m_btnHKLVG.SetWindowText(LoadText(IDS_SHORTCUTPAGECLEAR, _T("Clear")));
    else
        TRANSLATE(m_btnHKLVG, IDC_BUTTON_SETUPKEYS, _T("Setup keys"));
    if(m_hkMinRestore.size())
        m_btnHKMin.SetWindowText(LoadText(IDS_SHORTCUTPAGECLEAR, _T("Clear")));
    else
        TRANSLATE(m_btnHKMin, IDC_BUTTON_SETUPKEYS, _T("Setup keys"));
}

void CShortcutsPage::OnBnClickedButtonHkva()
{
    if(m_hkVoiceAct.size())
        m_hkVoiceAct.clear();
    else
    {
        CKeyCompDlg dlg;
        dlg.DoModal();
        m_hkVoiceAct = dlg.m_Hotkey;
    }
    UpdateHotKeys();
}

void CShortcutsPage::OnBnClickedButtonHkiv()
{
    if(m_hkVolumePlus.size())
        m_hkVolumePlus.clear();
    else
    {
        CKeyCompDlg dlg;
        dlg.DoModal();

        m_hkVolumePlus = dlg.m_Hotkey;
    }
    UpdateHotKeys();
}

void CShortcutsPage::OnBnClickedButtonHklv()
{
    if(m_hkVolumeMinus.size())
        m_hkVolumeMinus.clear();
    else
    {
        CKeyCompDlg dlg;
        dlg.DoModal();

        m_hkVolumeMinus = dlg.m_Hotkey;
    }
    UpdateHotKeys();
}

void CShortcutsPage::OnBnClickedButtonHkma()
{
    if(m_hkMuteAll.size())
        m_hkMuteAll.clear();
    else
    {
        CKeyCompDlg dlg;
        dlg.DoModal();

        m_hkMuteAll = dlg.m_Hotkey;
    }
    UpdateHotKeys();
}

void CShortcutsPage::OnBnClickedButtonHkivg()
{
    if(m_hkGainPlus.size())
        m_hkGainPlus.clear();
    else
    {
        CKeyCompDlg dlg;
        dlg.DoModal();

        m_hkGainPlus = dlg.m_Hotkey;
    }
    UpdateHotKeys();
}

void CShortcutsPage::OnBnClickedButtonHklvg()
{
    if(m_hkGainMinus.size())
        m_hkGainMinus.clear();
    else
    {
        CKeyCompDlg dlg;
        dlg.DoModal();

        m_hkGainMinus = dlg.m_Hotkey;
    }
    UpdateHotKeys();
}

void CShortcutsPage::OnBnClickedButtonHkmin()
{
    if(m_hkMinRestore.size())
        m_hkMinRestore.clear();
    else
    {
        CKeyCompDlg dlg;
        dlg.DoModal();

        m_hkMinRestore = dlg.m_Hotkey;
    }
    UpdateHotKeys();
}

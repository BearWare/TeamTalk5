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

#pragma once
#include "afxwin.h"
#include "KeyCompDlg.h"

// CShortcutsPage dialog

class CShortcutsPage : public CPropertyPage
{
    DECLARE_DYNAMIC(CShortcutsPage)

public:
    CShortcutsPage();
    virtual ~CShortcutsPage();

// Dialog Data
    enum { IDD = IDD_PROPPAGE_SHORTCUTS };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    DECLARE_MESSAGE_MAP()
    void UpdateHotKeys();
public:
    CEdit m_wndHKVA;
    CEdit m_wndHKIV;
    CEdit m_wndHKLV;
    CEdit m_wndHKMA;
    CEdit m_wndHKIVG;
    CEdit m_wndHKLVG;
    teamtalk::HotKey m_hkVoiceAct;
    teamtalk::HotKey m_hkVolumePlus;
    teamtalk::HotKey m_hkVolumeMinus;
    teamtalk::HotKey m_hkMuteAll;
    teamtalk::HotKey m_hkGainPlus;
    teamtalk::HotKey m_hkGainMinus;
    teamtalk::HotKey m_hkMinRestore;
    virtual BOOL OnInitDialog();
    afx_msg void OnBnClickedButtonHkva();
    afx_msg void OnBnClickedButtonHkiv();
    afx_msg void OnBnClickedButtonHklv();
    afx_msg void OnBnClickedButtonHkma();
    afx_msg void OnBnClickedButtonHkivg();
    afx_msg void OnBnClickedButtonHklvg();
    CButton m_btnHKVA;
    CButton m_btnHKIV;
    CButton m_btnHKLV;
    CButton m_btnHKMA;
    CButton m_btnHKIVG;
    CButton m_btnHKLVG;
    CButton m_btnHKMin;
    CEdit m_wndHKMin;
    afx_msg void OnBnClickedButtonHkmin();
};

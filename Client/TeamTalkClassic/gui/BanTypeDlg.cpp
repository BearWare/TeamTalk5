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

// gui\BanTypeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "BanTypeDlg.h"
#include "afxdialogex.h"


// CBanTypeDlg dialog

IMPLEMENT_DYNAMIC(CBanTypeDlg, CDialogEx)

CBanTypeDlg::CBanTypeDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG_BANTYPE, pParent)
{
#ifndef _WIN32_WCE
	EnableActiveAccessibility();
#endif
}

CBanTypeDlg::~CBanTypeDlg()
{
}

void CBanTypeDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_COMBO_BANTYPE, m_wndBanType);
}


BEGIN_MESSAGE_MAP(CBanTypeDlg, CDialogEx)
END_MESSAGE_MAP()


// CBanTypeDlg message handlers


BOOL CBanTypeDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    TRANSLATE(*this, IDD);

    CString szText = LoadText(IDS_BANTYPEDLGIPADDR, _T("Ban IP-address"));
    TRANSLATE_ITEM(IDC_STATIC_IPADDR, szText);
    int i = m_wndBanType.AddString(szText);
    m_wndBanType.SetItemData(i, DWORD_PTR(BANTYPE_IPADDR));
    szText = LoadText(IDS_BANTYPEDLGUSERNAME, _T("Ban username"));
    TRANSLATE_ITEM(IDC_STATIC_USERNAME, szText);
    i = m_wndBanType.AddString(szText); 
    m_wndBanType.SetItemData(i, DWORD_PTR(BANTYPE_USERNAME));
    m_wndBanType.SetCurSel(0);
    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}


void CBanTypeDlg::OnOK()
{
    CDialogEx::OnOK();

    m_uBanTypes = BanTypes(m_wndBanType.GetItemData(m_wndBanType.GetCurSel()));
}

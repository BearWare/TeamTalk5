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


// NewItemDlg.cpp : implementation file
//

#include "stdafx.h"
#include "LanguageEditor.h"
#include "NewItemDlg.h"
#include ".\newitemdlg.h"


// CNewItemDlg dialog

IMPLEMENT_DYNAMIC(CNewItemDlg, CDialog)
CNewItemDlg::CNewItemDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CNewItemDlg::IDD, pParent)
	, m_nItemID(0)
	, m_szItemText(_T(""))
	, m_hAccel(NULL)
{
}

CNewItemDlg::~CNewItemDlg()
{
}

void CNewItemDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_ITEMID, m_nItemID);
	DDX_Text(pDX, IDC_EDIT_ITEMTEXT, m_szItemText);
}


BEGIN_MESSAGE_MAP(CNewItemDlg, CDialog)
END_MESSAGE_MAP()


// CNewItemDlg message handlers

BOOL CNewItemDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	//load accelerators
	m_hAccel = ::LoadAccelerators(AfxGetResourceHandle(), (LPCTSTR)IDR_ACCELERATOR2);
	if (!m_hAccel)
		MessageBox(_T("The accelerator table was not loaded"));

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CNewItemDlg::PreTranslateMessage(MSG* pMsg)
{
	if (WM_KEYFIRST <= pMsg->message && pMsg->message <= WM_KEYLAST)
		if (m_hAccel && ::TranslateAccelerator(m_hWnd, m_hAccel, pMsg))
			return TRUE;
	return CDialog::PreTranslateMessage(pMsg);
}

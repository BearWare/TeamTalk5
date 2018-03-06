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


// GotoItemDlg.cpp : implementation file
//

#include "stdafx.h"
#include "LanguageEditor.h"
#include "GotoItemDlg.h"


// CGotoItemDlg dialog

IMPLEMENT_DYNAMIC(CGotoItemDlg, CDialog)
CGotoItemDlg::CGotoItemDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CGotoItemDlg::IDD, pParent)
	, m_nItemID(-1)
{
}

CGotoItemDlg::~CGotoItemDlg()
{
}

void CGotoItemDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_nItemID);
}


BEGIN_MESSAGE_MAP(CGotoItemDlg, CDialog)
END_MESSAGE_MAP()


// CGotoItemDlg message handlers

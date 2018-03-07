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

// gui\DirectConDlg.cpp : implementation file
//

#include "stdafx.h"
#include "../Resource.h"
#include "DirectConDlg.h"
#include ".\directcondlg.h"


// CDirectConDlg dialog

IMPLEMENT_DYNAMIC(CDirectConDlg, CDialog)
CDirectConDlg::CDirectConDlg(CWnd* pParent /*=NULL*/)
    : CDialog(CDirectConDlg::IDD, pParent)
    , m_bDisable(FALSE)
    , m_szMsg(_T(""))
{
}

CDirectConDlg::~CDirectConDlg()
{
}

void CDirectConDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Check(pDX, IDC_CHECK_DISABLE, m_bDisable);
    DDX_Text(pDX, IDC_STATIC_MSG, m_szMsg);
}


BEGIN_MESSAGE_MAP(CDirectConDlg, CDialog)
END_MESSAGE_MAP()


// CDirectConDlg message handlers

BOOL CDirectConDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    TRANSLATE(*this, IDD);

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

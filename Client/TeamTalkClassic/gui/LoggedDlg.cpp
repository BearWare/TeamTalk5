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

// LoggedDlg.cpp : implementation file
//

#include "stdafx.h"
#include "../Resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CLoggedDlg dialog


CLoggedDlg::CLoggedDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CLoggedDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLoggedDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CLoggedDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLoggedDlg)
	DDX_Control(pDX, IDC_LIST1, m_wndList);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLoggedDlg, CDialog)
	//{{AFX_MSG_MAP(CLoggedDlg)
	ON_BN_CLICKED(IDC_BUTTON1, OnButton1)
	ON_WM_SHOWWINDOW()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLoggedDlg message handlers

void CLoggedDlg::WriteLine(CString str)
{
	CTime t = CTime::GetCurrentTime();
	CString s;s.Format(_T("%d:%d:%d, %s"), t.GetHour(), t.GetMinute(), t.GetSecond(), str);
	m_wndList.AddString(s);
}

void CLoggedDlg::OnButton1() 
{
	m_wndList.ResetContent();	
}

void CLoggedDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);
}

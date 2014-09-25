// .\gui\FileShareDlg.cpp : implementation file
//

#include "stdafx.h"
#include "../TeamTalk3.h"
#include "FileShareTab.h"


// CFileShareTab dialog

IMPLEMENT_DYNAMIC(CFileShareTab, CDialog)
CFileShareTab::CFileShareTab(CWnd* pParent /*=NULL*/)
	: CDialog(CFileShareTab::IDD, pParent)
{
}

CFileShareTab::~CFileShareTab()
{
}

void CFileShareTab::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_FILES, m_wndFileList);
}

void CFileShareTab::SetNotifier(CWnd* pWnd)
{
	m_pNotifyWnd = pWnd;
}

BEGIN_MESSAGE_MAP(CFileShareTab, CDialog)
	ON_WM_SIZE()
END_MESSAGE_MAP()


// CFileShareTab message handlers


BOOL CFileShareTab::OnInitDialog()
{
	CDialog::OnInitDialog();

	static CResizer::CBorderInfo s_bi[] = {

		{IDC_LIST_FILES,	
		{CResizer::eFixed, IDC_MAIN, CResizer::eLeft}, 
		{CResizer::eFixed, IDC_MAIN, CResizer::eTop},  
		{CResizer::eFixed, IDC_MAIN, CResizer::eRight},
		{CResizer::eFixed, IDC_MAIN, CResizer::eBottom}}, 
	};

	const nSize = sizeof(s_bi)/sizeof(s_bi[0]);
	m_resizer.Init(m_hWnd, NULL, s_bi, nSize);

	m_wndFileList.InsertColumn(0, "Name");
	m_wndFileList.InsertColumn(1, "Size");
	m_wndFileList.InsertColumn(2, "Owner");
	m_wndFileList.InsertColumn(4, "Date");

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CFileShareTab::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	m_resizer.Move();
}

void CFileShareTab::OnOK()
{
}

void CFileShareTab::OnCancel()
{
}

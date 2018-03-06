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


// LanguageEditorDlg.cpp : implementation file
//

#include "stdafx.h"
#include "LanguageEditor.h"
#include "LanguageEditorDlg.h"
#include "NewItemDlg.h"
#include "FindDlg.h"
#include "GotoItemDlg.h"
#include "Helper.h"

#include <string>
#include ".\languageeditordlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace std;

std::string TO_LOCAL_STR(CString str)
{
	char buff[MAX_PATH] = {0};

#if defined(_UNICODE) || defined(UNICODE)
	wcstombs(buff, str.GetBuffer(), sizeof(buff)-1);
#else
	strncpy(buff, str.GetBuffer(), sizeof(buff)-1);
#endif
	return buff;
}


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogExx
{
public:
	CAboutDlg();

	// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogExx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogExx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogExx)
END_MESSAGE_MAP()


// CLanguageEditorDlg dialog



CLanguageEditorDlg::CLanguageEditorDlg(CWnd* pParent /*=NULL*/)
: CDialogExx(CLanguageEditorDlg::IDD, pParent)
, m_bMasterLng(FALSE)
, m_bTransLng(FALSE)
, m_bMasterMod(FALSE)
, m_bTransMod(FALSE)
, m_hAccel(NULL)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CLanguageEditorDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogExx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_ITEMID, m_wndCurItemID);
	DDX_Control(pDX, IDC_EDIT_ORGLANGTEXT, m_wndOrgItemText);
	DDX_Control(pDX, IDC_EDIT_NEWLANGTEXT, m_wndTransItemText);
	DDX_Control(pDX, IDC_EDIT_MASTERFILE, m_wndMasterFile);
	DDX_Control(pDX, IDC_EDIT_TRANSFILE, m_wndTransFile);
}

BEGIN_MESSAGE_MAP(CLanguageEditorDlg, CDialogExx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_MASTER, OnBnClickedButtonMaster)
	ON_BN_CLICKED(IDC_BUTTON_EXISTING, OnBnClickedButtonExisting)
	ON_BN_CLICKED(IDC_BUTTON_NEW, OnBnClickedButtonNew)
	ON_COMMAND(ID_ITEMS_NEWITEM, OnItemsNewitem)
	ON_COMMAND(ID_ITEMS_NEXTITEM, OnItemsNextitem)
	ON_COMMAND(ID_ITEMS_PREVIOUSITEM, OnItemsPreviousitem)
	ON_COMMAND(ID_ITEMS_DELETEITEM, OnItemsDeleteitem)
	ON_UPDATE_COMMAND_UI(ID_ITEMS_DELETEITEM, OnUpdateItemsDeleteitem)
	ON_UPDATE_COMMAND_UI(ID_ITEMS_NEWITEM, OnUpdateItemsNewitem)
	ON_UPDATE_COMMAND_UI(ID_ITEMS_NEXTITEM, OnUpdateItemsNextitem)
	ON_UPDATE_COMMAND_UI(ID_ITEMS_PREVIOUSITEM, OnUpdateItemsPreviousitem)
	ON_COMMAND(ID_FILE_EXIT, OnFileExit)
	ON_WM_CLOSE()
	ON_COMMAND(ID_ITEMS_PREVIOUSUNTRANSLATEDITEM, OnItemsPreviousuntranslateditem)
	ON_UPDATE_COMMAND_UI(ID_ITEMS_PREVIOUSUNTRANSLATEDITEM, OnUpdateItemsPreviousuntranslateditem)
	ON_COMMAND(ID_ITEMS_NEXTUNTRANSLATEDITEM, OnItemsNextuntranslateditem)
	ON_UPDATE_COMMAND_UI(ID_ITEMS_NEXTUNTRANSLATEDITEM, OnUpdateItemsNextuntranslateditem)
	ON_UPDATE_COMMAND_UI(ID_ITEMS_FINDITEM, OnUpdateItemsFinditem)
	ON_COMMAND(ID_ITEMS_FINDITEM, OnItemsFinditem)
	ON_COMMAND(ID_ITEMS_FIRSTITEM, OnItemsFirstitem)
	ON_UPDATE_COMMAND_UI(ID_ITEMS_FIRSTITEM, OnUpdateItemsFirstitem)
	ON_COMMAND(ID_ITEMS_LASTITEM, OnItemsLastitem)
	ON_UPDATE_COMMAND_UI(ID_ITEMS_LASTITEM, OnUpdateItemsLastitem)
	ON_COMMAND(ID_ITEMS_GOTOITEM, OnItemsGotoitem)
	ON_UPDATE_COMMAND_UI(ID_ITEMS_GOTOITEM, OnUpdateItemsGotoitem)
	ON_COMMAND(ID_FILE_NEWTRANSLATEDLANGUAGE, OnFileNewtranslatedlanguage)
	ON_COMMAND(ID_FILE_OPENMASTERLANGUAGE, OnFileOpenmasterlanguage)
	ON_COMMAND(ID_FILE_OPENTRANSLATEDLANGUAGE, OnFileOpentranslatedlanguage)
	ON_COMMAND(ID_FILE_SAVEMASTERLANGUAGE, OnFileSavemasterlanguage)
//	ON_UPDATE_COMMAND_UI(ID_FILE_OPENTRANSLATEDLANGUAGE, OnUpdateFileOpentranslatedlanguage)
	ON_COMMAND(ID_FILE_SAVETRANSLATEDLANGUAGE, OnFileSavetranslatedlanguage)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVETRANSLATEDLANGUAGE, OnUpdateFileSavetranslatedlanguage)
	ON_UPDATE_COMMAND_UI(ID_FILE_SAVEMASTERLANGUAGE, OnUpdateFileSavemasterlanguage)
	ON_COMMAND(ID_HELP_ABOUT, OnHelpAbout)
    ON_UPDATE_COMMAND_UI(ID_ITEMS_SORTITEMS, &CLanguageEditorDlg::OnUpdateItemsSortitems)
    ON_COMMAND(ID_ITEMS_SORTITEMS, &CLanguageEditorDlg::OnItemsSortitems)
END_MESSAGE_MAP()


// CLanguageEditorDlg message handlers

BOOL CLanguageEditorDlg::OnInitDialog()
{
	CDialogExx::OnInitDialog();

	//load accelerators
	m_hAccel = ::LoadAccelerators(AfxGetResourceHandle(), (LPCTSTR)IDR_ACCELERATOR1);
	if (!m_hAccel)
		MessageBox(_T("The accelerator table was not loaded"));
	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon


	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CLanguageEditorDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogExx::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CLanguageEditorDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogExx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CLanguageEditorDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CLanguageEditorDlg::OnBnClickedButtonMaster()
{
	OnFileOpenmasterlanguage();
}

void CLanguageEditorDlg::OnBnClickedButtonExisting()
{
	OnFileOpentranslatedlanguage();
}

void CLanguageEditorDlg::OnBnClickedButtonNew()
{
	OnFileNewtranslatedlanguage();
}

BOOL CLanguageEditorDlg::PreTranslateMessage(MSG* pMsg)
{
	if (WM_KEYFIRST <= pMsg->message && pMsg->message <= WM_KEYLAST)
		if (m_hAccel)
		{
			int ret = ::TranslateAccelerator(m_hWnd, m_hAccel, pMsg);
			if(pMsg->wParam == VK_RETURN && ret)
				return TRUE;
		}

		return CDialogExx::PreTranslateMessage(pMsg);
}

void CLanguageEditorDlg::OnItemsNewitem()
{
	if(m_bMasterLng)
	{
		CNewItemDlg dlg;
		if(dlg.DoModal() == IDOK)
		{
			m_masterlng.AddItem(dlg.m_nItemID, CONVERTSTR( dlg.m_szItemText ));
			m_bMasterMod = TRUE;
			m_masterlng.SortItems();
			m_translng.SortItems();
			SaveLanguageItem();
			ShowLanguageItem(dlg.m_nItemID);
		}
	}
}

void CLanguageEditorDlg::OnUpdateItemsNewitem(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_bMasterLng);
}

void CLanguageEditorDlg::OnItemsDeleteitem()
{
	int id = GetCurLanguageItem();
	if(m_bMasterLng && id >= 0)
	{
		int next = m_masterlng.GetNextItem(id);
		m_translng.RemoveItem(id);
		m_masterlng.RemoveItem(id);
		ShowLanguageItem(next);
		m_bMasterMod = TRUE;
		if(m_bTransLng)
			m_bTransMod = TRUE;
	}
}

void CLanguageEditorDlg::OnUpdateItemsDeleteitem(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_bMasterLng && GetCurLanguageItem() != -1);
}

void CLanguageEditorDlg::OnItemsNextitem()
{
	int id = GetCurLanguageItem();
	if(m_bMasterLng && id>=0)
	{
		SaveLanguageItem();
		id = m_masterlng.GetNextItem(id);
		if(id >= 0)
			ShowLanguageItem(id);
	}
}

void CLanguageEditorDlg::OnUpdateItemsNextitem(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_bMasterLng && m_masterlng.GetNextItem(GetCurLanguageItem()) != -1);
}

void CLanguageEditorDlg::OnItemsPreviousitem()
{
	int id = GetCurLanguageItem();

	if(m_bMasterLng && id>=0)
	{
		SaveLanguageItem();
		id = m_masterlng.GetPrevItem(id);
		if(id>=0)
			ShowLanguageItem(id);
	}
}

void CLanguageEditorDlg::OnUpdateItemsPreviousitem(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_bMasterLng && m_masterlng.GetPrevItem(GetCurLanguageItem()) != -1);
}

void CLanguageEditorDlg::ShowLanguageItem(int id)
{
	if(id>=0)
	{
		string str = m_masterlng.GetItemText(id);
		if(!str.empty())
		{
			CString szText = CONVERTSTR( str.c_str() );
			CString szID, szOrg, szTrans;
			szID.Format(_T("%d"), id);
			m_wndCurItemID.SetWindowText(szID);
			m_wndOrgItemText.SetWindowText( CONVERTSTR( m_masterlng.GetItemText(id).c_str() ) );
			m_wndTransItemText.SetWindowText( CONVERTSTR( m_translng.GetItemText(id).c_str() ) );
		}
	}
	else
	{
		m_wndCurItemID.SetWindowText( _T("") );
		m_wndOrgItemText.SetWindowText( _T("") );
		m_wndTransItemText.SetWindowText( _T("") );
	}
}

void CLanguageEditorDlg::SaveLanguageItem()
{
	CString szText;
	int id = GetCurLanguageItem();

	if(id>=0)
	{
		m_wndTransItemText.GetWindowText(szText);
		m_translng.AddItem(id, CONVERTSTR( szText.GetBuffer() ));
		if(m_bTransLng)
			m_bTransMod = TRUE;
	}
}

int CLanguageEditorDlg::GetCurLanguageItem()
{
	CString szItemID;
	m_wndCurItemID.GetWindowText(szItemID);
	if(szItemID.GetLength())
		return _ttoi(szItemID);
	return -1;
}

int CLanguageEditorDlg::GetNextUntranslatedItem(int id)
{
	while(id != -1)
	{
		id = m_masterlng.GetNextItem(id);
		if(m_translng.GetItemText(id).empty())
			return id;
	}
	return -1;
}

int CLanguageEditorDlg::GetPrevUntranslatedItem(int id)
{
	while(id != -1)
	{
		id = m_masterlng.GetPrevItem(id);
		if(m_translng.GetItemText(id).empty())
			return id;
	}
	return -1;
}

void CLanguageEditorDlg::OnFileExit()
{
	OnClose();
}

void CLanguageEditorDlg::OnClose()
{
	if(m_bMasterMod && AfxMessageBox(_T("Save changes to master language file?"), MB_YESNO, 0) == IDYES)
	{
		OnFileSavemasterlanguage();
	}
	if( (m_bTransLng && m_bTransMod) && AfxMessageBox(_T("Save changes to translated language file?"), MB_YESNO, 0) == IDYES)
	{
		OnFileSavetranslatedlanguage();
	}

	CDialog::OnCancel();
}

void CLanguageEditorDlg::OnCancel()
{
}

void CLanguageEditorDlg::OnOK()
{
}

void CLanguageEditorDlg::OnItemsPreviousuntranslateditem()
{
	SaveLanguageItem();
	ShowLanguageItem(GetPrevUntranslatedItem(GetCurLanguageItem()));
}

void CLanguageEditorDlg::OnUpdateItemsPreviousuntranslateditem(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(GetPrevUntranslatedItem(GetCurLanguageItem()) != -1);
}

void CLanguageEditorDlg::OnItemsNextuntranslateditem()
{
	SaveLanguageItem();
	ShowLanguageItem(GetNextUntranslatedItem(GetCurLanguageItem()));
}

void CLanguageEditorDlg::OnUpdateItemsNextuntranslateditem(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(GetNextUntranslatedItem(GetCurLanguageItem()) != -1);
}

void CLanguageEditorDlg::OnItemsFinditem()
{
	CFindDlg findDlg;
	findDlg.m_szFindText = m_szLastFind;
	CWnd* wnd = GetFocus();
	if(wnd)
	{
		BOOL bMaster = wnd->m_hWnd == m_wndOrgItemText.m_hWnd;
		BOOL bTrans = wnd->m_hWnd == m_wndTransItemText.m_hWnd;

		if( (bMaster || bTrans) && findDlg.DoModal() == IDOK)
		{
			m_szLastFind = findDlg.m_szFindText;

			int id = GetCurLanguageItem();
			CString szText;
			while(id != -1)
			{
				if(bMaster)
				{
					id = m_masterlng.GetNextItem(id);
					szText = CONVERTSTR( m_masterlng.GetItemText(id).c_str() );
				}
				if(bTrans)
				{
					id = m_translng.GetNextItem(id);
					szText = CONVERTSTR( m_translng.GetItemText(id).c_str() );
				}
				szText = szText.MakeLower();
				findDlg.m_szFindText = findDlg.m_szFindText.MakeLower();
				if(szText.Find(findDlg.m_szFindText) != -1)
				{
					SaveLanguageItem();
					ShowLanguageItem(id);
					break;
				}
			}
		}
	}
}

void CLanguageEditorDlg::OnUpdateItemsFinditem(CCmdUI *pCmdUI)
{
	CWnd* wnd = GetFocus();
	if(wnd)
	{
		BOOL bMaster = wnd->m_hWnd == m_wndTransItemText.m_hWnd;
		BOOL bTrans = wnd->m_hWnd == m_wndOrgItemText.m_hWnd;
		pCmdUI->Enable((bTrans || bMaster) && (m_bTransLng || m_bMasterLng));
	}
}

void CLanguageEditorDlg::OnItemsFirstitem()
{
	if(m_bMasterLng)
	{
		int id = m_masterlng.GetFirstItem();
		if(id>=0)
		{
			SaveLanguageItem();
			ShowLanguageItem(id);
		}
	}
}

void CLanguageEditorDlg::OnUpdateItemsFirstitem(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_bMasterLng);
}

void CLanguageEditorDlg::OnItemsLastitem()
{
	if(m_bMasterLng)
	{
		int id = m_masterlng.GetFirstItem();
		int lastid = id;
		while(id != -1)
		{
			lastid = id;
			id = m_masterlng.GetNextItem(id);
		}
		if(lastid>=0)
		{
			SaveLanguageItem();
			ShowLanguageItem(lastid);
		}
	}
}

void CLanguageEditorDlg::OnUpdateItemsLastitem(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_bMasterLng);
}

void CLanguageEditorDlg::OnItemsGotoitem()
{
	CGotoItemDlg dlg;
	dlg.m_nItemID = GetCurLanguageItem();
	if(dlg.DoModal() == IDOK)
	{
		SaveLanguageItem();
		ShowLanguageItem(dlg.m_nItemID);
	}
}

void CLanguageEditorDlg::OnUpdateItemsGotoitem(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_bMasterLng);
}

void CLanguageEditorDlg::OnFileNewtranslatedlanguage()
{
	if(m_bTransMod && AfxMessageBox(_T("Save changes in translated file?"), MB_YESNO, 0) == IDYES)
	{
		m_translng.SaveFile();
	}

	TCHAR szFilters[] = _T("Language Files (*.lng)|*.lng|All Files (*.*)|*.*||");

	CFileDialog fileDlg(FALSE, _T("lng"), _T("*.lng"), OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY, szFilters, this);
	if(fileDlg.DoModal() == IDOK)
	{
		CString pathName = fileDlg.GetPathName();
		string filename = TO_LOCAL_STR(pathName);
		::DeleteFile(pathName);

		if(!m_translng.CreateFile(filename))
			AfxMessageBox(_T("Failed to create file!"));
		else 
		{
			m_wndTransFile.SetWindowText(fileDlg.GetFileName());
			m_bTransLng = TRUE;
			m_bTransMod = FALSE;
			m_wndTransItemText.SetReadOnly(FALSE);
		}
	}
}

void CLanguageEditorDlg::OnFileOpenmasterlanguage()
{
	TCHAR szFilters[] = _T("Language Files (*.lng)|*.lng|All Files (*.*)|*.*||");

	CFileDialog fileDlg(TRUE, _T("lng"), _T("*.lng"), OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, szFilters, this);
	if(fileDlg.DoModal() == IDOK)
	{
		CString pathName = fileDlg.GetPathName();
		string filename = TO_LOCAL_STR(pathName);
		if(!m_masterlng.LoadFile(filename))
			AfxMessageBox(_T("Failed to load file!"));
		else
		{
			m_wndMasterFile.SetWindowText(fileDlg.GetFileName());
			m_bMasterLng = TRUE;
			m_bTransLng = FALSE;
			m_bMasterMod = m_bTransMod = FALSE;
			m_masterlng.SortItems();
			ShowLanguageItem(m_masterlng.GetFirstItem());
		}
	}
}

void CLanguageEditorDlg::OnFileOpentranslatedlanguage()
{
	if(m_bTransMod && AfxMessageBox(_T("Save changes in translated file?"), MB_YESNO, 0) == IDYES)
	{
		m_translng.SaveFile();
	}

	TCHAR szFilters[] = _T("Language Files (*.lng)|*.lng|All Files (*.*)|*.*||");

	CFileDialog fileDlg(TRUE, _T("lng"), _T("*.lng"), OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, szFilters, this);
	if(fileDlg.DoModal() == IDOK)
	{
		CString pathName = fileDlg.GetPathName();
		string filename = TO_LOCAL_STR(pathName);
		if(!m_translng.LoadFile(filename))
			AfxMessageBox(_T("Failed to Load file!"));
		else 
		{
			m_wndTransFile.SetWindowText(fileDlg.GetFileName());
			m_bTransLng = TRUE;
			m_bTransMod = FALSE;
			m_translng.SortItems();
			ShowLanguageItem(GetCurLanguageItem());
			m_wndTransItemText.SetReadOnly(FALSE);
		}
	}
}

void CLanguageEditorDlg::OnFileSavemasterlanguage()
{
	SaveLanguageItem();
	m_masterlng.SortItems();
	m_masterlng.SaveFile();
	m_bMasterMod = FALSE;
}

void CLanguageEditorDlg::OnUpdateFileSavemasterlanguage(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_bMasterMod);
}

void CLanguageEditorDlg::OnFileSavetranslatedlanguage()
{
	SaveLanguageItem();
	m_translng.SortItems();
	m_translng.SaveFile();
	m_bTransMod = FALSE;
}

void CLanguageEditorDlg::OnUpdateFileSavetranslatedlanguage(CCmdUI *pCmdUI)
{
	pCmdUI->Enable(m_bTransMod);
}

void CLanguageEditorDlg::OnHelpAbout()
{
	CAboutDlg about;
	about.DoModal();
}


void CLanguageEditorDlg::OnUpdateItemsSortitems(CCmdUI *pCmdUI)
{
    pCmdUI->Enable(m_bMasterLng || m_bTransLng);
}


void CLanguageEditorDlg::OnItemsSortitems()
{
    if(m_bMasterLng)
    {
        m_masterlng.SortItems();
        m_bMasterMod = TRUE;
    }
    if(m_bTransLng)
    {
        m_translng.SortItems();
        m_bTransMod = TRUE;
    }
}

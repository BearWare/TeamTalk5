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


// LanguageEditorDlg.h : header file
//

#pragma once
#include "afxwin.h"

#include "DialogEx.h"

#include <settings/XMLLanguages.h>

// CLanguageEditorDlg dialog
class CLanguageEditorDlg : public CDialogExx
{
// Construction
public:
	CLanguageEditorDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_LANGUAGEEDITOR_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;
	HACCEL m_hAccel; // accelerator table
	CString m_szLastFind;

	void SaveLanguageItem();
	void ShowLanguageItem(int id);
	int GetCurLanguageItem();
	int GetNextUntranslatedItem(int id);
	int GetPrevUntranslatedItem(int id);

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CEdit m_wndNewLang;
	CComboBox m_wndCurLang;
	CEdit m_wndCurItemID;
	CEdit m_wndOrgItemText;
	CEdit m_wndTransItemText;
	XMLLanguages m_translng, m_masterlng;
	BOOL m_bMasterLng, m_bTransLng, m_bMasterMod, m_bTransMod;
	afx_msg void OnBnClickedButtonMaster();
	afx_msg void OnBnClickedButtonExisting();
	afx_msg void OnBnClickedButtonNew();
	CEdit m_wndMasterFile;
	CEdit m_wndTransFile;
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnItemsNewitem();
	afx_msg void OnItemsNextitem();
	afx_msg void OnItemsPreviousitem();
	afx_msg void OnItemsDeleteitem();
	afx_msg void OnUpdateItemsDeleteitem(CCmdUI *pCmdUI);
	afx_msg void OnUpdateItemsNewitem(CCmdUI *pCmdUI);
	afx_msg void OnUpdateItemsNextitem(CCmdUI *pCmdUI);
	afx_msg void OnUpdateItemsPreviousitem(CCmdUI *pCmdUI);
	afx_msg void OnFileExit();
	afx_msg void OnClose();
protected:
	virtual void OnCancel();
	virtual void OnOK();
public:
	afx_msg void OnItemsPreviousuntranslateditem();
	afx_msg void OnUpdateItemsPreviousuntranslateditem(CCmdUI *pCmdUI);
	afx_msg void OnItemsNextuntranslateditem();
	afx_msg void OnUpdateItemsNextuntranslateditem(CCmdUI *pCmdUI);
	afx_msg void OnUpdateItemsFinditem(CCmdUI *pCmdUI);
	afx_msg void OnItemsFinditem();
	afx_msg void OnItemsFirstitem();
	afx_msg void OnUpdateItemsFirstitem(CCmdUI *pCmdUI);
	afx_msg void OnItemsLastitem();
	afx_msg void OnUpdateItemsLastitem(CCmdUI *pCmdUI);
	afx_msg void OnItemsGotoitem();
	afx_msg void OnUpdateItemsGotoitem(CCmdUI *pCmdUI);
	afx_msg void OnFileNewtranslatedlanguage();
	afx_msg void OnFileOpenmasterlanguage();
	afx_msg void OnFileOpentranslatedlanguage();
	afx_msg void OnFileSavemasterlanguage();
//	afx_msg void OnUpdateFileOpentranslatedlanguage(CCmdUI *pCmdUI);
	afx_msg void OnFileSavetranslatedlanguage();
	afx_msg void OnUpdateFileSavetranslatedlanguage(CCmdUI *pCmdUI);
	afx_msg void OnUpdateFileSavemasterlanguage(CCmdUI *pCmdUI);
	afx_msg void OnHelpAbout();
    afx_msg void OnUpdateItemsSortitems(CCmdUI *pCmdUI);
    afx_msg void OnItemsSortitems();
 };

#pragma once
#include "afxcmn.h"

#include "Resizer.h"
#include "FilesListCtrl.h"
#include <vector>

#include "MyTab.h"

// CFilesTab dialog

class CFilesTab : public CMyTab
{
    DECLARE_DYNAMIC(CFilesTab)

public:
    CFilesTab(CWnd* pParent = NULL);   // standard constructor
    virtual ~CFilesTab();

// Dialog Data
    enum { IDD = IDD_TAB_FILES };

    void AddFile(int nChannelID, int nFileID);
    void RemoveFile(int nChannelID, int nFileID);
    void UpdateFiles(int nChannelID);

    std::vector<int> GetSelectedFiles();
    HACCEL m_hAccel; // accelerator table
    int GetIndexFromFileID(int nFileID);
protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    DECLARE_MESSAGE_MAP()

    void ResizeHeader();
    CResizer m_resizer;
    int m_nChannelID;
public:
    virtual BOOL OnInitDialog();
    CFilesListCtrl m_wndFiles;
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnNMRclickListFiles(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnLvnBegindragListFiles(NMHDR *pNMHDR, LRESULT *pResult);
    virtual BOOL PreTranslateMessage(MSG* pMsg);
};

#pragma once

// CFilesListCtrl

#define WM_FILESLISTCTRL_FILESDROPPED WM_USER+1

class CFilesListCtrl : public CListCtrl
{
    DECLARE_DYNAMIC(CFilesListCtrl)

public:
    CFilesListCtrl();
    virtual ~CFilesListCtrl();
    CStringList m_Files;

protected:
    DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnDropFiles(HDROP hDropInfo);
    afx_msg void OnHdnDividerdblclick(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnLvnBegindrag(NMHDR *pNMHDR, LRESULT *pResult);
};



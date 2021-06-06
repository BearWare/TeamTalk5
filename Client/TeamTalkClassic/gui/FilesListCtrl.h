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

#pragma once

// CFilesListCtrl

#define WM_FILESLISTCTRL_FILESDROPPED (WM_USER+1)

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
    afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
};



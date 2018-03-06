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
#include "afxwin.h"
#include "SessionTreeCtrl.h"

// CSessionTreeCtrlWin32

class CSessionTreeCtrlWin32 : public CSessionTreeCtrl
{
    DECLARE_DYNAMIC(CSessionTreeCtrlWin32)

public:
    CSessionTreeCtrlWin32();
    virtual ~CSessionTreeCtrlWin32();

protected:
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
    afx_msg void OnRclick(NMHDR* pNMHDR, LRESULT* pResult);
//    afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
    DECLARE_MESSAGE_MAP()

private:
    HTREEITEM m_hTempSelection;
};



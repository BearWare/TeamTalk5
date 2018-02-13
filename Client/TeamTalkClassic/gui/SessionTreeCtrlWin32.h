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



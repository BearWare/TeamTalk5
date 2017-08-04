#pragma once

#include "XPTabCtrl.h"

// CMyTabCtrl

class CMyTabCtrl : public CXPTabCtrl
{
    DECLARE_DYNAMIC(CMyTabCtrl)

public:
    CMyTabCtrl();
    virtual ~CMyTabCtrl();

protected:
    void Sizing();
    DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnSize(UINT nType, int cx, int cy);
    virtual BOOL PreTranslateMessage(MSG* pMsg);
    afx_msg void OnTcnSelchange(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnKillFocus(CWnd* pNewWnd);
};



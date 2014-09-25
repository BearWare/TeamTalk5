#pragma once


// CUserButton
#define WM_USERBUTTON_DRAGGING WM_USER+1

class CUserButton : public CButton
{
    DECLARE_DYNAMIC(CUserButton)

public:
    CUserButton();
    virtual ~CUserButton();
    UINT m_nBtnID;

protected:
    DECLARE_MESSAGE_MAP()
    BOOL m_bDown;
public:
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
};



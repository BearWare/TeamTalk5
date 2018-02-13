#pragma once


// CBitmapSlider

class CBitmapSlider : public CSliderCtrl
{
    DECLARE_DYNAMIC(CBitmapSlider)

public:
    CBitmapSlider();
    virtual ~CBitmapSlider();

protected:
    DECLARE_MESSAGE_MAP()

private:
    CBitmap m_Background, m_Normal, m_Down;
    BOOL m_bMouseDown;
    BOOL m_bMouseLeft;
    CRgn  m_WinRgn;

public:
    void SetBitmap(CBitmap& background, CBitmap& normal, CBitmap& down);
    void InitializeStyle();
    afx_msg void OnPaint();
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnCaptureChanged(CWnd *pWnd);
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg LRESULT OnMouseLeave(WPARAM wParam, LPARAM lParam);
    void GetSliderRect(CRect& rect);
private:
    void SetNewPos(CPoint point);
public:
    void GetBackgroundRect(CRect& rect);
};



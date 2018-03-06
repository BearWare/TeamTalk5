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



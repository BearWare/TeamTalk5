// BitmapSlider.cpp : implementation file
//

#include "stdafx.h"
#include "BitmapSlider.h"


// CBitmapSlider

IMPLEMENT_DYNAMIC(CBitmapSlider, CSliderCtrl)
CBitmapSlider::CBitmapSlider() :
m_bMouseDown(FALSE),
m_bMouseLeft(FALSE)
{
}

CBitmapSlider::~CBitmapSlider()
{
    m_Background.DeleteObject();
    m_Normal.DeleteObject();
    m_Down.DeleteObject();
}


BEGIN_MESSAGE_MAP(CBitmapSlider, CSliderCtrl)
    ON_WM_PAINT()
    ON_WM_LBUTTONDOWN()
    ON_WM_LBUTTONUP()
    ON_WM_MOUSEMOVE()
    ON_WM_CAPTURECHANGED()
    ON_WM_ERASEBKGND()
    ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
END_MESSAGE_MAP()



// CBitmapSlider message handlers


void CBitmapSlider::SetBitmap(CBitmap& background, CBitmap& normal, CBitmap& down)
{
    m_Background.DeleteObject();
    m_Normal.DeleteObject();
    m_Down.DeleteObject();
    
    m_Background.Attach(background); m_Normal.Attach(normal); m_Down.Attach(down);
}

void CBitmapSlider::OnPaint()
{
    CPaintDC dc(this); // device context for painting
    // Do not call CSliderCtrl::OnPaint() for painting messages
    CDC memdc;
    memdc.CreateCompatibleDC(NULL);
    memdc.SelectObject(m_Background);

    CRect r;
    GetClientRect(&r);
    dc.BitBlt(0, 0, r.Width(), r.Height(), &memdc, 0, 0, SRCCOPY);
    CRect Tr;
    GetSliderRect(Tr);/*SLIDERRECT*/
    if (m_bMouseDown)
        memdc.SelectObject(m_Down);
    else
        memdc.SelectObject(m_Normal);
    dc.BitBlt(Tr.left, Tr.top, Tr.Width(), Tr.Height(), &memdc, 0, 0, SRCCOPY);
}

void CBitmapSlider::OnLButtonDown(UINT nFlags, CPoint point)
{
    //CSliderCtrl::OnLButtonDown(nFlags, point);
    
    SetNewPos(point);
    m_bMouseDown = TRUE;

    //::SystemParametersInfo(
    Invalidate();
}

void CBitmapSlider::OnLButtonUp(UINT nFlags, CPoint point)
{
    CSliderCtrl::OnLButtonUp(nFlags, point);
    m_bMouseDown = FALSE;

    Invalidate();
}

void CBitmapSlider::OnMouseMove(UINT nFlags, CPoint point)
{
    //CSliderCtrl::OnMouseMove(nFlags, point);
    
    //check for left handed
    if(GetSystemMetrics(SM_SWAPBUTTON))
        m_bMouseDown = ((GetAsyncKeyState(VK_RBUTTON) & 0x8000) == 0x8000)? TRUE : FALSE;
    else
        m_bMouseDown = ((GetAsyncKeyState(VK_LBUTTON) & 0x8000) == 0x8000)? TRUE : FALSE;

    if(m_bMouseDown)
    {
        SetNewPos(point);
        Invalidate();
    }
    
}

void CBitmapSlider::OnCaptureChanged(CWnd *pWnd)
{
    if(m_bMouseDown)
    {
        ReleaseCapture();
        Invalidate();
    }
    CSliderCtrl::OnCaptureChanged(pWnd);
}

BOOL CBitmapSlider::OnEraseBkgnd(CDC* pDC)
{
    return FALSE;
    //return CSliderCtrl::OnEraseBkgnd(pDC);
}

void CBitmapSlider::GetSliderRect(CRect& rect)
{
    //gets the size of the bmp being moved
    BITMAP bm;
    
    GetObject(m_Normal, sizeof(bm), &bm);
    
    GetThumbRect(&rect);

    if ((GetStyle() & TBS_VERT) == TBS_VERT) 
    {
        int o    = rect.Height()/2;
        rect.left   = 0;
        rect.top    = (rect.top + o) - (bm.bmHeight / 2);
        rect.right  = rect.left + bm.bmWidth;
        if(rect.top < 0)
            rect.top = 0;

        rect.bottom = rect.top + bm.bmHeight;
    }
    else
    {
        BITMAP bg;
        GetObject(m_Background, sizeof(bg), &bg);
        int o    = rect.Width() / 2;
        rect.top = (bg.bmHeight - bm.bmHeight) / 2;
        if(rect.top < 0)
            rect.top = 0;
        rect.bottom = rect.top + bm.bmHeight;
        rect.left = (rect.left + o) - (bm.bmWidth/2);
        rect.right = rect.left + bm.bmWidth;
    }
}

void CBitmapSlider::SetNewPos(CPoint point)
{
    CRect rect;
    GetClientRect(&rect);
    CRect thumbrect;
    GetThumbRect(&thumbrect);
    CRect slider;
    GetSliderRect(slider);
    
    int min,max;
    GetRange(min,max);

    if ((GetStyle() & TBS_VERT) == TBS_VERT)
    {
        float percent = (float)point.y / (float)rect.Height();
        int pos = (int)(percent * (float)max);
        TRACE("Percent %f Point %d\n", percent, point.y);
        SetPos(pos);
    }
    else
    {
        float percent = (float)point.x / (float)rect.Width();
        int pos = (int)(percent * (float)max);
        SetPos(pos);
    }
}

void CBitmapSlider::GetBackgroundRect(CRect& rect)
{
    BITMAP bm;
    GetObject(m_Background, sizeof(bm), &bm);

    rect.top = 0;
    rect.left = 0;
    rect.right = bm.bmWidth;
    rect.bottom = bm.bmHeight;    
}

LRESULT CBitmapSlider::OnMouseLeave(WPARAM wParam, LPARAM lParam)
{
    m_bMouseDown = FALSE;
    m_bMouseLeft = TRUE;
    TRACE("Mouse leave\n");
    return TRUE;
}

void CBitmapSlider::InitializeStyle()
{
    CRect back;
    GetBackgroundRect(back);

    CRect r;
    GetClientRect(&r);
    m_WinRgn.CreateRoundRectRgn( r.left, r.top, r.left+back.right, r.top+back.bottom, 3, 3);
    SetWindowRgn(m_WinRgn , TRUE );
}
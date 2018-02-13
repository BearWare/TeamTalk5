// MyBitmapButton.cpp : implementation file
//

#include "stdafx.h"
#include "MyBitmapButton.h"
#include ".\mybitmapbutton.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMyBitmapButton

CMyBitmapButton::CMyBitmapButton()
{
}

CMyBitmapButton::~CMyBitmapButton()
{
}

void CMyBitmapButton::SetState(BOOL bState)
{
}

UINT CMyBitmapButton::GetState( ) const
{
    return CBitmapButton::GetState();
}

void CMyBitmapButton::SetBitmaps(HBITMAP hNormal/* = 0*/, HBITMAP hSelected/* = 0*/, HBITMAP hFocused/* = 0*/, HBITMAP hDisabled/* = 0*/)
{
  if(hNormal)
    m_bitmap.Attach(hNormal);
  if(hSelected)
    m_bitmapSel.Attach(hSelected);
  if(hFocused)
    m_bitmapFocus.Attach(hFocused);
  if(hDisabled)
    m_bitmapDisabled.Attach(hDisabled);
  SizeToContent();
}

BEGIN_MESSAGE_MAP(CMyBitmapButton, CBitmapButton)
    //{{AFX_MSG_MAP(CMyBitmapButton)
    ON_WM_KILLFOCUS()
    ON_WM_SETFOCUS()
    //}}AFX_MSG_MAP
    ON_WM_KEYDOWN()
    ON_WM_KEYUP()
    ON_WM_SYSKEYUP()
    ON_WM_CONTEXTMENU()
//  ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMyBitmapButton message handlers

void CMyBitmapButton::OnKillFocus(CWnd* pNewWnd) 
{
//    CBitmapButton::OnKillFocus(pNewWnd);
    
}

void CMyBitmapButton::OnSetFocus(CWnd* pOldWnd) 
{
//    CBitmapButton::OnSetFocus(pOldWnd);
    
}


void CMyBitmapButton::SetDown(BOOL bState)
{
    CBitmapButton::SetState(bState);
}

void CMyBitmapButton::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
}

void CMyBitmapButton::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
}

void CMyBitmapButton::OnSysKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
}

void CMyBitmapButton::OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/)
{
}


void CMyBitmapButton::PreSubclassWindow()
{
  CBitmapButton::PreSubclassWindow();

  UINT nStyle = GetButtonStyle();
  // make sure we are an owner draw button
  if( (nStyle & BS_OWNERDRAW) != BS_OWNERDRAW)
    SetButtonStyle( nStyle | BS_OWNERDRAW );
}

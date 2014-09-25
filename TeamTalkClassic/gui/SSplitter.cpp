//------------------------------------------------------------------------------
// File    : SSplitter.cpp
// Version : 1.1
// Date    : 20. January 2004
// Author  : Alexander Atamas
// Email   : atamas@mail.univ.kiev.ua
// Web     : 
// Systems : VC6.0 (Run under Window 98, Windows Nt)
// Remarks : based on Paul DiLascia's WinMgr code
//

// 
// You are free to use/modify this code but leave this header intact.
// This class is public domain so you are free to use it any of your 
// applications (Freeware, Shareware, Commercial). 
// All I ask is that you let me know so that if you have a real winner I can
// brag to my buddies that some of my code is in your app. I also wouldn't 
// mind if you sent me a copy of your application since I like to play with
// new stuff.
//------------------------------------------------------------------------------

#include "stdafx.h"
#include "SSplitter.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSSplitter

CSSplitter::CSSplitter()
{
  m_pWndLeft				= NULL;
  m_pWndRight				= NULL;
  m_pWndTop				= NULL;
  m_pWndBottom			= NULL;

  m_nSplitterWidth		= 3;   // the width of slitter

  m_bMovingHorizSplitter	= FALSE;
  m_bDraggingHoriz		= FALSE;	
  m_bMovingVertSplitter	= FALSE;
  m_bDraggingVert			= FALSE;
  m_bHiddenBottomPane		= FALSE;
  m_bHiddenRightPane		= FALSE;
  m_bHiddenLeftPane		= FALSE;
  m_bHorizSplitter		= FALSE;
  m_bVertSplitter			= FALSE;
}

CSSplitter::~CSSplitter()
{
}


BEGIN_MESSAGE_MAP(CSSplitter, CStatic)
  //{{AFX_MSG_MAP(CSSplitter)
  ON_WM_SIZE()
  ON_WM_LBUTTONDOWN()
  ON_WM_LBUTTONUP()
  ON_WM_MOUSEMOVE()
  ON_WM_DESTROY()
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSSplitter message handlers

CSSplitter::Create(DWORD dwStyle, 
                   CWnd*	pParentWnd, 
                   CWnd* pFPane, 
                   CWnd* pSPane,
                   UINT nID,
                   const RECT& rc,
                   UINT nFConstr,
                   UINT nSConstr)
{
  m_nID  = nID;
  CStatic::Create(NULL, dwStyle, rc, pParentWnd, nID);

  pFPane->SetParent(this);
  pFPane->SetOwner(pParentWnd);

  pSPane->SetParent(this);
  pSPane->SetOwner(pParentWnd);

  m_nMaxTop		= nFConstr;
  m_nMaxBottom	= nSConstr;
  m_nMaxLeft		= nFConstr;
  m_nMaxRight		= nSConstr;

  if ( dwStyle & SS_VERT )
    InitVertSplitter(
    this,				
    pFPane,				
    pSPane,
    nFConstr,
    nSConstr
    ); 

  if ( dwStyle & SS_HORIZ )
    InitHorizSplitter(
    this,				
    pFPane,				
    pSPane,
    nFConstr,
    nSConstr
    );

  return TRUE;
}

BOOL CSSplitter::PreCreateWindow(CREATESTRUCT& cs) 
{
  cs.style |= SS_NOTIFY; // to notify its parent of mouse events	
  return CStatic::PreCreateWindow(cs);
}

void CSSplitter::OnSize(UINT nType, int cx, int cy) 
{
  CStatic::OnSize(nType, cx, cy);

  CRect rect;
  GetClientRect( &rect );

  if ( m_bVertSplitter && (!m_bHiddenRightPane) && (!m_bHiddenLeftPane) ){
    CPoint pt;
    pt.x = m_nLeftPaneWidth + m_nSplitterWidth/2;
    pt.y = 0;
    MoveVertPanes( pt );
  }

  if ( m_bVertSplitter && m_bHiddenRightPane && (m_pWndLeft->GetSafeHwnd()) ){
    m_pWndLeft->MoveWindow(&rect);
  }

  if ( m_bVertSplitter && m_bHiddenLeftPane && (m_pWndRight->GetSafeHwnd()) ){
    m_pWndRight->MoveWindow(&rect);
  }

  if ( m_bHorizSplitter && m_bHiddenBottomPane && (m_pWndTop->GetSafeHwnd()) ){
    m_pWndTop->MoveWindow(&rect);
    return;
  }

  if ( m_bHorizSplitter && (!m_bHiddenBottomPane) ){ 

    CPoint pt;
    pt.x = 0;
    pt.y = cy - m_nBottomPaneHeight - m_nSplitterWidth/2;

    SetHorizConstraint(pt);
    MoveHorizPanes( pt );
  }

}

void CSSplitter::MoveVertPanes(CPoint SplitPoint)
{
  CRect rect;
  GetClientRect( &rect );

  m_rcVertSplitter.SetRect(		
    SplitPoint.x - m_nSplitterWidth,
    rect.top,
    SplitPoint.x + m_nSplitterWidth,
    rect.bottom
    );

  if ( m_pWndLeft->GetSafeHwnd() ){

    m_rcWndLeft.SetRect(
      rect.left,
      rect.top,
      SplitPoint.x - m_nSplitterWidth/2,
      rect.bottom
      );
    m_pWndLeft->MoveWindow(&m_rcWndLeft);
  }

  if ( m_pWndRight->GetSafeHwnd() ){

    m_rcWndRight.SetRect(
      m_rcWndLeft.right + m_nSplitterWidth, 
      rect.top,		
      rect.right,
      rect.bottom
      );
    m_pWndRight->MoveWindow(&m_rcWndRight);
  }

}


void CSSplitter::MoveHorizPanes(CPoint SplitPoint)
{
  CRect rect;
  GetClientRect( &rect );

  m_rcHorizSplitter.SetRect(		
    0,
    SplitPoint.y - m_nSplitterWidth,
    rect.right,
    SplitPoint.y + m_nSplitterWidth
    ); 

  m_rcWndBottom.SetRect(
    rect.left,
    SplitPoint.y + m_nSplitterWidth/2,
    rect.right,
    rect.bottom
    );

  if ( m_pWndBottom->GetSafeHwnd() ){
    m_pWndBottom->MoveWindow(&m_rcWndBottom);
  }

  if ( m_pWndTop->GetSafeHwnd() ){
    m_rcWndTop.SetRect(
      rect.left, 
      rect.top,
      rect.right,
      m_rcWndBottom.top - m_nSplitterWidth
      );
    m_pWndTop->MoveWindow(&m_rcWndTop);
  }

}

void CSSplitter::OnLButtonDown(UINT nFlags, CPoint point) 
{
  CClientDC dc( this );
  dc.DPtoLP( &point );

  if ( m_bVertSplitter && (m_rcVertSplitter.PtInRect( point )) ){
    GetWindowRect(&m_rcVertBar); 
    m_bDraggingVert=TRUE;
    m_ptVertOriginal = m_ptVertPrevious = MakePtVert(point);
    DrawVertBar();					 
    SetCapture();
    m_hwndPrevFocusVert = ::SetFocus(m_hWnd);  
    ::SetCursor( AfxGetApp()->LoadStandardCursor ((LPCTSTR)IDC_SIZEWE));
    m_bMovingVertSplitter = TRUE;
  }

  if ( m_bHorizSplitter && (m_rcHorizSplitter.PtInRect( point )) )
  {
    GetWindowRect(&m_rcHorizBar); 
    m_bDraggingHoriz=TRUE;
    m_ptHorizOriginal = m_ptHorizPrevious = MakePtHoriz(point);
    DrawHorizBar();					
    SetCapture();
    m_hwndPrevFocusHoriz = ::SetFocus(m_hWnd);  
    ::SetCursor( AfxGetApp()->LoadStandardCursor ((LPCTSTR)IDC_SIZENS));
    m_bMovingHorizSplitter = TRUE;
  }

  CStatic::OnLButtonDown(nFlags, point);
}

void CSSplitter::OnLButtonUp(UINT nFlags, CPoint point) 
{
  CClientDC dc( this );
  dc.DPtoLP( &point );

  if ( m_bVertSplitter && m_bDraggingVert && (m_bMovingVertSplitter) )
  {
    SetVertConstraint(point);
    point = MakePtVert(point);					
    CPoint ptDelta = point-m_ptVertOriginal;    
    DrawVertBar();								
    m_bDraggingVert = FALSE;
    MoveVertPanes( point );
    m_nLeftPaneWidth = m_rcWndLeft.Width();
    m_bMovingVertSplitter = FALSE;
    ReleaseCapture();
  }

  if ( m_bHorizSplitter && m_bDraggingHoriz && m_bMovingHorizSplitter)
  {
    point = MakePtHoriz(point);				
    SetHorizConstraint(point);
    CPoint ptDelta = point-m_ptHorizOriginal;   
    DrawHorizBar();								
    MoveHorizPanes( point );	
    m_nBottomPaneHeight = m_rcWndBottom.Height();
    m_bDraggingHoriz = FALSE;
    m_bMovingHorizSplitter = FALSE;
    ReleaseCapture();
  }

  CStatic::OnLButtonUp(nFlags, point);
}

void CSSplitter::OnMouseMove(UINT nFlags, CPoint point) 
{
  CClientDC dc( this );
  dc.DPtoLP( &point );

  if ( m_bVertSplitter && (m_rcVertSplitter.PtInRect( point )) )
    ::SetCursor( AfxGetApp()->LoadStandardCursor ((LPCTSTR)IDC_SIZEWE));

  if ( m_bVertSplitter && m_bDraggingVert ) {
    DrawVertBar();				
    SetVertConstraint(point);
    point = MakePtVert(point);	
    CPoint ptDelta = point-m_ptVertPrevious;
    m_rcVertBar += ptDelta;	
    DrawVertBar();				
    m_ptVertPrevious = point;	
  }

  if ( m_bHorizSplitter && m_rcHorizSplitter.PtInRect( point ) )
    ::SetCursor( AfxGetApp()->LoadStandardCursor ((LPCTSTR)IDC_SIZENS));

  if (m_bHorizSplitter && m_bDraggingHoriz ) {
    DrawHorizBar();				
    SetHorizConstraint(point);
    point = MakePtHoriz(point);	
    CPoint ptDelta = point-m_ptHorizPrevious;
    m_rcHorizBar += ptDelta;	
    DrawHorizBar();				
    m_ptHorizPrevious = point;	
  }

  CStatic::OnMouseMove(nFlags, point);
}

void CSSplitter::InitVertSplitter(
                                  CWnd*	pParentWnd,				
                                  CWnd*	pWndLeft,				
                                  CWnd*	pWndRight,
                                  UINT	nMaxLeft,
                                  UINT	nMaxRight)
{
  m_bVertSplitter	 = TRUE;
  m_bHorizSplitter = FALSE;

  m_pWndParent = pParentWnd;

  m_pWndLeft   = pWndLeft;
  m_pWndRight  = pWndRight;

  m_nMaxLeft = nMaxLeft;
  m_nMaxRight = nMaxRight;

  CRect rect;
  GetClientRect( &rect );

  CWinApp* pApp = AfxGetApp();
  CString strKey;
  strKey.Format(_T("%s%d"), _T("splt\\init_v"), m_nID);
  int nLeftPaneWidth = pApp->
    GetProfileInt(
    LPCTSTR(strKey),
    _T("VertSplitPos"), 
    2*nMaxLeft
    );

  m_rcWndLeft.SetRect(
    rect.left,
    rect.top,
    nLeftPaneWidth,
    rect.bottom
    );

  if (!m_bHiddenRightPane){
    CPoint InitPoint(nLeftPaneWidth + m_nSplitterWidth/2, 0);
    MoveVertPanes( InitPoint );
    m_nLeftPaneWidth = m_rcWndLeft.Width();
  }

  if ( (m_bHiddenRightPane)&&(m_pWndLeft->GetSafeHwnd()) ){
    m_pWndRight->ShowWindow(SW_HIDE);
    m_pWndLeft->MoveWindow(&rect);
  }

  if (!m_bHiddenLeftPane){
    CPoint InitPoint(nLeftPaneWidth + m_nSplitterWidth/2, 0);
    MoveVertPanes( InitPoint );
    m_nLeftPaneWidth = m_rcWndLeft.Width();
  }

  if ( (m_bHiddenLeftPane)&&(m_pWndLeft->GetSafeHwnd()) ){
    m_pWndLeft->ShowWindow(SW_HIDE);
    m_pWndRight->MoveWindow(&rect);
  }

}


void CSSplitter::InitHorizSplitter(
                                   CWnd*	pParentWnd,				
                                   CWnd*	pWndTop,				
                                   CWnd*	pWndBottom,
                                   UINT	nMaxTop,
                                   UINT	nMaxBottom)
{
  m_bVertSplitter	 = FALSE;
  m_bHorizSplitter = TRUE;

  m_pWndParent = pParentWnd;

  m_pWndTop	   = pWndTop;
  m_pWndBottom   = pWndBottom;

  m_nMaxTop	 = nMaxTop;
  m_nMaxBottom = nMaxBottom;

  CRect rect;
  GetClientRect( &rect );

  CWinApp* pApp = AfxGetApp();
  CString strKey;
  strKey.Format(_T("%s%d"), _T("splt\\init_h"), m_nID);
  m_nBottomPaneHeight = pApp->
    GetProfileInt(
    LPCTSTR(strKey),
    _T("HorizSplitPos"), 
    2*nMaxBottom
    );

  if ( !m_bHiddenBottomPane ){
    CPoint InitPoint(0, rect.bottom - m_nBottomPaneHeight - m_nSplitterWidth/2);
    MoveHorizPanes( InitPoint );
  }

  if ( m_bHiddenBottomPane && (m_pWndTop->GetSafeHwnd()) ){
    m_pWndBottom->ShowWindow(SW_HIDE);
    m_pWndTop->MoveWindow(&rect);
  }

}

void CSSplitter::DrawVertBar()
{
  CWnd* pParentWnd = GetParent();
  CWindowDC dc(pParentWnd);
  CRect rcWin;
  pParentWnd->GetWindowRect(&rcWin);	 
  CRect rc = m_rcVertBar;						
  rc -= rcWin.TopLeft();					 
  OnDrawVertBar(dc, rc);						 
}

void CSSplitter::OnDrawVertBar(CDC &dc, CRect &rc)
{
  CBrush brush(GetSysColor(COLOR_3DFACE));
  CBrush* pOldBrush = dc.SelectObject(&brush);

  dc.PatBlt(
    rc.left + m_rcVertSplitter.left + m_nSplitterWidth, 
    rc.top, 
    m_nSplitterWidth, 
    rc.Height(), 
    PATINVERT);

  dc.SelectObject(pOldBrush);
}

void CSSplitter::DrawHorizBar()
{
  CWnd* pParentWnd = GetParent();
  CWindowDC dc(pParentWnd);
  CRect rcWin;
  pParentWnd->GetWindowRect(&rcWin);	 
  CRect rc = m_rcHorizBar;						 
  rc -= rcWin.TopLeft();					 
  OnDrawHorizBar(dc, rc);						 
}

void CSSplitter::OnDrawHorizBar(CDC &dc, CRect &rc)
{
  CBrush brush(GetSysColor(COLOR_3DFACE));
  CBrush* pOldBrush = dc.SelectObject(&brush);

  dc.PatBlt(
    rc.left, 
    rc.top + m_rcHorizSplitter.top + m_nSplitterWidth/2, 
    rc.Width(), 
    m_nSplitterWidth, 
    PATINVERT
    );

  dc.SelectObject(pOldBrush);
}

void CSSplitter::CancelVertDrag()
{
  DrawVertBar();								
  ReleaseCapture();						
  ::SetFocus(m_hwndPrevFocusVert);		
  m_bDraggingVert = FALSE;
}

void CSSplitter::CancelHorizDrag()
{
  DrawHorizBar();								
  ReleaseCapture();						
  ::SetFocus(m_hwndPrevFocusHoriz);		
  m_bDraggingHoriz = FALSE;
}

void CSSplitter::SetVertConstraint(CPoint &pt)
{
  CRect rect;
  GetClientRect( &rect );

  if ( pt.x > (rect.right - m_nMaxRight) )
    pt.x = rect.right - m_nMaxRight;
  if ( pt.x < (rect.left + m_nMaxLeft) )
    pt.x = rect.left + m_nMaxLeft;
}

void CSSplitter::SetHorizConstraint(CPoint &pt)
{
  CRect rect;
  GetClientRect( &rect );

  if ( pt.y > (rect.bottom - m_nMaxBottom) )
    pt.y = rect.bottom - m_nMaxBottom;
  if ( pt.y < (rect.top + m_nMaxTop) )
    pt.y = rect.top + m_nMaxTop;
}


BOOL CSSplitter::PreTranslateMessage(MSG* pMsg) 
{
  if ( (pMsg->message == WM_KEYDOWN)&&( pMsg->wParam == VK_ESCAPE )&& m_bDraggingVert	)
    CancelVertDrag();

  if ( (pMsg->message == WM_KEYDOWN)&&( pMsg->wParam == VK_ESCAPE )&& m_bDraggingHoriz )
    CancelHorizDrag();

  return CStatic::PreTranslateMessage(pMsg);
}

void CSSplitter::OnDestroy() 
{
  CStatic::OnDestroy();

  CWinApp* pApp = AfxGetApp();
  CString strKey;
  strKey.Format(_T("%s%d"), _T("splt\\init_v"), m_nID); 


  if ( !(m_rcWndLeft.IsRectEmpty()) ){

    pApp->WriteProfileInt(
      LPCTSTR(strKey),
      _T("VertSplitPos"), 
      m_rcWndLeft.right
      );	
  }


  if ( !(m_rcWndBottom.IsRectEmpty()) ){

    strKey.Format(_T("%s%d"), _T("splt\\init_h"), m_nID);
    pApp->WriteProfileInt(
      LPCTSTR(strKey),
      _T("HorizSplitPos"), 
      m_rcWndBottom.Height()
      );
  }

}

void CSSplitter::ShowRightPane()
{
  if(!m_bVertSplitter)
    return;

  m_bHiddenRightPane	= FALSE;

  m_nLeftPaneWidth = m_rcWndLeft.Width();
  CPoint InitPoint(m_rcWndLeft.right + m_nSplitterWidth/2, 0);
  MoveVertPanes( InitPoint );

  if ( m_pWndRight->GetSafeHwnd() )
    m_pWndRight->ShowWindow(SW_SHOW);

}

void CSSplitter::HideRightPane()
{
  if(!m_bVertSplitter)
    return;

  if (m_bHiddenRightPane)
    return;

  if ( m_pWndLeft->GetSafeHwnd() ){

    m_bHiddenRightPane	= TRUE;
    m_rcVertSplitter.SetRectEmpty();

    CRect rect;
    GetClientRect( &rect );

    m_pWndRight->ShowWindow(SW_HIDE);
    m_pWndLeft->MoveWindow(&rect);
  }
}

void CSSplitter::ShowLeftPane()
{
  if(!m_bVertSplitter)
    return;

  m_bHiddenLeftPane	= FALSE;

  m_nLeftPaneWidth = m_rcWndLeft.Width();
  CPoint InitPoint(m_rcWndLeft.right + m_nSplitterWidth/2, 0);
  MoveVertPanes( InitPoint );

  if ( m_pWndLeft->GetSafeHwnd() )
    m_pWndLeft->ShowWindow(SW_SHOW);
}

void CSSplitter::HideLeftPane()
{
  if(!m_bVertSplitter)
    return;

  if (m_bHiddenLeftPane)
    return;

  if ( m_pWndRight->GetSafeHwnd() ){

    m_bHiddenLeftPane	= TRUE;
    m_rcVertSplitter.SetRectEmpty();

    CRect rect;
    GetClientRect( &rect );

    m_pWndLeft->ShowWindow(SW_HIDE);
    m_pWndRight->MoveWindow(&rect);
  }
}

void CSSplitter::ShowBottomPane()
{
  if (!m_bHorizSplitter)
    return;

  m_bHiddenBottomPane	= FALSE;

  if ( !m_rcWndBottom.IsRectEmpty() )
    m_nBottomPaneHeight = m_rcWndBottom.Height();

  CRect rect;
  GetClientRect( &rect );
  CPoint InitPoint(0, rect.bottom - m_nBottomPaneHeight - m_nSplitterWidth/2 );
  MoveHorizPanes( InitPoint );

  if ( m_pWndBottom->GetSafeHwnd() )
    m_pWndBottom->ShowWindow(SW_SHOW);

}

void CSSplitter::HideBottomPane()
{
  if (!m_bHorizSplitter)
    return;

  if (m_bHiddenBottomPane)
    return;

  if ( m_pWndBottom->GetSafeHwnd() ){

    m_bHiddenBottomPane	= TRUE;
    m_rcHorizSplitter.SetRectEmpty();

    CRect rect;
    GetClientRect( &rect );

    m_pWndBottom->ShowWindow(SW_HIDE);
    m_pWndTop->MoveWindow(&rect);
  }
}

void CSSplitter::MakeHorizSplitter()
{
  if ( m_bHorizSplitter )
    return;

  if ( (m_pWndTop==NULL) && (m_pWndBottom==NULL)	){

    InitHorizSplitter(
      m_pWndParent,				
      m_pWndLeft,				
      m_pWndRight,
      m_nMaxTop,
      m_nMaxBottom
      );

    return;
  }

  if ( (m_pWndTop!=NULL) && (m_pWndBottom!=NULL) && (!m_rcWndBottom.IsRectEmpty()) ){

    m_bHorizSplitter		= TRUE;
    m_bVertSplitter			= FALSE;

    CRect rect;
    GetClientRect(&rect);

    CPoint InitPoint(0, rect.bottom - m_nBottomPaneHeight - m_nSplitterWidth/2 );
    SetHorizConstraint(InitPoint);
    m_nBottomPaneHeight = m_rcWndBottom.Height();

    if ( !m_bHiddenBottomPane )
      MoveHorizPanes( InitPoint );
  }

}

void CSSplitter::MakeVertSplitter()
{
  if ( m_bVertSplitter )
    return;

  if ( (m_pWndLeft==NULL) && (m_pWndRight==NULL) ){

    InitVertSplitter(
      m_pWndParent,				
      m_pWndTop,				
      m_pWndBottom,
      m_nMaxLeft,
      m_nMaxRight
      );

    return;
  }

  if ( (m_pWndLeft!=NULL) && (m_pWndRight!=NULL) && (!m_rcWndLeft.IsRectEmpty()) ){

    m_bHorizSplitter		= FALSE;
    m_bVertSplitter			= TRUE;

    m_pWndLeft	= m_pWndTop;
    m_pWndRight = m_pWndBottom;

    m_nLeftPaneWidth = m_rcWndLeft.Width();
    CPoint InitPoint(m_rcWndLeft.right + m_nSplitterWidth/2, 0);
    MoveVertPanes( InitPoint );
  }

}


UINT CSSplitter::GetSplitterStyle()
{
  UINT nStyle;

  if ( m_bHorizSplitter )
    nStyle = SS_HORIZ;

  if ( m_bVertSplitter )
    nStyle = SS_VERT;

  return nStyle;
}

int CSSplitter::GetSplitterPos()
{
  int nSplitterPos;
  UINT nSplitterStyle = GetSplitterStyle();

  if ( nSplitterStyle == SS_VERT )
    nSplitterPos = m_rcWndLeft.right;

  if ( nSplitterStyle == SS_HORIZ )
    nSplitterPos = m_rcWndBottom.top;

  return nSplitterPos;
}

void CSSplitter::SetSplitterPos(int nPos)
{
  UINT nSplitterStyle = GetSplitterStyle();

  CRect rect;
  GetClientRect( &rect );

  if ( nSplitterStyle == SS_VERT ){

    m_rcWndLeft.SetRect(
      rect.left,
      rect.top,
      nPos,
      rect.bottom
      );

    if (!m_bHiddenRightPane){
      CPoint InitPoint(nPos + m_nSplitterWidth/2, 0);
      MoveVertPanes( InitPoint );
      m_nLeftPaneWidth = m_rcWndLeft.Width();
    }

    if ( (m_bHiddenRightPane)&&(m_pWndLeft->GetSafeHwnd()) ){
      m_pWndRight->ShowWindow(SW_HIDE);
      m_pWndLeft->MoveWindow(&rect);
    }
  }

  if ( nSplitterStyle == SS_HORIZ ){

    m_rcWndBottom.SetRect(
      rect.left,
      nPos,
      rect.right,
      rect.bottom
      );

    m_nBottomPaneHeight = m_rcWndBottom.Height();


    if ( !m_bHiddenBottomPane ){
      CPoint InitPoint(0, nPos - m_nSplitterWidth/2);
      MoveHorizPanes( InitPoint );
    }

    if ( m_bHiddenBottomPane && (m_pWndTop->GetSafeHwnd()) ){
      m_pWndBottom->ShowWindow(SW_HIDE);
      m_pWndTop->MoveWindow(&rect);
    }
  }

}

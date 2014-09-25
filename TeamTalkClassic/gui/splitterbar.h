// CSplitterBar.h : header file
//
/////////////////////////////////////////////////////////////////////////////
#ifndef __SPLITTER_BAR_H__
#define __SPLITTER_BAR_H__

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#define WM_SPLITTER_MOVED    WM_USER+1000
/////////////////////////////////////////////////////////////////////////////
// CSplitterBar

class CSplitterBar : public CWnd
{
    DECLARE_DYNAMIC(CSplitterBar)

    enum DRAGFLAG { DRAG_ENTER=0, DRAG_EXIT=1, DRAGGING=2 };
protected:
    CRect    m_rectSplitter;
    CRect    m_rectDragPrev;
    CRect    m_rectDragCurt;
    BOOL    m_bDragging;

    CWnd    *m_pwndLeftPane;    //left pane window
    CWnd    *m_pwndRightPane;    //right pane window

    int        m_cxLeftMost;        //left most, relative to parent window
    int        m_cxRightMost;        //right most, relative to parent window

    BOOL    m_bHorizontal;
// Construction
public:
    CSplitterBar();
    BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID, BOOL bHorizontal=FALSE);

// Attributes
public:
    BOOL IsCursorOverSplitter( const CPoint& ptCursor );

// Operations
public:
    void DrawDraggingBar(CPoint point,DRAGFLAG df=DRAGGING);
    void SetPanes(CWnd *pwndLeftPane,CWnd *pwndRightPane);
    void MovePanes();

// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CSplitterBar)
    //}}AFX_VIRTUAL

// Implementation
public:
    virtual ~CSplitterBar();

    // Generated message map functions
protected:
    //{{AFX_MSG(CSplitterBar)
    afx_msg void OnPaint();
    afx_msg LRESULT OnNcHitTest(CPoint point);
    afx_msg int  OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif 

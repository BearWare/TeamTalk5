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

#if !defined(AFX_MYBITMAPBUTTON_H__243A5FF8_01BB_4F23_9268_7D4BF3166022__INCLUDED_)
#define AFX_MYBITMAPBUTTON_H__243A5FF8_01BB_4F23_9268_7D4BF3166022__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MyBitmapButton.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMyBitmapButton window

class CMyBitmapButton : public CBitmapButton
{
// Construction
public:
    CMyBitmapButton();

// Attributes
public:

// Operations
public:

// Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CMyBitmapButton)
    //}}AFX_VIRTUAL

// Implementation
public:
    void SetDown(BOOL bState);
    virtual ~CMyBitmapButton();
    virtual void SetState(BOOL bState);
    virtual UINT GetState( ) const;
  
  void SetBitmaps(HBITMAP hNormal = 0, HBITMAP hSelected = 0, HBITMAP hFocused = 0, HBITMAP hDisabled = 0);

    // Generated message map functions
protected:
    //{{AFX_MSG(CMyBitmapButton)
    afx_msg void OnKillFocus(CWnd* pNewWnd);
    afx_msg void OnSetFocus(CWnd* pOldWnd);
    //}}AFX_MSG

    DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnSysKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
protected:
  virtual void PreSubclassWindow();
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MYBITMAPBUTTON_H__243A5FF8_01BB_4F23_9268_7D4BF3166022__INCLUDED_)

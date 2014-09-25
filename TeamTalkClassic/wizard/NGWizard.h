#if !defined(__NGWizard_H__)
#define __NGWizard_H__
/************************************************************************
*
*    CNGWizard - Wizard97 class which supports resizing
*
*    Written by Andy Metcalfe (andy.metcalfe@lineone.net)
*
************************************************************************
*                                                                       
*  Filename    : NGWizard.h
*
*  Compiler    : Microsoft Visual C++ 6.0, Service Pack 3 or later
*                                                                       
*  Target                                                               
*  Environment : Win98/Me/NT/2000
*
*                This code will also work on Windows 95, although
*                controls hich should have transparent backgrounds
*                may not display correctly.
*
*                Version 5.80 of the common controls library is required
*
*  NOTE:
*
*    Your are free to use this code in your own products, PROVIDED
*    this notice is not removed or modified.
*
************************************************************************/

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

// NGWizard.h : header file
//


#if !defined(__NGWizardPage_H__)
#include "NGWizardPage.h"
#endif

#include "ResizeCtrl.h"


/////////////////////////////////////////////////////////////////////////////
// CNGWizard

class CNGWizard : public    CPropertySheet
{
    DECLARE_DYNAMIC(CNGWizard)

    // Construction
public:
    CNGWizard(UINT nIDCaption = 0,
        CWnd* pParentWnd = NULL,
        UINT nPage = 0);

    CNGWizard(LPCTSTR pszCaption,
        CWnd* pParentWnd = NULL,
        UINT nPage = 0);

    virtual ~CNGWizard(void);


    // Attributes
protected:
    CDIBitmap                m_bmpHeader;
    CBrush                    m_HollowBrush;

    UINT                    m_uWatermarkID;
    int                        m_eWatermarkStyle;
    UINT                    m_uHeaderID;
    int                        m_eHeaderStyle;


    BOOL                    m_bResizeable;
    BOOL                    m_bShowFinishAlways;    // If TRUE, Next and Finish can both be seen
    // at the same time. Otherwise, Finish will
    // replace Next (as standard in most Wizards)

    // The CResizeCtrl object below is required so we can resize this
    // sheet
    CResizeCtrl                m_Resizer;

    // Operations
public:
    virtual    void                    AddPage(CNGWizardPage* pPage);

    BOOL                    SetWatermark(    UINT uWatermarkID,
        int eStyle = WIZ97_BITMAP_STRETCH);

    BOOL                    SetHeader(        UINT uHeaderID,
        int eStyle = WIZ97_BITMAP_STRETCH);

    void                    EnableWizardButtons(DWORD dwFlags, BOOL bEnable);

    BOOL                    IsResizeable(void) const
    { return m_bResizeable; }

    BOOL                    AllowResize(BOOL bResize);


protected:

    // Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CNGWizard)
public:
    virtual    BOOL                    OnInitDialog(void);
    //}}AFX_VIRTUAL


    // Implementation
private:
    void                    AdjustPage(void);

    BOOL                    SetInitialisingFlag(BOOL bInitialising);

    BOOL                    EnableDlgControl(UINT uID, BOOL bEnable);


    // Generated message map functions
protected:
    //{{AFX_MSG(CNGWizard)
    afx_msg BOOL                    OnEraseBkgnd(CDC* pDC);
    afx_msg HBRUSH                    OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
    afx_msg BOOL                    OnQueryNewPalette(void);
    afx_msg void                    OnPaletteChanged(CWnd* pFocusWnd);
    afx_msg    BOOL                    OnClickedPrev(UINT uID);
    afx_msg    BOOL                    OnClickedNext(UINT uID);
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(__NGWizard_H__)

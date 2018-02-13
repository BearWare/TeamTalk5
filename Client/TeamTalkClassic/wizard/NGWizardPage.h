#if !defined(__NGWizardPage_H__)
#define __NGWizardPage_H__
/************************************************************************
*
*    CNGWizardPage - Wizard97 page class which supports resizing
*
*    Written by Andy Metcalfe (andy.metcalfe@lineone.net)
*
************************************************************************
*                                                                       
*  Filename    : NGWizardPage.h
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

// NGWizardPage.h : header file
//

#include "DIBitmap.h"
#include "ResizeCtrl.h"

class CNGWizard;

enum
{
    WIZ97_BITMAP_TILE = 0,        // Tile the bitmap to fill the page
    WIZ97_BITMAP_STRETCH,        // Stretch the bitmap so it fits to the page
    WIZ97_BITMAP_CENTER            // center the bitmap inside the page (watermark only)
};

/////////////////////////////////////////////////////////////////////////////
// CNGWizardPage dialog
//

class CNGWizardPage : public CPropertyPage
{
    DECLARE_DYNAMIC(CNGWizardPage)



    // Construction
public:
    CNGWizardPage(    UINT uTemplateID,
        UINT uCaptionID = 0,
        UINT uTitleID = 0,
        UINT uSubtitleID = 0);


    CNGWizardPage(    LPCTSTR lpszTemplateName,
        UINT uCaptionID = 0,
        UINT uTitleID = 0,
        UINT uSubtitleID = 0);

    CNGWizardPage(void);

    virtual    ~CNGWizardPage(void);

    // Attributes
protected:

    // Dialog Data
    //{{AFX_DATA(CNGWizardPage)
    //}}AFX_DATA

    BOOL                    m_bResizeable;            // Set TRUE by the parent Wizard if this page can be resized
    BOOL                    m_bInitialising;        // Set TRUE by the parent Wizard during initialisation

    // The CResizer object below is required so we can define the controls
    // within this page can be moved/resized when the page is resized.
    //
    // See OnInitDialog() for how this is done.
    CResizeCtrl                m_Resizer;


    // Operations
public:
    BOOL                    IsResizeable(void) const
    { return m_bResizeable; }

    BOOL                    IsInitialising(void) const
    { return m_bInitialising; }

    // Provided to allow the parent wizard to mark this page as initiailsed
    void                    SetInitialisingFlag(BOOL bInitialising)
    { m_bInitialising = bInitialising; }
protected:
    CNGWizard*                GetParent(void);

    void                    EnableWizardButtons(DWORD dwFlags, BOOL bEnable);


    // Overrides
    // ClassWizard generate virtual function overrides
    //{{AFX_VIRTUAL(CNGWizardPage)
public:
protected:
    virtual    BOOL                    OnInitDialog(void);
    virtual    BOOL                    OnWizardFinish(void);
    //}}AFX_VIRTUAL

    // Implementation
public:

    BOOL                    SetWatermark(    UINT uWatermarkID,
        int eStyle = WIZ97_BITMAP_STRETCH);

protected:
    void                    CommonConstruct(void);

    CDIBitmap                m_bmpWatermark;
    int                        m_eWatermarkStyle;
    CBrush                    m_HollowBrush;

    // Generated message map functions
    //{{AFX_MSG(CNGWizardPage)
    afx_msg BOOL                    OnEraseBkgnd(CDC* pDC);
    afx_msg HBRUSH                    OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
    afx_msg BOOL                    OnQueryNewPalette(void);
    afx_msg void                    OnPaletteChanged(CWnd* pFocusWnd);
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(__NGWizardPage_H__)

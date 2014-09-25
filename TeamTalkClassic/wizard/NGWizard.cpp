/************************************************************************
*
*    CNGWizard - Wizard97 class which supports resizing
*
*    Written by Andy Metcalfe (andy.metcalfe@lineone.net)
*
************************************************************************
*                                                                       
*  Filename    : NGWizard.cpp
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

// NGWizard.cpp : implementation file
//

#include "stdafx.h"

#include "ResizeCtrl.h"
#include "NGWizard.h"
#include "BmpPalette.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CNGWizard

IMPLEMENT_DYNAMIC(CNGWizard, CPropertySheet)

    CNGWizard::CNGWizard(UINT nIDCaption /* = 0*/,
    CWnd* pParentWnd /*= NULL*/,
    UINT nPage /* = 0*/)
    :CPropertySheet(nIDCaption, pParentWnd, nPage)
{
    SetWizardMode();

    m_bShowFinishAlways    = FALSE;
    m_bResizeable        = FALSE;

    m_uWatermarkID        = 0;
    m_eWatermarkStyle    = -1;
    m_uHeaderID            = 0;
    m_eHeaderStyle        = -1;
}


CNGWizard::CNGWizard(LPCTSTR pszCaption,
    CWnd* pParentWnd /*= NULL*/,
    UINT nPage /* = 0*/)
    :CPropertySheet(pszCaption, pParentWnd, nPage)
{

    SetWizardMode();

    m_bShowFinishAlways    = FALSE;
    m_bResizeable        = FALSE;

    m_uWatermarkID        = 0;
    m_eWatermarkStyle    = -1;
    m_uHeaderID            = 0;
    m_eHeaderStyle        = -1;
}


CNGWizard::~CNGWizard(void)
{
}


BEGIN_MESSAGE_MAP(CNGWizard, CPropertySheet)
    //{{AFX_MSG_MAP(CNGWizard)
    ON_WM_ERASEBKGND()
    ON_WM_CTLCOLOR()
    ON_WM_QUERYNEWPALETTE()
    ON_WM_PALETTECHANGED()
    //}}AFX_MSG_MAP

    ON_COMMAND_EX(        ID_WIZBACK,                OnClickedPrev)
    ON_COMMAND_EX(        ID_WIZNEXT,                OnClickedNext)

END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CNGWizard Virtual Overrides

BOOL CNGWizard::OnInitDialog(void)
{
    BOOL bResult = CPropertySheet::OnInitDialog();

    // In order to correctly repaint the watermark and header bitmaps
    // when the wizard is resized the windows must be created with
    // the CS_HREDRAW and CS_VREDRAW window class styles
    // 
    // This should be done by registering a custom window class
    // For now, just modifiy the global one
    DWORD style = GetClassLong(m_hWnd, GCL_STYLE);
    style = style | CS_VREDRAW | CS_HREDRAW;
    SetClassLong(m_hWnd, GCL_STYLE, style);

    if (m_bShowFinishAlways)
    {
        // Move the Back and Next buttons to the left
        // This code is necessary as the finish button is
        // hidden under the Next button and invisible by
        // default!
        //
        // For details, refer to MSDN Knowledge Base article Q143210
        // ("How to Add the Finish Button to a Wizard Property Sheet")
        //
        CRect rect, tmp;
        CSize shift;
        CWnd* pTemp;
        CWnd* pNext;

        // Get the position of the Cancel Button
        pTemp = GetDlgItem(IDCANCEL);
        if (pTemp==NULL)
            return bResult;

        pTemp->GetWindowRect(tmp);

        // Get the position of the Next Button
        pNext = GetDlgItem(ID_WIZNEXT);
        if (pNext==NULL)
            return bResult;

        pNext->GetWindowRect(rect);

        // Calculate the distance to shift the Next and Back buttons
        shift.cx = tmp.left-rect.right+rect.Width();
        shift.cy = 0;

        // Get the position of the Back Button
        pTemp = GetDlgItem(ID_WIZBACK);
        if (pTemp==NULL)
            return bResult;

        pTemp->GetWindowRect(tmp);

        // move the Back button
        tmp -= shift;
        ScreenToClient(tmp);
        pTemp->MoveWindow(tmp);

        // move the Next button
        ScreenToClient(rect);
        tmp = rect-shift;
        pNext->MoveWindow(tmp);

        // Show (but disable) the Finish Button
        pTemp = GetDlgItem(ID_WIZFINISH);
        if (pTemp==NULL)
            return bResult;

        pTemp->ShowWindow(SW_SHOW);
        pTemp->EnableWindow(FALSE);
    }

    if (m_bResizeable)
    {
        // This doesn't work yet - hence disabled
        //ModifyStyle(0, WS_MAXIMIZEBOX);

        // Mark all pages as initialising. This allows them
        // to determine that any OnSetActive() calls they
        // get are a consequence of the resizer being initialised
        // in PropertySheetPrepare(), and act accordingly
        SetInitialisingFlag(TRUE);

        m_Resizer.PropertySheetPrepare(this);
        m_Resizer.Create(this);
        m_Resizer.WizardAddEtchedBar(this, 0, 100, 100, 0);
        m_Resizer.PropertySheetAddButtons(this, 100, 100, 0, 0);
        m_Resizer.PropertySheetAddPages(this,  0,  0, 100, 100 );
        m_Resizer.SetMinimumTrackingSize();
        m_Resizer.WizardSaveMargins(this);
        m_Resizer.SetGripEnabled(TRUE);

        // If the page has a Wizard97 header, we need to resize the etched bar
        // immediately below it as well.
        //
        // By inspection (Spy++), the border is a static control with ID 0x3027
        m_Resizer.Add( 0x3027,    0, 0, 100, 0);

        SetInitialisingFlag(FALSE);
    }



    return bResult;
}


/////////////////////////////////////////////////////////////////////////////
// CNGWizard Operations

BOOL CNGWizard::AllowResize(BOOL bResize)
{
    ASSERT(m_hWnd == NULL);

    m_bResizeable = bResize;

    return TRUE;
}


BOOL CNGWizard::SetWatermark(UINT uWatermarkID,
    int eStyle /*WIZ97_BITMAP_TILE*/)
{
    ASSERT(!::IsWindow(m_hWnd));        // Must do this BEFORE OnInitDialog()

    m_uWatermarkID        = uWatermarkID;
    m_eWatermarkStyle    = eStyle;

    // Set the corresponding flags in the PROPSHEETHEADER of the wizard
    // so we'll get the Wizard97 look
    m_psh.hbmWatermark = NULL;
    m_psh.dwFlags |= (PSH_USEHBMWATERMARK | PSH_WATERMARK | PSH_WIZARD97);

    return TRUE;
}


BOOL CNGWizard::SetHeader(UINT uHeaderID,
    int eStyle /*WIZ97_BITMAP_TILE*/)
{
    ASSERT(!::IsWindow(m_hWnd));            // Must do this BEFORE OnInitDialog()

    m_uHeaderID        = uHeaderID;
    m_eHeaderStyle    = eStyle;

    if (m_bmpHeader.LoadResource(uHeaderID) )
    {
        VERIFY(m_HollowBrush.CreateStockObject(HOLLOW_BRUSH));

        // Set the corresponding flags in the PROPSHEETHEADER of the wizard
        // so we'll get the Wizard97 look
        m_psh.hbmHeader = NULL;
        m_psh.dwFlags |= (PSH_USEHBMHEADER | PSH_HEADER | PSH_WIZARD97);
    }
    return TRUE;
}



//    Enable/disable selected buttons
//
//    dwFlags can be any combination of:
//
//        PSWIZB_BACK        Back button
//        PSWIZB_NEXT        Next button
//        PSWIZB_FINISH    Finish button
//
void CNGWizard::EnableWizardButtons(DWORD dwFlags, BOOL bEnable)
{
    if (dwFlags & PSWIZB_BACK)
    {
        EnableDlgControl(ID_WIZBACK, bEnable);
    }
    if (dwFlags & PSWIZB_NEXT)
    {
        EnableDlgControl(ID_WIZNEXT, bEnable);
    }
    if (dwFlags & PSWIZB_FINISH)
    {
        EnableDlgControl(ID_WIZFINISH, bEnable);
    }
}


void CNGWizard::AddPage(CNGWizardPage* pPage)
{
    if ( (pPage->m_psp.dwFlags & PSP_HIDEHEADER) != 0)
    {
        pPage->SetWatermark(m_uWatermarkID, m_eWatermarkStyle);
    }
    CPropertySheet::AddPage(pPage);
}


/////////////////////////////////////////////////////////////////////////////
// CNGWizard Implementation

void CNGWizard::AdjustPage(void)
{
    if (m_bResizeable && (GetActivePage() != NULL) )
    {
        m_Resizer.WizardAdjustPage(this);
    }
}


// Mark all pages as initialised. This allows them
// to determine that any OnSetActive() calls they
// get are the genuine article, and not a consequence
// of the resizer being initialised
BOOL CNGWizard::SetInitialisingFlag(BOOL bInitialising)
{
    BOOL bResult = FALSE;

    // Mark all pages as initialised
    // This is so they know the next OnSetActive() they
    // get will be a REAL one...
    for (int nPage = 0; nPage < GetPageCount(); nPage++ )
    {
        CNGWizardPage* pPage = DYNAMIC_DOWNCAST(    CNGWizardPage,
            GetPage(nPage) );
        if (pPage != NULL)
        {
            pPage->SetInitialisingFlag(bInitialising);

            bResult = TRUE;
        }
    }
    return bResult;
}


BOOL CNGWizard::EnableDlgControl(UINT uID, BOOL bEnable)
{
    CWnd* pWnd = GetDlgItem(uID);
    if (NULL != pWnd)
    {
        pWnd->EnableWindow(bEnable);
        return TRUE;
    }
    ASSERT(FALSE);
    return FALSE;
}


/////////////////////////////////////////////////////////////////////////////
// CNGWizard Message Handlers

BOOL CNGWizard::OnEraseBkgnd(CDC* pDC) 
{
    BOOL bResult = CPropertySheet::OnEraseBkgnd(pDC);

    if (!m_bmpHeader.IsEmpty() )
    {
        CPropertyPageEx* pPage = (CPropertyPageEx*)GetActivePage( );
        if (NULL != pPage)
        {
            BOOL bHasWizard97Header = ( (pPage->m_psp.dwFlags & PSP_HIDEHEADER) == 0);
            if (bHasWizard97Header && (m_bmpHeader.GetPixelPtr() != NULL) )
            {
                // If the page has a header, we need to paint the area above the border.
                // By inspection (Spy++), the border is a static control with ID 0x3027
                CWnd* pTopBorder = GetDlgItem(0x3027);
                ASSERT(NULL != pTopBorder);
                if (NULL != pTopBorder)
                {
                    CRect rectTopBorder;
                    pTopBorder->GetWindowRect(rectTopBorder);

                    CRect rc;
                    GetClientRect(rc);

                    ScreenToClient(rectTopBorder);

                    rc.bottom = rectTopBorder.top - 1;

                    int x = 0, y = 0;

                    switch (m_eHeaderStyle)
                    {
                    case WIZ97_BITMAP_STRETCH:
                        // Stretch bitmap so it will best fit to the dialog
                        m_bmpHeader.DrawDIB(pDC, 0, 0, rc.Width(), rc.Height());
                        bResult = TRUE;
                        break;

                    default:
                        // Tile the bitmap
                        while (y < rc.Height())
                        {
                            while (x < rc.Width())
                            {
                                m_bmpHeader.DrawDIB(pDC, x, y);
                                x += m_bmpHeader.GetWidth();
                            }
                            x = 0;
                            y += m_bmpHeader.GetHeight();
                        }
                        bResult = TRUE;
                        break;
                    }
                }
            }
        }
    }
    return bResult;
}


HBRUSH CNGWizard::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
    if (!m_bmpHeader.IsEmpty() )
    {
        CPropertyPageEx* pPage = (CPropertyPageEx*)GetActivePage( );
        if (NULL != pPage)
        {
            BOOL bHasWizard97Header = ( (pPage->m_psp.dwFlags & PSP_HIDEHEADER) == 0);
            if (bHasWizard97Header && (m_bmpHeader.GetPixelPtr() != NULL) )
            {
                // If the page has a header, we need to paint the area above the border.
                // By inspection (Spy++), the border is a static control with ID 0x3027
                CWnd* pTopBorder = GetDlgItem(0x3027);
                ASSERT(NULL != pTopBorder);
                if (NULL != pTopBorder)
                {
                    CRect rectTopBorder;
                    pTopBorder->GetWindowRect(rectTopBorder);
                    ScreenToClient(rectTopBorder);

                    CRect rectCtrl;
                    pWnd->GetWindowRect(rectCtrl);

                    ScreenToClient(rectCtrl);

                    if (rectCtrl.top < rectTopBorder.bottom)
                    {
                        switch(nCtlColor)
                        {
                        case CTLCOLOR_STATIC:
                            // The Slider Control has CTLCOLOR_STATIC, but doesn't let
                            // the background shine through,
                            TCHAR lpszClassName[255];
                            GetClassName(pWnd->m_hWnd, lpszClassName, 255);
                            if (_tcscmp(lpszClassName, TRACKBAR_CLASS) == 0)
                            {
                                return CPropertySheet::OnCtlColor(pDC, pWnd, nCtlColor);
                            }

                        case CTLCOLOR_BTN:
                            // let static controls shine through
                            pDC->SetBkMode(TRANSPARENT);
                            return HBRUSH(m_HollowBrush);

                        default:
                            break;
                        }
                    }
                }
            }
        }
    }
    // if we reach this line, we haven't set a brush so far
    return CPropertySheet::OnCtlColor(pDC, pWnd, nCtlColor);
}


BOOL CNGWizard::OnQueryNewPalette(void) 
{
    if (!m_bmpHeader.IsEmpty() )
    {
        CPalette* pPal = m_bmpHeader.GetPalette();
        if ( (NULL != pPal) && (NULL != GetSafeHwnd()) )
        {
            CClientDC dc(this);
            CPalette* pOldPalette = dc.SelectPalette(pPal, FALSE);
            UINT nChanged = dc.RealizePalette();
            dc.SelectPalette(pOldPalette, TRUE);

            if (nChanged == 0)
            {
                return FALSE;
            }
            Invalidate();

            return TRUE;
        }
    }
    return CPropertySheet::OnQueryNewPalette();
}


void CNGWizard::OnPaletteChanged(CWnd* pFocusWnd) 
{
    if (!m_bmpHeader.IsEmpty() )
    {
        CPalette* pPal = m_bmpHeader.GetPalette();
        if ( (NULL != pPal) &&
            (NULL != GetSafeHwnd()) &&
            (this != pFocusWnd) &&
            !IsChild(pFocusWnd) )
        {
            CClientDC dc(this);
            CPalette* pOldPalette = dc.SelectPalette(pPal, TRUE);
            UINT nChanged = dc.RealizePalette();
            dc.SelectPalette(pOldPalette, TRUE);

            if (0 != nChanged)
            {
                Invalidate();
            }
        }
    }
    else
    {
        CPropertySheet::OnPaletteChanged(pFocusWnd);
    }
}


BOOL CNGWizard::OnClickedPrev(UINT uID)
{
    // Because WizPrev and WizNext resizes a page to the initial size
    // we need to change the size...
    //
    // Note that we MUST allow the default processing to happen first, or
    // AdjustPage() will work on the current page instead of the new one...
    Default();

    AdjustPage();

    // We must set m_nModalResult to the ID of the button pressed
    // if we want the return value from the wizard to be correct.
    //
    // See CPropertySheet::OnCommand() for details
    m_nModalResult = uID;

    return TRUE;
}


BOOL CNGWizard::OnClickedNext(UINT uID)
{
    // Because WizPrev and WizNext resizes a page to the initial size
    // we need to change the size...
    //
    // Note that we MUST allow the default processing to happen first, or
    // AdjustPage() will work on the current page instead of the new one...
    Default();

    AdjustPage();

    // We must set m_nModalResult to the ID of the button pressed
    // if we want the return value from the wizard to be correct.
    //
    // See CPropertySheet::OnCommand() for details
    m_nModalResult = uID;

    return TRUE;
}

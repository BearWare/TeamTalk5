/************************************************************************
*
*    CNGWizardPage - Wizard97 page class which supports resizing
*
*    Written by Andy Metcalfe (andy.metcalfe@lineone.net)
*
************************************************************************
*                                                                       
*  Filename    : NGWizardPage.cpp
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

// NGWizardPage.cpp : implementation file
//

#include "StdAfx.h"

#include "ResizeCtrl.h"
#include "NGWizard.h"
#include "NGWizardPage.h"
#include "BmpPalette.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNGWizardPage property page

IMPLEMENT_DYNAMIC(CNGWizardPage, CPropertyPage)


    CNGWizardPage::CNGWizardPage(    UINT uTemplateID,
    UINT uCaptionID /*= 0*/,
    UINT uTitleID /*= 0*/,
    UINT uSubtitleID /*= 0*/)
    : CPropertyPage(uTemplateID, uCaptionID, uTitleID, uSubtitleID)
{
    CommonConstruct();
}


CNGWizardPage::CNGWizardPage(    LPCTSTR lpszTemplateName,
    UINT uCaptionID /*= 0*/,
    UINT uTitleID /*= 0*/,
    UINT uSubtitleID /*= 0*/)
    : CPropertyPage(lpszTemplateName, uCaptionID, uTitleID, uSubtitleID)
{
    CommonConstruct();
}


CNGWizardPage::CNGWizardPage(void) : CPropertyPage()
{
    CommonConstruct();

    //{{AFX_DATA_INIT(CNGWizardPage)
    //}}AFX_DATA_INIT
}

CNGWizardPage::~CNGWizardPage(void)
{
}


BEGIN_MESSAGE_MAP(CNGWizardPage, CPropertyPage)
    //{{AFX_MSG_MAP(CNGWizardPage)
    ON_WM_ERASEBKGND()
    ON_WM_CTLCOLOR()
    ON_WM_QUERYNEWPALETTE()
    ON_WM_PALETTECHANGED()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()



BOOL CNGWizardPage::OnInitDialog(void) 
{
    CPropertyPage::OnInitDialog();

    CNGWizard* pWizard = GetParent();
    if (NULL != pWizard)
    {
        m_bResizeable    = pWizard->IsResizeable();

        if (m_bResizeable)
        {
            m_Resizer.Create(this);
        }
    }
    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}


//    Get a type safe pointer to our parent
//    Returns NULL if the enclosing sheet is not a CWizard
CNGWizard* CNGWizardPage::GetParent(void)
{
    // Enable the finish button (this could be temporary code)
    CWnd* pParent = CPropertyPage::GetParent();
    if (NULL != pParent)
    {
        CNGWizard* pWizard = DYNAMIC_DOWNCAST(CNGWizard, pParent);
        if (NULL == pWizard)
        {
            ASSERT(FALSE);
        }
        return pWizard;
    }
    return NULL;    // no parent if we get this far
}


void CNGWizardPage::EnableWizardButtons(DWORD dwFlags, BOOL bEnable)
{
    CNGWizard* pWizard = GetParent();
    if (NULL != pWizard)
    {
        pWizard->EnableWizardButtons(dwFlags, bEnable);
    }
}


/************************************************************************
*    This override is necessary to overcome a bug in the MFC wizard
*    implementation in which data is not transferred from the current
*    page to the corresponding variables when the Finish button is pressed.
*
*    The bug arises because CDialog::UpdateData() is not called by the
*    framework when Finish is pressed; hence this override.
*
*    Refer to MSDN Knowledge Base article Q150349 ("BUG: Finish Button
*    Fails Data Transfer from Page to Variables") for further information
*
************************************************************************/

BOOL CNGWizardPage::OnWizardFinish(void)
{
    if (!UpdateData(TRUE) )
    {
        TRACE0("UpdateData failed during wizard finish\n");

        return FALSE;
    }
    return CPropertyPage::OnWizardFinish();
}


/////////////////////////////////////////////////////////////////////////////
// CNGWizardPage implementation

void CNGWizardPage::CommonConstruct(void)
{
    m_bResizeable    = FALSE;

    m_eWatermarkStyle = WIZ97_BITMAP_TILE;
}


BOOL CNGWizardPage::SetWatermark(UINT uResource, int eStyle /*CWizard::WIZ97_BITMAP_STRETCH*/)
{
    m_eWatermarkStyle = eStyle;

    ASSERT(    WIZ97_BITMAP_TILE == m_eWatermarkStyle  ||
        WIZ97_BITMAP_STRETCH == m_eWatermarkStyle ||
        WIZ97_BITMAP_CENTER == m_eWatermarkStyle);

    if (m_bmpWatermark.LoadResource(uResource) )
    {
        VERIFY(m_HollowBrush.CreateStockObject(HOLLOW_BRUSH));

        return TRUE;
    }
    return FALSE;
}


/////////////////////////////////////////////////////////////////////////////
// CNGWizardPage message handlers

BOOL CNGWizardPage::OnEraseBkgnd(CDC* pDC) 
{
    if (!m_bmpWatermark.IsEmpty() )
    {
        if (m_bmpWatermark.GetPixelPtr() != NULL)
        {
            ASSERT(    WIZ97_BITMAP_TILE == m_eWatermarkStyle  ||
                WIZ97_BITMAP_STRETCH == m_eWatermarkStyle ||
                WIZ97_BITMAP_CENTER == m_eWatermarkStyle);

            CRect rc;
            GetClientRect(rc);

            int x = 0, y = 0;

            switch(m_eWatermarkStyle)
            {
            case WIZ97_BITMAP_CENTER:
                // Center the bitmap
                CPropertyPage::OnEraseBkgnd(pDC);

                x = (rc.Width() - m_bmpWatermark.GetWidth()) / 2;
                y = (rc.Height() - m_bmpWatermark.GetHeight()) / 2;
                m_bmpWatermark.DrawDIB(pDC, x, y);
                break;

            case WIZ97_BITMAP_STRETCH:
                // stretch bitmap so it will best fit to the dialog
                m_bmpWatermark.DrawDIB(pDC, 0, 0, rc.Width(), rc.Height());
                break;

            default:
                // Tile the bitmap
                while(y < rc.Height())
                {
                    while(x < rc.Width())
                    {
                        m_bmpWatermark.DrawDIB(pDC, x, y);
                        x += m_bmpWatermark.GetWidth();
                    }
                    x = 0;
                    y += m_bmpWatermark.GetHeight();
                }
                break;
            }
            return TRUE;
        }
    }
    // No bitmap set. behave like a normal property page
    return CPropertyPage::OnEraseBkgnd(pDC);
}


HBRUSH CNGWizardPage::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
    if (!m_bmpWatermark.IsEmpty() )
    {
        if (NULL != m_bmpWatermark.GetPixelPtr() )
        {
            switch (nCtlColor)
            {
            case CTLCOLOR_STATIC:
                // The Slider Control has CTLCOLOR_STATIC, but doesn't let
                // the background shine through,
                TCHAR lpszClassName[255];
                GetClassName(pWnd->m_hWnd, lpszClassName, 255);
                if(_tcscmp(lpszClassName, TRACKBAR_CLASS) == 0)
                {
                    return CPropertyPage::OnCtlColor(pDC, pWnd, nCtlColor);
                }

            case CTLCOLOR_BTN:
                // Let static controls shine through
                pDC->SetBkMode(TRANSPARENT);
                return HBRUSH(m_HollowBrush);

            default:
                break;
            }
        }
    }
    // If we reach this line, we haven't set a brush so far
    return CPropertyPage::OnCtlColor(pDC, pWnd, nCtlColor);
}


BOOL CNGWizardPage::OnQueryNewPalette(void) 
{
    if (!m_bmpWatermark.IsEmpty() )
    {
        CPalette* pPal = m_bmpWatermark.GetPalette();
        if ( (NULL != pPal) && (NULL != GetSafeHwnd() ) )
        {
            CClientDC dc(this);
            CPalette* pOldPalette = dc.SelectPalette(pPal, FALSE);
            UINT nChanged = dc.RealizePalette();
            dc.SelectPalette(pOldPalette, TRUE);

            if (0 == nChanged)
            {
                return FALSE;
            }
            Invalidate();
            return TRUE;
        }
    }
    return CPropertyPage::OnQueryNewPalette();
}


void CNGWizardPage::OnPaletteChanged(CWnd* pFocusWnd) 
{
    if (!m_bmpWatermark.IsEmpty() )
    {
        CPalette* pPal = m_bmpWatermark.GetPalette();
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
        CPropertyPage::OnPaletteChanged(pFocusWnd);
    }
}

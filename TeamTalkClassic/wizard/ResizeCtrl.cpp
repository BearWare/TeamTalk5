// ResizeCtrl.cpp: implementation of the CResizeCtrl class.
//
//
// Written by Herbert Menke (h.menke@gmx.de)
// Copyright (c) 2000.
//
// This code may be used in compiled form in any way you desire. This
// file may be redistributed unmodified by any means PROVIDING it is 
// not sold for profit without the authors written consent, and 
// providing that this notice and the authors name is included. If 
// the source code in  this file is used in any commercial application 
// then acknowledgement must be made to the author of this file 
// (in whatever form you wish).
//
// This file is provided "as is" with no expressed or implied warranty.
//
// Expect bugs.
// 
// Please use and enjoy. Please let me know of any bugs/mods/improvements 
// that you have found/implemented and I will fix/incorporate them into this
// file. 
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ResizeCtrl.h"
#include <afxtempl.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

// Specification struct
struct RSRect
{
    int left;   // Specifies the percentage change in the position of the left edge 
    // of the object relative to the total change in the parent form’s width. 
    int top;    // Specifies the percentage change in the position of the top 
    // of the object relative to the total change in the parent form’s height.
    int width;  // Specifies the percentage change in the width of the object 
    // relative to the total change in the parent form’s width.
    int height; // Specifies the percentage change in the height of the object 
    // relative to the total change in the parent form’s height.
};


struct CRPItemState
{
    HWND   handle;    // Handle of Control
    RSRect pending;   // pending Resize pixels
    RSRect part;      // resize specifications
};

class CResizeArray : public CArray<CRPItemState,CRPItemState> { };

static const TCHAR m_szResizeProperty[]    = _T("___CResizeCtrl___Class___");
static const UINT m_idGetResizeCtrl        = ::RegisterWindowMessage( _T("WM_GETRESIZECTRL") );

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CResizeCtrl::CResizeCtrl( )
    : m_array       ( NULL  )
    , m_hWndParent  ( NULL  )
    , m_maxPart     ( 100   )
    , m_inResize    ( FALSE )
    , m_prevWndProc ( NULL  )
    , m_enabled     ( FALSE )
    , m_minTracking ( CSize( -1, -1 ) )
    , m_maxTracking ( CSize( -1, -1 ) )
    , m_hitCode     ( 0 )
    , m_inMouseMove ( FALSE )
    , m_delta       ( CSize( 0, 0 ) )
    , m_windowRect  ( CRect( 0, 0, 0, 0 ) )
    , m_gripEnabled ( FALSE )
    , m_margins     ( NULL )
{
}

CResizeCtrl::CResizeCtrl( HWND hWndParent, BOOL enable, int maxPart, BOOL isPropertyPage )
    : m_array( NULL )
    , m_hWndParent  ( NULL  )
    , m_maxPart     ( 100   )
    , m_inResize    ( FALSE )
    , m_prevWndProc ( NULL  )
    , m_enabled     ( FALSE )
    , m_minTracking ( CSize( -1, -1 ) )
    , m_maxTracking ( CSize( -1, -1 ) )
    , m_hitCode     ( 0 )
    , m_inMouseMove ( FALSE )
    , m_delta       ( CSize( 0, 0 ) )
    , m_gripEnabled ( FALSE )
    , m_margins     ( NULL )
{
    Create( hWndParent, enable, maxPart, isPropertyPage );

}

CResizeCtrl::CResizeCtrl( CWnd * wndParent, BOOL enable, int maxPart )
    : m_array       ( NULL  )
    , m_hWndParent  ( NULL  )
    , m_maxPart     ( 100   )
    , m_inResize    ( FALSE )
    , m_prevWndProc ( NULL  )
    , m_enabled     ( FALSE )
    , m_minTracking ( CSize( -1, -1 ) )
    , m_maxTracking ( CSize( -1, -1 ) )
    , m_hitCode     ( 0 )
    , m_inMouseMove ( FALSE )
    , m_delta       ( CSize( 0, 0 ) )
    , m_gripEnabled ( FALSE )
    , m_margins     ( NULL )
{
    Create( wndParent, enable, maxPart );
}

CResizeCtrl::~CResizeCtrl()
{
    if( m_array )
    {
        SetEnabled( FALSE );
        m_array->RemoveAll();
        delete m_array;
    }
    if( m_margins )
        delete m_margins;
}

BOOL CResizeCtrl::Create( CWnd * wndParent, BOOL enable, int maxPart )
{
    ASSERT( wndParent );
    if( wndParent )
    {
        return Create( wndParent->GetSafeHwnd(), enable, maxPart,
            wndParent->IsKindOf( RUNTIME_CLASS( CPropertyPage ) ));
    }
    return FALSE;
}

BOOL CResizeCtrl::Create( HWND hWndParent, BOOL enable, int maxPart, BOOL isPropertyPage )
{
    ASSERT( !m_array );
    if( m_array )
        return FALSE;

    m_array = new CResizeArray();
    ASSERT( m_array );
    ASSERT( hWndParent );
    ASSERT( maxPart > 0 );
    m_hWndParent  = hWndParent;
    m_maxPart     = maxPart;
    m_inResize    = FALSE;
    m_prevWndProc = NULL;
    m_enabled     = FALSE;
    m_minTracking.cx  =
        m_minTracking.cy  =
        m_maxTracking.cx  =
        m_maxTracking.cy  = -1;
    if( isPropertyPage )
        m_hasResizingBorder = TRUE;
    else    
        m_hasResizingBorder = (::GetWindowLong( hWndParent, GWL_STYLE ) & WS_THICKFRAME ) == WS_THICKFRAME;
    if( enable )
        SetEnabled( TRUE );
    else
        ::GetWindowRect( hWndParent , &m_windowRect );
    return TRUE;
}



//
// Enabled Property
//

BOOL CResizeCtrl::SetEnabled( BOOL enable )
{
    if( !m_array )
        return FALSE;
    ASSERT ( m_array );

    if( m_enabled != enable )
    {
        ASSERT( m_hWndParent  );
        ::GetWindowRect( m_hWndParent, &m_windowRect );
        // remove subclassing
        if( FALSE == enable )
        {
            ASSERT( m_prevWndProc );
//#ifdef _WIN64
//            ::SetWindowLong( m_hWndParent, GWLP_WNDPROC, reinterpret_cast<LONG>( m_prevWndProc ) );
//#else
//            ::SetWindowLong( m_hWndParent, GWL_WNDPROC, reinterpret_cast<LONG>( m_prevWndProc ) );
//#endif
            m_prevWndProc = NULL;
            ::RemoveProp( m_hWndParent, m_szResizeProperty );
            if( m_hasResizingBorder == FALSE )
                ChangeStyle( enable );  
        }
        else
        {
            m_hitCode       = 0;
            m_delta.cx      =
                m_delta.cy      = 0;
            m_inResize      = 
                m_inMouseMove   = FALSE;
            //m_hWndFocus     = NULL;

            WNDPROC wndProc = CResizeCtrl::WndProc;
            if( m_hasResizingBorder == FALSE )
            {
                ChangeStyle( enable );  
            }
            CRect   rect;
            ::GetClientRect( m_hWndParent, &rect );
            m_size.cx = rect.Width();
            m_size.cy = rect.Height();

            ::SetProp( m_hWndParent, m_szResizeProperty, reinterpret_cast<HANDLE>(this) );
//#ifdef _WIN64
//            m_prevWndProc = reinterpret_cast<WNDPROC>( ::GetWindowLong( m_hWndParent, GWLP_WNDPROC ) );
//            ::SetWindowLong( m_hWndParent, GWLP_WNDPROC, reinterpret_cast<LONG>( wndProc ) );
//#else
//            m_prevWndProc = reinterpret_cast<WNDPROC>( ::GetWindowLong( m_hWndParent, GWL_WNDPROC ) );
//            ::SetWindowLong( m_hWndParent, GWL_WNDPROC, reinterpret_cast<LONG>( wndProc ) );
//#endif
        }
        m_enabled = enable;
        if( m_gripEnabled )
        {
            GetGripRect( m_gripRect, TRUE );
        }
        return TRUE;
    }
    return FALSE;
}
BOOL CResizeCtrl::GetEnabled() const
{
    return m_enabled;
}

//
// GripEnabled Property
//

BOOL CResizeCtrl::SetGripEnabled( BOOL showGrip )
{
    if( m_gripEnabled != showGrip )
    {
        m_gripEnabled = showGrip;
        if( m_enabled )
            GetGripRect( m_gripRect, TRUE );
    }
    return FALSE;
}
BOOL CResizeCtrl::GetGripEnabled() const
{
    return m_gripEnabled;
}

// resizeInfo is a null terminated array of CResizeInfo

BOOL CResizeCtrl::Add( const CResizeInfo * resizeInfo )
{
    ASSERT ( m_array );

    BOOL result = TRUE;
    while( result == TRUE && resizeInfo->ctlID > 0 )
    {
        result &= Add( resizeInfo->ctlID,
            resizeInfo->left,
            resizeInfo->top,
            resizeInfo->width,
            resizeInfo->height );
        resizeInfo++;               
    }
    return result;
}

BOOL CResizeCtrl::Add( int ctlID,  int left, int top, int width, int height )
{
    ASSERT ( m_array );

    return Add( ::GetDlgItem( m_hWndParent, ctlID), left, top, width, height );
}

BOOL CResizeCtrl::Add( CWnd * wndCtl, int left, int top, int width, int height )
{
    ASSERT ( m_array );

    if( wndCtl )
        return Add( wndCtl->GetSafeHwnd(), left, top, width, height );
    return FALSE;

}

BOOL CResizeCtrl::Add(HWND hWndCtl, int left, int top, int width, int height)
{
    ASSERT ( m_array );

    if( left < 0 || left > m_maxPart )
    {
        return FALSE;
    }
    if( top < 0 || top > m_maxPart )
    {
        return FALSE;
    }
    if( width < 0 || width > m_maxPart )
    {
        return FALSE;
    }
    if( height < 0 || height > m_maxPart )
    {
        return FALSE;
    }

    if( ( left + width ) > m_maxPart )
    {
        return FALSE;
    }
    if( ( top + height) > m_maxPart )
    {
        return FALSE;
    }

    if( !::IsWindow( hWndCtl))
        return FALSE;

    CRPItemState item;

    item.part.left   = left;
    item.part.top    = top;
    item.part.width  = width;
    item.part.height = height;
    item.pending.left   =
        item.pending.top    =
        item.pending.width  =
        item.pending.height = 0;
    item.handle         = hWndCtl;

    return m_array->Add( item ) >= 0 ;
}

BOOL CResizeCtrl::Remove( int ctlID )
{
    ASSERT ( m_array );

    return Remove( ::GetDlgItem( m_hWndParent, ctlID ) );
}

BOOL CResizeCtrl::Remove( CWnd * wndCtl )
{
    ASSERT ( m_array );

    if( wndCtl )
        return Remove( wndCtl->GetSafeHwnd () );
    return FALSE;  
}

BOOL CResizeCtrl::Remove(HWND hWndCtl)
{
    ASSERT ( m_array );

    if( !::IsWindow( hWndCtl))
        return FALSE;

    INT_PTR upperBound = m_array->GetUpperBound ();
    for( int current = 0; current <= upperBound; current++ )
    {
        if( m_array->GetAt( current ).handle == hWndCtl )
        {
            m_array->RemoveAt( current );
            return TRUE;
        }
    }
    return FALSE;
}

BOOL CResizeCtrl::GetWindowRect( RECT * rect )
{
    if( rect )
    {
        *rect = m_windowRect;
        return TRUE;
    }
    else
        return FALSE;
}
#if 0
BOOL CResizeCtrl::CalcValue(int delta, int part, int & pending, long &position, BOOL isSize)
{
    if( part > 0 )
    {
        int toAdd = ( delta * part ) + pending;
        if( toAdd != 0 )
        {
            position  += ( toAdd / m_maxPart );
            pending    =   toAdd % m_maxPart ;
            // avoid negative width or height
            if( TRUE == isSize && position < 0 )
            {
                pending += ( position * m_maxPart );
                position = 0;
            }
            return TRUE;
        }
    }
    return FALSE;
}
#endif
BOOL CResizeCtrl::CalcValue(int delta, int part, int & pending, long &position, BOOL isSize)
{
    if( part > 0 )
    {
        int toAdd = delta * part;
        if( toAdd != 0 )
        {
            pending   +=   toAdd % m_maxPart ;

            if( pending >= m_maxPart )
            {
                pending -= m_maxPart;
                position++;
            }
            else if( pending <= -m_maxPart )
            {
                pending += m_maxPart;
                position--;
            }

            position  += ( toAdd / m_maxPart );

            // avoid negative width or height
            if( TRUE == isSize && position < 0 )
            {
                pending += ( position * m_maxPart );
                position = 0;
            }
            return TRUE;
        }
    }
    return FALSE;
}

void CResizeCtrl::Resize(int cx, int cy)
{
    ASSERT ( m_array );

    if( FALSE == m_inResize )
    {
        m_inResize     = TRUE;
        INT_PTR upperBound = m_array->GetUpperBound();
        if( upperBound >= 0 )
        {
            int deltaX = cx - m_size.cx;
            int deltaY = cy - m_size.cy;

            if( deltaX != 0 || deltaY != 0 )
            {
                UINT flags = SWP_NOZORDER;
                if( !IsWindowVisible( m_hWndParent ) )
                    flags |= SWP_NOREDRAW | SWP_NOACTIVATE;

                //        static int m_count = 0;
                //        int count = ++m_count;

                CRPItemState * items = m_array->GetData();
                HDWP  hdwp = ::BeginDeferWindowPos( 0 );
                //        TRACE( "%08d Start Resize hwnd: 0x%8.8x Flags: 0x%8.8x\n", count, m_hWndParent, flags );
                for( int current = 0; current <= upperBound; current++, items++ )
                {
                    RECT rcItem;


                    ::GetWindowRect( items->handle, & rcItem );
                    ::MapWindowPoints( HWND_DESKTOP, m_hWndParent, (LPPOINT)(RECT*)&rcItem, 2 );
                    rcItem.right  -= rcItem.left;
                    rcItem.bottom -= rcItem.top;

                    BOOL changed = FALSE;

                    changed |= CalcValue( deltaX, items->part.left,   items->pending.left,   rcItem.left,   FALSE );
                    changed |= CalcValue( deltaX, items->part.width,  items->pending.width,  rcItem.right,  TRUE );
                    changed |= CalcValue( deltaY, items->part.top,    items->pending.top,    rcItem.top,    FALSE );
                    changed |= CalcValue( deltaY, items->part.height, items->pending.height, rcItem.bottom, TRUE );

                    if( changed )
                    {
                        hdwp = ::DeferWindowPos( hdwp, items->handle, NULL,
                            rcItem.left, rcItem.top,
                            rcItem.right, rcItem.bottom, flags    );
                    }

                }
                //      TRACE( "%08d End   Resize hwnd: 0x%8.8x Flags: 0x%8.8x\n", count, m_hWndParent, flags );
                ::EndDeferWindowPos( hdwp );
                m_size.cx = cx;
                m_size.cy = cy;
            }
        }
        m_inResize = FALSE;
    }
}


LRESULT CALLBACK CResizeCtrl::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;
    CResizeCtrl * This = reinterpret_cast<CResizeCtrl *>(::GetProp( hWnd, m_szResizeProperty ));
    if( This )
    {
        WNDPROC prevWndProc = This->m_prevWndProc;
        if( FALSE == This->ProcessMessage( msg, wParam, lParam, result ) )
            result =  ::CallWindowProc( prevWndProc, hWnd, msg, wParam, lParam );
    }
    return result;
}



void CResizeCtrl::ChangeStyle(BOOL enable)
{
    ASSERT( m_hWndParent );
    ASSERT( m_hasResizingBorder == FALSE );
    CRect rect;

    BOOL hasMenu = ::GetMenu( m_hWndParent ) != NULL;
    long style   = ::GetWindowLong( m_hWndParent, GWL_STYLE );
    ::GetWindowRect( m_hWndParent, &rect );

    // retrieve client Rectangle
    RECT oldClientRect;
    ::GetClientRect( m_hWndParent, &oldClientRect );
    RECT newClientRect = oldClientRect;

    // adjust rect with current style
    ::AdjustWindowRect( &oldClientRect, style, hasMenu );

    if( enable )
    {
        style |= WS_THICKFRAME;
    }
    else
    {
        style &= ~WS_THICKFRAME;
    }

    // adjust rect with new style
    ::AdjustWindowRect( &newClientRect, style, hasMenu );

    // and adjust the windowrect, so that the clientrect remains equal
    rect.left   += ( newClientRect.left   - oldClientRect.left );
    rect.right  += ( newClientRect.right  - oldClientRect.right );
    rect.top    += ( newClientRect.top    - oldClientRect.top   );
    rect.bottom += ( newClientRect.bottom - oldClientRect.bottom );

    ::SetWindowLong( m_hWndParent, GWL_STYLE, style );
    ::SetWindowPos ( m_hWndParent, HWND_DESKTOP, rect.left, rect.top, rect.Width(), rect.Height(),
        SWP_NOZORDER | SWP_NOACTIVATE  );

}

BOOL CResizeCtrl::ProcessMessage(UINT message, WPARAM wParam, LPARAM lParam, LRESULT & result)
{
    BOOL handled = FALSE;
    if( message == m_idGetResizeCtrl )
    {
        result  = reinterpret_cast<LRESULT>( this);
        handled = TRUE;
    }
    else if( m_gripEnabled && WM_NCHITTEST == message )
    {
        POINT pt = { (int)LOWORD(lParam), (int)HIWORD(lParam) };
        ::ScreenToClient( m_hWndParent, &pt);
        if( ::PtInRect ( m_gripRect, pt ) )
        {
            result  = HTBOTTOMRIGHT;
            handled = TRUE;
        }
    }
    else if( m_gripEnabled && WM_PAINT == message )
    {
        // First let the previous windowproc handle the WM_SIZE message
        result = ::CallWindowProc( m_prevWndProc, m_hWndParent, message, wParam, lParam );
        HDC hDC = ::GetDC( m_hWndParent );
        DrawFrameControl(hDC, &m_gripRect, DFC_SCROLL, DFCS_SCROLLSIZEGRIP);
        ::ReleaseDC( m_hWndParent, hDC ); 
        // flag message handled
        handled = TRUE;
    }
    else if( WM_DESTROY == message )
    {
        // Remove subclassing
        SetEnabled( FALSE );
    }
    else if( WM_SIZE == message )
    {
        // First let the previous windowproc handle the WM_SIZE message
        result = ::CallWindowProc( m_prevWndProc, m_hWndParent, message, wParam, lParam );
        // resize all registered controls
        Resize( (int)LOWORD(lParam) , (int)HIWORD(lParam) );
        if( m_gripEnabled )
        {
            ::InvalidateRect( m_hWndParent, m_gripRect, TRUE );
            GetGripRect( m_gripRect, TRUE );
        }
        // flag message handled
        handled = TRUE;
    }
    else if( WM_GETMINMAXINFO == message )
    {
        // First let the previous windowproc handle the WM_GETMINMAXINFO message
        result = ::CallWindowProc( m_prevWndProc, m_hWndParent, message, wParam, lParam );
        MINMAXINFO * lpMMI = (MINMAXINFO * )lParam;
        if( m_minTracking.cx == -1 )
            m_minTracking.cx = lpMMI->ptMinTrackSize.x;

        if( m_minTracking.cy == -1 )
            m_minTracking.cy = lpMMI->ptMinTrackSize.y;

        if( m_maxTracking.cx == -1 )
            m_maxTracking.cx = lpMMI->ptMaxTrackSize.x;

        if( m_maxTracking.cy == -1 )
            m_maxTracking.cy = lpMMI->ptMaxTrackSize.y;

        lpMMI->ptMinTrackSize.x = m_minTracking.cx;
        lpMMI->ptMinTrackSize.y = m_minTracking.cy;
        lpMMI->ptMaxTrackSize.x = m_maxTracking.cx;
        lpMMI->ptMaxTrackSize.y = m_maxTracking.cy;
        // flag message handled
        handled = TRUE;
    }
    else
    {
        // if the original window had no resizing border
        // we must serve WM_MOUSE, WM_NCLBUTTONDOWN and
        // WM_LBUTTONUP to enable resizing
        if( !m_hasResizingBorder )
        {
            POINT mousePostion;
            RECT  currentRect;

            if( WM_MOUSEMOVE == message )
            {
                if (m_hitCode && !m_inMouseMove )
                {
                    m_inMouseMove = TRUE;

                    ::GetCursorPos( &mousePostion );
                    mousePostion.x += m_delta.cx;
                    mousePostion.y += m_delta.cy;

                    RECT  m_previsionRect;

                    ::GetWindowRect( m_hWndParent, &currentRect );
                    m_previsionRect = currentRect;

                    switch( m_hitCode )
                    {
                    case HTTOPLEFT     : currentRect.left   = mousePostion.x; // fall through
                    case HTTOP         : currentRect.top    = mousePostion.y;    break;

                    case HTBOTTOMRIGHT : currentRect.right  = mousePostion.x; // fall through
                    case HTBOTTOM      : currentRect.bottom = mousePostion.y; break;

                    case HTBOTTOMLEFT  : currentRect.bottom = mousePostion.y; // fall through
                    case HTLEFT        : currentRect.left   = mousePostion.x;    break;

                    case HTTOPRIGHT    : currentRect.top    = mousePostion.y; // fall through
                    case HTRIGHT       : currentRect.right  = mousePostion.x;    break;
                    }
                    if (!::EqualRect( &currentRect, &m_previsionRect ))
                    {
                        int width  = currentRect.right - currentRect.left;
                        int height = currentRect.bottom - currentRect.top;

                        ::SetWindowPos( m_hWndParent, HWND_DESKTOP, currentRect.left, currentRect.top,    
                            width, height, SWP_NOZORDER | SWP_NOACTIVATE );
                    }
                    m_inMouseMove = FALSE;
                }
            }
            else if( WM_NCLBUTTONDOWN == message )
            {
                ::GetCursorPos( &mousePostion );
                ::GetWindowRect( m_hWndParent, &currentRect );

                m_hitCode  = int(wParam);
                m_delta.cx = 
                    m_delta.cy = 0;

                switch( m_hitCode )
                {
                case HTTOPLEFT     : m_delta.cx = currentRect.left   - mousePostion.x; // fall through
                case HTTOP         : m_delta.cy = currentRect.top    - mousePostion.y; break;

                case HTBOTTOMRIGHT : m_delta.cx = currentRect.right  - mousePostion.x; // fall through
                case HTBOTTOM      : m_delta.cy = currentRect.bottom - mousePostion.y; break;

                case HTBOTTOMLEFT  : m_delta.cy = currentRect.bottom - mousePostion.y; // fall through
                case HTLEFT        : m_delta.cx = currentRect.left   - mousePostion.x; break;

                case HTTOPRIGHT    : m_delta.cy = currentRect.top    - mousePostion.y; // fall through
                case HTRIGHT       : m_delta.cx = currentRect.right  - mousePostion.x; break;

                default            : m_hitCode = 0; break;
                }

                if (m_hitCode)
                {
                    ::SetCapture( m_hWndParent );
                }

            }
            else if( WM_LBUTTONUP == message )
            {
                if (m_hitCode != 0)
                {
                    ::ReleaseCapture();
                    m_hitCode   = 0;
                }
            }
        }
    }
    return handled;
}
///////////////////////////////////////////////////////////////////////
// MinMaxInfo Support

BOOL  CResizeCtrl::SetMinimumTrackingSize( const CSize & size )
{
    m_minTracking = size;
    return TRUE;
}
BOOL  CResizeCtrl::SetMinimumTrackingSize()
{
    RECT  rect;
    ::GetWindowRect( m_hWndParent, &rect );
    return SetMinimumTrackingSize( CSize( rect.right - rect.left, rect.bottom - rect.top ) );
}
CSize CResizeCtrl::GetMinimumTrackingSize( )
{
    return m_minTracking;
}

BOOL  CResizeCtrl::SetMaximumTrackingSize( const CSize & size )
{
    m_maxTracking = size;
    return TRUE;
}
CSize CResizeCtrl::GetMaximumTrackingSize( )
{
    return m_maxTracking;
}


void CResizeCtrl::GetGripRect(RECT & rect, BOOL redraw)
{
    GetClientRect( m_hWndParent, &rect );
    rect.left = rect.right  - ::GetSystemMetrics(SM_CXVSCROLL) ;
    rect.top  = rect.bottom - ::GetSystemMetrics(SM_CYHSCROLL) ;
    if( redraw )
        ::InvalidateRect( m_hWndParent, &rect, TRUE );
}

//
// Support for CPropertyPage
//

CResizeCtrl * CResizeCtrl::GetResizeCtrl( CWnd * wnd )
{
    if( wnd )
        return GetResizeCtrl( wnd->GetSafeHwnd() );
    else
        return NULL;
}
CResizeCtrl * CResizeCtrl::GetResizeCtrl( HWND hWnd )
{
    if( ::IsWindow( hWnd ) )
    {
        return reinterpret_cast<CResizeCtrl*>( ::SendMessage( hWnd, m_idGetResizeCtrl, 0, 0 ) );
    }
    else
        return NULL;
}

//
// Support for CPropertySheet and Wizard
//

// Prepares the propertysheet to ensure that
// all pages are created , should be called
// before the control is enabled 
BOOL CResizeCtrl::PropertySheetPrepare( CPropertySheet * sheet )
{
    if( GetEnabled() == FALSE && sheet )
    {
        int count        = sheet->GetPageCount();
        int selectedPage = sheet->GetActiveIndex();
        int currentPage = 0;
        // Activated all pages except the current active page
        // this ensures, that all pages are created
        for( currentPage = 0; currentPage < count; currentPage++ )
        {
            if( currentPage != selectedPage )
                sheet->SetActivePage(currentPage );
        }
        // reselect the active page
        if( --currentPage != selectedPage )
            sheet->SetActivePage( selectedPage );
        return count > 0 ;
    }
    return FALSE;
}

// Adds the buttons of PropertySheet or Wizard
BOOL CResizeCtrl::PropertySheetAddButtons( CPropertySheet * sheet, int left, int top, int width, int height )
{
    if( sheet )
    {
        HWND hWnd = sheet->GetSafeHwnd();
        BOOL ok = TRUE;
        if( sheet->IsWizard() )
        {
            // Add Next/Prev...
            ok &= Add( ::GetDlgItem( hWnd , 0x3023 )    ,left, top, width, height );
            ok &= Add( ::GetDlgItem( hWnd , 0x3024 )    ,left, top, width, height );
            ok &= Add( ::GetDlgItem( hWnd , 0x3025 )    ,left, top, width, height );
        }
        else
        {
            ok &= Add( ::GetDlgItem( hWnd , ID_APPLY_NOW  )    ,left, top, width, height );
        }
        ok &= Add( ::GetDlgItem( hWnd , IDOK )      ,left, top, width, height );
        ok &= Add( ::GetDlgItem( hWnd , IDCANCEL )  ,left, top, width, height );
        ok &= Add( ::GetDlgItem( hWnd , IDHELP   )  ,left, top, width, height );
        return ok;
    }
    return FALSE;
}
// Adds the pages of PropertySheet or Wizard
BOOL CResizeCtrl::PropertySheetAddPages( CPropertySheet * sheet, int left, int top, int width, int height )
{
    if( sheet )
    {
        BOOL ok    = Add( sheet->GetTabControl(), left, top, width, height );;
        int  count = sheet->GetPageCount();
        for( int currentPage = 0; currentPage < count; currentPage++ )
        {
            ok &= Add( sheet->GetPage( currentPage), left, top, width, height );
        }
        return ok;
    }
    return FALSE;
}
// Adds the etched bar beneath the pages
BOOL CResizeCtrl::WizardAddEtchedBar( CPropertySheet * sheet, int left, int top, int width, int height )
{
    if( sheet && sheet->IsWizard() )
    {
        return Add( ::GetDlgItem( sheet->GetSafeHwnd() , 0x3026 ), left, top, width, height );
    }
    return FALSE;
}

// saves the the delta-size between the wizard-size and a page-size
// because WizBack and WizNext resizes a page to the initial Size
BOOL CResizeCtrl::WizardSaveMargins( CPropertySheet * sheet )
{
    if( sheet )
    {
        if( NULL == m_margins )
            m_margins = new CRect;
        ASSERT( m_margins );

        sheet->GetWindowRect( m_margins );
        CRect rect;
        CPropertyPage *  page = sheet->GetActivePage( );
        page->GetWindowRect( &rect );
        m_margins->left    = rect.left - m_margins->left;
        m_margins->top     = rect.top  - m_margins->top;
        m_margins->right  -= rect.right;
        m_margins->bottom -= rect.bottom;
        return TRUE;
    }
    return FALSE;
}


// restores propertypage size after WizBack or WizNext
BOOL CResizeCtrl::WizardAdjustPage( CPropertySheet * sheet )
{
    if (sheet && m_margins)
    {
        if (sheet->GetActivePage() == NULL)
        {
            return FALSE;
        }
        // because WizBack and WizNext resizes
        // a page to the initial Size
        // we need to change the size
        CRect rect;
        sheet->GetWindowRect( &rect );

        BOOL bHasWizard97Header = FALSE;

        // The following is needed to handle pages with WIZARD97 header bitmaps
        if (sheet->IsKindOf(RUNTIME_CLASS(CPropertySheetEx) ) )
        {
            BOOL bIsWizard97 = ( ( ((CPropertySheetEx*)sheet)->m_psh.dwFlags & PSH_WIZARD97) != 0);
            if (bIsWizard97)
            {
                CPropertyPageEx* pPage = (CPropertyPageEx*)sheet->GetActivePage();
                if (pPage != NULL)
                {
                    bHasWizard97Header = ( (pPage->m_psp.dwFlags & PSP_HIDEHEADER) == 0);
                }
            }
        }
        if (bHasWizard97Header)
        {
            // If the page has a header, we need to move it below the border.
            // By inspection (Spy++), the border is a static control with ID 0x3027
            CWnd* pTopBorder = sheet->GetDlgItem(0x3027);
            ASSERT(pTopBorder != NULL);
            if (pTopBorder != NULL)
            {
                CRect rectTopBorder;
                pTopBorder->GetWindowRect(rectTopBorder);

                // With the exception of the bottom, the margins are already
                // accounted for by the position of the top border...
                rect.left    = rectTopBorder.left;
                rect.top    = rectTopBorder.top;
                rect.right    -= m_margins->right;
                rect.bottom    -= m_margins->bottom;
            }
        }
        else
        {
            rect.left   += m_margins->left;
            rect.top    += m_margins->top;
            rect.right  -= m_margins->right;
            rect.bottom -= m_margins->bottom;
        }
        sheet->ScreenToClient( &rect );
        CPropertyPage *  page = sheet->GetActivePage( );
        page->MoveWindow( &rect );
        return TRUE;
    }
    return FALSE;
}


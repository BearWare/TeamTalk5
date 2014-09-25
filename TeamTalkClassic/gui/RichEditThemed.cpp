//////////////////////////////////////////////////////////////////////////////
//    Themed RichEdit Class Helper for Visual C++                                //
//  Created by Patchou, originally for Messenger Plus!                        //
//                                                                            //
//    RichEditThemed.cpp   -   Last Revision: 08/12/2004                        //
//  Purpose: adds Windows XP theme support to Rich Edit controls            //
//                                                                            //
//  This library is free as in NOT under GPL license. You can use it in any //
//  program, in the binary form of your choice, and without being asked to  //
//  add unnecessary files to your redistribution package. Free source       //
//  should neither be restrained by a 6 page license nor should it force    //
//  you to share the source of any part of a program you do not wish to     //
//  share. The meaning of the word freedom is the same for everyone, no     //
//  license should have the pretension of redefining it. If you want to     //
//  share some source code, please use a similar copyright notice, your     //
//  peers will be grateful.                                                    //
//                                                                            //
//  Free redistribution and usage of the content of this file is permitted  //
//  as long as the above statement is kept, unchanged, at its original      //
//  location.                                                                //
//                                                                            //
//////////////////////////////////////////////////////////////////////////////

//If your project does not use a precompiled header, comment the first line and uncomment the next ones
#include "stdafx.h"
//#define _WIN32_WINNT 0x0501
//#include <windows.h>
//#include <richedit.h>

//Library header file (required)
#include "RichEditThemed.h"

//This STL map is used to keep track of all the instances of the class
std::map<HWND,CRichEditThemed*> CRichEditThemed::m_aInstances;

//The theme-specific functions are imported at runtime for backward compatibility reasons
//A nicer alternative if you are using Visual C++ 7.0 or above is the use of the "delay load" mechanism
HMODULE CRichEditThemed::m_hUxTheme = NULL;
HTHEME (WINAPI *CRichEditThemed::pOpenThemeData)(HWND, LPCWSTR) = NULL;
HRESULT (WINAPI *CRichEditThemed::pCloseThemeData)(HTHEME) = NULL;
HRESULT (WINAPI *CRichEditThemed::pDrawThemeBackground)(HTHEME, HDC, int, int, const RECT*, const RECT *) = NULL;
HRESULT (WINAPI *CRichEditThemed::pGetThemeBackgroundContentRect)(HTHEME, HDC, int, int,  const RECT *, RECT *) = NULL;
BOOL (WINAPI *CRichEditThemed::pIsThemeActive)() = NULL;
HRESULT (WINAPI *CRichEditThemed::pDrawThemeParentBackground)(HWND, HDC, RECT*) = NULL;
BOOL (WINAPI *CRichEditThemed::pIsThemeBackgroundPartiallyTransparent)(HTHEME, int, int) = NULL;

//
// This function is the one and only public function your program must use
// It needs to be called during the creation/initialisation of the parent window
//////////////////////////////////////////////////////////////////////////////
bool CRichEditThemed::Attach(HWND hRichEdit)
{
    if(IsWindow(hRichEdit)) //It is your responsibility to ensure that the handle parameter is, indeed, a richedit window
    {
        //Prevent double subclassing
        if(CRichEditThemed::m_aInstances.find(hRichEdit) == CRichEditThemed::m_aInstances.end())
        {
            //If this function fails, this version of Windows doesn't support themes
            if(InitLibrary())
            {
                //Note: the object will be automatically deleted when the richedit control dies
                CRichEditThemed *obj = new CRichEditThemed(hRichEdit);
                return true;
            }
        }
    }
    return false;
}

//
//////////////////////////////////////////////////////////////////////////////
CRichEditThemed::CRichEditThemed(HWND hRichEdit) : m_hRichEdit(hRichEdit), m_bThemedBorder(false)
{
    //Subclass the richedit control, this way, the caller doesn't have to relay the messages by itself
    m_aInstances[hRichEdit] = this;
#ifdef _WIN64
    m_pOriginalWndProc = (WNDPROC)SetWindowLong(hRichEdit, GWLP_WNDPROC, (LONG)&RichEditStyledProc);
#else
    m_pOriginalWndProc = (WNDPROC)SetWindowLong(hRichEdit, GWL_WNDPROC, (LONG)&RichEditStyledProc);
#endif
    //Check the current state of the richedit control
    ZeroMemory(&m_rcClientPos, sizeof(RECT));
    VerifyThemedBorderState();
}

//
//////////////////////////////////////////////////////////////////////////////
CRichEditThemed::~CRichEditThemed()
{
    //Unload the UxTheme library if it is not needed anymore by a control
    m_aInstances.erase(m_hRichEdit);
    if(m_aInstances.empty())
    {
        pOpenThemeData =  NULL;
        pCloseThemeData = NULL;
        pDrawThemeBackground = NULL;
        pGetThemeBackgroundContentRect = NULL;
        pIsThemeActive = NULL;
        pDrawThemeParentBackground = NULL;
        pIsThemeBackgroundPartiallyTransparent = NULL;

        FreeLibrary(m_hUxTheme);
        m_hUxTheme = NULL;
    }
}

//
//////////////////////////////////////////////////////////////////////////////
bool CRichEditThemed::InitLibrary()
{
    //Are we already initialised?
    if(pOpenThemeData && pCloseThemeData && pDrawThemeBackground && pGetThemeBackgroundContentRect &&
        pIsThemeActive && pDrawThemeParentBackground && pIsThemeBackgroundPartiallyTransparent)
        return true;

    //Try to get the function pointers of the UxTheme library
    if(!m_hUxTheme)
    {
        m_hUxTheme = LoadLibrary(_T("UxTheme.dll"));
        if(!m_hUxTheme)
            return false;
    }

    pOpenThemeData = (HTHEME (WINAPI *)(HWND, LPCWSTR))GetProcAddress(m_hUxTheme, "OpenThemeData");
    pCloseThemeData = (HRESULT (WINAPI *)(HTHEME))GetProcAddress(m_hUxTheme, "CloseThemeData");
    pDrawThemeBackground = (HRESULT (WINAPI *)(HTHEME, HDC, int, int, const RECT*, const RECT *))GetProcAddress(m_hUxTheme, "DrawThemeBackground");
    pGetThemeBackgroundContentRect = (HRESULT (WINAPI *)(HTHEME, HDC, int, int,  const RECT *, RECT *))GetProcAddress(m_hUxTheme, "GetThemeBackgroundContentRect");
    pIsThemeActive = (BOOL (WINAPI *)())GetProcAddress(m_hUxTheme, "IsThemeActive");
    pDrawThemeParentBackground = (HRESULT (WINAPI *)(HWND, HDC, RECT*))GetProcAddress(m_hUxTheme, "DrawThemeParentBackground");
    pIsThemeBackgroundPartiallyTransparent = (BOOL (WINAPI *)(HTHEME, int, int))GetProcAddress(m_hUxTheme, "IsThemeBackgroundPartiallyTransparent");

    if(pOpenThemeData && pCloseThemeData && pDrawThemeBackground && pGetThemeBackgroundContentRect &&
        pIsThemeActive && pDrawThemeParentBackground && pIsThemeBackgroundPartiallyTransparent)
        return true;
    else
        return false;
}

//
//////////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK CRichEditThemed::RichEditStyledProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    //This function is the subclassed winproc of the richedit control
    //It is used to monitor the actions of the control, in a nice and transparent manner
    std::map<HWND,CRichEditThemed*>::iterator itCurInstance = m_aInstances.find(hwnd);
    if(itCurInstance != m_aInstances.end())
    {
        //A winproc is always static, this one is common to all the richedit controls managed by this class
        //We need to get a pointer to the object controlling the richedit which is receiving this message
        CRichEditThemed *pObj = itCurInstance->second;
        
        //If you get a compilation error here, it is probably because _WIN32_WINNT is not defined to at least 0x0501
        if(uMsg == WM_THEMECHANGED || uMsg == WM_STYLECHANGED)
        {
            //Someone just changed the style of the richedit control or the user changed its theme
            //Make sure the control is being kept up to date by verifying its state
            pObj->VerifyThemedBorderState();
        }
        else if(uMsg == WM_NCPAINT)
        {
            //Let the control paint its own non-client elements (such as its scrollbars)
            LRESULT nOriginalReturn = CallWindowProc(pObj->m_pOriginalWndProc, hwnd, uMsg, wParam, lParam);

            //Draw the theme, if necessary
            if(pObj->OnNCPaint())
                return 0;
            else
                return nOriginalReturn;
        }
        else if(uMsg == WM_ENABLE)
        {
            //Redraw the border depending on the state of the richedit control
            RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE|RDW_NOCHILDREN|RDW_UPDATENOW|RDW_FRAME);
        }
        else if(uMsg == WM_NCCALCSIZE)
        {
            //If wParam is FALSE, we don't need to make any calculation
            if(wParam)
            {
                //Ask the control to first calculate the space it needs
                LRESULT nOriginalReturn = CallWindowProc(pObj->m_pOriginalWndProc, hwnd, uMsg, wParam, lParam);

                //Alter the size for our own border, if necessary
                NCCALCSIZE_PARAMS *csparam = (NCCALCSIZE_PARAMS*)lParam;
                if(pObj->OnNCCalcSize(csparam))
                    return WVR_REDRAW;
                else
                    return nOriginalReturn;
            }
        }
        else if(uMsg == WM_DESTROY)
        {
            //Fail-safe: don't restore the original wndproc pointer if it has been modified since the creation of this object
            if(IsWindow(hwnd))
            {
                
#ifdef _WIN64
                if((WNDPROC)GetWindowLong(hwnd, GWLP_WNDPROC) == &RichEditStyledProc)
                    SetWindowLong(hwnd, GWLP_WNDPROC, (LONG)pObj->m_pOriginalWndProc);
#else
                if((WNDPROC)GetWindowLong(hwnd, GWL_WNDPROC) == &RichEditStyledProc)
                    SetWindowLong(hwnd, GWL_WNDPROC, (LONG)pObj->m_pOriginalWndProc);
#endif
            }

            //Send the message now so that we can safely delete the object afterward
            LRESULT nToReturn = CallWindowProc(pObj->m_pOriginalWndProc, hwnd, uMsg, wParam, lParam);

            delete pObj;
            return nToReturn;
        }
        return CallWindowProc(pObj->m_pOriginalWndProc, hwnd, uMsg, wParam, lParam);
    }
    else
        return 0;
}

//
//////////////////////////////////////////////////////////////////////////////
void CRichEditThemed::VerifyThemedBorderState()
{
    bool bCurrentThemedBorder = m_bThemedBorder;
    m_bThemedBorder = false;

    //First, check if the control is supposed to have a border
    if(bCurrentThemedBorder
        || (GetWindowLong(m_hRichEdit, GWL_STYLE) & WS_BORDER || GetWindowLong(m_hRichEdit, GWL_EXSTYLE) & WS_EX_CLIENTEDGE))
    {
        //Check if a theme is presently active
        if(pIsThemeActive())
        {
            //Remove the border style, we don't want the control to draw its own border
            m_bThemedBorder = true;
            if(GetWindowLong(m_hRichEdit, GWL_STYLE) & WS_BORDER)
                SetWindowLong(m_hRichEdit, GWL_STYLE, GetWindowLong(m_hRichEdit, GWL_STYLE)^WS_BORDER);
            if(GetWindowLong(m_hRichEdit, GWL_EXSTYLE) & WS_EX_CLIENTEDGE)
                SetWindowLong(m_hRichEdit, GWL_EXSTYLE, GetWindowLong(m_hRichEdit, GWL_EXSTYLE)^WS_EX_CLIENTEDGE);
        }
    }

    //Recalculate the NC area and repaint the window
    SetWindowPos(m_hRichEdit, NULL, NULL, NULL, NULL, NULL, SWP_NOMOVE|SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE|SWP_FRAMECHANGED);
    RedrawWindow(m_hRichEdit, NULL, NULL, RDW_INVALIDATE|RDW_NOCHILDREN|RDW_UPDATENOW|RDW_FRAME);
}

//
//////////////////////////////////////////////////////////////////////////////
bool CRichEditThemed::OnNCPaint()
{
    try
    {
        if(m_bThemedBorder)
        {
            HTHEME hTheme = pOpenThemeData(m_hRichEdit, L"edit");
            if(hTheme)
            {
                HDC hdc = GetWindowDC(m_hRichEdit);

                //Clip the DC so that we only draw on the non-client area
                RECT rcBorder;
                GetWindowRect(m_hRichEdit, &rcBorder);
                rcBorder.right -= rcBorder.left; rcBorder.bottom -= rcBorder.top;
                rcBorder.left = rcBorder.top = 0;

                RECT rcClient; memcpy(&rcClient, &rcBorder, sizeof(RECT));
                rcClient.left += m_rcClientPos.left;
                rcClient.top += m_rcClientPos.top;
                rcClient.right -= m_rcClientPos.right;
                rcClient.bottom -= m_rcClientPos.bottom;
                ExcludeClipRect(hdc, rcClient.left, rcClient.top, rcClient.right, rcClient.bottom);

                //Make sure the background is in a proper state
                if(pIsThemeBackgroundPartiallyTransparent(hTheme, EP_EDITTEXT, ETS_NORMAL))
                    pDrawThemeParentBackground(m_hRichEdit, hdc, &rcBorder);

                //Draw the border of the edit box
                int nState;
                if(!IsWindowEnabled(m_hRichEdit))
                    nState = ETS_DISABLED;
                else if(SendMessage(m_hRichEdit, EM_GETOPTIONS, NULL, NULL) & ECO_READONLY)
                    nState = ETS_READONLY;
                else
                    nState = ETS_NORMAL;
                
                pDrawThemeBackground(hTheme, hdc, EP_EDITTEXT, nState, &rcBorder, NULL);
                pCloseThemeData(hTheme);

                ReleaseDC(m_hRichEdit, hdc);
                return true;
            }
        }
    }
    catch(...)
    {
        //No exception is supposed to be thrown here but you can never be too safe
        //Trace an error here with your favorite TRACE macro
    }
    return false;
}

//
//////////////////////////////////////////////////////////////////////////////
bool CRichEditThemed::OnNCCalcSize(NCCALCSIZE_PARAMS *csparam)
{
    try
    {
        //Here, we indicate to Windows that the non-client area of the richedit control is not what it thinks it should be
        //This gives us the necessary space to draw the special border later on
        if(m_bThemedBorder)
        {
            //Load the theme associated with edit boxes
            HTHEME hTheme = pOpenThemeData(m_hRichEdit, L"edit");
            if(hTheme)
            {
                bool bToReturn = false;

                //Get the size required by the current theme to be displayed properly
                RECT rcClient; ZeroMemory(&rcClient, sizeof(RECT));
                HDC hdc = GetDC(GetParent(m_hRichEdit));
                if(pGetThemeBackgroundContentRect(hTheme, hdc, EP_EDITTEXT, ETS_NORMAL,
                    &csparam->rgrc[0], &rcClient) == S_OK)
                {
                    //Add a pixel to every edge so that the client area is not too close to the border drawn by the theme (thus simulating a native edit box)
                    InflateRect(&rcClient, -1, -1);

                    m_rcClientPos.left = rcClient.left-csparam->rgrc[0].left;
                    m_rcClientPos.top = rcClient.top-csparam->rgrc[0].top;
                    m_rcClientPos.right = csparam->rgrc[0].right-rcClient.right;
                    m_rcClientPos.bottom = csparam->rgrc[0].bottom-rcClient.bottom;

                    memcpy(&csparam->rgrc[0], &rcClient, sizeof(RECT));
                    bToReturn = true;
                }
                ReleaseDC(GetParent(m_hRichEdit), hdc);
                pCloseThemeData(hTheme);

                return bToReturn;
            }
        }

    }
    catch(...)
    {
        //No exception is supposed to be thrown here but you can never be too safe
        //Trace an error here with your favorite TRACE macro
    }
    return false;
}

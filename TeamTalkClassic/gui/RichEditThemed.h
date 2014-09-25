//////////////////////////////////////////////////////////////////////////////
//    Themed RichEdit Class Helper for Visual C++                                //
//  Created by Patchou, originally for Messenger Plus!                        //
//                                                                            //
//    RichEditThemed.h     -   Last Revision: 08/12/2004                        //
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
#pragma once

//_WIN32_WINNT must be defined to 0x0501 (or above) before including windows.h
//These files must be included in your project, preferably in stdafx.h if this is a Visual C++ Project
#include <tchar.h>        //Used for ANSI/Unicode compatibility
#include <map>            //Used internally by the class
#include <uxtheme.h>    //Used to access the UxTheme library
#include <tmschema.h>    //Definitions used by some of the UxTheme library functions

class CRichEditThemed
{
public:
    //This function must be called during the creation of your window, like in WM_INITDIALOG or WM_CREATE
    //It MUST be called before the richedit control is displayed for the first time
    static bool Attach(HWND hRichEdit);

private:
    CRichEditThemed(HWND hRichEdit);
    ~CRichEditThemed();

    void VerifyThemedBorderState();
    bool OnNCPaint();
    bool OnNCCalcSize(NCCALCSIZE_PARAMS *csparam);

    static bool InitLibrary();
    static LRESULT CALLBACK RichEditStyledProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
    static std::map<HWND,CRichEditThemed*> m_aInstances;

    HWND m_hRichEdit;
    WNDPROC m_pOriginalWndProc;
    bool m_bThemedBorder;
    RECT m_rcClientPos;

    //Function pointers from the UxTheme library
    static HMODULE m_hUxTheme;
    static HTHEME (WINAPI *pOpenThemeData)(HWND, LPCWSTR);
    static HRESULT (WINAPI *pCloseThemeData)(HTHEME);
    static HRESULT (WINAPI *pDrawThemeBackground)(HTHEME, HDC, int, int, const RECT*, const RECT *);
    static HRESULT (WINAPI *pGetThemeBackgroundContentRect)(HTHEME, HDC, int, int,  const RECT *, RECT *);
    static BOOL (WINAPI *pIsThemeActive)();
    static BOOL (WINAPI *pIsThemeBackgroundPartiallyTransparent)(HTHEME, int, int);
    static HRESULT (WINAPI *pDrawThemeParentBackground)(HWND, HDC, RECT*);
};

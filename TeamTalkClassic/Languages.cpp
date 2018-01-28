/*
 * Copyright (c) 2005-2017, BearWare.dk
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
 * This source code is part of the TeamTalk 5 SDK owned by
 * BearWare.dk. All copyright statements may not be removed 
 * or altered from any source distribution. If you use this
 * software in a product, an acknowledgment in the product 
 * documentation is required.
 *
 */

#include "StdAfx.h"
#include "Languages.h"
#define APSTUDIO_INVOKED        //we need the latest resource number

#include "Resource.h"

#define LANGUAGE_DIRECTORY _T("lng_classic")

#include <memory>

using namespace std;

Languages::Languages()
{
}

Languages::~Languages()
{
}

Languages* Languages::Instance(LPCTSTR szLangFile /*= 0*/)
{
    static Languages lng;
    
    if(szLangFile)
        lng.LoadLanguage(szLangFile);

    return &lng;
}

BOOL Languages::LoadLanguage(CString szLangFile)
{
    szLangFile = LANGUAGE_DIRECTORY _T("\\") + szLangFile;
    string localfile = STR_LOCAL(szLangFile);

    if(m_language.LoadFile(localfile))
    {
        string str;
        for (int iID = 0; iID < _APS_NEXT_COMMAND_VALUE; iID++)
        {
            str = m_language.GetItemText(iID);
            if(!str.empty())
            {
                CString szWord = STR_UTF8( str.c_str() );
                szWord.Replace(_T("\\t"), _T("\t"));
                m_mItems[iID] = szWord;
            }

            if(iID == _APS_NEXT_RESOURCE_VALUE)
                iID = 999;
            if(iID == _APS_NEXT_CONTROL_VALUE)
                iID = 32767;
        }

        m_szLanguage = szLangFile;
        return !m_mItems.empty();
    }
    else
        return FALSE;
}

CString Languages::GetCurLanguage()
{
    return m_szLanguage;
}

void Languages::ClearLanguage()
{
    m_mItems.clear();
    m_szLanguage.Empty();
}

CString Languages::GetItemText(UINT nItemID)
{
    CString szText = _T("UNKNOWN");
    mapitem_t::iterator ite = m_mItems.find(nItemID);
    if(ite != m_mItems.end())
        szText = ite->second;
    //else
    //    TRACE(_T("Missing Item ID %d\n"), nItemID);
    return szText;
}

BOOL Languages::GetItemText(UINT nItemID, CString& szText)
{
    mapitem_t::iterator ite = m_mItems.find(nItemID);
    if(ite != m_mItems.end())
    {
        szText = ite->second;
        return TRUE;
    }
    //else
    //    TRACE(_T("Missing Item ID %d\n"), nItemID);

    return FALSE;
}


void Languages::Translate(CDialog& dialog, UINT nResourceID)
{
    //set translated text for every know control ID
    for (int iID = 0; iID < _APS_NEXT_CONTROL_VALUE; iID++)
    {
        mapitem_t::iterator ite = m_mItems.find(iID);
        if(ite != m_mItems.end() && dialog.GetDlgItem(iID))
        {
            dialog.SetDlgItemText(iID, ite->second);
        }
    }

    //set translated text on the dialog
    mapitem_t::iterator ite = m_mItems.find(nResourceID);
    if(ite != m_mItems.end())
        dialog.SetWindowText(ite->second);
    //else
    //    TRACE(_T("Missing Item ID %d\n"), nResourceID);

    CMenu* menu = dialog.GetMenu();
    if(menu)
        Translate(*menu);
}

void Languages::Translate(CWnd& wnd, UINT nItemID, LPCTSTR lpszDefaultText/* = NULL*/)
{
    //set translated text on the window
    mapitem_t::iterator ite = m_mItems.find(nItemID);
    if(ite != m_mItems.end())
        wnd.SetWindowText(ite->second);
    else if(lpszDefaultText)
        wnd.SetWindowText(lpszDefaultText);
    //else
    //    TRACE(_T("Missing Item ID %d\n"), nItemID);
}

void Languages::Translate(CMenu& menu)
{
    int count = menu.GetMenuItemCount();
    for(int i=0;i<count;i++)
    {
        CMenu* subMenu = menu.GetSubMenu(i);
        if(subMenu)
            Translate(*subMenu);
    }

    //set translated text for every know control ID
    for(int i=0;i<count;i++)
    {
        UINT id = menu.GetMenuItemID(i);
        CString szText;
        if(GetItemText(id, szText))
            VERIFY(menu.ModifyMenu(i, MF_BYPOSITION | MF_STRING, id, szText));
    }
}

void GetLanguageFiles(CStringList& languagefiles)
{
    TCHAR szCurDir[MAX_PATH], szLngDir[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, szCurDir);
    _tcscpy(szLngDir, szCurDir);
    _tcsncat(szLngDir, _T("\\"), MAX_PATH);
    _tcsncat(szLngDir, LANGUAGE_DIRECTORY, MAX_PATH);
    
    if(SetCurrentDirectory(szLngDir))
    {
        WIN32_FIND_DATA FindFileData;
        HANDLE hFind;
        hFind = FindFirstFile(_T("*.lng"), &FindFileData);
        while(INVALID_HANDLE_VALUE != hFind)
        {
            CString szLang = FindFileData.cFileName;
            languagefiles.AddTail(szLang);
            if(!FindNextFile(hFind, &FindFileData))
                break;
        }
        FindClose(hFind);
    }
    SetCurrentDirectory(szCurDir);
}

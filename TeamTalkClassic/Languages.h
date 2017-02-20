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

#pragma once

#include "settings/XMLLanguages.h"

#include <map>

class Languages
{
private:
    Languages();

public:
    static Languages* Instance(LPCTSTR szLangFile = 0);
    ~Languages();

    BOOL LoadLanguage(CString szLangFile);

    CString GetCurLanguage();

    void ClearLanguage();

    //translate an entire dialog
    void Translate(CDialog& dialog, UINT nResourceID = 0);
    //translate a CWnd if the item ID exists
    void Translate(CWnd& wnd, UINT nItemID, LPCTSTR lpszDefaultText = NULL);
    //translate a CMenu
    void Translate(CMenu& menu);

    CString GetItemText(UINT nItemID);
    BOOL GetItemText(UINT nItemID, CString& szText);

private:
    typedef std::map<UINT, CString> mapitem_t;
    mapitem_t m_mItems;
    XMLLanguages m_language;
    CString m_szLanguage;
    static Languages* m_pInstance;
};

#define TRANSLATE Languages::Instance()->Translate
#define TRANSLATE_ITEM Languages::Instance()->GetItemText

void GetLanguageFiles(CStringList& languagefiles);

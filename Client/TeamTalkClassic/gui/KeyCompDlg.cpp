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

// gui\KeyCompDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "KeyCompDlg.h"
#include "TeamTalkDlg.h"

extern TTInstance* ttInst;

// CKeyCompDlg dialog

IMPLEMENT_DYNAMIC(CKeyCompDlg, CDialog)
CKeyCompDlg::CKeyCompDlg(CWnd* pParent /*=NULL*/)
: CDialog(CKeyCompDlg::IDD, pParent)
{
}

CKeyCompDlg::~CKeyCompDlg()
{
}

void CKeyCompDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_EDIT_KEYCOMP, m_KeyEdit);
}


BEGIN_MESSAGE_MAP(CKeyCompDlg, CDialog)
    ON_MESSAGE(WM_TEAMTALK_HOTKEYEVENT, OnKeyStroke)
END_MESSAGE_MAP()


// CKeyCompDlg message handlers

BOOL CKeyCompDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    TRANSLATE(*this, IDD);

    ShowKeys();

    if(!TT_HotKey_InstallTestHook(ttInst, GetSafeHwnd(), WM_TEAMTALK_HOTKEYEVENT))
        AfxMessageBox(LoadText(IDS_KEYCOMPERROR, _T("Error setting keyboard hook. Try restarting the program.")));

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}


LRESULT CKeyCompDlg::OnKeyStroke(WPARAM wParam, LPARAM lParam)
{
    if(lParam)
    {
        m_setDownKeys.insert(UINT(wParam));
        m_Hotkey.push_back(UINT(wParam));
        ShowKeys();

    }
    else
    {
        m_setUpKeys.insert(UINT(wParam));
        if(m_setDownKeys.size() == m_setUpKeys.size() && m_setUpKeys.size() != 0)
            PostMessage(WM_COMMAND, IDOK);
    }

    return TRUE;
}

void CKeyCompDlg::ShowKeys()
{

    CString s = GetHotkeyString(m_Hotkey);


    //if(m_Hotkey.bWin)
    //    s += "Win";    

    //if(m_Hotkey.bCtrl)
    //    s += (s.IsEmpty())? "Ctrl" : " + Ctrl";

    //if(m_Hotkey.bAlt)
    //    s += (s.IsEmpty())? "Alt" : " + Alt";

    //if(m_Hotkey.bShift)
    //    s += (s.IsEmpty())? "Shift" : " + Shift";

    //if(m_Hotkey.nVK)
    //{
    //    CString keystr = GetVirtualKeyString(m_Hotkey.nVK);
    //    if(s.IsEmpty()) s = keystr;
    //    else
    //    {
    //        s += " + ";
    //        s += keystr;
    //    }
    //}

    m_KeyEdit.SetWindowText(s);
}

CString CKeyCompDlg::GetHotkeyString(teamtalk::HotKey hotkey)
{
    CString s;
    if(hotkey.empty())
        return s;

    TTCHAR key[TT_STRLEN] = {};
    size_t i;
    for(i=0;i<hotkey.size()-1;i++)
    {
        TT_HotKey_GetKeyString(ttInst, hotkey[i], key);
        s += CString(key) + _T(" + ");
    }

    TT_HotKey_GetKeyString(ttInst, hotkey[i], key);

    s += key;

    return s;
}


void CKeyCompDlg::OnOK()
{
    TT_HotKey_RemoveTestHook(ttInst);

    CDialog::OnOK();
}

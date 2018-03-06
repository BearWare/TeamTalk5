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

// gui\Win32SessionTreeCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "SessionTreeCtrlWin32.h"


// CSessionTreeCtrlWin32

IMPLEMENT_DYNAMIC(CSessionTreeCtrlWin32, CSessionTreeCtrl)

CSessionTreeCtrlWin32::CSessionTreeCtrlWin32() :
m_hTempSelection(0)
{
}

CSessionTreeCtrlWin32::~CSessionTreeCtrlWin32()
{
}


BEGIN_MESSAGE_MAP(CSessionTreeCtrlWin32, CSessionTreeCtrl)
    ON_WM_CONTEXTMENU()
    ON_NOTIFY_REFLECT(NM_RCLICK, OnRclick)
//    ON_WM_CHAR()
END_MESSAGE_MAP()



// CSessionTreeCtrlWin32 message handlers

void CSessionTreeCtrlWin32::OnRclick(NMHDR* pNMHDR, LRESULT* pResult) 
{
    CSessionTreeCtrlWin32 & tc = *this;

    CPoint pt1,pt2;
    GetCursorPos(&pt1);
    GetCursorPos(&pt2);
    /* Convert to screen co-ords for hittesting */
    tc.ScreenToClient( &pt1 );

    UINT test;
    HTREEITEM hti = tc.HitTest( pt1, &test );

    // Did the click occur on an item
    if ( ( hti != NULL ) && ( test & TVHT_ONITEM ) )
    {
        HTREEITEM htCur = tc.GetSelectedItem();
        // Store the value of the right clicked item only *if*
        // it's different from the currently selected item.
        //
        if ( hti != htCur || m_hTempSelection == 0)
        {
            Select(hti,TVGN_CARET);
            m_hTempSelection = hti;
        }
    }
    else
        m_hTempSelection = NULL;

    // Do the context menu
    OnContextMenu( this, pt2 );

    *pResult = 0;
}

void CSessionTreeCtrlWin32::OnContextMenu(CWnd* pWnd, CPoint point) 
{
    CSessionTreeCtrl & tc = *this;

    CMenu* menu = AfxGetMainWnd()->GetMenu();

    CMenu * pop;
  if( GetSelectedUser()>0 )
    pop = menu->GetSubMenu(2);//USER MENU
  else
    pop = menu->GetSubMenu(3);//CHANNELS MENU
    
    /* If activated by the keyboard, get
     * the position of the selected item
     */
    if ( point.x == -1 )
    {
        HTREEITEM ht = tc.GetSelectedItem();
        RECT rect;
        tc.GetItemRect( ht, &rect, true );
        tc.ClientToScreen( &rect );
        /* Offset the popup menu origin so
         * we can read some of the text
         */
        point.x = rect.left + 15;
        point.y = rect.top + 8;
    }

    /* To ensure that commands and menu update handling is done
     * properly, make the parent of the menu the main frame window
     */
    CWnd * pMenuParent = AfxGetMainWnd();
    UINT uCmd = pop->TrackPopupMenu(    TPM_RETURNCMD | TPM_LEFTALIGN |
                                        TPM_RIGHTBUTTON, point.x, point.y,
                                        pMenuParent, NULL );
    /* Menu item chosen ? */
    if ( uCmd != 0 )
    {
        /* Temporarily select any right-clicked item that's
         * different from the currently selected item
         */
        if ( m_hTempSelection )
        {
            HTREEITEM hOldSel = tc.GetSelectedItem();
            tc.Select( m_hTempSelection, TVGN_CARET );
            /* Execute the selected menu command */
            pMenuParent->SendMessage( WM_COMMAND, uCmd, 0 );
            tc.Select( hOldSel, TVGN_CARET );
        }
        else
        {
            /* Execute the selected menu command */
            pMenuParent->SendMessage( WM_COMMAND, uCmd, 0 );
        }
    }

    /* No longer need the temporary value */
    m_hTempSelection = NULL;
    //popupmenu.DestroyMenu();    
}

//void CSessionTreeCtrlWin32::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
//{
//    //CSessionTreeCtrl::OnChar(nChar, nRepCnt, nFlags);
//}

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

// RoomBox.cpp : implementation file
//

#include "stdafx.h"
#include "RoomBox.h"

// CRoomBox

IMPLEMENT_DYNAMIC(CRoomBox, CStatic)
CRoomBox::CRoomBox()
{
}

CRoomBox::~CRoomBox()
{
}


BEGIN_MESSAGE_MAP(CRoomBox, CStatic)
    ON_WM_PAINT()
END_MESSAGE_MAP()



// CRoomBox message handlers


void CRoomBox::OnPaint()
{
    CPaintDC dc(this); // device context for painting
    // Do not call CStatic::OnPaint() for painting messages

    CRect r;
    GetClientRect(&r);
     BOOL m_bMemDC = FALSE;
       CDC* pDC = &dc;

    //erase background
       pDC->FillSolidRect(&r, GetSysColor(COLOR_BTNFACE));
    
    CPen penDark(PS_SOLID, 1, GetSysColor(COLOR_3DDKSHADOW));
    pDC->SelectObject(&penDark);

    //paint +
    pDC->MoveTo(0,r.Height()/2);    //x-akse
    pDC->LineTo(r.Width(), r.Height()/2);

    pDC->MoveTo(r.Width()/2,0); //y-akse
    pDC->LineTo(r.Width()/2, r.Height());
}

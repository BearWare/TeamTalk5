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

#include "stdafx.h"
#include "MySliderCtrl.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

// CVertSliderCtrl

IMPLEMENT_DYNAMIC(CMySliderCtrl, CSliderCtrl)
CMySliderCtrl::CMySliderCtrl()
{
}

CMySliderCtrl::~CMySliderCtrl()
{
}

void CMySliderCtrl::SetPos(int nPos)
{
    if ((GetStyle() & TBS_VERT) == TBS_VERT)
        CSliderCtrl::SetPos(GetRangeMax()-nPos);
    else
        CSliderCtrl::SetPos(nPos);    
}

int CMySliderCtrl::GetPos( ) const
{
    if ((GetStyle() & TBS_VERT) == TBS_VERT)
        return GetRangeMax() - CSliderCtrl::GetPos();
    else
        return CSliderCtrl::GetPos();
}

BEGIN_MESSAGE_MAP(CMySliderCtrl, CSliderCtrl)
END_MESSAGE_MAP()



// CVertSliderCtrl message handlers


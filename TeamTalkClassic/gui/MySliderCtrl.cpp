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


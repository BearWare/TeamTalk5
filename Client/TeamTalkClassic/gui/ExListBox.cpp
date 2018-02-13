// ExListBox.cpp : implementation file
//

#include "stdafx.h"

#include "ExListBox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CExListBox

CExListBox::CExListBox()
{
}

CExListBox::~CExListBox()
{
}


BEGIN_MESSAGE_MAP(CExListBox, CListBox)
	//{{AFX_MSG_MAP(CExListBox)
		// NOTE - the ClassWizard will add and remove mapping macros here.

	//}}AFX_MSG_MAP
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTW, 0, 0xFFFF, OnToolTipText)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, OnToolTipText)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CExListBox message handlers

void CExListBox::PreSubclassWindow() 
{
	CListBox::PreSubclassWindow();
	EnableToolTips(TRUE);

}

INT_PTR CExListBox::OnToolHitTest(CPoint point, TOOLINFO * pTI) const
{
	int row;
	RECT cellrect;   // cellrect		- to hold the bounding rect
	BOOL tmp = FALSE;
	row  = ItemFromPoint(point,tmp);  //we call the ItemFromPoint function to determine the row,
	//note that in NT this function may fail  use the ItemFromPointNT member function

	if ( row == -1 ) 
		return -1;

	//set up the TOOLINFO structure. GetItemRect(row,&cellrect);
	GetItemRect(row,&cellrect);
	pTI->rect = cellrect;
	pTI->hwnd = m_hWnd;
	pTI->uId = (UINT)((row));   //The ‘uId’ is assigned a value according to the row value.
	pTI->lpszText = LPSTR_TEXTCALLBACK;
	return pTI->uId;

}


//Define OnToolTipText(). This is the handler for the TTN_NEEDTEXT notification from 
//support ansi and unicode 
BOOL CExListBox::OnToolTipText( UINT id, NMHDR * pNMHDR, LRESULT * pResult )
{
	// need to handle both ANSI and UNICODE versions of the message
	TOOLTIPTEXTA* pTTTA = (TOOLTIPTEXTA*)pNMHDR;
	TOOLTIPTEXTW* pTTTW = (TOOLTIPTEXTW*)pNMHDR;
	CString strTipText;
	UINT_PTR nID = pNMHDR->idFrom;

	
	GetText(int(nID) ,strTipText);

#ifndef _UNICODE
	if (pNMHDR->code == TTN_NEEDTEXTA)
		lstrcpyn(pTTTA->szText, strTipText, 80);
	else
		_mbstowcsz(pTTTW->szText, strTipText, 80);
#else
	if (pNMHDR->code == TTN_NEEDTEXTA)
		_wcstombsz(pTTTA->szText, strTipText, 80);
	else
		lstrcpyn(pTTTW->szText, strTipText, 80);
#endif
	*pResult = 0;

	return TRUE;    
}




UINT CExListBox::ItemFromPoint2(CPoint pt, BOOL& bOutside) const

// CListBox::ItemFromPoint does not work on NT.

{
	int nFirstIndex, nLastIndex;
	//GetFirstAndLastIndex(nFirstIndex, nLastIndex);
	nFirstIndex = GetTopIndex();
	nLastIndex = nFirstIndex  + GetCount(); 


	
	bOutside = TRUE;
	
	CRect Rect;
	int nResult = -1;
	
	for (int i = nFirstIndex; nResult == -1 && i <= nLastIndex; i++)
	{
		if (GetItemRect(i, &Rect) != LB_ERR)
		{
			if (Rect.PtInRect(pt))
			{
				nResult  = i;
				bOutside = FALSE;
			}
		}
		
	}
	
	return nResult;
}


/*
 * Copyright (c) 2005-2014, BearWare.dk
 * 
 * Contact Information:
 *
 * Bjoern D. Rasmussen
 * Skanderborgvej 40 4-2
 * DK-8000 Aarhus C
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


#include "stdafx.h"
#include "MyPropertySheet.h"
#include ".\mypropertysheet.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// CMyPropertySheet

IMPLEMENT_DYNAMIC(CMyPropertySheet, CPropertySheet)
CMyPropertySheet::CMyPropertySheet(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
    :CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
{
}

CMyPropertySheet::CMyPropertySheet(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
    :CPropertySheet(pszCaption, pParentWnd, iSelectPage)
{
}

CMyPropertySheet::~CMyPropertySheet()
{
}


BEGIN_MESSAGE_MAP(CMyPropertySheet, CPropertySheet)
END_MESSAGE_MAP()


// CMyPropertySheet message handlers

BOOL CMyPropertySheet::OnInitDialog()
{

    BOOL bResult = CPropertySheet::OnInitDialog();

    CWnd* pWnd = GetDlgItem(IDHELP);
    if(pWnd)
        pWnd->ShowWindow(SW_HIDE);


    return bResult;
}


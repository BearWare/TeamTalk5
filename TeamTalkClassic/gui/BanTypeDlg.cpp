// gui\BanTypeDlg.cpp : implementation file
//

#include "stdafx.h"
#include "resource.h"
#include "BanTypeDlg.h"
#include "afxdialogex.h"


// CBanTypeDlg dialog

IMPLEMENT_DYNAMIC(CBanTypeDlg, CDialogEx)

CBanTypeDlg::CBanTypeDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG_BANTYPE, pParent)
{
#ifndef _WIN32_WCE
	EnableActiveAccessibility();
#endif
}

CBanTypeDlg::~CBanTypeDlg()
{
}

void CBanTypeDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_COMBO_BANTYPE, m_wndBanType);
}


BEGIN_MESSAGE_MAP(CBanTypeDlg, CDialogEx)
END_MESSAGE_MAP()


// CBanTypeDlg message handlers


BOOL CBanTypeDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    TRANSLATE(*this, IDD);

    CString szText = _T("Ban IP-address");
    TRANSLATE_ITEM(IDC_STATIC_IPADDR, szText);
    int i = m_wndBanType.AddString(szText);
    m_wndBanType.SetItemData(i, DWORD_PTR(BANTYPE_IPADDR));
    szText = _T("Ban username");
    TRANSLATE_ITEM(IDC_STATIC_USERNAME, szText);
    i = m_wndBanType.AddString(szText); 
    m_wndBanType.SetItemData(i, DWORD_PTR(BANTYPE_USERNAME));
    m_wndBanType.SetCurSel(0);
    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}


void CBanTypeDlg::OnOK()
{
    CDialogEx::OnOK();

    m_uBanTypes = m_wndBanType.GetItemData(m_wndBanType.GetCurSel());
}

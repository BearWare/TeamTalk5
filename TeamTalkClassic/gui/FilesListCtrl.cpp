// gui\FilesListCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "FilesListCtrl.h"
#include ".\fileslistctrl.h"
#include <afxole.h>

// CFilesListCtrl

IMPLEMENT_DYNAMIC(CFilesListCtrl, CListCtrl)

CFilesListCtrl::CFilesListCtrl()
{
}

CFilesListCtrl::~CFilesListCtrl()
{
}


BEGIN_MESSAGE_MAP(CFilesListCtrl, CListCtrl)
    ON_WM_DROPFILES()
    ON_NOTIFY(HDN_DIVIDERDBLCLICKA, 0, OnHdnDividerdblclick)
    ON_NOTIFY(HDN_DIVIDERDBLCLICKW, 0, OnHdnDividerdblclick)
    ON_NOTIFY_REFLECT(LVN_BEGINDRAG, OnLvnBegindrag)
END_MESSAGE_MAP()



// CFilesListCtrl message handlers


void CFilesListCtrl::OnDropFiles(HDROP hDropInfo)
{
    TCHAR szFilePath[MAX_PATH] = {0};
    UINT count = DragQueryFile(hDropInfo, 0xFFFFFFFF, szFilePath, MAX_PATH);
    for(int i=0;i<count;i++)
    {
        DragQueryFile(hDropInfo, i, szFilePath, MAX_PATH);
        m_Files.AddTail(szFilePath);
    }

    CWnd *pMenuParent = AfxGetMainWnd();
    if(pMenuParent)
        pMenuParent->SendMessage(WM_FILESLISTCTRL_FILESDROPPED);

    m_Files.RemoveAll();

    CListCtrl::OnDropFiles(hDropInfo);
}

void CFilesListCtrl::OnHdnDividerdblclick(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMHEADER phdr = reinterpret_cast<LPNMHEADER>(pNMHDR);
    *pResult = 0;
    //int maxWidth = 0;
    //for(int i=0;i<GetItemCount();i++)
    //{
    //    CRect rect;
    //    GetItemRect(i, rect, LVIR_LABEL);
    //    maxWidth = max(rect.Width(), maxWidth);
    //}
    CHeaderCtrl* ctrl = GetHeaderCtrl();
    if(ctrl)
    {
        ::SendMessage( m_hWnd, // handle to destination control 
            (UINT) LVM_SETCOLUMNWIDTH, // message ID 
            (WPARAM) phdr->iItem, // = (WPARAM) (int) iCol 
            (LPARAM) LVSCW_AUTOSIZE_USEHEADER  // = (LPARAM) MAKELPARAM ((int) cx, 0) 
            );
    }
}

void CFilesListCtrl::OnLvnBegindrag(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
    //CString szFileName = GetItemText(pNMLV->iItem,0);
    //HANDLE hData = ::GlobalAlloc (GMEM_MOVEABLE | GMEM_SHARE, (1 + szFileName.GetLength()) * sizeof(TCHAR));
    //LPTSTR pString = (LPTSTR)::GlobalLock (hData);
    //_tcscpy(pString,szFileName.GetBuffer());

    //COleDataSource ods;
    //ods.CacheGlobalData (CF_HDROP, hData);
    //ods.DoDragDrop (DROPEFFECT_COPY);

    //::GlobalUnlock (hData);

    *pResult = 0;
}

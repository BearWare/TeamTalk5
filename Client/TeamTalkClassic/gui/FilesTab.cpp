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

// FilesTab.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "FilesTab.h"
#include ".\filestab.h"

extern TTInstance* ttInst;

// CFilesTab dialog

IMPLEMENT_DYNAMIC(CFilesTab, CMyTab)
CFilesTab::CFilesTab(CWnd* pParent /*=NULL*/)
    : CMyTab(CFilesTab::IDD, pParent)
    , m_nChannelID(-1)
    , m_hAccel(0)
{
}

CFilesTab::~CFilesTab()
{
}

void CFilesTab::DoDataExchange(CDataExchange* pDX)
{
    CMyTab::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST_FILES, m_wndFiles);
}

void CFilesTab::ResizeHeader()
{
    CRect rect;
    if(!IsWindow(m_wndFiles.m_hWnd))
        return;
    m_wndFiles.GetClientRect(rect);

    static bool first = true;
    if(first)
    {
        int sizeWidth = rect.Width() / 3;
        m_wndFiles.SetColumnWidth(0, rect.Width()-sizeWidth);
        m_wndFiles.SetColumnWidth(1, sizeWidth);
        //m_wndFiles.SetColumnWidth(2, sizeWidth);
        first = false;
    }
    else
    {
        int sizeWidth = m_wndFiles.GetColumnWidth(1);
        m_wndFiles.SetColumnWidth(0, rect.Width()-sizeWidth);
        m_wndFiles.SetColumnWidth(1, sizeWidth);
        m_wndFiles.SetColumnWidth(2, sizeWidth);
    }
}

void CFilesTab::AddFile(int nChannelID, int nFileID)
{
    //don't update the view if it's not the current channel
    if(nChannelID != m_nChannelID)
        return;

    RemoteFile remotefile = {};
    int count = m_wndFiles.GetItemCount();
    if( TT_GetChannelFile(ttInst, nChannelID, nFileID, &remotefile) &&
        GetIndexFromFileID(nFileID) == -1)
    {
        int itemIndex = m_wndFiles.InsertItem(count, remotefile.szFileName, 0);
        INT64 size = remotefile.nFileSize / 1024;
        if(remotefile.nFileSize > 0 && remotefile.nFileSize < 1024)
            size = 1;
        CString num;
        num.Format(LoadText(IDS_FILETABKB, _T("%u KB")), UINT(size));
        m_wndFiles.SetItem(itemIndex, 1, LVIF_TEXT, num, 0, 0, 0, 0, 0);
        m_wndFiles.SetItem(itemIndex, 2, LVIF_TEXT, remotefile.szUsername, 0, 0, 0, 0, 0);
        m_wndFiles.SetItemData(itemIndex, nFileID);
    }
}

void CFilesTab::RemoveFile(int nChannelID, int nFileID)
{
    //don't update the view if it's not the current channel
    if(nChannelID != m_nChannelID)
        return;

    int index = GetIndexFromFileID(nFileID);
    if(index >= 0)
        m_wndFiles.DeleteItem(index);

}

int CFilesTab::GetIndexFromFileID(int nFileID)
{
    int count = m_wndFiles.GetItemCount();
    for(int i=0;i<count;i++)
    {
        if(m_wndFiles.GetItemData(i) == nFileID)
            return i;
    }
    return -1;
}

void CFilesTab::UpdateFiles(int nChannelID)
{
    m_nChannelID = nChannelID;
    m_wndFiles.DeleteAllItems();
    int nCount = 0;
    TT_GetChannelFiles(ttInst, nChannelID, NULL, &nCount);
    if(nCount)
    {
        std::vector<RemoteFile> files(nCount);
        TT_GetChannelFiles(ttInst, nChannelID, &files[0], &nCount);

        for(int i=0;i<nCount;i++)
            AddFile(nChannelID, files[i].nFileID);
    }
}

std::vector<int> CFilesTab::GetSelectedFiles()
{
    std::vector<int> fileids;
    int count = m_wndFiles.GetItemCount();
    for(int i=0;i<count;i++)
    {
        if(m_wndFiles.GetItemState(i, LVIS_SELECTED) == LVIS_SELECTED)
            fileids.push_back(INT32(m_wndFiles.GetItemData(i)));
    }
    return fileids;
}


BEGIN_MESSAGE_MAP(CFilesTab, CMyTab)
    ON_WM_SIZE()
    ON_NOTIFY(NM_RCLICK, IDC_LIST_FILES, OnNMRclickListFiles)
    ON_NOTIFY(LVN_BEGINDRAG, IDC_LIST_FILES, OnLvnBegindragListFiles)
END_MESSAGE_MAP()


// CFilesTab message handlers

BOOL CFilesTab::OnInitDialog()
{
    CMyTab::OnInitDialog();

    static CResizer::CBorderInfo s_bi[] = {

        {IDC_LIST_FILES,    
        {CResizer::eFixed, IDC_MAIN, CResizer::eLeft}, 
        {CResizer::eFixed, IDC_MAIN, CResizer::eTop},  
        {CResizer::eFixed, IDC_MAIN, CResizer::eRight},
        {CResizer::eFixed, IDC_MAIN, CResizer::eBottom}}, 
    };
    const int nSize = sizeof(s_bi)/sizeof(s_bi[0]);
    m_resizer.Init(m_hWnd, NULL, s_bi, nSize);

    m_wndFiles.InsertColumn(0, LoadText(IDS_FILETABNAME, _T("Name")));
    m_wndFiles.InsertColumn(1, LoadText(IDS_FILETABSIZE, _T("Size")), LVCFMT_RIGHT);
    m_wndFiles.InsertColumn(2, LoadText(IDS_FILETABUSER, _T("User")), LVCFMT_RIGHT);

    TRANSLATE(*this, IDD);
    SetAccessibleName(m_wndFiles, LoadText(IDS_FILESLISTLAB, _T("Files list")));
    //ResizeHeader();
    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

void CFilesTab::OnSize(UINT nType, int cx, int cy)
{
    CMyTab::OnSize(nType, cx, cy);
    
    m_resizer.Move();
    ResizeHeader();
}

void CFilesTab::OnNMRclickListFiles(NMHDR *pNMHDR, LRESULT *pResult)
{
    *pResult = 0;
    CMenu* menu = AfxGetMainWnd()->GetMenu();
    ASSERT(menu);
    CMenu * pop = menu->GetSubMenu(3);
    ASSERT(pop);

    CWnd * pMenuParent = AfxGetMainWnd();
    CPoint point;
    ::GetCursorPos(&point);
    UINT uCmd = pop->TrackPopupMenu(    TPM_RETURNCMD | TPM_LEFTALIGN |
                                        TPM_RIGHTBUTTON, point.x, point.y,
                                        pMenuParent, NULL );
    if ( uCmd != 0 )
            pMenuParent->SendMessage( WM_COMMAND, uCmd, 0 );
}

void CFilesTab::OnLvnBegindragListFiles(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
    *pResult = 0;
}

BOOL CFilesTab::PreTranslateMessage(MSG* pMsg)
{
    if (WM_KEYFIRST <= pMsg->message && pMsg->message <= WM_KEYLAST)
        if (m_hAccel && ::TranslateAccelerator(AfxGetMainWnd()->m_hWnd, m_hAccel, pMsg))
            return TRUE;

    return CMyTab::PreTranslateMessage(pMsg);
}

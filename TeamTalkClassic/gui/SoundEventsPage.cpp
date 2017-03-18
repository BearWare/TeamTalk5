/*
 * Copyright (c) 2005-2017, BearWare.dk
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
 * This source code is part of the TeamTalk 5 SDK owned by
 * BearWare.dk. All copyright statements may not be removed 
 * or altered from any source distribution. If you use this
 * software in a product, an acknowledgment in the product 
 * documentation is required.
 *
 */

#include "stdafx.h"
#include "Resource.h"
#include "SoundEventsPage.h"

#if defined(ENABLE_TOLK)
#include <Tolk.h>
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// CSoundEventsPage dialog

IMPLEMENT_DYNAMIC(CSoundEventsPage, CPropertyPage)
CSoundEventsPage::CSoundEventsPage()
    : CPropertyPage(CSoundEventsPage::IDD)
  , m_uSoundEvents(SOUNDEVENT_NONE)
{
}

CSoundEventsPage::~CSoundEventsPage()
{
}

void CSoundEventsPage::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_TREE_SOUNDEVENTS, m_wndTree);
}

BOOL CSoundEventsPage::OnInitDialog()
{
    CPropertyPage::OnInitDialog();

    TRANSLATE(*this, IDD);

    // clear out checks of filenames that are not set
    std::map<SoundEvent, CString>::iterator i;
    for(i=m_SoundFiles.begin();i!=m_SoundFiles.end();++i)
    {
        if(i->second.IsEmpty())
            m_uSoundEvents &= ~i->first;
    }

    SetTimer(1, 0, NULL);

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

CString CSoundEventsPage::GetEventFilePath(const CString& szInitFile)
{
    CString filetypes = _T("Wave files (*.wav)|*.wav|All files (*.*)|*.*|");
    CFileDialog dlg(TRUE, 0, szInitFile.IsEmpty()?0:szInitFile, OFN_FILEMUSTEXIST| OFN_HIDEREADONLY, filetypes, this);
    TCHAR s[MAX_PATH];
    CString szFilePath;
    GetCurrentDirectory(MAX_PATH, s);
    if(dlg.DoModal() == IDOK)
        szFilePath = dlg.GetPathName();
    SetCurrentDirectory(s);

    return szFilePath;
}

void CSoundEventsPage::ToggleItem(HTREEITEM hItem)
{
    SoundEvent e = SoundEvent(m_wndTree.GetItemData(hItem));
    if(m_wndTree.GetCheck(hItem))
        m_uSoundEvents &= ~e;
    else
    {
        CString szFilePath = GetEventFilePath(m_SoundFiles[e]);
        if(!szFilePath.IsEmpty())
        {
            m_uSoundEvents |= e;
            m_SoundFiles[e] = szFilePath;
        }
        else
            m_wndTree.SetCheck(hItem, FALSE);
    }

    SetTimer(1, 0, NULL);
}

BEGIN_MESSAGE_MAP(CSoundEventsPage, CPropertyPage)
    ON_NOTIFY(TVN_KEYDOWN, IDC_TREE_SOUNDEVENTS, &CSoundEventsPage::OnTvnKeydownTreeSoundevents)
    ON_NOTIFY(NM_CLICK, IDC_TREE_SOUNDEVENTS, &CSoundEventsPage::OnNMClickTreeSoundevents)
    ON_WM_TIMER()
END_MESSAGE_MAP()


// CSoundEventsPage message handlers

void CSoundEventsPage::OnNMClickTreeSoundevents(NMHDR *pNMHDR, LRESULT *pResult)
{
    // TODO: Add your control notification handler code here
    *pResult = 0;
    UINT uFlags = 0;
    CPoint pt(0, 0);
    GetCursorPos(&pt);
    m_wndTree.ScreenToClient(&pt);
    HTREEITEM hItem = m_wndTree.HitTest(pt, &uFlags);
    if(NULL != hItem && (TVHT_ONITEMSTATEICON  & uFlags))
    {
        m_wndTree.SelectItem(hItem);
        ToggleItem(hItem);
    }
    *pResult = 0;
}

void CSoundEventsPage::OnTvnKeydownTreeSoundevents(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMTVKEYDOWN pTVKeyDown = reinterpret_cast<LPNMTVKEYDOWN>(pNMHDR);
    
    HTREEITEM hItem = m_wndTree.GetSelectedItem();
    if(pTVKeyDown->wVKey == VK_SPACE && hItem)
        ToggleItem(hItem);
    *pResult = 0;
}


void CSoundEventsPage::OnTimer(UINT_PTR nIDEvent)
{
    CPropertyPage::OnTimer(nIDEvent);
    KillTimer(nIDEvent);

    m_wndTree.DeleteAllItems();

    CString szText;
    szText.LoadString(IDS_USER_JOIN);
    HTREEITEM hItem = m_wndTree.InsertItem(szText);
    m_wndTree.SetItemData(hItem, SOUNDEVENT_USER_JOIN);
    m_wndTree.SetCheck(hItem, (m_uSoundEvents & SOUNDEVENT_USER_JOIN) != 0);
    
    szText.LoadString(IDS_USER_LEFT);
    hItem = m_wndTree.InsertItem(szText);
    m_wndTree.SetItemData(hItem, SOUNDEVENT_USER_LEFT);
    m_wndTree.SetCheck(hItem, (m_uSoundEvents & SOUNDEVENT_USER_LEFT) != 0);

    szText.LoadString(IDS_USER_TEXTMSG);
    hItem = m_wndTree.InsertItem(szText);
    m_wndTree.SetItemData(hItem, SOUNDEVENT_USER_TEXTMSG);
    m_wndTree.SetCheck(hItem, (m_uSoundEvents & SOUNDEVENT_USER_TEXTMSG) != 0);

    szText.LoadString(IDS_USER_CHANMSG);
    hItem = m_wndTree.InsertItem(szText);
    m_wndTree.SetItemData(hItem, SOUNDEVENT_USER_CHANNEL_TEXTMSG);
    m_wndTree.SetCheck(hItem, (m_uSoundEvents & SOUNDEVENT_USER_CHANNEL_TEXTMSG) != 0);

    szText.LoadString(IDS_USER_QUESTIONMODE);
    hItem = m_wndTree.InsertItem(szText);
    m_wndTree.SetItemData(hItem, SOUNDEVENT_USER_QUESTIONMODE);
    m_wndTree.SetCheck(hItem, (m_uSoundEvents & SOUNDEVENT_USER_QUESTIONMODE) != 0);

    szText.LoadString(IDS_USER_DESKACCESS_REQ);
    hItem = m_wndTree.InsertItem(szText);
    m_wndTree.SetItemData(hItem, SOUNDEVENT_USER_DESKTOP_ACCESS);
    m_wndTree.SetCheck(hItem, (m_uSoundEvents & SOUNDEVENT_USER_DESKTOP_ACCESS) != 0);

    szText.LoadString(IDS_USER_NEWVIDEOSESSION);
    hItem = m_wndTree.InsertItem(szText);
    m_wndTree.SetItemData(hItem, SOUNDEVENT_USER_VIDEOSESSION_NEW);
    m_wndTree.SetCheck(hItem, (m_uSoundEvents & SOUNDEVENT_USER_VIDEOSESSION_NEW) != 0);

    szText.LoadString(IDS_USER_NEWDESKTOPSESSION);
    hItem = m_wndTree.InsertItem(szText);
    m_wndTree.SetItemData(hItem, SOUNDEVENT_USER_DESKTOPSESSION_NEW);
    m_wndTree.SetCheck(hItem, (m_uSoundEvents & SOUNDEVENT_USER_DESKTOPSESSION_NEW) != 0);

    szText.LoadString(IDS_CONNECTION_SERVERLOST);
    hItem = m_wndTree.InsertItem(szText);
    m_wndTree.SetItemData(hItem, SOUNDEVENT_CONNECTION_LOST);
    m_wndTree.SetCheck(hItem, (m_uSoundEvents & SOUNDEVENT_CONNECTION_LOST) != 0);

    szText.LoadString(IDS_HOTKEY);
    hItem = m_wndTree.InsertItem(szText);
    m_wndTree.SetItemData(hItem, SOUNDEVENT_PUSHTOTALK);
    m_wndTree.SetCheck(hItem, (m_uSoundEvents & SOUNDEVENT_PUSHTOTALK) != 0);

    szText.LoadString(IDS_VOICEACTIVATED);
    hItem = m_wndTree.InsertItem(szText);
    m_wndTree.SetItemData(hItem, SOUNDEVENT_VOICEACTIVATED);
    m_wndTree.SetCheck(hItem, (m_uSoundEvents & SOUNDEVENT_VOICEACTIVATED) != 0);

    szText.LoadString(IDS_VOICEDEACTIVATED);
    hItem = m_wndTree.InsertItem(szText);
    m_wndTree.SetItemData(hItem, SOUNDEVENT_VOICEDEACTIVATED);
    m_wndTree.SetCheck(hItem, (m_uSoundEvents & SOUNDEVENT_VOICEDEACTIVATED) != 0);

    szText.LoadString(IDS_FILES_UPDATED);
    hItem = m_wndTree.InsertItem(szText);
    m_wndTree.SetItemData(hItem, SOUNDEVENT_FILES_UPDATED);
    m_wndTree.SetCheck(hItem, (m_uSoundEvents & SOUNDEVENT_FILES_UPDATED) != 0);

    szText.LoadString(IDS_TRANSFER_COMPLETE);
    hItem = m_wndTree.InsertItem(szText);
    m_wndTree.SetItemData(hItem, SOUNDEVENT_FILETX_COMPLETE);
    m_wndTree.SetCheck(hItem, (m_uSoundEvents & SOUNDEVENT_FILETX_COMPLETE) != 0);

    szText.LoadString(IDS_CHANNELSILENT);
    hItem = m_wndTree.InsertItem(szText);
    m_wndTree.SetItemData(hItem, SOUNDEVENT_CHANNEL_SILENT);
    m_wndTree.SetCheck(hItem, (m_uSoundEvents & SOUNDEVENT_CHANNEL_SILENT) != 0);

}

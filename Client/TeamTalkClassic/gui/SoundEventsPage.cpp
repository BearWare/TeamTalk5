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
    CString filetypes = LoadText(IDS_SOUNDEVENTSWAVFILES, _T("Wave files (*.wav)|*.wav|All files (*.*)|*.*|"));
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

    DWORD_PTR dwItemData = SOUNDEVENT_NONE;
    if(m_wndTree.GetSelectedItem())
        dwItemData = m_wndTree.GetItemData(m_wndTree.GetSelectedItem());

    m_wndTree.DeleteAllItems();

    CString szText;
    szText.LoadString(IDS_USER_JOIN);
    TRANSLATE_ITEM(IDS_USER_JOIN, szText);
    HTREEITEM hItem = m_wndTree.InsertItem(szText);
    m_wndTree.SetItemData(hItem, SOUNDEVENT_USER_JOIN);
    m_wndTree.SetCheck(hItem, (m_uSoundEvents & SOUNDEVENT_USER_JOIN) != 0);
    
    szText.LoadString(IDS_USER_LEFT);
    TRANSLATE_ITEM(IDS_USER_LEFT, szText);
    hItem = m_wndTree.InsertItem(szText);
    m_wndTree.SetItemData(hItem, SOUNDEVENT_USER_LEFT);
    m_wndTree.SetCheck(hItem, (m_uSoundEvents & SOUNDEVENT_USER_LEFT) != 0);

    szText.LoadString(IDS_USER_TEXTMSG);
    TRANSLATE_ITEM(IDS_USER_TEXTMSG, szText);
    hItem = m_wndTree.InsertItem(szText);
    m_wndTree.SetItemData(hItem, SOUNDEVENT_USER_TEXTMSG);
    m_wndTree.SetCheck(hItem, (m_uSoundEvents & SOUNDEVENT_USER_TEXTMSG) != 0);

    szText.LoadString(IDS_USER_CHANMSG);
    TRANSLATE_ITEM(IDS_USER_CHANMSG, szText);
    hItem = m_wndTree.InsertItem(szText);
    m_wndTree.SetItemData(hItem, SOUNDEVENT_USER_CHANNEL_TEXTMSG);
    m_wndTree.SetCheck(hItem, (m_uSoundEvents & SOUNDEVENT_USER_CHANNEL_TEXTMSG) != 0);

    szText.LoadString(IDS_USER_BCASTMSG);
    TRANSLATE_ITEM(IDS_USER_BCASTMSG, szText);
    hItem = m_wndTree.InsertItem(szText);
    m_wndTree.SetItemData(hItem, SOUNDEVENT_USER_BROADCAST_TEXTMSG);
    m_wndTree.SetCheck(hItem, (m_uSoundEvents & SOUNDEVENT_USER_BROADCAST_TEXTMSG) != 0);

    szText.LoadString(IDS_USER_QUESTIONMODE);
    TRANSLATE_ITEM(IDS_USER_QUESTIONMODE, szText);
    hItem = m_wndTree.InsertItem(szText);
    m_wndTree.SetItemData(hItem, SOUNDEVENT_USER_QUESTIONMODE);
    m_wndTree.SetCheck(hItem, (m_uSoundEvents & SOUNDEVENT_USER_QUESTIONMODE) != 0);

    szText.LoadString(IDS_USER_DESKACCESS_REQ);
    TRANSLATE_ITEM(IDS_USER_DESKACCESS_REQ, szText);
    hItem = m_wndTree.InsertItem(szText);
    m_wndTree.SetItemData(hItem, SOUNDEVENT_USER_DESKTOP_ACCESS);
    m_wndTree.SetCheck(hItem, (m_uSoundEvents & SOUNDEVENT_USER_DESKTOP_ACCESS) != 0);

    szText.LoadString(IDS_USER_NEWVIDEOSESSION);
    TRANSLATE_ITEM(IDS_USER_NEWVIDEOSESSION, szText);
    hItem = m_wndTree.InsertItem(szText);
    m_wndTree.SetItemData(hItem, SOUNDEVENT_USER_VIDEOSESSION_NEW);
    m_wndTree.SetCheck(hItem, (m_uSoundEvents & SOUNDEVENT_USER_VIDEOSESSION_NEW) != 0);

    szText.LoadString(IDS_USER_NEWDESKTOPSESSION);
    TRANSLATE_ITEM(IDS_USER_NEWDESKTOPSESSION, szText);
    hItem = m_wndTree.InsertItem(szText);
    m_wndTree.SetItemData(hItem, SOUNDEVENT_USER_DESKTOPSESSION_NEW);
    m_wndTree.SetCheck(hItem, (m_uSoundEvents & SOUNDEVENT_USER_DESKTOPSESSION_NEW) != 0);

    szText.LoadString(IDS_CONNECTION_SERVERLOST);
    TRANSLATE_ITEM(IDS_CONNECTION_SERVERLOST, szText);
    hItem = m_wndTree.InsertItem(szText);
    m_wndTree.SetItemData(hItem, SOUNDEVENT_CONNECTION_LOST);
    m_wndTree.SetCheck(hItem, (m_uSoundEvents & SOUNDEVENT_CONNECTION_LOST) != 0);

    szText.LoadString(IDS_HOTKEY);
    TRANSLATE_ITEM(IDS_HOTKEY, szText);
    hItem = m_wndTree.InsertItem(szText);
    m_wndTree.SetItemData(hItem, SOUNDEVENT_PUSHTOTALK);
    m_wndTree.SetCheck(hItem, (m_uSoundEvents & SOUNDEVENT_PUSHTOTALK) != 0);

    szText.LoadString(IDS_VOICEACTIVATED);
    TRANSLATE_ITEM(IDS_VOICEACTIVATED, szText);
    hItem = m_wndTree.InsertItem(szText);
    m_wndTree.SetItemData(hItem, SOUNDEVENT_VOICEACTIVATED);
    m_wndTree.SetCheck(hItem, (m_uSoundEvents & SOUNDEVENT_VOICEACTIVATED) != 0);

    szText.LoadString(IDS_VOICEDEACTIVATED);
    TRANSLATE_ITEM(IDS_VOICEDEACTIVATED, szText);
    hItem = m_wndTree.InsertItem(szText);
    m_wndTree.SetItemData(hItem, SOUNDEVENT_VOICEDEACTIVATED);
    m_wndTree.SetCheck(hItem, (m_uSoundEvents & SOUNDEVENT_VOICEDEACTIVATED) != 0);

    szText.LoadString(IDS_ENABLEVOX);
    TRANSLATE_ITEM(IDS_ENABLEVOX, szText);
    hItem = m_wndTree.InsertItem(szText);
    m_wndTree.SetItemData(hItem, SOUNDEVENT_ENABLE_VOICEACTIVATION);
    m_wndTree.SetCheck(hItem, (m_uSoundEvents & SOUNDEVENT_ENABLE_VOICEACTIVATION) != 0);

    szText.LoadString(IDS_DISABLEVOX);
    TRANSLATE_ITEM(IDS_DISABLEVOX, szText);
    hItem = m_wndTree.InsertItem(szText);
    m_wndTree.SetItemData(hItem, SOUNDEVENT_DISABLE_VOICEACTIVATION);
    m_wndTree.SetCheck(hItem, (m_uSoundEvents & SOUNDEVENT_DISABLE_VOICEACTIVATION) != 0);

    szText.LoadString(IDS_ME_ENABLEVOX);
    TRANSLATE_ITEM(IDS_ME_ENABLEVOX, szText);
    hItem = m_wndTree.InsertItem(szText);
    m_wndTree.SetItemData(hItem, SOUNDEVENT_ME_ENABLE_VOICEACTIVATION);
    m_wndTree.SetCheck(hItem, (m_uSoundEvents & SOUNDEVENT_ME_ENABLE_VOICEACTIVATION) != 0);

    szText.LoadString(IDS_ME_DISABLEVOX);
    TRANSLATE_ITEM(IDS_ME_DISABLEVOX, szText);
    hItem = m_wndTree.InsertItem(szText);
    m_wndTree.SetItemData(hItem, SOUNDEVENT_ME_DISABLE_VOICEACTIVATION);
    m_wndTree.SetCheck(hItem, (m_uSoundEvents & SOUNDEVENT_ME_DISABLE_VOICEACTIVATION) != 0);

    szText.LoadString(IDS_FILES_UPDATED);
    TRANSLATE_ITEM(IDS_FILES_UPDATED, szText);
    hItem = m_wndTree.InsertItem(szText);
    m_wndTree.SetItemData(hItem, SOUNDEVENT_FILES_UPDATED);
    m_wndTree.SetCheck(hItem, (m_uSoundEvents & SOUNDEVENT_FILES_UPDATED) != 0);

    szText.LoadString(IDS_TRANSFER_COMPLETE);
    TRANSLATE_ITEM(IDS_TRANSFER_COMPLETE, szText);
    hItem = m_wndTree.InsertItem(szText);
    m_wndTree.SetItemData(hItem, SOUNDEVENT_FILETX_COMPLETE);
    m_wndTree.SetCheck(hItem, (m_uSoundEvents & SOUNDEVENT_FILETX_COMPLETE) != 0);

    szText.LoadString(IDS_CHANNELSILENT);
    TRANSLATE_ITEM(IDS_CHANNELSILENT, szText);
    hItem = m_wndTree.InsertItem(szText);
    m_wndTree.SetItemData(hItem, SOUNDEVENT_CHANNEL_SILENT);
    m_wndTree.SetCheck(hItem, (m_uSoundEvents & SOUNDEVENT_CHANNEL_SILENT) != 0);

    szText.LoadString(IDS_TXQUEUEHEAD);
    TRANSLATE_ITEM(IDS_TXQUEUEHEAD, szText);
    hItem = m_wndTree.InsertItem(szText);
    m_wndTree.SetItemData(hItem, SOUNDEVENT_TRANSMITQUEUE_HEAD);
    m_wndTree.SetCheck(hItem, (m_uSoundEvents & SOUNDEVENT_TRANSMITQUEUE_HEAD) != 0);

    szText.LoadString(IDS_TXQUEUESTOP);
    TRANSLATE_ITEM(IDS_TXQUEUESTOP, szText);
    hItem = m_wndTree.InsertItem(szText);
    m_wndTree.SetItemData(hItem, SOUNDEVENT_TRANSMITQUEUE_STOP);
    m_wndTree.SetCheck(hItem, (m_uSoundEvents & SOUNDEVENT_TRANSMITQUEUE_STOP) != 0);

    hItem = GetItemDataItem(m_wndTree, dwItemData);
    if(hItem)
        m_wndTree.SelectItem(hItem);
}

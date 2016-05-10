// TextToSpeechPage.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "TextToSpeechPage.h"
#include <queue>

// CTextToSpeechPage dialog

IMPLEMENT_DYNAMIC(CTextToSpeechPage, CPropertyPage)

CTextToSpeechPage::CTextToSpeechPage()
	: CPropertyPage(CTextToSpeechPage::IDD)
    , m_uTTSEvents(TTS_ALL)
{

}

CTextToSpeechPage::~CTextToSpeechPage()
{
}

void CTextToSpeechPage::DoDataExchange(CDataExchange* pDX)
{
    CPropertyPage::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_TREE_TTS, m_wndTree);
}


BEGIN_MESSAGE_MAP(CTextToSpeechPage, CPropertyPage)
    ON_NOTIFY(NM_CLICK, IDC_TREE_TTS, &CTextToSpeechPage::OnNMClickTreeTts)
    ON_NOTIFY(TVN_KEYDOWN, IDC_TREE_TTS, &CTextToSpeechPage::OnTvnKeydownTreeTts)
    ON_WM_TIMER()
END_MESSAGE_MAP()


// CTextToSpeechPage message handlers


BOOL CTextToSpeechPage::OnInitDialog()
{
    CPropertyPage::OnInitDialog();

    HTREEITEM hUser = m_wndTree.InsertItem(_T("User events"));
    m_wndTree.SetItemData(hUser, TTS_USER_ALL);
    m_wndTree.SetItemData(m_wndTree.InsertItem(_T("User logged in channel"), hUser), TTS_USER_LOGGEDIN);
    m_wndTree.SetItemData(m_wndTree.InsertItem(_T("User logged out channel"), hUser), TTS_USER_LOGGEDOUT);
    m_wndTree.SetItemData(m_wndTree.InsertItem(_T("User joined channel"), hUser), TTS_USER_JOINED);
    m_wndTree.SetItemData(m_wndTree.InsertItem(_T("User left channel"), hUser), TTS_USER_LEFT);
    m_wndTree.SetItemData(m_wndTree.InsertItem(_T("Private text messages"), hUser), TTS_USER_TEXTMSG_PRIVATE);
    m_wndTree.SetItemData(m_wndTree.InsertItem(_T("Channel text messages"), hUser), TTS_USER_TEXTMSG_CHANNEL);
    m_wndTree.SetItemData(m_wndTree.InsertItem(_T("Broadcast text messages"), hUser), TTS_USER_TEXTMSG_BROADCAST);
    m_wndTree.Expand(hUser, TVE_EXPAND);

    HTREEITEM hSubscriptions = m_wndTree.InsertItem(_T("Subscription changes"));
    m_wndTree.SetItemData(hSubscriptions, TTS_SUBSCRIPTIONS_ALL);
    m_wndTree.SetItemData(m_wndTree.InsertItem(_T("Private text message subscription"), hSubscriptions), TTS_SUBSCRIPTIONS_TEXTMSG_PRIVATE);
    m_wndTree.SetItemData(m_wndTree.InsertItem(_T("Channel text message subscription"), hSubscriptions), TTS_SUBSCRIPTIONS_TEXTMSG_CHANNEL);
    m_wndTree.SetItemData(m_wndTree.InsertItem(_T("Broadcast text message subscription"), hSubscriptions), TTS_SUBSCRIPTIONS_TEXTMSG_BROADCAST);
    m_wndTree.SetItemData(m_wndTree.InsertItem(_T("Voice stream subscription"), hSubscriptions), TTS_SUBSCRIPTIONS_VOICE);
    m_wndTree.SetItemData(m_wndTree.InsertItem(_T("Video stream subscription"), hSubscriptions), TTS_SUBSCRIPTIONS_VIDEO);
    m_wndTree.SetItemData(m_wndTree.InsertItem(_T("Desktop stream subscription"), hSubscriptions), TTS_SUBSCRIPTIONS_DESKTOP);
    m_wndTree.SetItemData(m_wndTree.InsertItem(_T("Desktop access subscription"), hSubscriptions), TTS_SUBSCRIPTIONS_DESKTOPINPUT);
    m_wndTree.SetItemData(m_wndTree.InsertItem(_T("Media file stream subscription"), hSubscriptions), TTS_SUBSCRIPTIONS_MEDIAFILE);
    m_wndTree.SetItemData(m_wndTree.InsertItem(_T("Intercept private text message subscription"), hSubscriptions), TTS_SUBSCRIPTIONS_INTERCEPT_TEXTMSG_PRIVATE);
    m_wndTree.SetItemData(m_wndTree.InsertItem(_T("Intercept channel text message subscription"), hSubscriptions), TTS_SUBSCRIPTIONS_INTERCEPT_TEXTMSG_CHANNEL);
    m_wndTree.SetItemData(m_wndTree.InsertItem(_T("Intercept voice stream subscription"), hSubscriptions), TTS_SUBSCRIPTIONS_INTERCEPT_VOICE);
    m_wndTree.SetItemData(m_wndTree.InsertItem(_T("Intercept video stream subscription"), hSubscriptions), TTS_SUBSCRIPTIONS_INTERCEPT_VIDEO);
    m_wndTree.SetItemData(m_wndTree.InsertItem(_T("Intercept desktop stream subscription"), hSubscriptions), TTS_SUBSCRIPTIONS_INTERCEPT_DESKTOP);
    m_wndTree.SetItemData(m_wndTree.InsertItem(_T("Intercept desktop access subscription"), hSubscriptions), TTS_SUBSCRIPTIONS_INTERCEPT_DESKTOPINPUT);
    m_wndTree.SetItemData(m_wndTree.InsertItem(_T("Intercept media file stream subscription"), hSubscriptions), TTS_SUBSCRIPTIONS_INTERCEPT_MEDIAFILE);
    m_wndTree.Expand(hSubscriptions, TVE_EXPAND);

    HTREEITEM hClassroom = m_wndTree.InsertItem(_T("Classroom changes"));
    m_wndTree.SetItemData(hClassroom, TTS_CLASSROOM_ALL);
    m_wndTree.SetItemData(m_wndTree.InsertItem(_T("Voice transmission allowed"), hClassroom), TTS_CLASSROOM_VOICE_TX);
    m_wndTree.SetItemData(m_wndTree.InsertItem(_T("Video transmission allowed"), hClassroom), TTS_CLASSROOM_VIDEO_TX);
    m_wndTree.SetItemData(m_wndTree.InsertItem(_T("Desktop transmission allowed"), hClassroom), TTS_CLASSROOM_DESKTOP_TX);
    m_wndTree.SetItemData(m_wndTree.InsertItem(_T("Media file transmission allowed"), hClassroom), TTS_CLASSROOM_MEDIAFILE_TX);
    m_wndTree.Expand(hClassroom, TVE_EXPAND);

    m_wndTree.EnsureVisible(hUser);
    // The MFC tree control stinks so we have to schedule a timer to do updates...
    SetTimer(1, 0, NULL);

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

void CTextToSpeechPage::Sync()
{
    std::queue<HTREEITEM> items;
    for(items.push(m_wndTree.GetRootItem());items.size();items.pop())
    {
        HTREEITEM hItem = items.front();

        TTSEvents uEvents = m_wndTree.GetItemData(hItem);
        m_wndTree.SetCheck(hItem, (uEvents & m_uTTSEvents) == uEvents);

        hItem = m_wndTree.GetNextSiblingItem(items.front());
        if(hItem != NULL)
            items.push(hItem);
        hItem = m_wndTree.GetChildItem(items.front());
        if(hItem != NULL)
            items.push(hItem);
    }
}

BOOL CTextToSpeechPage::OnKillActive()
{
    return CPropertyPage::OnKillActive();
}

void CTextToSpeechPage::OnNMClickTreeTts(NMHDR *pNMHDR, LRESULT *pResult)
{
    UINT uFlags = 0;
    CPoint pt(0, 0);
    GetCursorPos(&pt);
    m_wndTree.ScreenToClient(&pt);
    HTREEITEM hItem = m_wndTree.HitTest(pt, &uFlags);
    if(NULL != hItem && (TVHT_ONITEMSTATEICON  & uFlags))
    {
        m_wndTree.SelectItem(hItem);
        if(m_wndTree.GetCheck(hItem))
            m_uTTSEvents &= ~m_wndTree.GetItemData(hItem);
        else
            m_uTTSEvents |= m_wndTree.GetItemData(hItem);
        TRACE(_T("Hit %s, check %d\n"), m_wndTree.GetItemText(hItem), m_wndTree.GetCheck(hItem));
    }
    *pResult = 0;
    SetTimer(1, 0, NULL);
}


void CTextToSpeechPage::OnTvnKeydownTreeTts(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMTVKEYDOWN pTVKeyDown = reinterpret_cast<LPNMTVKEYDOWN>(pNMHDR);

    HTREEITEM hItem = m_wndTree.GetSelectedItem();
    if(pTVKeyDown->wVKey == VK_SPACE && hItem)
    {
        if(m_wndTree.GetCheck(hItem))
            m_uTTSEvents &= ~m_wndTree.GetItemData(hItem);
        else
            m_uTTSEvents |= m_wndTree.GetItemData(hItem);
    }
    *pResult = 0;
    SetTimer(1, 0, NULL);
}


void CTextToSpeechPage::OnTimer(UINT_PTR nIDEvent)
{
    CPropertyPage::OnTimer(nIDEvent);
    KillTimer(1);
    Sync();
}

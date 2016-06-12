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
    CString szText;
    szText.LoadString(IDS_PROPPAGE_USEREVENTS);
    TRANSLATE_ITEM(IDS_PROPPAGE_USEREVENTS, szText);
    HTREEITEM hUser = m_wndTree.InsertItem(szText);
    m_wndTree.SetItemData(hUser, TTS_USER_ALL);

    szText.LoadString(IDS_PROPPAGE_USERLOGIN);
    TRANSLATE_ITEM(IDS_PROPPAGE_USERLOGIN, szText);
    m_wndTree.SetItemData(m_wndTree.InsertItem(szText, hUser), TTS_USER_LOGGEDIN);
    szText.LoadString(IDS_PROPPAGE_USERLOGOUT);
    TRANSLATE_ITEM(IDS_PROPPAGE_USERLOGOUT, szText);
    m_wndTree.SetItemData(m_wndTree.InsertItem(szText, hUser), TTS_USER_LOGGEDOUT);
    szText.LoadString(IDS_PROPPAGE_USERJOINED);
    TRANSLATE_ITEM(IDS_PROPPAGE_USERJOINED, szText);
    m_wndTree.SetItemData(m_wndTree.InsertItem(szText, hUser), TTS_USER_JOINED);
    szText.LoadString(IDS_PROPPAGE_USERLEFT);
    TRANSLATE_ITEM(IDS_PROPPAGE_USERLEFT, szText);
    m_wndTree.SetItemData(m_wndTree.InsertItem(szText, hUser), TTS_USER_LEFT);
    szText.LoadString(IDS_PROPPAGE_USERJOINED_SAME);
    TRANSLATE_ITEM(IDS_PROPPAGE_USERJOINED_SAME, szText);
    m_wndTree.SetItemData(m_wndTree.InsertItem(szText, hUser), TTS_USER_JOINED_SAME);
    szText.LoadString(IDS_PROPPAGE_USERLEFT_SAME);
    TRANSLATE_ITEM(IDS_PROPPAGE_USERLEFT_SAME, szText);
    m_wndTree.SetItemData(m_wndTree.InsertItem(szText, hUser), TTS_USER_LEFT_SAME);
    szText.LoadString(IDS_PROPPAGE_TXTMSG_PRIV);
    TRANSLATE_ITEM(IDS_PROPPAGE_TXTMSG_PRIV, szText);
    m_wndTree.SetItemData(m_wndTree.InsertItem(szText, hUser), TTS_USER_TEXTMSG_PRIVATE);
    szText.LoadString(IDS_PROPPAGE_TXTMSG_CHAN);
    TRANSLATE_ITEM(IDS_PROPPAGE_TXTMSG_CHAN, szText);
    m_wndTree.SetItemData(m_wndTree.InsertItem(szText, hUser), TTS_USER_TEXTMSG_CHANNEL);
    szText.LoadString(IDS_PROPPAGE_TXTMSG_BCAST);
    TRANSLATE_ITEM(IDS_PROPPAGE_TXTMSG_BCAST, szText);
    m_wndTree.SetItemData(m_wndTree.InsertItem(szText, hUser), TTS_USER_TEXTMSG_BROADCAST);
    m_wndTree.Expand(hUser, TVE_EXPAND);

    szText.LoadString(IDS_PROPPAGE_SUB_CHANGE);
    TRANSLATE_ITEM(IDS_PROPPAGE_SUB_CHANGE, szText);
    HTREEITEM hSubscriptions = m_wndTree.InsertItem(szText);
    m_wndTree.SetItemData(hSubscriptions, TTS_SUBSCRIPTIONS_ALL);
    szText.LoadString(IDS_PROPPAGE_SUB_TXTMSG_PRIV);
    TRANSLATE_ITEM(IDS_PROPPAGE_SUB_TXTMSG_PRIV, szText);
    m_wndTree.SetItemData(m_wndTree.InsertItem(szText, hSubscriptions), TTS_SUBSCRIPTIONS_TEXTMSG_PRIVATE);
    szText.LoadString(IDS_PROPPAGE_SUB_TXTMSG_CHAN);
    TRANSLATE_ITEM(IDS_PROPPAGE_SUB_TXTMSG_CHAN, szText);
    m_wndTree.SetItemData(m_wndTree.InsertItem(szText, hSubscriptions), TTS_SUBSCRIPTIONS_TEXTMSG_CHANNEL);
    szText.LoadString(IDS_PROPPAGE_SUB_TXTMSG_BCAST);
    TRANSLATE_ITEM(IDS_PROPPAGE_SUB_TXTMSG_BCAST, szText);
    m_wndTree.SetItemData(m_wndTree.InsertItem(szText, hSubscriptions), TTS_SUBSCRIPTIONS_TEXTMSG_BROADCAST);
    szText.LoadString(IDS_PROPPAGE_SUB_VOICE);
    TRANSLATE_ITEM(IDS_PROPPAGE_SUB_VOICE, szText);
    m_wndTree.SetItemData(m_wndTree.InsertItem(szText, hSubscriptions), TTS_SUBSCRIPTIONS_VOICE);
    szText.LoadString(IDS_PROPPAGE_SUB_VIDEO);
    TRANSLATE_ITEM(IDS_PROPPAGE_SUB_VIDEO, szText);
    m_wndTree.SetItemData(m_wndTree.InsertItem(szText, hSubscriptions), TTS_SUBSCRIPTIONS_VIDEO);
    szText.LoadString(IDS_PROPPAGE_SUB_DESKTOP);
    TRANSLATE_ITEM(IDS_PROPPAGE_SUB_DESKTOP, szText);
    m_wndTree.SetItemData(m_wndTree.InsertItem(szText, hSubscriptions), TTS_SUBSCRIPTIONS_DESKTOP);
    szText.LoadString(IDS_PROPPAGE_SUB_DESKTOPINPUT);
    TRANSLATE_ITEM(IDS_PROPPAGE_SUB_DESKTOPINPUT, szText);
    m_wndTree.SetItemData(m_wndTree.InsertItem(szText, hSubscriptions), TTS_SUBSCRIPTIONS_DESKTOPINPUT);
    szText.LoadString(IDS_PROPPAGE_SUB_MEDIAFILE);
    TRANSLATE_ITEM(IDS_PROPPAGE_SUB_MEDIAFILE, szText);
    m_wndTree.SetItemData(m_wndTree.InsertItem(szText, hSubscriptions), TTS_SUBSCRIPTIONS_MEDIAFILE);
    szText.LoadString(IDS_PROPPAGE_SUBINT_TXTMSG_PRIV);
    TRANSLATE_ITEM(IDS_PROPPAGE_SUBINT_TXTMSG_PRIV, szText);
    m_wndTree.SetItemData(m_wndTree.InsertItem(szText, hSubscriptions), TTS_SUBSCRIPTIONS_INTERCEPT_TEXTMSG_PRIVATE);
    szText.LoadString(IDS_PROPPAGE_SUBINT_TXTMSG_CHAN);
    TRANSLATE_ITEM(IDS_PROPPAGE_SUBINT_TXTMSG_CHAN, szText);
    m_wndTree.SetItemData(m_wndTree.InsertItem(szText, hSubscriptions), TTS_SUBSCRIPTIONS_INTERCEPT_TEXTMSG_CHANNEL);
    szText.LoadString(IDS_PROPPAGE_SUBINT_VOICE);
    TRANSLATE_ITEM(IDS_PROPPAGE_SUBINT_VOICE, szText);
    m_wndTree.SetItemData(m_wndTree.InsertItem(szText, hSubscriptions), TTS_SUBSCRIPTIONS_INTERCEPT_VOICE);
    szText.LoadString(IDS_PROPPAGE_SUBINT_VIDEO);
    TRANSLATE_ITEM(IDS_PROPPAGE_SUBINT_VIDEO, szText);
    m_wndTree.SetItemData(m_wndTree.InsertItem(szText, hSubscriptions), TTS_SUBSCRIPTIONS_INTERCEPT_VIDEO);
    szText.LoadString(IDS_PROPPAGE_SUBINT_DESKTOP);
    TRANSLATE_ITEM(IDS_PROPPAGE_SUBINT_DESKTOP, szText);
    m_wndTree.SetItemData(m_wndTree.InsertItem(szText, hSubscriptions), TTS_SUBSCRIPTIONS_INTERCEPT_DESKTOP);
    szText.LoadString(IDS_PROPPAGE_SUBINT_DESKTOPINPUT);
    TRANSLATE_ITEM(IDS_PROPPAGE_SUBINT_DESKTOPINPUT, szText);
    m_wndTree.SetItemData(m_wndTree.InsertItem(szText, hSubscriptions), TTS_SUBSCRIPTIONS_INTERCEPT_DESKTOPINPUT);
    szText.LoadString(IDS_PROPPAGE_SUBINT_MEDIAFILE);
    TRANSLATE_ITEM(IDS_PROPPAGE_SUBINT_MEDIAFILE, szText);
    m_wndTree.SetItemData(m_wndTree.InsertItem(szText, hSubscriptions), TTS_SUBSCRIPTIONS_INTERCEPT_MEDIAFILE);
    m_wndTree.Expand(hSubscriptions, TVE_EXPAND);

    szText.LoadString(IDS_PROPPAGE_CLASSROOM_CHANGE);
    TRANSLATE_ITEM(IDS_PROPPAGE_CLASSROOM_CHANGE, szText);
    HTREEITEM hClassroom = m_wndTree.InsertItem(szText);
    m_wndTree.SetItemData(hClassroom, TTS_CLASSROOM_ALL);
    szText.LoadString(IDS_PROPPAGE_CLASSROOM_VOICE);
    TRANSLATE_ITEM(IDS_PROPPAGE_CLASSROOM_VOICE, szText);
    m_wndTree.SetItemData(m_wndTree.InsertItem(szText, hClassroom), TTS_CLASSROOM_VOICE_TX);
    szText.LoadString(IDS_PROPPAGE_CLASSROOM_VIDEO);
    TRANSLATE_ITEM(IDS_PROPPAGE_CLASSROOM_VIDEO, szText);
    m_wndTree.SetItemData(m_wndTree.InsertItem(szText, hClassroom), TTS_CLASSROOM_VIDEO_TX);
    szText.LoadString(IDS_PROPPAGE_CLASSROOM_DESKTOP);
    TRANSLATE_ITEM(IDS_PROPPAGE_CLASSROOM_DESKTOP, szText);
    m_wndTree.SetItemData(m_wndTree.InsertItem(szText, hClassroom), TTS_CLASSROOM_DESKTOP_TX);
    szText.LoadString(IDS_PROPPAGE_CLASSROOM_MEDIAFILE);
    TRANSLATE_ITEM(IDS_PROPPAGE_CLASSROOM_MEDIAFILE, szText);
    m_wndTree.SetItemData(m_wndTree.InsertItem(szText, hClassroom), TTS_CLASSROOM_MEDIAFILE_TX);
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

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

// SessionTreeCtrl.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "TeamTalkBase.h"
#include "SessionTreeCtrl.h"

using namespace std;

extern TTInstance* ttInst;
extern int nTextLimit;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSessionTreeCtrl

// GetItemData for channels hold a UINT for its channelid
// GetItemData for users hold UINT (nUserID)

IMPLEMENT_DYNAMIC(CSessionTreeCtrl, CTreeCtrl)


static int CALLBACK SortTree(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
    // lParamSort contains a pointer to the tree control.
    // The lParam of an item is just its handle, 
    // as specified with <A HREF="_mfc_ctreectrl.3a3a.setitemdata.htm">SetItemData</A>
    CSessionTreeCtrl* pTreeCtrl = reinterpret_cast<CSessionTreeCtrl*>(lParamSort);

    if( (lParam1 & CHANNEL_ITEMDATA) && (lParam2 & USER_ITEMDATA) )
        return 1;
    else if( (lParam1 & USER_ITEMDATA) && (lParam2 & CHANNEL_ITEMDATA) )
        return -1;
    else if( (lParam1 & CHANNEL_ITEMDATA) && (lParam2 & CHANNEL_ITEMDATA) )
    {
        HTREEITEM hItem1 = pTreeCtrl->GetChannelItem(lParam1 & ID_ITEMDATA);
        HTREEITEM hItem2 = pTreeCtrl->GetChannelItem(lParam2 & ID_ITEMDATA);
        ASSERT(hItem1 && hItem2);
        switch (pTreeCtrl->GetSortOrder())
        {
        case SORT_TREE_POLULATED :
        {
            int nChanID1 = (ID_ITEMDATA & pTreeCtrl->GetItemData(hItem1));
            int nChanID2 = (ID_ITEMDATA & pTreeCtrl->GetItemData(hItem2));
            size_t nCount1 = pTreeCtrl->GetUsers(nChanID1).size();
            size_t nCount2 = pTreeCtrl->GetUsers(nChanID2).size();
            if (nCount1 != nCount2)
                return nCount1 > nCount2? -1 : 1;
        }
        case SORT_TREE_ASCENDING :
        default :
            CString szItem1 = pTreeCtrl->GetItemText(hItem1);
            CString szItem2 = pTreeCtrl->GetItemText(hItem2);
            return szItem1.CompareNoCase(szItem2);
        }
    }
    else if( (lParam1 & USER_ITEMDATA) && (lParam2 & USER_ITEMDATA) )
    {
        HTREEITEM hItem1 = pTreeCtrl->GetUserItem(lParam1 & ID_ITEMDATA);
        HTREEITEM hItem2 = pTreeCtrl->GetUserItem(lParam2 & ID_ITEMDATA);
        ASSERT(hItem1 && hItem2);
        CString szItem1 = pTreeCtrl->GetItemText(hItem1);
        CString szItem2 = pTreeCtrl->GetItemText(hItem2);
        return szItem1.CompareNoCase(szItem2);
    }
    else
    {
        ASSERT(0);
        return 0;
    }
}

CSessionTreeCtrl::CSessionTreeCtrl()
: m_nMyChannel(0)
, m_bShowUserCount(FALSE)
, m_hItemDrag(NULL)
, m_hItemDrop(NULL)
, m_pDragImage(NULL)
, m_bLDragging(FALSE)
{
}

CSessionTreeCtrl::~CSessionTreeCtrl()
{
}

void CSessionTreeCtrl::Initialize()
{
    VERIFY(m_bmp.LoadBitmap(MAKEINTRESOURCE(IDB_BITMAP_SESSION2)));

    //VERIFY(m_bmp.LoadBitmap(MAKEINTRESOURCE(IDB_BITMAP16)));
#if defined(UNDER_CE)
    VERIFY(m_ImageList.Create(17, 16, ILC_MASK, 0, 0));
#else
    VERIFY(m_ImageList.Create(17, 16, ILC_MASK | ILC_COLOR32, 0, 0));
#endif

    COLORREF ref = RGB(133,134,115);
    m_ImageList.Add(&m_bmp, ref);
    //m_ImageList.Create(IDB_BITMAP_SESSION, 17, 1, RGB(255,255,255));
    SetImageList(&m_ImageList, TVSIL_NORMAL);
}

void CSessionTreeCtrl::ClearChannels()
{
    DeleteAllItems();
    m_channels.clear();
    m_users.clear();
}

void CSessionTreeCtrl::UpdMyChannel(int nChannelID)
{
    HTREEITEM hItem = GetChannelItem(m_nMyChannel);
    if(hItem)
    {
        HTREEITEM hNewItem = GetChannelItem(nChannelID);
        HTREEITEM hTmp = hItem;
        while( hTmp != GetRootItem())
        {
            Expand(hTmp, TVE_COLLAPSE);
            hTmp = GetParentItem(hTmp);
        }

        //make list of old users
        vector<HTREEITEM> vecItems;
        int nImg = USER_SILENT, n;
        HTREEITEM hIteItem = GetChildItem(hItem);
        while(hIteItem)
        {
            GetItemImage(hIteItem, nImg, n);
            if(IsUser(nImg))
                vecItems.push_back(hIteItem);

            hIteItem = GetNextSiblingItem(hIteItem);
        }

        for(size_t i=0;i<vecItems.size();i++)
            UserItemMinus(vecItems[i], UserTalking);
    }

    hItem = GetChannelItem(nChannelID);
    while(hItem)
    {
        ChannelItemPlus(hItem, ChannelOpened);
        Expand(hItem, TVE_EXPAND);
        hItem = GetParentItem(hItem);
    }

    m_nMyChannel = nChannelID;
}

void CSessionTreeCtrl::UpdateParentChannels(int nChannelID)
{
    //update channel user count
    Channel chan;
    while(GetChannel(nChannelID, chan))
    {
        UpdateChannel(chan);
        nChannelID = chan.nParentID;
    }
}

void CSessionTreeCtrl::UpdServerName(const ServerProperties& prop)
{
    Channel chan;
    if(GetChannel(GetRootChannelID(m_channels), chan))
        UpdateChannel(chan);
}

HTREEITEM CSessionTreeCtrl::GetUserItem(int nUserID) const
{
    HTREEITEM result = 0;
    HTREEITEM item = GetRootItem();
    std::queue<HTREEITEM> items;

    items.push(item);

    while(items.size() && result == 0)
    {
        HTREEITEM item = items.front();
        items.pop();
        item = GetChildItem(item);
        while(item != NULL)
        {
            DWORD_PTR dwItemData = GetItemData(item);
            if(dwItemData & USER_ITEMDATA)
            {
                if(nUserID == (dwItemData & ID_ITEMDATA))
                {
                    result = item;
                    break;
                }
            }
            else if(dwItemData & CHANNEL_ITEMDATA)
                items.push(item);
            item = GetNextSiblingItem(item);
        }
    }
    return result;
}

HTREEITEM CSessionTreeCtrl::GetChannelItem(int nChannelID) const
{
    HTREEITEM result = 0;

    HTREEITEM item = GetRootItem();
    std::queue<HTREEITEM> items;

    items.push(item);

    while(items.size() && result == 0)
    {
        HTREEITEM item = items.front();
        items.pop();
        while(item != NULL)
        {
            DWORD_PTR dwItemData = GetItemData(item);
            if(dwItemData & CHANNEL_ITEMDATA)
            {
                int nNum = (dwItemData & ID_ITEMDATA);
                if(nChannelID == nNum)
                {
                    result = item;
                    break;
                }
                items.push( GetChildItem(item) );
            }
            item = GetNextSiblingItem(item);
        }
    }

    return result;
}

BOOL CSessionTreeCtrl::IsUser(int nImageIndex) const
{
    return nImageIndex >= USER_INDEX_START && nImageIndex <= USER_INDEX_END;
}

BOOL CSessionTreeCtrl::IsUserTalking(int nImageIndex) const
{
    switch(nImageIndex)
    {
    case USER_TALKING :
    case USER_TALKING_OP :
    case USER_TALKING_MSG :
    case USER_TALKING_MSG_OP :
    case USER_TALKING_AWAY :
    case USER_TALKING_AWAY_OP :
    case USER_TALKING_AWAY_MSG :
    case USER_TALKING_AWAY_MSG_OP :
        return TRUE;
    default : return FALSE;
    }
}

BOOL CSessionTreeCtrl::IsUserAway(int nImageIndex) const
{
    switch(nImageIndex)
    {
    case USER_SILENT_AWAY :
    case USER_SILENT_AWAY_OP :
    case USER_TALKING_AWAY :
    case USER_TALKING_AWAY_OP :
    case USER_SILENT_AWAY_MSG :
    case USER_SILENT_AWAY_MSG_OP :
    case USER_TALKING_AWAY_MSG :
    case USER_TALKING_AWAY_MSG_OP :
        return TRUE;
    default : return FALSE;
    }
}

BOOL CSessionTreeCtrl::IsUserMessaged(int nImageIndex) const
{
    switch(nImageIndex)
    {
    case USER_SILENT_MSG :
    case USER_SILENT_MSG_OP :
    case USER_TALKING_MSG :
    case USER_TALKING_MSG_OP :
    case USER_SILENT_AWAY_MSG :
    case USER_SILENT_AWAY_MSG_OP :
    case USER_TALKING_AWAY_MSG :
    case USER_TALKING_AWAY_MSG_OP :
        return TRUE;
    default :
        return FALSE;
    }
}

BOOL CSessionTreeCtrl::IsUserOperator(int nImageIndex) const
{
    switch(nImageIndex)
    {
    case USER_SILENT_OP :
    case USER_TALKING_OP :
    case USER_SILENT_MSG_OP :
    case USER_TALKING_MSG_OP :
    case USER_SILENT_AWAY_OP :
    case USER_TALKING_AWAY_OP :
    case USER_SILENT_AWAY_MSG_OP :
    case USER_TALKING_AWAY_MSG_OP :
        return TRUE;
    default :
        return FALSE;
    }
}

BOOL CSessionTreeCtrl::IsChannel(int nImageIndex) const
{
    return nImageIndex >= CHANNEL_INDEX_START && nImageIndex <= CHANNEL_INDEX_END;
}

BOOL CSessionTreeCtrl::IsChannelMessaged(int nImageIndex) const
{
    switch(nImageIndex)
    {
    case CHANNEL_CLOSED_MSG :
    case CHANNEL_CLOSED_LOCK_MSG :
    case CHANNEL_OPENED_MSG :
    case CHANNEL_OPENED_LOCK_MSG :
    case ROOT_CLOSED_MSG :
    case ROOT_CLOSED_LOCK_MSG :
    case ROOT_OPENED_MSG :
    case ROOT_OPENED_LOCK_MSG :
        return TRUE;
    default :
        return FALSE;
    }
}

BOOL CSessionTreeCtrl::IsChannelOpened(int nImageIndex) const
{
    switch(nImageIndex)
    {
    case CHANNEL_OPENED :
    case CHANNEL_OPENED_LOCK :
    case CHANNEL_OPENED_MSG :
    case CHANNEL_OPENED_LOCK_MSG :
    case ROOT_OPENED :
    case ROOT_OPENED_LOCK :
    case ROOT_OPENED_MSG :
    case ROOT_OPENED_LOCK_MSG :
        return TRUE;
    default :
        return FALSE;
    }
}

BOOL CSessionTreeCtrl::IsChannelLocked(int nImageIndex) const
{
    switch(nImageIndex)
    {
    case CHANNEL_CLOSED_LOCK :
    case CHANNEL_CLOSED_LOCK_MSG :
    case CHANNEL_OPENED_LOCK :
    case CHANNEL_OPENED_LOCK_MSG :
    case ROOT_CLOSED_LOCK :
    case ROOT_CLOSED_LOCK_MSG :
    case ROOT_OPENED_LOCK :
    case ROOT_OPENED_LOCK_MSG :
        return TRUE;
    default :
        return FALSE;
    }
}

int CSessionTreeCtrl::GetMyChannelID() const
{
    return m_nMyChannel;
}

int CSessionTreeCtrl::GetSelectedChannel(bool bIncludeUserChan/* = false*/) const
{
    HTREEITEM h = GetSelectedItem();

    if(h)
    {
        int img, img2;
        GetItemImage(h, img, img2);
        if(IsChannel(img))
        {
            int nChannelID = (GetItemData(h) & ID_ITEMDATA);
            ASSERT(m_channels.find(nChannelID) != m_channels.end());
            return nChannelID;
        }
        if(bIncludeUserChan && IsUser(img))
        {
            int nUserID = (GetItemData(h) & ID_ITEMDATA);
            User user = {};
            if(GetUser(nUserID, user))
                return user.nChannelID;
        }
    }
    return 0;
}

Channel CSessionTreeCtrl::GetSelChannel() const
{
    int nChanID = GetSelectedChannel(TRUE);
    if (GetChannels().find(nChanID) != GetChannels().end())
        return GetChannels().at(nChanID);

    Channel chan;
    ZERO_STRUCT(chan);
    return chan;
}

int CSessionTreeCtrl::GetSelectedUser() const
{
    HTREEITEM h = GetSelectedItem();

    if(h)
    {
        int img, img2;
        GetItemImage(h, img, img2);
        if(IsUser(img))
        {
            int nUserID = (GetItemData(h) & ID_ITEMDATA);
            ASSERT(m_users.find(nUserID) != m_users.end());
            return nUserID;
        }
    }
    return 0;
}

std::vector<User> CSessionTreeCtrl::GetSelectedUsers() const
{
    std::vector<User> users;
    HTREEITEM hItem = GetRootItem();
    while((hItem = GetNextVisibleItem(hItem)) != NULL)
    {
        DWORD_PTR dwItemData = GetItemData(hItem);
        if ((dwItemData & USER_ITEMDATA) != 0 &&
            (GetItemState(hItem, TVIS_SELECTED) & TVIS_SELECTED))
        {
            User user;
            if(GetUser((int)(dwItemData & ID_ITEMDATA), user))
                users.push_back(user);
        }
    }
    return users;
}

void CSessionTreeCtrl::SetUserTalking(int nUserID, BOOL bTalking)
{
    HTREEITEM hItem = GetUserItem(nUserID);
    if(hItem)
    {
        if(bTalking)
            UserItemPlus(hItem, UserTalking);
        else
            UserItemMinus(hItem, UserTalking);
    }
}

void CSessionTreeCtrl::SetUserMessage(int nUserID, BOOL bMessaged)
{
    HTREEITEM hItem = GetUserItem(nUserID);
    if(hItem)
    {
        if(bMessaged)
            UserItemPlus(hItem, UserMessage);
        else
            UserItemMinus(hItem, UserMessage);
    }
}

void CSessionTreeCtrl::SetChannelMessage(int nChannelID, BOOL bMessaged)
{
    HTREEITEM hItem = GetChannelItem(nChannelID);
    ASSERT(hItem);
    if(hItem)
    {
        if( bMessaged )
        {
            SetItemState(hItem, TVIS_BOLD, TVIS_BOLD);
            ChannelItemPlus(hItem, ChannelMessage);
        }
        else
        {
            SetItemState(hItem, 0, TVIS_BOLD);
            ChannelItemMinus(hItem, ChannelMessage);
        }
    }
}

void CSessionTreeCtrl::AddUser(const User& user)
{
    m_users[user.nUserID] = user;

    //return if user is not shown in tree ctrl
    if(user.nChannelID == 0)
        return;

    HTREEITEM hChanItem = GetChannelItem(user.nChannelID);
    ASSERT(hChanItem);
    if(hChanItem)
    {
        int nImg = USER_SILENT;
        HTREEITEM hNewItem = InsertItem(LimitText(GetUserText(user.nUserID)),
                                        USER_SILENT, USER_SILENT, hChanItem, TVI_FIRST);
        SetItemData(hNewItem, (DWORD)(user.nUserID | USER_ITEMDATA));

        if(user.uUserState & USERSTATE_VOICE)
            UserItemPlus(hNewItem, UserTalking);

        if((user.nStatusMode & STATUSMODE_MASK) == STATUSMODE_AWAY)
            UserItemPlus(hNewItem, UserAway);

        if(IsUserOperator(user.nUserID, user.nChannelID))
            UserItemPlus(hNewItem, UserOperator);

        SortItem(GetParentItem(hNewItem));

        if(user.nUserID == TT_GetMyUserID(ttInst))
        {
            UpdMyChannel(user.nChannelID);
        }

        if(user.nUserID == TT_GetMyUserID(ttInst))
        {
            Select(hChanItem, TVGN_CARET);
            SetItemState(hNewItem, TVIS_BOLD, TVIS_BOLD);
            SetUserTalking(user.nUserID, IsMyselfTalking());
        }
        else
            SetUserTalking(user.nUserID, user.uUserState & USERSTATE_VOICE);

        UpdateParentChannels(user.nChannelID);
    }
}

void CSessionTreeCtrl::UpdateUser(const User& user)
{
    ASSERT(m_users.find(user.nUserID) != m_users.end());
    m_users[user.nUserID] = user;

    //return if user is not shown in tree ctrl
    if(user.nChannelID == 0)
        return;

    HTREEITEM hUserItem = GetUserItem(user.nUserID);
    ASSERT(hUserItem);
    if(hUserItem)
    {
        SetItemText(hUserItem, LimitText(GetUserText(user.nUserID)));
        if((user.nStatusMode & STATUSMODE_MASK) == STATUSMODE_AWAY)
            UserItemPlus(hUserItem, UserAway);
        else
            UserItemMinus(hUserItem, UserAway);

        SortItem(GetParentItem(hUserItem));
        if(user.nUserID == TT_GetMyUserID(ttInst))
        {
            SetItemState(hUserItem, TVIS_BOLD, TVIS_BOLD);
            SetUserTalking(user.nUserID, IsMyselfTalking());
        }
        else
            SetUserTalking(user.nUserID, user.uUserState & USERSTATE_VOICE);
    }
}

void CSessionTreeCtrl::RemoveUser(const User& user)
{
    ASSERT(m_users.find(user.nUserID) != m_users.end());

    User oldUser;
    if(!GetUser(user.nUserID, oldUser))
        return;

    HTREEITEM hItem = GetUserItem(user.nUserID);
    if(hItem)
        DeleteItem(hItem);
    m_users[user.nUserID] = user;

    UpdateParentChannels(oldUser.nChannelID);
}

void CSessionTreeCtrl::AddChannel(const Channel& channel)
{
    m_channels[channel.nChannelID] = channel;
    if(channel.nChannelID == GetRootChannelID(m_channels))  //it's the root
    {
        HTREEITEM hRoot = InsertItem( _T(""), ROOT_CLOSED, ROOT_CLOSED);
        SetItemData(hRoot, (channel.nChannelID | CHANNEL_ITEMDATA));
        SetItemText(hRoot, GetChannelText(channel.nChannelID));
        if(channel.bPassword)
            ChannelItemPlus(hRoot, ChannelLocked);
    }
    else
    {
        ASSERT(_tcslen(channel.szName));
        ASSERT(channel.nParentID != 0);
        int nParentID = channel.nParentID;
        HTREEITEM hParent = GetChannelItem(nParentID);
        ASSERT(hParent);
        if(hParent)
        {
            HTREEITEM hNewItem = InsertItem(LimitText(GetChannelText(channel.nChannelID)),
                                            CHANNEL_CLOSED, CHANNEL_CLOSED, hParent);
            SetItemData(hNewItem, (channel.nChannelID | CHANNEL_ITEMDATA));
            if(channel.bPassword)
                ChannelItemPlus(hNewItem, ChannelLocked);

            SortItem(hParent);
        }
    }
}

void CSessionTreeCtrl::UpdateChannel(const Channel& chan)
{
    m_channels[chan.nChannelID] = chan;
    HTREEITEM hItem = GetChannelItem(chan.nChannelID);
    if(hItem)
    {
        SetItemText(hItem, GetChannelText(chan.nChannelID));

        if(chan.bPassword)
            ChannelItemPlus(hItem, ChannelLocked);
        else
            ChannelItemMinus(hItem, ChannelLocked);

        users_t users = GetChannelUsers(m_users, chan.nChannelID);
        users_t::const_iterator ite;
        for(ite = users.begin();ite != users.end();ite++)
        {
            int nUserID = ite->first;
            HTREEITEM hItem = GetUserItem(nUserID);
            if(hItem)
            {
                int nImg = USER_SILENT, n;
                GetItemImage(hItem, nImg, n);
                if(IsUserOperator(nUserID, chan.nChannelID) && !IsUserOperator(nImg))
                    UserItemPlus(hItem, UserOperator);
                else if(!IsUserOperator(nUserID, chan.nChannelID) && IsUserOperator(nImg))
                    UserItemMinus(hItem, UserOperator);
            }
        }

        SortItem(GetParentItem(hItem));
    }
}

void CSessionTreeCtrl::RemoveChannel(const Channel& chan)
{
    channels_t::iterator ite = m_channels.find(chan.nChannelID);
    if(ite != m_channels.end())
        m_channels.erase(ite);

    HTREEITEM hItem = GetChannelItem(chan.nChannelID);
    if(hItem)
        DeleteItem(hItem);
}

void CSessionTreeCtrl::ExpandAll()
{
    std::queue<HTREEITEM> treeitemqueue;
    HTREEITEM hItem = GetRootItem();

    if(hItem)
        treeitemqueue.push(hItem);

    while(treeitemqueue.size())
    {
        HTREEITEM hItem = treeitemqueue.front();
        treeitemqueue.pop();
        hItem = GetChildItem(hItem);
        while(hItem)
        {
            EnsureVisible(hItem);
            int nImg = 0;
            GetItemImage(hItem,nImg,nImg);
            if(IsChannel(nImg))
                treeitemqueue.push(hItem);
            hItem = GetNextSiblingItem(hItem);
        }
    }
}

void CSessionTreeCtrl::CollapseAll()
{
    std::queue<HTREEITEM> treeitemqueue;
    HTREEITEM hItem = GetRootItem();

    if(hItem)
        Expand(hItem, TVE_COLLAPSE);
}

BEGIN_MESSAGE_MAP(CSessionTreeCtrl, CTreeCtrl)
    //{{AFX_MSG_MAP(CSessionTreeCtrl)
    //}}AFX_MSG_MAP
    ON_NOTIFY_REFLECT(TVN_ITEMEXPANDED, OnTvnItemexpanded)
    ON_NOTIFY_REFLECT(TVN_BEGINDRAG, OnTvnBegindrag)
    ON_WM_MOUSEMOVE()
    ON_WM_LBUTTONUP()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSessionTreeCtrl message handlers

void CSessionTreeCtrl::OnTvnItemexpanded(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);

    HTREEITEM hItem = pNMTreeView->itemNew.hItem;
    int nChannelID = int(GetItemData(hItem));
    if(nChannelID & CHANNEL_ITEMDATA)
    {
        nChannelID &= ID_ITEMDATA;
        switch(pNMTreeView->action)
        {
        case 2 :
            {
                ChannelItemPlus(hItem, ChannelOpened);
                UpdateParentChannels(nChannelID);
                break;
            }
        case 1 :
            {
                ChannelItemMinus(hItem, ChannelOpened);
                UpdateParentChannels(nChannelID);
                break;
            }
        }
    }

    *pResult = 0;
}

void CSessionTreeCtrl::UserItemPlus(HTREEITEM hItem, UserFlags plusState)
{
    ASSERT(hItem);
    int nImg = 0, nImg2 = 0;
    if(hItem && GetItemImage(hItem, nImg, nImg2))
    {
        ASSERT(IsUser(nImg));
        if(IsUser(nImg))
        {
            if(plusState == UserTalking && !IsUserTalking(nImg))
                nImg += 2;
            else if(plusState == UserOperator && !IsUserOperator(nImg))
                nImg += 1;
            else if(plusState == UserMessage && !IsUserMessaged(nImg))
                nImg += 4;
            else if(plusState == UserAway && !IsUserAway(nImg))
                nImg += 8;

            SetItemImage(hItem, nImg, nImg);
            ASSERT(nImg >= USER_INDEX_START && nImg <= USER_INDEX_END); //within users
        }
    }
    ASSERT(IsUser(nImg));
}

void CSessionTreeCtrl::UserItemMinus(HTREEITEM hItem, UserFlags minusState)
{
    ASSERT(hItem);
    int nImg = 0, nImg2 = 0;
    if(hItem && GetItemImage(hItem, nImg, nImg2))
    {
        ASSERT(IsUser(nImg));
        if(IsUser(nImg))
        {
            if(minusState == UserTalking && IsUserTalking(nImg))
                nImg -= 2;
            else if(minusState == UserOperator && IsUserOperator(nImg))
                nImg -= 1;
            else if(minusState == UserMessage && IsUserMessaged(nImg))
                nImg -= 4;
            else if(minusState == UserAway && IsUserAway(nImg))
                nImg -= 8;

            SetItemImage(hItem, nImg, nImg);
            ASSERT(nImg >= USER_INDEX_START && nImg <= USER_INDEX_END); //within users
        }
    }
    ASSERT(IsUser(nImg));
}

void CSessionTreeCtrl::ChannelItemPlus(HTREEITEM hItem, ChannelStates plusState)
{
    ASSERT(hItem);
    int nImg = 0, nImg2 = 0;
    if(hItem && GetItemImage(hItem, nImg, nImg2))
    {
        ASSERT(IsChannel(nImg));
        if(IsChannel(nImg))
        {
            if(plusState == ChannelLocked && !IsChannelLocked(nImg))
                nImg += 1;
            else if(plusState == ChannelMessage && !IsChannelMessaged(nImg))
                nImg += 2;
            else if(plusState == ChannelOpened && !IsChannelOpened(nImg))
                nImg += 4;

            SetItemImage(hItem, nImg, nImg);
        }
    }
    ASSERT(nImg >= CHANNEL_INDEX_START && nImg <= CHANNEL_INDEX_END); //within channels
}

void CSessionTreeCtrl::ChannelItemMinus(HTREEITEM hItem, ChannelStates minusState)
{
    ASSERT(hItem);
    int nImg = 0, nImg2 = 0;
    if(hItem && GetItemImage(hItem, nImg, nImg2))
    {
        ASSERT(IsChannel(nImg));
        if(IsChannel(nImg))
        {
            if(minusState == ChannelLocked && IsChannelLocked(nImg))
                nImg -= 1;
            else if(minusState == ChannelMessage && IsChannelMessaged(nImg))
                nImg -= 2;
            else if(minusState == ChannelOpened && IsChannelOpened(nImg))
                nImg -= 4;

            SetItemImage(hItem, nImg, nImg);
        }
    }
    ASSERT(nImg >= CHANNEL_INDEX_START && nImg <= CHANNEL_INDEX_END); //within channels
}

BOOL CSessionTreeCtrl::IsShowingUserCount() const
{
    return m_bShowUserCount;
}

void CSessionTreeCtrl::ShowUserCount(BOOL bShow)
{
    m_bShowUserCount = bShow;

    CString szName;
    std::queue<HTREEITEM> qItems;
    HTREEITEM hItem = GetRootItem();
    if(hItem)
    {
        qItems.push(hItem);
        while(!qItems.empty())
        {
            hItem = qItems.front();
            qItems.pop();

            int nChannelID = (GetItemData(hItem) & ID_ITEMDATA);
            SetItemText(hItem, GetChannelText(nChannelID));
            hItem = GetChildItem(hItem);

            while(hItem)
            {
                int nImg1 = 0, nImg2 = 0;
                GetItemImage(hItem, nImg1, nImg2);
                if(IsChannel(nImg1))
                    qItems.push(hItem);

                hItem = GetNextSiblingItem(hItem);
            }
        }
    }
}

void CSessionTreeCtrl::SetSortOrder(SortOrder order)
{
    m_sortOrder = order;
}

BOOL CSessionTreeCtrl::GetChannel(int nChannelID, Channel& outChan) const
{
    Channel chan = {};
    auto ite = m_channels.find(nChannelID);
    if(ite != m_channels.end())
    {
        outChan = ite->second;
        return TRUE;
    }
    return FALSE;
}

void CSessionTreeCtrl::AddUserMessage(int nUserID, const MyTextMessage& msg)
{
    msgmap_t::iterator ite = m_messages.find(nUserID);
    if(ite != m_messages.end())
        ite->second.push_back(msg);
    else
    {
        messages_t msgs;
        msgs.push_back(msg);
        m_messages[nUserID] = msgs;
    }
}

BOOL CSessionTreeCtrl::GetLastUserMessage(int nUserID, MyTextMessage& msg) const
{
    msgmap_t::const_iterator ite = m_messages.find(nUserID);
    if(ite != m_messages.end() && ite->second.size())
    {
        msg = ite->second[ite->second.size()-1];
        return TRUE;
    }
    return FALSE;
}

void CSessionTreeCtrl::SetUserMessages(int nUserID, const messages_t& msgs)
{
    m_messages[nUserID] = msgs;
}

messages_t CSessionTreeCtrl::GetUserMessages(int nUserID) const
{
    messages_t msgs;
    auto ite = m_messages.find(nUserID);
    if(ite != m_messages.end())
        return ite->second;
    return msgs;
}

const channels_t& CSessionTreeCtrl::GetChannels() const
{
    return m_channels;
}

BOOL CSessionTreeCtrl::IsUserOperator(int nUserID, int nChannelID) const
{
    return TT_IsChannelOperator(ttInst, nUserID, nChannelID);
}

users_t CSessionTreeCtrl::GetOperators(int nChannelID) const
{
    users_t users = GetChannelUsers(m_users, nChannelID);
    users_t ops;
    for(users_t::iterator ite=users.begin();ite!=users.end();ite++)
        if(IsUserOperator(ite->first, nChannelID))
            ops[ite->first] = ite->second;

    return ops;
}

BOOL CSessionTreeCtrl::GetUser(int nUserID, User& outUser) const
{
    auto ite = m_users.find(nUserID);
    if(ite != m_users.end())
    {
        outUser = ite->second;
        return TRUE;
    }
    return FALSE;
}

users_t CSessionTreeCtrl::GetUsers(int nChannelID) const
{
    return GetChannelUsers(m_users, nChannelID);
}

CString CSessionTreeCtrl::GetUserText(int nUserID) const
{
    CString szText;
    auto ite = m_users.find(nUserID);
    if(ite != m_users.end())
    {
        User user = ite->second;
        szText = GetDisplayName(user);

        if(_tcslen(user.szStatusMsg)>0)
        {
            CString szMsg = user.szStatusMsg;
            if(!szMsg.IsEmpty())
                szText = szText + _T("  (") + szMsg + _T(")");
        }
    }
    return LimitText(szText);
}

CString CSessionTreeCtrl::GetChannelText(int nChannelID) const
{
    CString szText;
    auto ite = m_channels.find(nChannelID);
    if(ite != m_channels.end())
    {
        ServerProperties prop = {};
        TT_GetServerProperties(ttInst, &prop); 
        if(IsShowingUserCount())
        {
            HTREEITEM hItem = GetChannelItem(nChannelID);
            int nCount = (int)GetChannelUsers(m_users, nChannelID).size();
            UINT uState = hItem? GetItemState(hItem, TVIS_EXPANDED) : 0;
            if (hItem && (uState & TVIS_EXPANDED) == 0)
            {
                //TRACE(_T("%s is not expanded\n"), ite->second.szName);
                channels_t subs = GetSubChannels(nChannelID, m_channels, TRUE);
                for(auto c=subs.begin();c!=subs.end();++c)
                {
                    nCount += (int)GetChannelUsers(m_users, c->first).size();
                }
            }
            else if (hItem && (uState & TVIS_EXPANDED) == TVIS_EXPANDED)
            {
                //TRACE(_T("%s is expanded\n"), ite->second.szName);
            } 

            if(GetRootChannelID(m_channels) == nChannelID)
                szText.Format(_T("%s (%d)"), prop.szServerName, nCount);
            else
                szText.Format(_T("%s (%d)"), ite->second.szName, nCount);
        }
        else
        {
            if(GetRootChannelID(m_channels) == nChannelID)
                szText.Format(_T("%s"), prop.szServerName);
            else
                szText.Format(_T("%s"), ite->second.szName);
        }
    }

    return LimitText(szText);
}

void CSessionTreeCtrl::OnTvnBegindrag(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
    *pResult = 0;

    m_hItemDrag = pNMTreeView->itemNew.hItem;
    m_hItemDrop = NULL;
    int img, imgSel;
    GetItemImage(m_hItemDrag, img, imgSel);
    if(IsChannel(img))
        return;

    m_pDragImage = CreateDragImage(m_hItemDrag);
    if( !m_pDragImage )
        return;

    m_bLDragging = TRUE;
    m_pDragImage->BeginDrag(0, CPoint(-15,-15));
    POINT pt = pNMTreeView->ptDrag;
    ClientToScreen( &pt );
    m_pDragImage->DragEnter(NULL, pt);
    SetCapture();
}

void CSessionTreeCtrl::OnMouseMove(UINT nFlags, CPoint point)
{
    CTreeCtrl::OnMouseMove(nFlags, point);

    HTREEITEM    hitem;
    UINT flags;

    if (m_bLDragging)
    {
        POINT pt = point;
        ClientToScreen( &pt );
        CImageList::DragMove(pt);
        if ((hitem = HitTest(point, &flags)) != NULL)
        {
            CImageList::DragShowNolock(FALSE);
            SelectDropTarget(hitem);
            m_hItemDrop = hitem;
            CImageList::DragShowNolock(TRUE);
        }
    }
}

void CSessionTreeCtrl::OnLButtonUp(UINT nFlags, CPoint point)
{
    CTreeCtrl::OnLButtonUp(nFlags, point);

    if (m_bLDragging)
    {
        m_bLDragging = FALSE;
        CImageList::DragLeave(this);
        CImageList::EndDrag();
        ReleaseCapture();

        delete m_pDragImage;

        // Remove drop target highlighting
        SelectDropTarget(NULL);

        if( m_hItemDrag == m_hItemDrop )
            return;

        int nUserID = (GetItemData(m_hItemDrag) & ID_ITEMDATA);
        int nChannelID = 0;

        int img, imgSel;
        GetItemImage(m_hItemDrop, img, imgSel);
        if(IsChannel(img))
            nChannelID = (GetItemData(m_hItemDrop) & ID_ITEMDATA);
        else
        {
            int userid = (GetItemData(m_hItemDrop) & ID_ITEMDATA);
            User user = {};
            TT_GetUser(ttInst, userid, &user);
            nChannelID = user.nChannelID;
        }

        CWnd* pWnd = GetParent();
        if(pWnd)
            pWnd->SendMessage(WM_SESSIONTREECTRL_MOVEUSER, nUserID, nChannelID);
    }
}

void CSessionTreeCtrl::SortItem(HTREEITEM hParentItem)
{
    //set up sorting
    TVSORTCB tvs = {};
    // Sort the tree control's items using my
    // callback procedure.
    tvs.hParent = hParentItem;
    tvs.lpfnCompare = SortTree;
    tvs.lParam = (LPARAM) this;
    VERIFY(SortChildrenCB(&tvs));
}

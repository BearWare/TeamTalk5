#if !defined(AFX_SESSIONTREECTRL_H__A952C6AA_A0CD_4E65_94BF_6DC748E035D2__INCLUDED_)
#define AFX_SESSIONTREECTRL_H__A952C6AA_A0CD_4E65_94BF_6DC748E035D2__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SessionTreeCtrl.h : header file
//
#include <queue>
#include <vector>

//picture IDs
#define USER_SILENT 0
#define USER_SILENT_OP 1
#define USER_TALKING 2
#define USER_TALKING_OP 3

#define USER_SILENT_MSG 4
#define USER_SILENT_MSG_OP 5
#define USER_TALKING_MSG 6
#define USER_TALKING_MSG_OP 7

#define USER_SILENT_AWAY 8
#define USER_SILENT_AWAY_OP 9
#define USER_TALKING_AWAY 10
#define USER_TALKING_AWAY_OP 11

#define USER_SILENT_AWAY_MSG 12
#define USER_SILENT_AWAY_MSG_OP 13
#define USER_TALKING_AWAY_MSG 14
#define USER_TALKING_AWAY_MSG_OP 15

#define CHANNEL_CLOSED 16
#define CHANNEL_CLOSED_LOCK 17
#define CHANNEL_CLOSED_MSG 18
#define CHANNEL_CLOSED_LOCK_MSG 19

#define CHANNEL_OPENED 20
#define CHANNEL_OPENED_LOCK 21
#define CHANNEL_OPENED_MSG 22
#define CHANNEL_OPENED_LOCK_MSG 23

#define ROOT_CLOSED 24
#define ROOT_CLOSED_LOCK 25
#define ROOT_CLOSED_MSG 26
#define ROOT_CLOSED_LOCK_MSG 27

#define ROOT_OPENED 28
#define ROOT_OPENED_LOCK 29
#define ROOT_OPENED_MSG 30
#define ROOT_OPENED_LOCK_MSG 31

#define USER_INDEX_START USER_SILENT
#define USER_INDEX_END USER_TALKING_AWAY_MSG_OP
#define CHANNEL_INDEX_START CHANNEL_CLOSED
#define CHANNEL_INDEX_END ROOT_OPENED_LOCK_MSG

#define WM_SESSIONTREECTRL_MOVEUSER WM_USER + 801

//used for item data
#define CHANNEL_ITEMDATA 0x10000
#define USER_ITEMDATA 0x20000
#define ID_ITEMDATA 0xFFFF

/////////////////////////////////////////////////////////////////////////////
// CSessionTreeCtrl window

enum UserFlags
{
    UserTalking,
    UserOperator,
    UserMessage,
    UserAway
};

enum ChannelStates
{
    ChannelOpened,
    ChannelLocked,
    ChannelMessage,
};

class CSessionTreeCtrl : public CTreeCtrl
{
    DECLARE_DYNAMIC(CSessionTreeCtrl)
    // Construction
public:
    CSessionTreeCtrl();

    // Attributes
public:

    // Operations
public:


    // Overrides
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CSessionTreeCtrl)
    //}}AFX_VIRTUAL

    // Implementation
public:
    virtual ~CSessionTreeCtrl();
    virtual void Initialize();

    virtual void ClearChannels();

    virtual void UpdServerName(const ServerProperties& prop);

    virtual int GetMyChannelID();
    virtual HTREEITEM GetUserItem(int nUserID);
    virtual HTREEITEM GetChannelItem(int nChannelID);

    virtual void AddUser(const User& user);
    virtual void UpdateUser(const User& user);
    virtual void RemoveUser(const User& user);
    virtual void AddChannel(const Channel& chan);
    virtual void UpdateChannel(const Channel& chan);
    virtual void RemoveChannel(const Channel& chan);

    void AddUserMessage(int nUserID, const TextMessage& msg);
    BOOL GetLastUserMessage(int nUserID, TextMessage& msg) const;
    void SetUserMessages(int nUserID, const messages_t& msgs);
    messages_t GetUserMessages(int nUserID);

    virtual void SetUserTalking(int nUserID, BOOL bTalking);
    virtual void SetUserMessage(int nUserID, BOOL bMessaged);
    virtual void SetChannelMessage(int nChannelID, BOOL bMessaged);

    virtual int GetSelectedChannel(bool bIncludeUserChan = false);
    virtual int GetSelectedUser();

    BOOL GetChannel(int nChannelID, Channel& outChan);
    BOOL GetUser(int nUserID, User& outUser);

    const channels_t& GetChannels();
    users_t GetUsers(int nChannelID); //nChannelID = 0 -> all users
    users_t GetOperators(int nChannelID);

    virtual void ExpandAll();
    virtual void CollapseAll();

    BOOL IsShowingUserCount();
    void ShowUserCount(BOOL bShow);
    void ShowUsername(BOOL bUsername);

    // Generated message map functions
protected:
    //{{AFX_MSG(CSessionTreeCtrl)
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()
protected:

    void UserItemPlus(HTREEITEM hItem, UserFlags plusState);
    void UserItemMinus(HTREEITEM hItem, UserFlags minusState);
    void ChannelItemPlus(HTREEITEM hItem, ChannelStates plusState);
    void ChannelItemMinus(HTREEITEM hItem, ChannelStates minusState);

    BOOL IsUserOperator(int nUserID, int nChannelID);

    CImageList m_ImageList;
    CBitmap m_bmp;
    channels_t m_channels;
    users_t m_users;
    msgmap_t m_messages;
    BOOL m_bShowUserCount;
    BOOL m_bShowUsername;
    int m_nMyChannel;

    //drag/drop stuff
    CImageList*    m_pDragImage;
    BOOL        m_bLDragging;
    HTREEITEM m_hItemDrop, m_hItemDrag;

    BOOL IsUser(int nImageIndex);
    BOOL IsUserTalking(int nImageIndex);
    BOOL IsUserAway(int nImageIndex);
    BOOL IsUserMessaged(int nImageIndex);
    BOOL IsUserOperator(int nImageIndex);
    BOOL IsChannel(int nImageIndex);
    BOOL IsChannelOpened(int nImageIndex);
    BOOL IsChannelLocked(int nImageIndex);
    BOOL IsChannelMessaged(int nImageIndex);

    virtual void UpdMyChannel(int nChannelID);

    CString GetUserText(int nUserID);
    CString GetChannelText(int nChannelID);

    void CollapseChannel(HTREEITEM hItem);
    void ExpandChannel(HTREEITEM hItem);

    void SortItem(HTREEITEM hParentItem);
public:
    afx_msg void OnTvnItemexpanded(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnTvnBegindrag(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SESSIONTREECTRL_H__A952C6AA_A0CD_4E65_94BF_6DC748E035D2__INCLUDED_)

#pragma once
#include <richedit.h>

// CChatRichEditCtrl

class CChatRichEditCtrl : public CRichEditCtrl
{
    DECLARE_DYNAMIC(CChatRichEditCtrl)

public:
    CChatRichEditCtrl();
    virtual ~CChatRichEditCtrl();
    CString AddMessage(CString szNick, CString szMessage);
    void AddBroadcastMessage(CString szMessage);
    void AddLogMesage(CString szMsg);
    void SetServerInfo(CString szServerName, CString szMOTD);
    void SetChannelInfo(int nChannelID);

    int GetVisibleLinesCount();
    BOOL m_bShowTimeStamp;

protected:
    DECLARE_MESSAGE_MAP()
    CRect m_rectLast;
    void ScrollDown(int nOldLineCount);

    BOOL m_bMinimized;

    CString GetChatTime();

public:
    afx_msg void OnEnLink(NMHDR *pNMHDR, LRESULT *pResult);
    CMenu m_Menu;
    afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
    afx_msg void OnPopupUndo();
    afx_msg void OnPopupSelectall();
    afx_msg void OnPopupCopy();
    afx_msg void OnPopupClearall();
    afx_msg void OnPopupDelete();
    afx_msg void OnPopupPaste();
    afx_msg void OnPopupCut();
    afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnSize(UINT nType, int cx, int cy);
};



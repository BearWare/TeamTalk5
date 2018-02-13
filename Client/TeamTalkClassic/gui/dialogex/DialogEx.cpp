/********************************************************************************************
* MOD-NAME      : DialogEx.cpp
* LONG-NAME     : 
*
* AUTHOR        : Martin Ziacek (Martin.Ziacek@pobox.sk)
* DEPARTMENT    : 
* TELEPHONE     : 
* CREATION-DATE : 22/02/2000 09:15:33
* SP-NO         : 
* FUNCTION      : 
* 
*********************************************************************************************/

#include "stdafx.h"
#include <afxpriv.h>
#include "DialogEx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#undef THIS_FILE
static char THIS_FILE[] = __FILE__;

/////////////////////////////////////////////////////////////////////////////
// CDialogExx

IMPLEMENT_DYNAMIC(CDialogExx, CDialog)

BEGIN_MESSAGE_MAP(CDialogExx, CDialog)
    //{{AFX_MSG_MAP(CDialogExx)
    ON_WM_ENTERIDLE()
    ON_MESSAGE(WM_POPMESSAGESTRING, OnPopMessageString)
    ON_MESSAGE(WM_SETMESSAGESTRING, OnSetMessageString)
    ON_WM_MENUSELECT()     
    ON_UPDATE_COMMAND_UI(ID_INDICATOR_NUM, OnUpdateKeyIndicator)
    ON_UPDATE_COMMAND_UI(ID_VIEW_STATUS_BAR, OnUpdateStatusBarMenu)
    ON_COMMAND(ID_VIEW_STATUS_BAR, OnStatusBarCheck)  
    ON_COMMAND(ID_VIEW_TOOLBAR, OnToolBarCheck)
    ON_UPDATE_COMMAND_UI(ID_VIEW_TOOLBAR, OnUpdateToolBarMenu) 
    ON_WM_MOUSEMOVE()
    ON_WM_INITMENUPOPUP()
    ON_MESSAGE(WM_KICKIDLE, OnKickIdle)
    ON_UPDATE_COMMAND_UI(ID_INDICATOR_CAPS, OnUpdateKeyIndicator)
    ON_UPDATE_COMMAND_UI(ID_INDICATOR_SCRL, OnUpdateKeyIndicator)
    ON_COMMAND(IDOK, OnOK)
    ON_COMMAND(IDCANCEL, OnCancel)
    //}}AFX_MSG_MAP
    ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTW, 0, 0xFFFF, OnToolTipText)
    ON_NOTIFY_EX_RANGE(TTN_NEEDTEXTA, 0, 0xFFFF, OnToolTipText)
END_MESSAGE_MAP()

CDialogExx::CDialogExx()
    : CDialog()
{
    m_bKillItself = TRUE;

    m_bResetPrompt = FALSE;

    m_bToolTips = FALSE;
    m_bStatusBar = FALSE;
    m_bToolBar = FALSE;
}

CDialogExx::CDialogExx(UINT nIDTemplate,CWnd* pParentWnd)
    : CDialog(nIDTemplate, pParentWnd)
{
    m_bKillItself = TRUE;

    m_bResetPrompt = FALSE;

    m_bToolTips = FALSE;
    m_bStatusBar = FALSE;
    m_bToolBar = FALSE;
}

BOOL CDialogExx::InitDialogEx(BOOL bToolTips /*= FALSE*/, BOOL bStatusBar /*= FALSE*/, UINT *pIndicators /*= NULL*/, UINT nIndicators /*= 0*/, UINT uiToolBar /*= 0*/, DWORD dwToolBarStyle /*= TBSTYLE_FLAT | WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC*/)
{
    if (bToolTips) {
        EnableToolTips(TRUE);

        m_bToolTips = TRUE;
    }

    if (bStatusBar)
    {
        if (pIndicators)
        {
            if (!m_CSBCtrlStatus.Create(this) ||
                !m_CSBCtrlStatus.SetIndicators(pIndicators,nIndicators )) {
                TRACE(_T("Failed to create status bar\n"));
                return FALSE;      // fail to create
            }
        }
        else 
        {
            UINT indicators[] = {
                                    ID_SEPARATOR,
                                    ID_INDICATOR_CAPS,
                                    ID_INDICATOR_NUM,
                                    ID_INDICATOR_SCRL,
                                };
            if (!m_CSBCtrlStatus.Create(this) ||
                !m_CSBCtrlStatus.SetIndicators(indicators,4)) {
                TRACE(_T("Failed to create status bar\n"));
                return FALSE;      // fail to create
            }
        }
        OnSetMessageString(AFX_IDS_IDLEMESSAGE);
        m_CSBCtrlStatus.SetPaneInfo(0, m_CSBCtrlStatus.GetItemID(0),SBPS_STRETCH, NULL );
        m_bStatusBar = TRUE;
    }

    if (uiToolBar)
    {
        if (m_CSToolBar.CreateEx(this,dwToolBarStyle))
        {

            m_CSToolBar.LoadToolBar(uiToolBar);
            if (m_bToolTips) m_CSToolBar.EnableToolTips();

            m_bToolBar = TRUE;

        } else {
            TRACE(_T("Failed to create toolbar bar\n"));
            return FALSE;
        }
    }

    if (bStatusBar || uiToolBar) {
        CRect rcClientStart;
        CRect rcClientNow;
        GetClientRect(rcClientStart);
        RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0, reposQuery, rcClientNow);
        
        CPoint ptOffset(rcClientNow.left - rcClientStart.left,rcClientNow.top - rcClientStart.top); 

        CRect  rcChild;                    
        CWnd* pwndChild = GetWindow(GW_CHILD);
        while (pwndChild) {                               
            pwndChild->GetWindowRect(rcChild);
            ScreenToClient(rcChild);
            rcChild.OffsetRect(ptOffset);
            pwndChild->MoveWindow(rcChild, FALSE);
            pwndChild = pwndChild->GetNextWindow();
        }

        CRect rcWindow;
        GetWindowRect(rcWindow);
        rcWindow.right += rcClientStart.Width() - rcClientNow.Width();
        rcWindow.bottom += rcClientStart.Height() - rcClientNow.Height();
        MoveWindow(rcWindow, FALSE);
        
        RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);
    }

    return TRUE;
}

void CDialogExx::PostNcDestroy()
{
    if (!m_bKillItself) {
        CDialog::PostNcDestroy();
    }
}

void CDialogExx::OnCancel() 
{
    if (m_bKillItself) {
        DestroyWindow();
    } else {
        CDialog::OnCancel();
    }
}

void CDialogExx::OnOK() 
{
    if (m_bKillItself) {
        if (!UpdateData(TRUE)) {
            TRACE(_T("UpdateData failed -- modeless dialog terminate\n"));
            return;
        }
        DestroyWindow();
    } else {
        CDialog::OnOK();
    }
}

INT_PTR CDialogExx::DoModal() 
{
    m_bKillItself = FALSE;

    return CDialog::DoModal();
}

void CDialogExx::OnStatusBarCheck() 
{
    m_CSBCtrlStatus.ShowWindow(m_CSBCtrlStatus.IsWindowVisible() ? SW_HIDE : SW_SHOWNA);

    if (m_bStatusBar) {
        CRect crStatusBar;
        CRect rcClientStart;
        CRect rcClientNow;
        CRect rcWindow;

        GetWindowRect(rcWindow);
        m_CSBCtrlStatus.GetWindowRect(crStatusBar);
        ScreenToClient(crStatusBar);
        GetClientRect(rcClientStart);

        RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0, reposQuery, rcClientNow);
        
        if (m_CSBCtrlStatus.IsWindowVisible()) {
            rcWindow.right += rcClientStart.Width() - rcClientNow.Width();
            rcWindow.bottom += crStatusBar.Height();
        } else {
            rcWindow.right += rcClientStart.Width() - rcClientNow.Width();
            rcWindow.bottom -= crStatusBar.Height();
        }
        MoveWindow(rcWindow, TRUE);
    }
}

void CDialogExx::OnUpdateStatusBarMenu(CCmdUI* pCmdUI) 
{
    ASSERT(pCmdUI->m_nID == ID_VIEW_STATUS_BAR);
    pCmdUI->SetCheck(m_CSBCtrlStatus.IsWindowVisible());
}

void CDialogExx::OnUpdateKeyIndicator(CCmdUI* pCmdUI)
{
    UINT nVK;

    switch (pCmdUI->m_nID) {
    case ID_INDICATOR_CAPS:
        nVK = VK_CAPITAL;
        break;
    case ID_INDICATOR_NUM:
        nVK = VK_NUMLOCK;
        break;
    case ID_INDICATOR_SCRL:
        nVK = VK_SCROLL;
        break;
    default:
        TRACE(_T("Warning: OnUpdateKeyIndicator - unknown indicator 0x%04X\n"), pCmdUI->m_nID);
        pCmdUI->ContinueRouting();
        return;
    }

    pCmdUI->Enable(::GetKeyState(nVK) & 1);

    ASSERT(pCmdUI->m_bEnableChanged);
}           

void CDialogExx::OnMenuSelect(UINT nItemID, UINT nFlags, HMENU hSysMenu) 
{
    CDialog::OnMenuSelect(nItemID,nFlags,hSysMenu);

    if (nFlags == 0xFFFF) {
        m_nIDTracking = AFX_IDS_IDLEMESSAGE;
        OnSetMessageString(m_nIDTracking);
        ASSERT(m_nIDTracking == m_nIDLastMessage);
    } else if (nItemID == 0 || nFlags & (MF_SEPARATOR|MF_POPUP|MF_MENUBREAK|MF_MENUBARBREAK)) {
        m_nIDTracking = 0;
    } else if (nItemID >= 0xF000 && nItemID < 0xF1F0) {
        m_nIDTracking = ID_COMMAND_FROM_SC(nItemID);
        ASSERT(m_nIDTracking >= AFX_IDS_SCFIRST && m_nIDTracking < AFX_IDS_SCFIRST + 31);
    } else {
        m_nIDTracking = nItemID;
    }        
}

LRESULT CDialogExx::OnSetMessageString(WPARAM wParam, LPARAM lParam)
{
    UINT    nIDMsg = (UINT)wParam;
    CString strMsg;
    TCHAR szFullText[256];
    
    if (nIDMsg) {
        if (nIDMsg == AFX_IDS_IDLEMESSAGE) {
            strMsg.LoadString(AFX_IDS_IDLEMESSAGE);
            if (m_CSBCtrlStatus.m_hWnd) m_CSBCtrlStatus.SetWindowText(strMsg);
        } else {
            if (strMsg.LoadString(nIDMsg) != 0) {
                CString strTipText;
                AfxLoadString(nIDMsg, szFullText);
                UINT i = 0;
                while (i++ < _tcslen(szFullText)) {
                    if (szFullText[i] == (TCHAR)'\n') {
                        strTipText = CString(szFullText).Left(i);
                        break;
                    }
                }
                if (m_CSBCtrlStatus.m_hWnd) m_CSBCtrlStatus.SetWindowText(strTipText);
            } else {
                TRACE(_T("Warning: no message line prompt for ID %x%04X\n"), nIDMsg);
            }
        }
    } else {
        if (m_CSBCtrlStatus.m_hWnd) m_CSBCtrlStatus.SetWindowText(strMsg);
    }
    
    UINT nIDLast     = m_nIDLastMessage;
    m_nIDLastMessage = nIDMsg;
    m_nIDTracking    = nIDMsg;
    return nIDLast;  
    
}

LRESULT CDialogExx::OnPopMessageString(WPARAM wParam, LPARAM lParam)
{
    if (m_nFlags & WF_NOPOPMSG) {
        return 0;
    }

    return SendMessage(WM_SETMESSAGESTRING, wParam, lParam);
}

void CDialogExx::OnEnterIdle(UINT nWhy, CWnd* pWho) 
{
    CDialog::OnEnterIdle(nWhy, pWho);

    if (AfxGetApp()->m_pMainWnd != this) {
        if (m_bStatusBar) m_CSBCtrlStatus.SendMessage(WM_IDLEUPDATECMDUI);
        if (m_bToolBar) m_CSToolBar.SendMessage(WM_IDLEUPDATECMDUI,TRUE);
    }

    if (nWhy != MSGF_MENU || m_nIDTracking == m_nIDLastMessage) {
        return;
    }
        
    OnSetMessageString(m_nIDTracking);
    ASSERT(m_nIDTracking == m_nIDLastMessage);            
}

void CDialogExx::OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu) 
{
    CDialog::OnInitMenuPopup(pPopupMenu,nIndex,bSysMenu);

    if (!bSysMenu) {
        ASSERT(pPopupMenu != NULL);
        
        CCmdUI state;        
        state.m_pMenu = pPopupMenu;
        ASSERT(state.m_pOther == NULL);
        
        state.m_nIndexMax = pPopupMenu->GetMenuItemCount();
        for (state.m_nIndex = 0; state.m_nIndex < state.m_nIndexMax; state.m_nIndex++) {
            state.m_nID = pPopupMenu->GetMenuItemID(state.m_nIndex);
            if (state.m_nID == 0)
                continue;
                
            ASSERT(state.m_pOther == NULL);
            ASSERT(state.m_pMenu != NULL);
            if (state.m_nID == (UINT)-1) {
                state.m_pSubMenu = pPopupMenu->GetSubMenu(state.m_nIndex);
                if (state.m_pSubMenu == NULL ||
                    (state.m_nID = state.m_pSubMenu->GetMenuItemID(0)) == 0 ||
                    state.m_nID == (UINT)-1) {                                 
                    continue;
                }
                state.DoUpdate(this, FALSE);
            } else {
                state.m_pSubMenu = NULL;
                state.DoUpdate(this, state.m_nID < 0xF000);
            }
        }
    }
}

void CDialogExx::OnToolBarCheck() 
{
    m_CSToolBar.ShowWindow(m_CSToolBar.IsWindowVisible() ? SW_HIDE : SW_SHOWNA);

    if (m_bToolBar) {
        CRect crToolBar;
        CRect rcClientStart;
        CRect rcClientNow;
        CRect rcWindow;

        GetWindowRect(rcWindow);
        m_CSToolBar.GetWindowRect(crToolBar);
        ScreenToClient(crToolBar);
        GetClientRect(rcClientStart);

        RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0, reposQuery, rcClientNow);
        
        CPoint ptOffset(
            rcClientNow.left - rcClientStart.left,
            rcClientNow.top - rcClientStart.top + 
            m_CSToolBar.IsWindowVisible() ? crToolBar.Height() : -1 * crToolBar.Height()); 

        CRect  rcChild;                    
        CWnd* pwndChild = GetWindow(GW_CHILD);
        while (pwndChild) {                               
            pwndChild->GetWindowRect(rcChild);
            ScreenToClient(rcChild);
            rcChild.OffsetRect(ptOffset);
            pwndChild->MoveWindow(rcChild, TRUE);
            pwndChild = pwndChild->GetNextWindow();
        }

        if (m_CSToolBar.IsWindowVisible()) {
            rcWindow.right += rcClientStart.Width() - rcClientNow.Width();
            rcWindow.bottom += crToolBar.Height();
        } else {
            rcWindow.right += rcClientStart.Width() - rcClientNow.Width();
            rcWindow.bottom -= crToolBar.Height();
        }
        MoveWindow(rcWindow, TRUE);
        
        RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);
    }
}

BOOL CDialogExx::OnToolTipText(UINT, NMHDR* pNMHDR, LRESULT* pResult)
{
    ASSERT(pNMHDR->code == TTN_NEEDTEXTA || pNMHDR->code == TTN_NEEDTEXTW);

    TOOLTIPTEXTA* pTTTA = (TOOLTIPTEXTA*)pNMHDR;
    TOOLTIPTEXTW* pTTTW = (TOOLTIPTEXTW*)pNMHDR;
    TCHAR szFullText[256];
    CString strTipText;
    CString strPromtpText;
    UINT_PTR nID = pNMHDR->idFrom;

    if (pNMHDR->code == TTN_NEEDTEXTA && (pTTTA->uFlags & TTF_IDISHWND) ||
        pNMHDR->code == TTN_NEEDTEXTW && (pTTTW->uFlags & TTF_IDISHWND)) {
        nID = ::GetDlgCtrlID((HWND)nID);
    }

    if ((nID != 0) && (nID != ID_VIEW_STATUS_BAR)) {
        AfxLoadString(UINT(nID), szFullText);
        AfxExtractSubString(strTipText, szFullText, 1, '\n');
        AfxExtractSubString(strPromtpText, szFullText, 0, '\n');

        if (!strPromtpText.IsEmpty()) {
            if (::IsWindow(m_CSBCtrlStatus.m_hWnd)) m_CSBCtrlStatus.SetWindowText(strPromtpText);
            m_bResetPrompt = TRUE;
        }
    }
#ifndef _UNICODE
    if (pNMHDR->code == TTN_NEEDTEXTA)
        lstrcpyn(pTTTA->szText, strTipText,(sizeof(pTTTA->szText)/sizeof(pTTTA->szText[0])));
    else
        _mbstowcsz(pTTTW->szText, strTipText,(sizeof(pTTTW->szText)/sizeof(pTTTW->szText[0])));
#else
    if (pNMHDR->code == TTN_NEEDTEXTA)
        _wcstombsz(pTTTA->szText, strTipText,(sizeof(pTTTA->szText)/sizeof(pTTTA->szText[0])));
    else
        lstrcpyn(pTTTW->szText, strTipText,(sizeof(pTTTW->szText)/sizeof(pTTTW->szText[0])));
#endif
    *pResult = 0;

    ::SetWindowPos(pNMHDR->hwndFrom, HWND_TOP, 0, 0, 0, 0,SWP_NOACTIVATE|SWP_NOSIZE|SWP_NOMOVE);

    return TRUE;
}

void CDialogExx::OnUpdateToolBarMenu(CCmdUI* pCmdUI) 
{
    ASSERT(pCmdUI->m_nID == ID_VIEW_TOOLBAR);
    pCmdUI->SetCheck(m_CSToolBar.IsWindowVisible());
}

void CDialogExx::OnMouseMove(UINT nFlags, CPoint point) 
{
    if (m_bResetPrompt) {
        OnSetMessageString(AFX_IDS_IDLEMESSAGE);
        m_bResetPrompt = FALSE;
    }

    CDialog::OnMouseMove(nFlags, point);
}

LRESULT CDialogExx::OnKickIdle(WPARAM, LPARAM lCount)
{
    if (m_bStatusBar) m_CSBCtrlStatus.SendMessage(WM_IDLEUPDATECMDUI);
    if (m_bToolBar) m_CSToolBar.SendMessage(WM_IDLEUPDATECMDUI,TRUE);

    return (lCount <= 2);
}

//BOOL CDialogExx::OnInitDialog()
//{
//    CDialog::OnInitDialog();
//
//    return TRUE;  // return TRUE unless you set the focus to a control
//    // EXCEPTION: OCX Property Pages should return FALSE
//}

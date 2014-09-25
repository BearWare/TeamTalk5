/////////////////////////////////////////////////////////////////////////////
// XPTabCtrl.h : header file
/////////////////////////////////////////////////////////////////////////////
//    Compiler:    Visual C++, Service Pack 5, Windows NT 
//    Tested on:    Windows NT, Windows XP
//
//    Created:    25/February/2004
//    Updated:    
//
//    Author:        Adi DEDIC
//  e-mail:        adi_dedic@hotmail.com
//  www:        http://web.onetel.net.uk/~adidedic/
//
//    Disclaimer
//    ----------
//    THIS SOFTWARE AND THE ACCOMPANYING FILES ARE DISTRIBUTED "AS IS" AND WITHOUT
//    ANY WARRANTIES WHETHER EXPRESSED OR IMPLIED. NO REPONSIBILITIES FOR POSSIBLE
//    DAMAGES OR EVEN FUNCTIONALITY CAN BE TAKEN. THE USER MUST ASSUME THE ENTIRE
//    RISK OF USING THIS SOFTWARE.
//
//    Terms of use
//    ------------
//    THIS SOFTWARE IS FREE FOR PERSONAL USE OR FREEWARE APPLICATIONS.
//    IF YOU USE THIS SOFTWARE IN COMMERCIAL OR SHAREWARE APPLICATIONS YOU
//    ARE GENTLY ASKED TO DONATE £1 (ONE GB PUND) TO THE AUTHOR
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_XPTABCTRL_H__B39AA2D4_684D_11D8_97C8_0002A522F13C__INCLUDED_)
#define AFX_XPTABCTRL_H__B39AA2D4_684D_11D8_97C8_0002A522F13C__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <vector>

/////////////////////////////////////////////////////////////////////////////
enum ETabOrientation
{    
    e_tabTop=0,
    e_tabBottom,
    e_tabLeft,
    e_tabRight,
    e_tabNOF,
};
/////////////////////////////////////////////////////////////////////////////
// CXPTabCtrl window
class CXPTabCtrl : public CTabCtrl
{
    DECLARE_DYNAMIC(CXPTabCtrl)
    ETabOrientation    m_eTabOrientation;    // current tab orientation
    CImageList        m_ilTabs;             // possible tab image list
    BOOL            m_bTabExtended;        // Use extended Themes functionalities
    int                m_ixSelOld;            // used to redraw deselected tab background
// Construction
public:
    CXPTabCtrl();
// Operations
public:
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CXPTabCtrl)
    //}}AFX_VIRTUAL
    void SetExtended(BOOL bTabExtended) { m_bTabExtended=bTabExtended; }
    void InitImageList(UINT nBitmapID);
    void AddTab(CWnd* pWnd, CString szCaption, int nImage);
    void SetOrientation(ETabOrientation eTabOrientation);

    void ResizeItems();
    // Generated message map functions
protected:
    //{{AFX_MSG(CXPTabCtrl)
    afx_msg void OnPaint();
    afx_msg void OnTabSelChanging(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnTabSelChanged (NMHDR* pNMHDR, LRESULT* pResult);
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

    void DrawThemesXpTabItem(CDC* pDC, int ixItem, const CRect& rcItem, UINT uiFlag);
    void DrawTabItem(CDC* pDC, int ixItem, const CRect& rcItemC, UINT uiFlags);
    BOOL IsExtendedTabTheamedXP();

    std::vector<CWnd*> m_vecTabs;
public:
    afx_msg void OnSize(UINT nType, int cx, int cy);
};
/////////////////////////////////////////////////////////////////////////////
// Helper functions
HRESULT DrawThemesPart(HDC hDC, int iPartId, int iStateId, LPCSTR uiPartNameID, LPRECT prcRcBx);
BOOL    IsThemeActiveXP();
int        DWordAlign(int n);
template <class T> void SwapVars(T& a,T& b) { T t=a; a=b; b=t; }
/////////////////////////////////////////////////////////////////////////////
//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.
#endif // !defined(AFX_XPTABCTRL_H__B39AA2D4_684D_11D8_97C8_0002A522F13C__INCLUDED_)

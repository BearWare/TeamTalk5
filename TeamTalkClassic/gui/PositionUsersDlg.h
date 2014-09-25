#pragma once
#include "afxwin.h"
#include "roombox.h"
#include "UserButton.h"
#include <vector>

// CPositionUsersDlg dialog

class CPositionUsersDlg : public CDialog
{
    DECLARE_DYNAMIC(CPositionUsersDlg)

public:
    CPositionUsersDlg(const users_t& users, CWnd* pParent = NULL);   // standard constructor
    virtual ~CPositionUsersDlg();
    void PositionUsers();
    void PositionButton(CUserButton* pBtn, int nUserID);
    void MoveButton(CUserButton* pBtn, int x, int y);

// Dialog Data
    enum { IDD = IDD_DIALOG_POSITION_USERS };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    afx_msg LRESULT OnUserButtonDragged(WPARAM wParam, LPARAM lParam);
    DECLARE_MESSAGE_MAP()

    typedef std::map<int, CUserButton*> map_userbtn_t;
    map_userbtn_t m_mapUserBtn;        // userid/btn
    void UpdatePosition(int nUserID, CUserButton* pBtn);
    users_t m_users;

public:
    CButton m_wndOK;
    CRoomBox m_wndRoom;
    virtual BOOL OnInitDialog();
    CStatic m_wndCoordinates;
    afx_msg void OnBnClickedButtonDefault();
    virtual BOOL PreTranslateMessage(MSG* pMsg);
  BOOL m_bPositionUsers;
  afx_msg void OnBnClickedCheckPositioning();
  CButton m_wndPositionBtn;
};

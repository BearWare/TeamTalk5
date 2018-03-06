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

#pragma once

#define WM_USERDESKTOPDLG_CLOSED WM_USER + 952
#define WM_USERDESKTOPDLG_ENDED WM_USER + 953

// CUserDesktopDlg dialog

class CUserDesktopDlg : public CDialog
{
	DECLARE_DYNAMIC(CUserDesktopDlg)

public:
	CUserDesktopDlg(int nUserID, CWnd* pParent = NULL);   // standard constructor
	virtual ~CUserDesktopDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_USERDESKTOP };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
    virtual void OnOK();
    virtual void OnCancel();

private:
    int m_nUserID;
    void ResizeToDesktopWindow();
    DesktopWindow* m_desktopwnd;

    void MenuCommand(UINT uCmd);

    void QueueMouseEvent(const CPoint& point, UINT32 uKeyCode, DesktopKeyState keystate);
    void QueueKeyboardEvent(UINT32 uKeyCode, DesktopKeyState keystate);
    void TranslateAndQueue(const DesktopInput& input);
    BOOL m_bRequestedAccess;
    std::vector<DesktopInput> m_SendInputQueue;

public:
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    virtual BOOL OnInitDialog();
    afx_msg void OnClose();
    afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnMButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
    virtual BOOL PreTranslateMessage(MSG* pMsg);
};

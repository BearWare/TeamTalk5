#pragma once
#include "afxcmn.h"
#include "afxwin.h"

// CUserVolumeDlg dialog

class CUserVolumeDlg : public CDialog
{
    DECLARE_DYNAMIC(CUserVolumeDlg)

public:
    CUserVolumeDlg(const User& user, CWnd* pParent = NULL);   // standard constructor
    virtual ~CUserVolumeDlg();

// Dialog Data
    enum { IDD = IDD_DIALOG_USERVOLUME };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    DECLARE_MESSAGE_MAP()

    User m_user;

    void UpdateProperties();
public:
    afx_msg void OnNMCustomdrawSliderUservol(NMHDR *pNMHDR, LRESULT *pResult);
    virtual BOOL OnInitDialog();
    CSliderCtrl m_wndMediaFileVol;
    CSliderCtrl m_wndVoiceVol;
    CButton m_wndMediaFileMuteLeft;
    CButton m_wndMediaFileMuteRight;
    CButton m_wndVoiceMuteLeft;
    CButton m_wndVoiceMuteRight;
    afx_msg void OnBnClickedCheckMuterightvoice();
    afx_msg void OnBnClickedCheckMuteleftvoice();
    afx_msg void OnBnClickedCheckMuteleftmediafile();
    afx_msg void OnBnClickedCheckMuterightmediafile();
    afx_msg void OnNMCustomdrawSliderVoicevol(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnNMCustomdrawSliderMediafilevol(NMHDR *pNMHDR, LRESULT *pResult);
};

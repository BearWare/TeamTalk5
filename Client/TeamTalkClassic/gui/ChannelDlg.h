#pragma once
#include "afxwin.h"
#include "afxcoll.h"
#include "afxcmn.h"


// CCreateChannelDlg dialog

class CChannelDlg : public CDialog
{
    DECLARE_DYNAMIC(CChannelDlg)

public:
    enum ChannelDlgType { CREATE_CHANNEL, UPDATE_CHANNEL, READONLY_CHANNEL};
    CChannelDlg(CChannelDlg::ChannelDlgType dlgType, CWnd* pParent = NULL);   // standard constructor
    virtual ~CChannelDlg();

    AudioCodec m_codec;

    void DisplayCodecControls(Codec nCodec);
    void GetBitrateLimits(Codec nCodec, int& nMinBps, 
                          int& nMaxBps, int& nDefaultBps);
    void UpdateCodec();
    // Dialog Data
    enum { IDD = IDD_DIALOG_CHANNEL };

protected:
    afx_msg void OnNMCustomdrawSliderQuality(NMHDR *pNMHDR, LRESULT *pResult);
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual void OnOK();
    virtual BOOL OnInitDialog();
    afx_msg void OnCbnSelchangeComboAudiochannels();
    afx_msg void OnNMCustomdrawSliderGainlevel(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnBnClickedCheckForceagc();
    afx_msg void OnCbnSelchangeComboCodec();
    afx_msg void OnEnChangeEditBitrate();
    afx_msg void OnEnChangeEditMaxbitrate();
    afx_msg void OnBnClickedCheckDtx();

    DECLARE_MESSAGE_MAP()

    Codec GetCurrentCodec();
    void ShowCurrentCodec();
    void DelayBounds();

public:
    ChannelDlgType m_nType;
    CString m_szChannelname;
    CString m_szChannelPassword;
    CString m_szChannelTopic;
    CString m_szOpPasswd;
    BOOL m_bStaticChannel;
    INT64 m_nDiskQuota;
    UINT m_nMaxUsers;
    BOOL m_bSingleTxChannel;
    BOOL m_bClassRoom;
    BOOL m_bEnableAGC;
    int m_nGainLevel;

    CStatic m_wndGrp;
    CEdit m_wndDiskQuota;
    CEdit m_wndMaxUsers;
    CEdit m_wndOpPasswd;
    CButton m_btnStaticChannel;
    CButton m_btnSingleChannel;
    CComboBox m_wndAudioChannels;
    CButton m_btnClassRoom;
    CButton m_btnForceAGC;
    CSliderCtrl m_wndGainSlider;
    CSliderCtrl m_wndQuality;
    CEdit m_wndPassword;
    CEdit m_wndChannelName;
    CEdit m_wndChanTopic;
    CStatic m_wndGainLabel;
    CComboBox m_wndCodec;
    CEdit m_wndBitrate;
    CEdit m_wndMaxBitrate;
    CButton m_wndDtx;
    CStatic m_wndSRLabel;
    CStatic m_wndACLabel;
    CStatic m_wndQualityLabel;
    CStatic m_wndBpsLabel;
    CStatic m_wndMaxBpsLabel;
    CStatic m_wndBpsLabel1;
    CStatic m_wndBpsLabel2;
    afx_msg void OnEnKillfocusEditBitrate();
    afx_msg void OnEnKillfocusEditMaxbitrate();
    CEdit m_wndTxDelay;
    int m_nTxDelay;
    CSpinButtonCtrl m_wndTxDelaySpin;
    afx_msg void OnDeltaposSpinDelay(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnEnKillfocusEditTxdelay();
    CStatic m_wndTxDelayLabel;
    CStatic m_wndTxDelayLabel1;
    BOOL m_bOpRecvOnly;
    CButton m_wndOpRecvOnly;
    CComboBox m_wndSampleRate;
    CComboBox m_wndCodecApp;
    CStatic m_wndAppLabel;
    CButton m_wndNoVoiceAct;
    BOOL m_bNoVoiceAct;
    CButton m_wndNoRecord;
    BOOL m_bNoRecord;
};

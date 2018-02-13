#pragma once
#include "afxwin.h"


// CMoveToChannelDlg dialog

class CMoveToChannelDlg : public CDialog
{
	DECLARE_DYNAMIC(CMoveToChannelDlg)

public:
	CMoveToChannelDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CMoveToChannelDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_MOVETOCHANNEL };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
    CString m_szChannel;
    CStringList m_Channels;
    virtual BOOL OnInitDialog();
    CComboBox m_wndChannels;
};

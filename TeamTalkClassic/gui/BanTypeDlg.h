#pragma once
#include "afxwin.h"


// CBanTypeDlg dialog

class CBanTypeDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CBanTypeDlg)

public:
	CBanTypeDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CBanTypeDlg();

// Dialog Data
	enum { IDD = IDD_DIALOG_BANTYPE };
    BanTypes m_uBanTypes = BANTYPE_NONE;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
    CComboBox m_wndBanType;
    virtual BOOL OnInitDialog();
    virtual void OnOK();
};

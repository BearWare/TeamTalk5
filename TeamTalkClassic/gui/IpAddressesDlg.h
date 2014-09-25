#pragma once

// CIpAddressesDlg dialog

class CIpAddressesDlg : public CDialog
{
    DECLARE_DYNAMIC(CIpAddressesDlg)

public:
    CIpAddressesDlg(CWnd* pParent = NULL);   // standard constructor
    virtual ~CIpAddressesDlg();

    CString GetIPAddressDescription();
    int GetIPs( CStringArray &arrIPS );
    UINT CStringIPToUINT(CString szIPAddress);


// Dialog Data
    enum { IDD = IDD_DIALOG_IPADDRESSES };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    DECLARE_MESSAGE_MAP()
public:
    virtual BOOL OnInitDialog();
    CString m_szIPDescription;
};

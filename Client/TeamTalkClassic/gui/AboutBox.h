#pragma once
#include "afxwin.h"

// CAboutBox dialog

class CAboutBox : public CDialog
{
    DECLARE_DYNAMIC(CAboutBox)

public:
    CAboutBox(CWnd* pParent = NULL);   // standard constructor
    virtual ~CAboutBox();

// Dialog Data
    enum { IDD = IDD_DIALOG_ABOUTBOX };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    DECLARE_MESSAGE_MAP()
public:
    CString m_strProgramTitle;
    CString m_strVersion;
    CStatic m_wndProgramTitle;
    CStatic m_wndVersion;
    virtual BOOL OnInitDialog();
    CFont font1;
    CFont font2;

    CStatic m_wndAuthor;
    CStatic m_wndCopyright;
    CStatic m_wndImage;
    CStatic m_wndCompiled;
    CString m_szCompiled;
};

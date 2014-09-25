#pragma once


// CMyTab

class CMyTab : public CDialog
{
    DECLARE_DYNAMIC(CMyTab)

public:
    CMyTab(UINT nIDTemplate, CWnd* pParent = NULL);
    virtual ~CMyTab();

protected:
    DECLARE_MESSAGE_MAP()
public:
    virtual BOOL PreTranslateMessage(MSG* pMsg);
protected:
    virtual void OnOK();
    virtual void OnCancel();
public:
};



#pragma once

#define WM_USERVIDEODLG_CLOSED WM_USER + 950
#define WM_USERVIDEODLG_ENDED WM_USER + 951
// CUserVideoDlg dialog

enum
{
    VIDEODLG_UPDATE_TIMERID = 1,
    VIDEODLG_LIVENESS_TIMERID = 2,
};

enum
{
    VIDEOTYPE_CAPTURE   = 0x10000,
    VIDEOTYPE_MEDIAFILE      = 0x20000,

    VIDEOTYPE_MASK      = 0xF0000,
    VIDEOTYPE_USERMASK     = 0x0FFFF
};

class CUserVideoDlg : public CDialog
{
	DECLARE_DYNAMIC(CUserVideoDlg)

public:
	CUserVideoDlg(int nUserID, CWnd* pParent = NULL);   // standard constructor
	CUserVideoDlg(int nUserID, int nTimedUpdateMSec, CWnd* pParent = NULL);   // standard constructor
	virtual ~CUserVideoDlg();

    void NewVideoFrame();
// Dialog Data
	enum { IDD = IDD_DIALOG_USERVIDEO };
private:

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //afx_msg LRESULT OnVideoFrame(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

    int m_nUserID;
    int m_nTimedUpdateMSec;
    int m_nDeadCounter;
    VideoFrame* m_VideoFrame;
public:
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
protected:
    virtual void OnOK();
    virtual void OnCancel();
public:
    afx_msg void OnTimer(UINT_PTR nIDEvent);
    virtual BOOL OnInitDialog();
protected:
    virtual void PostNcDestroy();
public:
    afx_msg void OnClose();
};

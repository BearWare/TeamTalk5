// UserVideoDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "UserVideoDlg.h"

extern TTInstance* ttInst;

// CUserVideoDlg dialog

IMPLEMENT_DYNAMIC(CUserVideoDlg, CDialog)

CUserVideoDlg::CUserVideoDlg(int nUserID, CWnd* pParent /*=NULL*/)
	: CDialog(CUserVideoDlg::IDD, pParent)
    , m_nUserID(nUserID)
    , m_nTimedUpdateMSec(0)
    , m_nDeadCounter(0)
    , m_VideoFrame()
{

}

CUserVideoDlg::CUserVideoDlg(int nUserID, int nTimedUpdateMSec, CWnd* pParent /*=NULL*/)
	: CDialog(CUserVideoDlg::IDD, pParent)
    , m_nUserID(nUserID)
    , m_nTimedUpdateMSec(nTimedUpdateMSec)
    , m_nDeadCounter(0)
    , m_VideoFrame(NULL)
{

}

CUserVideoDlg::~CUserVideoDlg()
{
    switch(m_nUserID & VIDEOTYPE_MASK)
    {
    case VIDEOTYPE_CAPTURE :
        TT_ReleaseUserVideoCaptureFrame(ttInst, m_VideoFrame);
        break;
    case VIDEOTYPE_MEDIAFILE :
        TT_ReleaseUserMediaVideoFrame(ttInst, m_VideoFrame);
        break;
    }
}

void CUserVideoDlg::NewVideoFrame()
{
    Invalidate();
    m_nDeadCounter = 0;
}

void CUserVideoDlg::ResizeWindow(const VideoFrame& frm)
{
    CRect rWnd, rClient;
    GetWindowRect(rWnd);
    GetClientRect(rClient);

    int nDiffW = frm.nWidth - rClient.Width();
    int nDiffH = frm.nHeight - rClient.Height();
    rWnd.right += nDiffW;
    rWnd.bottom += nDiffH;
        
    MoveWindow(rWnd);

    GetClientRect(rClient);
    ASSERT(rClient.Width() == frm.nWidth);
    ASSERT(rClient.Height() == frm.nHeight);
}

void CUserVideoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CUserVideoDlg, CDialog)
    ON_WM_ERASEBKGND()
    ON_WM_TIMER()
    ON_WM_CLOSE()
END_MESSAGE_MAP()


// CUserVideoDlg message handlers

BOOL CUserVideoDlg::OnEraseBkgnd(CDC* pDC)
{
    CRect rect;
    GetClientRect(rect);

    BOOL bInitialFrame = m_VideoFrame == NULL;

    VideoFrame* pTmpFrame = NULL;
    switch(m_nUserID & VIDEOTYPE_MASK)
    {
    case VIDEOTYPE_CAPTURE :
    {
        while((pTmpFrame = TT_AcquireUserVideoCaptureFrame(ttInst, (m_nUserID & VIDEOTYPE_USERMASK))))
        {
            TT_ReleaseUserVideoCaptureFrame(ttInst, m_VideoFrame);
            m_VideoFrame = pTmpFrame;
        }
    }
    break;
    case VIDEOTYPE_MEDIAFILE :
        while((pTmpFrame = TT_AcquireUserMediaVideoFrame(ttInst, (m_nUserID & VIDEOTYPE_USERMASK))))
        {
            TT_ReleaseUserMediaVideoFrame(ttInst, m_VideoFrame);
            m_VideoFrame = pTmpFrame;
        }
        break;
    }



    if(m_VideoFrame)
    {
        if(bInitialFrame)
            ResizeWindow(*m_VideoFrame);

        TT_PaintVideoFrame(pDC->m_hDC, rect.left, rect.top, rect.Width(),
                           rect.Height(), m_VideoFrame);
    }
    //while(TT_PaintVideoFrameEx(ttInst, m_nUserID, pDC->m_hDC, rect.left, 
    //                           rect.top, rect.Width(), rect.Height(),
    //                           150, 100, 100, 100));

    //return CDialog::OnEraseBkgnd(pDC);
    return FALSE;
}

void CUserVideoDlg::OnOK()
{
}

void CUserVideoDlg::OnCancel()
{
    CDialog::OnCancel();
}

void CUserVideoDlg::OnTimer(UINT_PTR nIDEvent)
{
    CDialog::OnTimer(nIDEvent);

    switch(nIDEvent)
    {
    case VIDEODLG_UPDATE_TIMERID : Invalidate(); break;
    case VIDEODLG_LIVENESS_TIMERID :
        if(m_nDeadCounter++>5)
        {
            KillTimer(VIDEODLG_LIVENESS_TIMERID);
            AfxGetMainWnd()->PostMessage(WM_USERVIDEODLG_ENDED, m_nUserID);
        }
        break;
    }
}

BOOL CUserVideoDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    TRANSLATE(*this, IDD);

    if(m_nTimedUpdateMSec)
        SetTimer(VIDEODLG_UPDATE_TIMERID, m_nTimedUpdateMSec, NULL);

    if(m_nUserID == 0)
        SetWindowText(_T("Local Video"));
    else
    {
        User user = {0};
        TT_GetUser(ttInst, (m_nUserID & VIDEOTYPE_USERMASK), &user);
        SetWindowText(CString(_T("Video - ")) + user.szNickname);
    }

    switch(m_nUserID & VIDEOTYPE_MASK)
    {
    case VIDEOTYPE_CAPTURE :
        m_VideoFrame = TT_AcquireUserVideoCaptureFrame(ttInst, (m_nUserID & VIDEOTYPE_USERMASK));
        break;
    case VIDEOTYPE_MEDIAFILE :
        m_VideoFrame = TT_AcquireUserMediaVideoFrame(ttInst, (m_nUserID & VIDEOTYPE_USERMASK));
        break;
    }

    if(m_VideoFrame && m_VideoFrame->nWidth && m_VideoFrame->nHeight)
    {
        ResizeWindow(*m_VideoFrame);
    }

    SetTimer(VIDEODLG_LIVENESS_TIMERID, 1000, NULL);

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

//LRESULT CUserVideoDlg::OnVideoFrame(WPARAM wParam, LPARAM lParam)
//{
//    Invalidate();
//    return TRUE;
//}

void CUserVideoDlg::PostNcDestroy()
{
    CDialog::PostNcDestroy();
}

void CUserVideoDlg::OnClose()
{
    CDialog::OnClose();
    AfxGetMainWnd()->PostMessage(WM_USERVIDEODLG_CLOSED, m_nUserID);
}

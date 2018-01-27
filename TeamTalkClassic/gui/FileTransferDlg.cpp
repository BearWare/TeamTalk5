// FileTransferDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "FileTransferDlg.h"
#include ".\filetransferdlg.h"

extern TTInstance* ttInst;

// CFileTransferDlg dialog

IMPLEMENT_DYNAMIC(CFileTransferDlg, CDialog)

CFileTransferDlg::CFileTransferDlg(int nTransferID, BOOL bAdmin, CWnd* pParent/* = NULL*/)
: CDialog(CFileTransferDlg::IDD, pParent)
, m_nTransferID(nTransferID)
, m_bCompleted(FALSE)
, m_nLastTransferred(0)
, m_nStartTime(0)
, m_bAutoClose(FALSE)
{
}

CFileTransferDlg::~CFileTransferDlg()
{
}

void CFileTransferDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_STATIC_FILENAME, m_wndFilename);
    DDX_Control(pDX, IDC_STATIC_SFILESIZE, m_wndFileSize);
    DDX_Control(pDX, IDC_STATIC_TRANSFERRED, m_wndTransferred);
    DDX_Control(pDX, IDC_PROGRESS1, m_wndProgressBar);
    DDX_Control(pDX, IDCANCEL, m_wndCancel);
    DDX_Control(pDX, IDC_STATIC_STHROUGHPUT, m_wndThroughput);
    DDX_Check(pDX, IDC_CHECK_AUTOCLOSE, m_bAutoClose);
    DDX_Control(pDX, IDC_CHECK_AUTOCLOSE, m_wndAutoClose);
    DDX_Control(pDX, IDC_BUTTON_OPEN, m_wndOpen);
}


BEGIN_MESSAGE_MAP(CFileTransferDlg, CDialog)
    ON_WM_TIMER()
    ON_BN_CLICKED(IDC_CHECK_AUTOCLOSE, OnBnClickedCheckAutoclose)
    ON_BN_CLICKED(IDC_BUTTON_OPEN, OnBnClickedButtonOpen)
END_MESSAGE_MAP()


// CFileTransferDlg message handlers

void CFileTransferDlg::OnTimer(UINT_PTR nIDEvent)
{
    CDialog::OnTimer(nIDEvent);
    if(!UpdateTransferInfo())
        KillTimer(nIDEvent);
}

void CFileTransferDlg::PostNcDestroy()
{
    CDialog::PostNcDestroy();
    delete this;
}

void CFileTransferDlg::OnCancel()
{
    CWnd* pParent = GetParent();
    if(pParent)
        pParent->SendMessage(WM_FILETRANSFERDLG_CLOSED, m_nTransferID, 0);

    if(!m_bCompleted)
        TT_CancelFileTransfer(ttInst, m_nTransferID);

    CDialog::OnCancel();
    DestroyWindow();
}

BOOL CFileTransferDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    SetIcon(AfxGetApp()->LoadIcon(IDR_MAINFRAME),TRUE);

    TRANSLATE(*this, IDD);

    m_wndProgressBar.SetRange(0,100);
    m_wndThroughput.SetWindowText(_T(""));
    m_wndTransferred.SetWindowText(_T(""));
    m_wndFilename.SetWindowText(_T(""));
    m_wndFileSize.SetWindowText(_T(""));

    FileTransfer transfer;
    if(TT_GetFileTransferInfo(ttInst, m_nTransferID, &transfer))
    {
        CString szTotalSize;
        if(m_nTotalSize>1024)
            szTotalSize.Format(_T("%I64d KBytes"), transfer.nFileSize/(INT64)1024);
        else
            szTotalSize.Format(_T("%I64d bytes"), transfer.nFileSize);
        SetWindowText(transfer.szRemoteFileName);
        m_nTotalSize = transfer.nFileSize;
        m_wndFilename.SetWindowText(transfer.szRemoteFileName);
        m_wndFileSize.SetWindowText(szTotalSize);
        m_szLocalFilename = transfer.szLocalFilePath;
    }

    m_nStartTime = ::GetTickCount();

    UpdateTransferInfo();

    SetTimer(1, 1000, NULL);

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CFileTransferDlg::UpdateTransferInfo()
{
    FileTransfer transfer;
    if(TT_GetFileTransferInfo(ttInst, m_nTransferID, &transfer) && !m_bCompleted)
    {
        CString szTransferred;
        szTransferred.Format(_T("%I64d"), transfer.nTransferred);
        
        float percent = 0.0f;
        if(m_nTotalSize>0)
            percent = 100.0f * (float)((double)transfer.nTransferred / (double)m_nTotalSize);
        else
            percent = 100.0f;

        CString szProgress;
        szProgress.Format(_T("%I64d/%I64d - %.2f %%"), transfer.nTransferred, m_nTotalSize, percent);
        m_wndTransferred.SetWindowText(szProgress);
        m_wndProgressBar.SetPos(int(percent));

        DWORD nTotalTime = (::GetTickCount() - m_nStartTime) / 1000;
        CString szThroughput;
        if(nTotalTime>0)
        {
            INT64 nBytesPerSec = transfer.nTransferred/nTotalTime;
            INT64 nBytesLastSec = transfer.nTransferred-m_nLastTransferred;
            if(nBytesPerSec/1024>0 && nBytesLastSec/1024>0)
                szThroughput.Format(_T("%I64d KB/sec, %I64d KB last sec."), nBytesPerSec/1024, nBytesLastSec/1024);
            else
                szThroughput.Format(_T("%I64d bytes/sec, %I64d bytes last sec."), nBytesPerSec, nBytesLastSec);
        }
        else
            szThroughput.Format(_T("%I64d bytes/sec"), (INT64)0);
        //szThroughput.Format(_T("%I64d bytes last sec."),transfer.nTransferred-m_nLastTransferred);
        m_wndThroughput.SetWindowText(szThroughput);
        m_nLastTransferred = transfer.nTransferred;
    }
    else
    {
        if(m_bCompleted)
        {
            CString szProgress;
            szProgress.Format(_T("%I64d/%I64d - %.2f %%"), m_nTotalSize, m_nTotalSize, 100.0f);
            m_wndTransferred.SetWindowText(szProgress);
            m_wndProgressBar.SetPos(100);
        }
        else
        {
        }
        return FALSE;
    }
    return TRUE;
}

void CFileTransferDlg::Completed()
{
    CString szText;
    GetWindowText(szText);
    szText += _T(" - Completed");
    m_wndCancel.SetWindowText(_T("OK"));
    m_bCompleted = TRUE;
    m_nLastTransferred = m_nTotalSize;

    m_wndOpen.EnableWindow(TRUE);
    if(m_wndAutoClose.GetCheck() == BST_CHECKED)
        OnCancel();
}

void CFileTransferDlg::Failed()
{
    CString szText;
    GetWindowText(szText);
    szText += _T(" - Failed");
}

void CFileTransferDlg::OnBnClickedCheckAutoclose()
{
    m_bAutoClose = m_wndAutoClose.GetCheck() == BST_CHECKED;
}

void CFileTransferDlg::OnBnClickedButtonOpen()
{
    int index = m_szLocalFilename.ReverseFind('\\');
    CString szWorkdir;
    if(index>0)
     szWorkdir = m_szLocalFilename.Left(index);
    HINSTANCE i = ShellExecute(this->m_hWnd,_T("open"),m_szLocalFilename,_T(""),szWorkdir,SW_SHOW);
}

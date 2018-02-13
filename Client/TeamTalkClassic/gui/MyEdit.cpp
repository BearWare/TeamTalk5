// gui\MyEdit.cpp : implementation file
//

#include "stdafx.h"
#include "MyEdit.h"
#include ".\myedit.h"


// CMyEdit

IMPLEMENT_DYNAMIC(CMyEdit, CEdit)
CMyEdit::CMyEdit()
: m_nLastIndex(-1)
{
}

CMyEdit::~CMyEdit()
{
}

BEGIN_MESSAGE_MAP(CMyEdit, CEdit)
  ON_WM_KEYDOWN()
END_MESSAGE_MAP()

// CMyEdit message handlers

LRESULT CMyEdit::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
    if(message == WM_PASTE)
    {
        if( OpenClipboard() )
        {
#if defined(UNICODE) || defined(_UNICODE)
            HANDLE hClipboardData = GetClipboardData(CF_UNICODETEXT);
#else
            HANDLE hClipboardData = GetClipboardData(CF_TEXT);
#endif
            // Call GlobalLock so that to retrieve a pointer
            // to the data associated with the handle returned
            // from GetClipboardData.
            TCHAR *pchData = (TCHAR*)GlobalLock(hClipboardData);

            // Set a local CString variable to the data
            // and then update the dialog with the Clipboard data
            CString szFromClipboard = pchData;
            //m_edtFromClipboard.SetWindowText(strFromClipboard);

            // Unlock the global memory.
            GlobalUnlock(hClipboardData);

            // Finally, when finished I simply close the Clipboard
            // which has the effect of unlocking it so that other
            // applications can examine or modify its contents.
            CloseClipboard();

            //////////////////////////////////////////////////////
            ///// DONT PASTE MORE THAN 2048 characters
            //////////////////////////////////////////////////////
            if(szFromClipboard.GetLength() > 2048*sizeof(TCHAR))
                return TRUE;

            bool bFound = false;
            for(int i=0;i<szFromClipboard.GetLength() && !bFound; i++)
            {
                if(szFromClipboard[i] == '\n')
                    bFound = true;
            }

            if(bFound)
            {
                int nStart = 0;
                CStringList listStr;
                CString szStr = szFromClipboard.Tokenize(_T("\r\n"), nStart);
                while(szStr != _T(""))
                {
                    listStr.AddTail(szStr);
                    szStr = szFromClipboard.Tokenize(_T("\r\n"), nStart);
                }

                for(POSITION pos=listStr.GetHeadPosition(); pos != NULL;)
                {
                    if(GetParent())
                    {
                        SetWindowText(listStr.GetNext(pos));
                        HWND hWnd = AfxGetMainWnd()->m_hWnd;
                        AfxGetMainWnd()->SendMessage(WM_COMMAND, IDOK, (LPARAM)hWnd);
                    }
                }

                if(listStr.GetCount())
                    return TRUE;
            }
        }
    }

    return CEdit::WindowProc(message, wParam, lParam);
}

void CMyEdit::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
  CEdit::OnKeyDown(nChar, nRepCnt, nFlags);
  if(!m_vecMsgs.empty())
  {
    if(nChar == VK_UP)
    {
      if(m_nLastIndex + 1 < int(m_vecMsgs.size()))
        m_nLastIndex++;

      if(m_nLastIndex>=0 && m_nLastIndex < int(m_vecMsgs.size()))
        SetWindowText(m_vecMsgs[m_nLastIndex]);
    }
    else if(nChar == VK_DOWN)
    {
      if(m_nLastIndex-1 >= -1)
        m_nLastIndex--;

      if(m_nLastIndex>=0 && m_nLastIndex < int(m_vecMsgs.size()))
        SetWindowText(m_vecMsgs[m_nLastIndex]);
      else
        SetWindowText(_T(""));
    }
  }
}

void CMyEdit::AddLastMessage(CString szLastMsg)
{
  m_vecMsgs.insert(m_vecMsgs.begin(), szLastMsg);
  if(m_vecMsgs.size()>MAX_HISTORY)
    m_vecMsgs.erase(m_vecMsgs.end()-1);
}

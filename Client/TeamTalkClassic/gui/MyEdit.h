/*
 * Copyright (c) 2005-2018, BearWare.dk
 * 
 * Contact Information:
 *
 * Bjoern D. Rasmussen
 * Kirketoften 5
 * DK-8260 Viby J
 * Denmark
 * Email: contact@bearware.dk
 * Phone: +45 20 20 54 59
 * Web: http://www.bearware.dk
 *
 * This source code is part of the TeamTalk SDK owned by
 * BearWare.dk. Use of this file, or its compiled unit, requires a
 * TeamTalk SDK License Key issued by BearWare.dk.
 *
 * The TeamTalk SDK License Agreement along with its Terms and
 * Conditions are outlined in the file License.txt included with the
 * TeamTalk SDK distribution.
 *
 */

#if !defined(MYEDIT_H)
#define MYEDIT_H

#include <vector>

#define MAX_HISTORY 20

// CMyEdit

class CMyEdit : public CEdit
{
    DECLARE_DYNAMIC(CMyEdit)

public:
    CMyEdit();
    virtual ~CMyEdit();

  void AddLastMessage(CString szLastMsg);

protected:
    DECLARE_MESSAGE_MAP()

protected:
    virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
  std::vector<CString> m_vecMsgs;
  int m_nLastIndex;
public:
  afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
};

#endif

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

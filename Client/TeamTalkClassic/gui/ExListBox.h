#if !defined(AFX_EXLISTBOX_H__202DD0DE_98E2_4300_A841_C20B49F46367__INCLUDED_)
#define AFX_EXLISTBOX_H__202DD0DE_98E2_4300_A841_C20B49F46367__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ExListBox.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CExListBox window

class CExListBox : public CListBox
{
// Construction
public:
	CExListBox();

// Attributes
public:
	INT_PTR OnToolHitTest(CPoint point, TOOLINFO * pTI) const;
	UINT ItemFromPoint2(CPoint pt, BOOL& bOutside) const;
//	void PreSubclassWindow() ;
	
	BOOL OnToolTipText( UINT id, NMHDR * pNMHDR, LRESULT * pResult );
	// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CExListBox)
	protected:
	virtual void PreSubclassWindow();
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CExListBox();

	// Generated message map functions
protected:
	//{{AFX_MSG(CExListBox)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_EXLISTBOX_H__202DD0DE_98E2_4300_A841_C20B49F46367__INCLUDED_)

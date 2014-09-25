#pragma once

#include "NewWizDialog.h"

// CWizMasterDlg dialog

class CWizMasterDlg : public CNewWizDialog
{
public:
	CWizMasterDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CWizMasterDlg();

// Dialog Data
	enum { IDD = IDD_WIZMASTERDLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
  virtual BOOL OnInitDialog();
};

 /*
 * Copyright (c) 2002-2016, BearWare.dk
 * 
 * Bjoern D. Rasmussen
 * Bedelundvej 79
 * DK-9830 Taars
 * Denmark
 * Email: contact@bearware.dk
 * Phone: +45 20 20 54 59
 * Web: http://www.bearware.dk
 *
 * This source code may not be modified or redistributed without the
 * copyright owner's written consent.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */


// NewItemDlg.cpp : implementation file
//

#include "stdafx.h"
#include "LanguageEditor.h"
#include "NewItemDlg.h"
#include ".\newitemdlg.h"


// CNewItemDlg dialog

IMPLEMENT_DYNAMIC(CNewItemDlg, CDialog)
CNewItemDlg::CNewItemDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CNewItemDlg::IDD, pParent)
	, m_nItemID(0)
	, m_szItemText(_T(""))
	, m_hAccel(NULL)
{
}

CNewItemDlg::~CNewItemDlg()
{
}

void CNewItemDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_ITEMID, m_nItemID);
	DDX_Text(pDX, IDC_EDIT_ITEMTEXT, m_szItemText);
}


BEGIN_MESSAGE_MAP(CNewItemDlg, CDialog)
END_MESSAGE_MAP()


// CNewItemDlg message handlers

BOOL CNewItemDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	//load accelerators
	m_hAccel = ::LoadAccelerators(AfxGetResourceHandle(), (LPCTSTR)IDR_ACCELERATOR2);
	if (!m_hAccel)
		MessageBox(_T("The accelerator table was not loaded"));

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CNewItemDlg::PreTranslateMessage(MSG* pMsg)
{
	if (WM_KEYFIRST <= pMsg->message && pMsg->message <= WM_KEYLAST)
		if (m_hAccel && ::TranslateAccelerator(m_hWnd, m_hAccel, pMsg))
			return TRUE;
	return CDialog::PreTranslateMessage(pMsg);
}

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


// GotoItemDlg.cpp : implementation file
//

#include "stdafx.h"
#include "LanguageEditor.h"
#include "GotoItemDlg.h"


// CGotoItemDlg dialog

IMPLEMENT_DYNAMIC(CGotoItemDlg, CDialog)
CGotoItemDlg::CGotoItemDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CGotoItemDlg::IDD, pParent)
	, m_nItemID(-1)
{
}

CGotoItemDlg::~CGotoItemDlg()
{
}

void CGotoItemDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_nItemID);
}


BEGIN_MESSAGE_MAP(CGotoItemDlg, CDialog)
END_MESSAGE_MAP()


// CGotoItemDlg message handlers

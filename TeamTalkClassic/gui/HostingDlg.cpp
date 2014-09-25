// gui\StaticChannelsDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "HostingDlg.h"

#include <string>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// CHostingDlg dialog

IMPLEMENT_DYNAMIC(CHostingDlg, CDialog)
CHostingDlg::CHostingDlg(const channels_t& channels, CWnd* pParent /*=NULL*/)
	: CDialog(CHostingDlg::IDD, pParent)
	, m_bCreateChannels(TRUE)
	, m_bOperators(TRUE)
	, m_bRemoteAdmin(FALSE)
{
  m_channels = channels;
}

CHostingDlg::~CHostingDlg()
{
}

void CHostingDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TREE_SESSION, m_wndTree);
	DDX_Check(pDX, IDC_CHECK_CREATECHANNELS, m_bCreateChannels);
	DDX_Check(pDX, IDC_CHECK_OPERATORS, m_bOperators);
	DDX_Control(pDX, IDC_CHECK_REMOTEADMIN, m_wndRAdminButton);
	DDX_Control(pDX, IDC_EDIT_ADMINUSERNAME, m_wndAdminUsername);
	DDX_Control(pDX, IDC_EDIT_ADMINPASSWORD, m_wndAdminPassword);
	DDX_Check(pDX, IDC_CHECK_REMOTEADMIN, m_bRemoteAdmin);
	DDX_Text(pDX, IDC_EDIT_ADMINPASSWORD, m_szAdminPassword);
	DDX_Text(pDX, IDC_EDIT_ADMINUSERNAME, m_szAdminUsername);
}

CString CHostingDlg::GetChannelPath(HTREEITEM hItem)
{
	CString szPath;
	HTREEITEM hRoot = m_wndTree.GetRootItem();
	while(hItem)
	{
		if(hItem != hRoot)
			szPath = m_wndTree.GetItemText(hItem) + _T("/") + szPath;
		else
			break;

		hItem = m_wndTree.GetParentItem(hItem); 
	}
	szPath = _T("/") + szPath;

	return szPath;
}

BEGIN_MESSAGE_MAP(CHostingDlg, CDialog)
	ON_BN_CLICKED(IDC_BUTTON_NEW, OnBnClickedButtonNew)
	ON_BN_CLICKED(IDC_BUTTON_UPDATE, OnBnClickedButtonUpdate)
	ON_BN_CLICKED(IDC_BUTTON_DELETE, OnBnClickedButtonDelete)
	ON_BN_CLICKED(IDC_CHECK_REMOTEADMIN, OnBnClickedCheckRemoteadmin)
END_MESSAGE_MAP()


// CHostingDlg message handlers

BOOL CHostingDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	TRANSLATE(*this, IDD);

	//load images for treectrl
	m_ImageList.Create(IDB_BITMAP_SESSION2, 17, 1, RGB(255,255,255));
	m_wndTree.SetImageList(&m_ImageList, TVSIL_NORMAL);

	std::queue< Channel > chanQueue;
	std::queue<HTREEITEM> itemQueue;

  int nRootID = GetRootChannelID(m_channels);
  channels_t::iterator ite = m_channels.find(nRootID);
  ASSERT(nRootID>0);
  ASSERT(ite != m_channels.end());

  if(nRootID>0 && ite != m_channels.end())
  {
    Channel chan = ite->second;

    HTREEITEM hItem = m_wndTree.InsertItem(m_szServerName, ROOT_CLOSED, ROOT_CLOSED);
    m_wndTree.SetItemData(hItem, chan.nChannelID);

    chanQueue.push(chan);
    itemQueue.push(hItem);

    while(chanQueue.size())
    {
      chan = chanQueue.front();
      chanQueue.pop();
      hItem = itemQueue.front();
      itemQueue.pop();

      channels_t subs = GetSubChannels(chan.nChannelID, m_channels);
      for(ite=subs.begin();ite!=subs.end();ite++)
      {
        HTREEITEM hNew = m_wndTree.InsertItem( CONVERTSTR( ite->second.szName ), CHANNEL_CLOSED, CHANNEL_CLOSED, hItem);
        m_wndTree.SetItemData(hNew, ite->second.nChannelID);
        chanQueue.push(ite->second);
        itemQueue.push(hNew);
      }
      m_wndTree.EnsureVisible(hItem);
    }
  }

	OnBnClickedCheckRemoteadmin();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}


void CHostingDlg::OnBnClickedButtonNew()
{
	CChannelDlg dlg(CChannelDlg::CREATE_CHANNEL, this);
	if(dlg.DoModal()==IDOK)
	{
		HTREEITEM hItem = m_wndTree.GetSelectedItem();
		//no channels at same level can have same name
		BOOL ok = TRUE;
		HTREEITEM hOther = m_wndTree.GetChildItem(hItem);
		while(hOther && ok)
    {
			if(m_wndTree.GetItemText(hOther).CompareNoCase(dlg.m_szChannelname) == 0)
				ok = FALSE;
			else
				hOther = m_wndTree.GetNextSiblingItem(hOther);
    }

		if(!ok)
		{
			AfxMessageBox(_T("You cannot have channels at the same level that have the same name"));
			return;
		}

    if(hItem)
    {
      int nParentID = m_wndTree.GetItemData(hItem);
      channels_t::iterator ite = m_channels.find(nParentID);
      ASSERT(ite != m_channels.end());
      if(ite != m_channels.end())
      {
        Channel newchan = {0};
        newchan.nParentID = nParentID;
        newchan.nChannelID = GetMaxChannelID(m_channels)+1;

				std::string tmp = CONVERTSTR( dlg.m_szChannelname );
				strncpy(newchan.szName, tmp.c_str(), sizeof(newchan.szName)-1);
				tmp = CONVERTSTR( dlg.m_szChannelPassword );
				strncpy(newchan.szPassword, tmp.c_str(), sizeof(newchan.szPassword)-1);
				tmp = CONVERTSTR( dlg.m_szChannelTopic );
				strncpy(newchan.szTopic, tmp.c_str(), sizeof(newchan.szTopic)-1);
				newchan.bPassword = strlen(newchan.szPassword)>0;
				m_channels[newchan.nChannelID] = newchan;

        HTREEITEM hNew = m_wndTree.InsertItem( CONVERTSTR( newchan.szName ),CHANNEL_CLOSED,CHANNEL_CLOSED, hItem);
        m_wndTree.SetItemData(hNew, newchan.nChannelID);
        m_wndTree.Expand(hItem, TVE_EXPAND);
      }
    }
	}

}

void CHostingDlg::OnBnClickedButtonUpdate()
{
	HTREEITEM hItem = m_wndTree.GetSelectedItem();
  int nChannelID = m_wndTree.GetItemData(hItem);

	CChannelDlg dlg(CChannelDlg::CREATE_CHANNEL, this);

  channels_t::iterator ite = m_channels.find(nChannelID);
  ASSERT(ite != m_channels.end());
	//note that root channel is tricky (doesn't have a channel name in ChannelPath)
  if(hItem && ite!=m_channels.end())
	{
    Channel chan = ite->second;

		//no root update allowed
    if(chan.nParentID == 0)
		{
			dlg.m_szChannelname = m_szServerName;
			dlg.m_szChannelPassword = m_szPassword;
			dlg.m_szChannelTopic = chan.szTopic;
			dlg.m_bPProtect = !m_szPassword.IsEmpty();
		}
		else
		{
      dlg.m_szChannelname = chan.szName;
      dlg.m_szChannelPassword = chan.szPassword;
      dlg.m_szChannelTopic = chan.szTopic;
      dlg.m_bPProtect = chan.bPassword;
		}
		if(dlg.DoModal()==IDOK)
    {
      if(chan.nParentID == 0)
      {
        m_szServerName = dlg.m_szChannelname;
        m_szPassword = dlg.m_szChannelPassword;

				std::string tmp = CONVERTSTR( dlg.m_szChannelTopic );
				strncpy(chan.szTopic, tmp.c_str(), sizeof(chan.szTopic)-1);

        m_channels[chan.nChannelID] = chan;

        m_wndTree.SetItemText(hItem, m_szServerName);
      }
      else
      {
				std::string tmp = CONVERTSTR( dlg.m_szChannelname );
				strncpy(chan.szName, tmp.c_str(), sizeof(chan.szName)-1);
				tmp = CONVERTSTR( dlg.m_szChannelPassword );
				strncpy(chan.szPassword, tmp.c_str(), sizeof(chan.szPassword)-1);
				tmp = CONVERTSTR( dlg.m_szChannelTopic );
				strncpy(chan.szTopic, tmp.c_str(), sizeof(chan.szTopic)-1);
				chan.bPassword = strlen(chan.szPassword)>0;

        m_channels[chan.nChannelID] = chan;

        m_wndTree.SetItemText(hItem, CONVERTSTR( chan.szName ));
        m_wndTree.Expand(hItem, TVE_EXPAND);
			}
		}
	}
}

void CHostingDlg::OnBnClickedButtonDelete()
{
	HTREEITEM hItem = m_wndTree.GetSelectedItem();	
	if(hItem == m_wndTree.GetRootItem())
	{
		AfxMessageBox(_T("You cannot delete the root channel"));
	}
	else
	{
    int nChannelID = m_wndTree.GetItemData(hItem);
    channels_t::iterator ite = m_channels.find(nChannelID);
    if(ite != m_channels.end())
    {
      m_channels.erase(ite);
      m_wndTree.DeleteItem(hItem);
    }
  }
}

void CHostingDlg::OnBnClickedCheckRemoteadmin()
{
	m_wndAdminPassword.EnableWindow(m_wndRAdminButton.GetCheck() == BST_CHECKED);
	m_wndAdminUsername.EnableWindow(m_wndRAdminButton.GetCheck() == BST_CHECKED);
}

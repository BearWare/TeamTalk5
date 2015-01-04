// gui\IpAddresses.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "IpAddressesDlg.h"
#include "AppInfo.h"
#include <winsock2.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// CIpAddressesDlg dialog

IMPLEMENT_DYNAMIC(CIpAddressesDlg, CDialog)
CIpAddressesDlg::CIpAddressesDlg(CWnd* pParent /*=NULL*/)
    : CDialog(CIpAddressesDlg::IDD, pParent)
    , m_szIPDescription(_T(""))
{
}

CIpAddressesDlg::~CIpAddressesDlg()
{
}

void CIpAddressesDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_EDIT_ADDRESSES, m_szIPDescription);
}

BEGIN_MESSAGE_MAP(CIpAddressesDlg, CDialog)
END_MESSAGE_MAP()


// CIpAddressesDlg message handlers

BOOL CIpAddressesDlg::OnInitDialog()
{
    m_szIPDescription = GetIPAddressDescription();

    CDialog::OnInitDialog();

    TRANSLATE(*this, IDD);

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

CString CIpAddressesDlg::GetIPAddressDescription()
{
    CStringArray arr;
    GetIPs(arr);

    CString s;
    if(arr.GetSize()>0)
    {
        s = _T("Alright, here's the list of IP-addresses available on your computer:\r\n\r\n");
        int i;
        for(i=0;i<arr.GetSize();i++)
            s += _T("- ") + arr.GetAt(i) +_T("\r\n");
        s += _T("\r\n");

        const UINT A = 0x0AFFFFFF;    //10.255.255.255
        const UINT B = 0xAC10FFFF;    //172.16.255.255 - 172.31.255.255
        const UINT C = 0xC0A8FFFF;    //192.168.255.255
        const UINT L = 0x7F000001;    //127.0.0.1
        
        int nPublic = 0;
        for(i=0;i<arr.GetSize();i++)
        {
            UINT ip = CStringIPToUINT(arr[i]);
            if(ip != 0)
            {
                if( ((ip & L) == ip) )
                    s+= _T("The IP-address \"") + arr.GetAt(i) + _T("\" is for a loop-back interface ")
                    _T("and not accesible for others than yourself.\r\n\r\n");
                else
                    if( ((ip & A) == ip) || ((ip & B) == ip) || ((ip & C) == ip) )
                        s += _T("The IP-address \"") + arr.GetAt(i) + _T("\" is a private IP-address, ")
                        _T("meaning that it can only be accessed by people from your local area ")
                        _T("network and NOT by people on the internet. You therefore cannot use this ")
                        _T("IP-address to host a ") APPNAME _T(" server on the internet. Still you should be able to connect ")
                        _T("to other ") APPNAME _T(" servers\r\n\r\n");
                    else
                    {
                        s += _T("The IP-address \"") + arr.GetAt(i) + _T("\" is a public IP-address that is ")
                            _T("accessible from the internet, so if you host a server then tell others to join ")
                            _T("this IP-address\r\n\r\n");
                        nPublic ++;
                    }
            }
            else
                s += _T("The IP-address \"") + arr.GetAt(i) + _T("\" is an unknown IP-address type. ")
                _T("May an IP-address version 6?\r\n\r\n");
        }
        if(nPublic == 0)
        {
            s += _T("Since you have no public IP-addresses available you cannot host a server ")
                _T("that is accessible by other internet users. If, however, you are allowed to ")
                _T("configure your proxy or router then try reading the section on Routers and proxies ")
                _T("in the manual that comes with ") APPNAME _T(".\r\nLast chance is to check out: ") WEBSITE
                _T("for a public server.\r\n");
        }
    }
    else
    {
        s = APPNAME _T(" was unable to retrieve any IP-addresses from your computer.\r\n\r\n");
    }
    return s;
}


int CIpAddressesDlg::GetIPs( CStringArray &arrIPS )
{
    BOOL bRet = TRUE;
    char name[255];
    int i=0;
    PHOSTENT hostinfo;

  //first, clear the list
    arrIPS.RemoveAll();

  //get the host name
  if( gethostname ( name, sizeof(name)) == 0)
  {
    // get the host info
    if((hostinfo = gethostbyname(name)) != NULL)
    {
      // now, loop until the address list = null
      while( hostinfo->h_addr_list[i] != NULL )
      {
        //get the IP address
        char *ip_addr = inet_ntoa (*(struct in_addr *)
                        hostinfo->h_addr_list[i]);
        //add it to the array
        arrIPS.Add(CString(ip_addr)); 
        //increment the counter
        i++;
      }
    }
  }
  //return the array count
  return arrIPS.GetSize();
}

UINT CIpAddressesDlg::CStringIPToUINT(CString szIPAddress)
{
    UINT ip=0;
    int offset=0;
    int i = 0;
    CString abcd[4];

    if(szIPAddress.Find('.') == -1)return 0;

    /* A.B.C.D */
    i = szIPAddress.Find('.', offset);
    abcd[0] = szIPAddress.Mid(offset,i-offset);
    offset = i+1;
    i = szIPAddress.Find('.', offset);
    abcd[1] = szIPAddress.Mid(offset,i-offset);
    offset = i+1;
    i = szIPAddress.Find('.', offset);
    abcd[2] = szIPAddress.Mid(offset,i-offset);
    offset = i+1;

    abcd[3] = szIPAddress.Mid(offset,szIPAddress.GetLength()-i);
    offset = i+1;

    if(szIPAddress.Find('.',offset) != -1)//ip v6
        return 0;

#if defined(_UNICODE) || defined(UNICODE)
    ip = _wtoi(abcd[0]) << 24;
    ip |= _wtoi(abcd[1]) << 16;
    ip |= _wtoi(abcd[2]) << 8;
    ip |= _wtoi(abcd[3]);
#else
    ip = atoi(abcd[0]) << 24;
    ip |= atoi(abcd[1]) << 16;
    ip |= atoi(abcd[2]) << 8;
    ip |= atoi(abcd[3]);
#endif    

    return ip;
}
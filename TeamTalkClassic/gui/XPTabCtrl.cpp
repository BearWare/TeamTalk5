/***********************************************************************************************************/
// XPTabCtrl.cpp : implementation file
/////////////////////////////////////////////////////////////////////////////
//    Compiler:    Visual C++, Service Pack 5, Windows NT 
//    Tested on:    Windows NT, Windows XP
//
//    Created:    25/February/2004
//    Updated:    
//
//    Author:        Adi DEDIC
//  e-mail:        adi_dedic@hotmail.com
//  www:        http://web.onetel.net.uk/~adidedic/
//
//    Disclaimer
//    ----------
//    THIS SOFTWARE AND THE ACCOMPANYING FILES ARE DISTRIBUTED "AS IS" AND WITHOUT
//    ANY WARRANTIES WHETHER EXPRESSED OR IMPLIED. NO REPONSIBILITIES FOR POSSIBLE
//    DAMAGES OR EVEN FUNCTIONALITY CAN BE TAKEN. THE USER MUST ASSUME THE ENTIRE
//    RISK OF USING THIS SOFTWARE.
//
//    Terms of use
//    ------------
//    THIS SOFTWARE IS FREE FOR PERSONAL USE OR FREEWARE APPLICATIONS.
//    IF YOU USE THIS SOFTWARE IN COMMERCIAL OR SHAREWARE APPLICATIONS YOU
//    ARE GENTLY ASKED TO DONATE £1 (ONE GB PUND) TO THE AUTHOR
/***********************************************************************************************************/
#include "StdAfx.h"
#include "XPTabCtrl.h"

//#define USE_DEFAULT_XP_TOPTAB        // XP top tab is drawn only for test purpose. To use default, uncoment this line

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/***********************************************************************************************************/
// constant string definitions here (or you can put it into resource string table)
#define IDS_UTIL_TAB            "TAB"
#define IDS_UTIL_UXTHEME        "UxTheme.dll"
#define IDS_UTIL_THEMEACT       "IsThemeActive"
#define IDS_UTIL_THEMEOPN       "OpenThemeData"
#define IDS_UTIL_THEMEBCKG      "DrawThemeBackground"

IMPLEMENT_DYNAMIC(CXPTabCtrl, CTabCtrl)

/***********************************************************************************************************/
// CXPTabCtrl
/***********************************************************************************************************/
CXPTabCtrl::CXPTabCtrl()
{
    m_bTabExtended=TRUE;            // default is to use extended tab (if it is Themes XP)
    m_eTabOrientation=e_tabBottom;    // default initial orientation is: bottom
    m_ixSelOld=-1;
}
//----------------------------------------------------------------------------------------------------------
// nBitmapID Resource IDs of the bitmap to be associated with the image list.
void CXPTabCtrl::InitImageList(UINT nBitmapID)
{
    if(!::IsWindow(GetSafeHwnd()) || m_ilTabs.operator HIMAGELIST())
    { ASSERT(FALSE); return; }                // tab control has to be created first and image list can be created only once
    if(m_ilTabs.Create(nBitmapID, 16, 1, RGB(0xFF,0,0xFF)))    // add an images list with appropriate background (transparent) color
        SetImageList(&m_ilTabs);
}

void CXPTabCtrl::AddTab(CWnd* pWnd, CString szCaption, int nImage)
{
    TCITEM item;
    item.mask = TCIF_TEXT|TCIF_PARAM|TCIF_IMAGE;
    item.lParam = (LPARAM) pWnd;
    item.pszText = szCaption.GetBuffer(256);
    item.iImage = nImage;
    int i = GetItemCount();
    InsertItem(i,&item);

    CRect rectTab;
    CRect rect;
    GetItemRect(GetCurSel(), rectTab);
    GetWindowRect(&rect);

    //ScreenToClient(&rect);
    pWnd->MoveWindow(10, rectTab.Height()+10, rect.Width()-20, rect.Height()-rectTab.Height()-20);
    //pWnd->SetWindowPos(NULL, 10, rectTab.Height()+10, rect.Width()-20, rect.Height()-rectTab.Height()-20,
    //    SWP_FRAMECHANGED | SWP_NOSIZE | SWP_NOZORDER);

    m_vecTabs.insert(m_vecTabs.begin()+i, pWnd);
    if(m_vecTabs.size()==1)
    {
        ResizeItems();
        pWnd->ShowWindow(SW_SHOW);
    }
}

void CXPTabCtrl::SetOrientation(ETabOrientation eTabOrientation)
{
    ModifyStyle(TCS_VERTICAL|TCS_BOTTOM|TCS_MULTILINE,(eTabOrientation & 1? TCS_BOTTOM : 0) | 
        (eTabOrientation>e_tabBottom? (TCS_VERTICAL|(TRUE && IsThemeActiveXP()?0:TCS_MULTILINE)):0));
    m_eTabOrientation = eTabOrientation;
}

//----------------------------------------------------------------------------------------------------------
// only three messages used
BEGIN_MESSAGE_MAP(CXPTabCtrl, CTabCtrl)
    //{{AFX_MSG_MAP(CXPTabCtrl)
    ON_WM_PAINT()
    ON_NOTIFY_REFLECT(TCN_SELCHANGING,OnTabSelChanging)
    ON_NOTIFY_REFLECT(TCN_SELCHANGE,  OnTabSelChanged)
    //}}AFX_MSG_MAP
    ON_WM_SIZE()
END_MESSAGE_MAP()
//----------------------------------------------------------------------------------------------------------
void CXPTabCtrl::OnPaint() 
{
    if(!IsExtendedTabTheamedXP())            // if it is not XP or it is not Themes, behave as default
    { Default(); return; }    
    CPaintDC dc(this);                        // device context for painting

    CRect rcClip; rcClip.SetRectEmpty();
    dc.GetClipBox(rcClip);

    // 1st paint the tab body
    CRect rcPage,rcItem,rcClient;
    GetClientRect(&rcPage);
    rcClient=rcPage;
    AdjustRect(FALSE,rcPage);

    switch(m_eTabOrientation)
    {    case e_tabTop:      rcClient.top   =rcPage.top   -2; break;
    case e_tabBottom: rcClient.bottom=rcPage.bottom+3; break;
    case e_tabLeft:      rcClient.left  =rcPage.left  -1; break;
    case e_tabRight:  rcClient.right =rcPage.right +3; break;
    default: ASSERT(FALSE); return;
    }
    UINT uiVertBottm;
    uiVertBottm =(m_eTabOrientation&1)? 8:0;        //  8=bottom
    uiVertBottm|=(m_eTabOrientation&2)?16:0;        // 16=vertical
    UINT uiFlags=1|uiVertBottm;                        //  1=body    
    DrawThemesXpTabItem(&dc, -1, rcClient,uiFlags);    // TABP_PANE=9,0,'TAB'

    int nTab=GetItemCount();                        // paint the tabs first and then the borders
    if(!nTab) return;                                // no tab pages added

    // 2nd paint the inactive tabs
    CRect ;
    TCHITTESTINFO hti;    hti.flags=0;
    ::GetCursorPos(&hti.pt); ScreenToClient(&hti.pt);
    int ixHot=HitTest(&hti);
    int ixSel=GetCurSel();

    for(int ixTab=0; ixTab<nTab; ixTab++)
    {    if(ixTab==ixSel)
    continue;
    VERIFY(GetItemRect(ixTab, &rcItem));
    if(m_eTabOrientation==e_tabLeft) rcItem.right++;
    uiFlags=uiVertBottm|(ixTab==ixHot?4:0);        // 4= hot
    DrawThemesXpTabItem(&dc,ixTab,rcItem,uiFlags);
    }
    // 3rd paint the active selected tab
    VERIFY(GetItemRect(ixSel, &rcItem));            // now selected tab
    rcItem.InflateRect(2,2);
    if(m_eTabOrientation==e_tabTop) rcItem.bottom--;
    uiFlags=uiVertBottm|2;                            // 2= selected
    DrawThemesXpTabItem(&dc, ixSel, rcItem,uiFlags);
}
//----------------------------------------------------------------------------------------------------------
// This function draws Themes Tab control parts: a) Tab-Body and b) Tab-tabs
void CXPTabCtrl::DrawThemesXpTabItem(CDC* pDC, int ixItem, const CRect& rcItem, UINT uiFlag) 
{            // uiFlag(1/0):1=Type(body/tab);2=Sel(y/n);4=Hot(y/n);8=bBottom(y/n);16=rotate(y/n)
    BOOL bBody  =(uiFlag& 1)?TRUE:FALSE;
    BOOL bSel   =(uiFlag& 2)?TRUE:FALSE;
    BOOL bHot   =(uiFlag& 4)?TRUE:FALSE;
    BOOL bBottom=(uiFlag& 8)?TRUE:FALSE;    // mirror
    BOOL bVertic=(uiFlag&16)?TRUE:FALSE;    // rotate
    BOOL bLeftTab=!bBottom && bVertic && !bBody;

    CSize szBmp=rcItem.Size();
    if(bVertic) SwapVars(szBmp.cx,szBmp.cy);
    // 1st draw background
    CDC     dcMem;    dcMem .CreateCompatibleDC(pDC);
    CBitmap bmpMem; bmpMem.CreateCompatibleBitmap(pDC,szBmp.cx,szBmp.cy);
    CBitmap* pBmpOld=dcMem.SelectObject(&bmpMem);
    CRect rcMem(CPoint(0,0),szBmp); if(bSel) rcMem.bottom++;
    if(bBody)
        DrawThemesPart(dcMem.GetSafeHdc(), 9, 0, (LPCSTR)IDS_UTIL_TAB, &rcMem);    // TABP_PANE=9,  0, 'TAB'
    else DrawThemesPart(dcMem.GetSafeHdc(), 1, bSel?3:(bHot?2:1), (LPCSTR)IDS_UTIL_TAB, &rcMem);
    // TABP_TABITEM=1, TIS_SELECTED=3:TIS_HOT=2:TIS_NORMAL=1, 'TAB'
    // 2nd init some extra parameters
    BITMAPINFO biOut; ZeroMemory(&biOut,sizeof(BITMAPINFO));    // Fill local pixel arrays
    BITMAPINFOHEADER& bihOut=biOut.bmiHeader;
    bihOut.biSize  =sizeof (BITMAPINFOHEADER);
    bihOut.biCompression=BI_RGB;
    bihOut.biPlanes=1;          bihOut.biBitCount=24;    // force as RGB: 3 bytes,24 bits -> good for rotating bitmap in any resolution
    bihOut.biWidth =szBmp.cx; bihOut.biHeight=szBmp.cy;

    int nBmpWdtPS=DWordAlign(szBmp.cx*3);
    int nSzBuffPS=((nBmpWdtPS*szBmp.cy)/8+2)*8;
    LPBYTE pcImg=NULL;
    if(bBottom || bVertic) { pcImg=new BYTE[nSzBuffPS]; ASSERT(pcImg); }
    int nStart=0,nLenSub=0;
    if(bBody && bBottom && !bVertic) nStart=3,nLenSub=4;    // if bottom oriented flip the body contest only (no shadows were flipped)
    // 3rd if it is left oriented tab, draw tab context before mirroring or rotating (before GetDIBits)
    if(bVertic)
    {    if(bBody || !bBottom) bihOut.biHeight=-szBmp.cy;
    if(!bBottom && !bBody && ixItem>=0)                    // 
    {    if(bSel) rcMem.bottom--;
    DrawTabItem(&dcMem, ixItem, rcMem, uiFlag); ixItem=-1;
    }    }                                                        // rotate or mirror
    // 4th get bits (for rotate) and mirror: body=(all except top) tab=(all except top)
    if(bVertic || bBottom)                                        // get bits: 
    {    GetDIBits(*pDC, bmpMem.operator HBITMAP(),nStart,szBmp.cy-nLenSub,pcImg,&biOut,DIB_RGB_COLORS);
    if(bBottom)                                    // mirror: body=(bottom and right) tab=(bottom and right)
    {    bihOut.biHeight=-szBmp.cy;                 // to mirror bitmap is eough to use negative height between Get/SetDIBits
    SetDIBits(*pDC, bmpMem.operator HBITMAP(),nStart,szBmp.cy-nLenSub,pcImg,&biOut,DIB_RGB_COLORS);
    if(bBody && bVertic)                    // when it is right oriented body -> flip twice, first flip border shadows, than flip shaded inside body again
    {    nStart=2; nLenSub=4; bihOut.biHeight=szBmp.cy;
    GetDIBits(*pDC, bmpMem.operator HBITMAP(),nStart,szBmp.cy-nLenSub,pcImg,&biOut,DIB_RGB_COLORS);
    bihOut.biHeight=-szBmp.cy;            // to mirror bitmap is eough to use negative height between Get/SetDIBits
    SetDIBits(*pDC, bmpMem.operator HBITMAP(),nStart,szBmp.cy-nLenSub,pcImg,&biOut,DIB_RGB_COLORS);
    }    }    }
    // 5th if it is bottom or right oriented tab, draw after mirroring background (do GetDIBits again)
    if(!bBody && ixItem>=0)                            // 
    {    if(bSel) rcMem.bottom--;
    DrawTabItem(&dcMem, ixItem, rcMem, uiFlag);
    if(bVertic)                                            // if it is right tab, do GetDIBits again
    {    bihOut.biHeight=-szBmp.cy;
    GetDIBits(*pDC, bmpMem.operator HBITMAP(),nStart,szBmp.cy-nLenSub,pcImg,&biOut,DIB_RGB_COLORS);
    }    }
    // 6th: do rotate now, finaly
    if(bVertic)                            // force rotating bitmap as RGB -> good for any resolution
    {    SwapVars(szBmp.cx,szBmp.cy);

    int nBmpWdtPD=DWordAlign(szBmp.cx*3);
    int nPadD=nBmpWdtPD-szBmp.cx*3;
    int nSzBuffPD=((nBmpWdtPD*szBmp.cy)/8+2)*8;
    LPBYTE pcImgRotate=new BYTE[nSzBuffPD]; ASSERT(pcImgRotate);
    int nWidth,nHeight=szBmp.cy,nHeight1=nHeight-1;
    //====================================
    //------------------------------------
    // here is the example how to speed up lengthy repeatetive processing by using inline assembler
#if !defined(_AMD64_)
#define __USE_MASM__        // the same processing is in C and in asm. To use C -> comment the beginning of this line
#endif
    // Do the actual whole RGB bitmap rotating in C or assembler
#ifndef __USE_MASM__
    LPBYTE pcImgS=pcImg;
    LPBYTE pcImgD=pcImgRotate;
    int ixHeight=0;
    BOOL bLast=FALSE;
    while(ixHeight<nHeight)                    // for all destination height lines
    {    
        nWidth=szBmp.cx;
        if(ixHeight==nHeight1) { bLast=TRUE; nWidth--; }

        while(nWidth--)
        {    
            *(PDWORD)pcImgD=*(PDWORD)pcImgS; // do all Rgb triplets read/write qicker as DWORD
            pcImgS+=nBmpWdtPS;    // increment source in padded source lines
            pcImgD+=3;            // increment destination in rgb triplets
        }

        if(bLast)                // when the last line, the last pixel - colud be a problem if bitmap DWORD alligned 
            for(int c=3;c;c--) 
                *pcImgD++=*pcImgS++;        // (only last three bytes available->could not read/write DWORD)!!
        else
        {    
            ixHeight++;
            pcImgD+=nPadD;                // destination bitmap horizontal padding to DWORD
            pcImgS=pcImg+(ixHeight*3);    // reset the source to the begining of the next vertical line
        }    
    }
#else    // __USE_MASM__
    nBmpWdtPS-=4;                    // adjust esi increment (due to esi self-incrementing by movsd)
    nWidth=szBmp.cx;
    __asm
    {        mov        esi, pcImg            // source index
    mov        edi, pcImgRotate    // destination index
    xor        ebx, ebx            // vertical counter
loop_height:
    mov        ecx, nWidth            // horizontal counter
        cmp        ebx, nHeight1        // check is it the last line
        jne        loop_width
        dec        ecx                    // if it is decremnt for the last pixel

loop_width:
    movsd                        // copies 4 bytes and increments source and destination by 4 (we need only 3 bytes copied 'one pixel' RGB triplet)
        dec        edi                    // adjust edi to 'as incremented by 3'
        add        esi,nBmpWdtPS        // adjust esi to the next source line
        loop    loop_width            // loop one hotizontal destination line 

        cmp        ebx, nHeight1        // check is it the last line
        je        do_last                // if not last, do incrementing here

        inc        ebx                    // increment vertical counter
        add        edi, nPadD            // adjust destination index by possible padding to DWORD
        mov        esi, ebx            // reset the source index: add vertical counter * 3 
        shl        esi, 1                // (is the same as * 2 +1*)
        add        esi, ebx            // +1*
        add        esi, pcImg            // add to the beginning of the source
        jmp        loop_height            // loop whole height

do_last:                        // the last pixel is done by
    movsw                        // moving first two bytes
        movsb                        // and than by moving the very last byte
    }
#endif //     __USE_MASM__
    dcMem.SelectObject(pBmpOld); bmpMem.DeleteObject();        // recreate rotated bitmap
    bmpMem.CreateCompatibleBitmap(pDC,szBmp.cx,szBmp.cy);
    dcMem.SelectObject(&bmpMem);
    bihOut.biWidth =szBmp.cx; bihOut.biHeight=bBody?-szBmp.cy:szBmp.cy;
    SetDIBits(*pDC, bmpMem.operator HBITMAP(),0,szBmp.cy,pcImgRotate,&biOut,DIB_RGB_COLORS); // set rotated bitmap bits
    delete pcImgRotate;
    }
    if(pcImg) delete pcImg;
    // 6th blit mirrored/rotated image to the screen
    pDC->BitBlt(rcItem.left,rcItem.top,szBmp.cx,szBmp.cy,&dcMem,0,0,SRCCOPY); // 
    dcMem.SelectObject(pBmpOld);
}
//----------------------------------------------------------------------------------------------------------
// draw tab item context: possible icon and text
void CXPTabCtrl::DrawTabItem(CDC* pDC, int ixItem, const CRect& rcItemC, UINT uiFlags)
{
    TC_ITEM tci;   CString sText;
    tci.mask      =TCIF_TEXT | TCIF_IMAGE;
    tci.pszText   =sText.GetBuffer(128);
    tci.cchTextMax=127;
    GetItem(ixItem,&tci);
    sText.ReleaseBuffer();

    BOOL bSel   =(uiFlags&2)?TRUE:FALSE;
    BOOL bBottom=(uiFlags&8)?TRUE:FALSE;
    CRect rcItem=rcItemC;
    if(bSel) rcItem.bottom -= 1;
    else     rcItem.bottom += 2;
    rcItem.left+=(bBottom?3:6);                    // text & icon
    rcItem.top +=(bBottom?3:2) + (bSel ? 1 : (bBottom?0:3));

    int nOldMode=pDC->SetBkMode(TRANSPARENT);
    HIMAGELIST hilTabs = (HIMAGELIST)TabCtrl_GetImageList(GetSafeHwnd());    // icon
    if(hilTabs && tci.iImage>=0)
    {    ImageList_Draw(hilTabs, tci.iImage, *pDC, rcItem.left+(bSel?2:0), rcItem.top+(bSel?0:-2), ILD_TRANSPARENT);
    rcItem.left+=19;
    }
    else rcItem.OffsetRect(-2,0);

    if(sText.GetLength())
    {    CFont* pOldFont=pDC->SelectObject(GetFont());        // prepare dc
    rcItem.right-=3;                        // text
    CRect rect(0,0,rcItem.Width(),20);
    ::DrawText(pDC->GetSafeHdc(),sText.GetBuffer(sText.GetLength()+4),-1,rect,DT_CALCRECT|DT_SINGLELINE|DT_MODIFYSTRING|DT_END_ELLIPSIS);
    sText.ReleaseBuffer();

    rcItem.OffsetRect((bBottom?-1:0),(bSel?1:-1));
    pDC->DrawText(sText, rcItem, DT_NOPREFIX|DT_CENTER);
    pDC->SelectObject(pOldFont);
    }
    pDC->SetBkMode(nOldMode);
}
//----------------------------------------------------------------------------------------------------------
BOOL CXPTabCtrl::IsExtendedTabTheamedXP()
{
    if(!m_bTabExtended || !::IsWindow(GetSafeHwnd()))
        return FALSE;
    DWORD dwStyle=GetStyle();
    m_eTabOrientation=(dwStyle&TCS_BOTTOM)?e_tabBottom:e_tabTop;
    if(dwStyle&TCS_VERTICAL)
        m_eTabOrientation=(m_eTabOrientation==e_tabTop)?e_tabLeft:e_tabRight;
#ifdef USE_DEFAULT_XP_TOPTAB
    if(m_eTabOrientation==e_tabTop)    return FALSE;
#endif
    return IsThemeActiveXP()?TRUE:FALSE;
}
//==========================================================================================================
// these two messages are necessary only to properly redraw deselected tab background, because 
void CXPTabCtrl::OnTabSelChanging(NMHDR* pNMHDR, LRESULT* pResult)    // selected rect was inflated by 2 points
{
    UNUSED_ALWAYS(pNMHDR);
    m_ixSelOld=GetCurSel();

    *pResult=0L;

    //hide current tab window
    if(!m_vecTabs.empty() && GetCurSel() < m_vecTabs.size())
    {
        m_vecTabs[GetCurSel()]->ShowWindow(SW_HIDE);
    }
}
//----------------------------------------------------------------------------------------------------------
void CXPTabCtrl::OnTabSelChanged(NMHDR* pNMHDR, LRESULT* pResult)
{
    UNUSED_ALWAYS(pNMHDR);
    if(m_ixSelOld>=0 && m_ixSelOld!=GetCurSel()  && IsExtendedTabTheamedXP())            // else  
    {    
        CWnd* pWndParent=GetParent();
        if(pWndParent)
        {    
            CRect rcOldSel; GetItemRect(m_ixSelOld, rcOldSel); rcOldSel.InflateRect(2,2);
            ClientToScreen(&rcOldSel); pWndParent->ScreenToClient(&rcOldSel);
            pWndParent->InvalidateRect(rcOldSel);
        }    
    }
    *pResult=1L;

    //hide current tab window
    if(!m_vecTabs.empty() && GetCurSel() < m_vecTabs.size())
    {
        m_vecTabs[GetCurSel()]->ShowWindow(SW_SHOW);
    }

    ResizeItems();
}
/***********************************************************************************************************/
// Helper functions
/***********************************************************************************************************/
int DWordAlign(int n)
{ const int rem=n%4; if(rem) n+=(4-rem); return n; }
//----------------------------------------------------------------------------------------------------------
BOOL IsThemeActiveEx()
{                                // check theme activity always (could change during application running)
    HINSTANCE hDll=LoadLibrary(CString((LPCTSTR)IDS_UTIL_UXTHEME));                            // 'UxTheme.dll'
    if(hDll==NULL) return FALSE;                // the DLL won't be available on anything except Windows XP
    UINT (PASCAL *pfnIsThemeActive)();
    (FARPROC&)pfnIsThemeActive=GetProcAddress(hDll,IDS_UTIL_THEMEACT);    // 'IsThemeActive'
    UINT uiThemeActive=0;
    if(pfnIsThemeActive)
        uiThemeActive=pfnIsThemeActive();            
    FreeLibrary(hDll);
    return uiThemeActive?TRUE:FALSE;
}
//----------------------------------------------------------------------------------------------------------
#define PACKVERSION(major,minor) MAKELONG(minor,major)
DWORD GetWinVersion()
{
    static DWORD c_dwWinVers=0;    // check win version only once (will not change during application)
    if(!c_dwWinVers)
    {    OSVERSIONINFO osvi;    ZeroMemory(&osvi, sizeof(OSVERSIONINFO));    // Initialize the OSVERSIONINFO structure.
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    GetVersionEx(&osvi);
    c_dwWinVers=PACKVERSION(osvi.dwMajorVersion,osvi.dwMinorVersion);
    }
    return c_dwWinVers;
}
//----------------------------------------------------------------------------------------------------------
BOOL IsWinXP()         { return GetWinVersion()>=PACKVERSION(5,1)?TRUE:FALSE; }
//----------------------------------------------------------------------------------------------------------
BOOL IsThemeActiveXP() { return (IsWinXP()  && IsThemeActiveEx())?TRUE:FALSE; }
//----------------------------------------------------------------------------------------------------------
#define WPART_NAME_SZ 128
HRESULT DrawThemesPart(HDC hDC, int iPartId, int iStateId, LPCSTR uiPartNameID, LPRECT prcBox)
{
    if(!IsWinXP()) return E_FAIL;
    HINSTANCE hDll=LoadLibrary(CString((LPCTSTR)IDS_UTIL_UXTHEME));                                // 'UxTheme.dll'
    if(!hDll) return E_FAIL;

    BOOL (PASCAL* pfnIsThemeActive)();    UINT hTheme=0;
    (FARPROC&)pfnIsThemeActive=GetProcAddress(hDll,IDS_UTIL_THEMEACT);        // 'IsThemeActive'
    HRESULT hResult=E_FAIL;

    if(pfnIsThemeActive && pfnIsThemeActive())
    {    
        CString sPartName((LPCTSTR)uiPartNameID);
        if(sPartName.GetLength()>0)
        {    
            WCHAR swPartName[WPART_NAME_SZ];
#if !defined(UNICODE) && !defined(_UNICODE)
            MultiByteToWideChar(CP_ACP,0,(LPCSTR)sPartName,-1,swPartName,sizeof(swPartName)/sizeof(WCHAR));
#else
            _tcscpy(swPartName, sPartName.GetBuffer());
#endif
            UINT (PASCAL* pfnOpenThemeData)(HWND hwnd, LPCWSTR pszClassList);
            (FARPROC&)pfnOpenThemeData=GetProcAddress(hDll,IDS_UTIL_THEMEOPN);// 'OpenThemeData'
            if(pfnOpenThemeData && (hTheme=pfnOpenThemeData(NULL, swPartName))!=0)
            {    
                UINT (PASCAL* pfnDrawThemeBackground)(UINT htheme,HDC hdc,int iPartID,int iStateID,const RECT* prcBx,const RECT* prcClip);
                (FARPROC&)pfnDrawThemeBackground=GetProcAddress(hDll,IDS_UTIL_THEMEBCKG);    // 'DrawThemeBackground'
                if(pfnDrawThemeBackground)
                    hResult=pfnDrawThemeBackground(hTheme, hDC, iPartId, iStateId, prcBox, NULL);
            }    
        }    
    }
    FreeLibrary(hDll);
    return hResult;
}
//----------------------------------------------------------------------------------------------------------

void CXPTabCtrl::OnSize(UINT nType, int cx, int cy)
{
    CTabCtrl::OnSize(nType, cx, cy);
    ResizeItems();
}

void CXPTabCtrl::ResizeItems()
{
    if(!m_vecTabs.empty() && GetCurSel()<m_vecTabs.size())
    {
        CWnd* pWnd = m_vecTabs[GetCurSel()];
        CRect rectTab;
        CRect rect;
        GetItemRect(GetCurSel(), rectTab);
        GetWindowRect(&rect);

        //ScreenToClient(&rect);
        switch(m_eTabOrientation)
        {
        default :
        case e_tabTop :
            pWnd->MoveWindow(10, rectTab.Height()+10, rect.Width()-20, rect.Height()-rectTab.Height()-20);
            break;
        case e_tabBottom :
            pWnd->MoveWindow(10, 10, rect.Width()-20, rect.Height() - rectTab.Height()-20);
            break;
        case e_tabRight :
            pWnd->MoveWindow(10, 10, rect.Width() - rectTab.Width() - 20, rect.Height() - 20);
            break;
        case e_tabLeft :
            pWnd->MoveWindow(10 + rectTab.Width(), 10, rect.Width() - rectTab.Width() - 20, rect.Height() - 20);
            break;
        }

    }
}

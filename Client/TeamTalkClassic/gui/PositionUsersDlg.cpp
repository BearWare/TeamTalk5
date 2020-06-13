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

// PositionUsersDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Resource.h"
#include "PositionUsersDlg.h"
#include <math.h>
#include <algorithm>
extern TTInstance* ttInst;

// CPositionUsersDlg dialog

IMPLEMENT_DYNAMIC(CPositionUsersDlg, CDialog)
CPositionUsersDlg::CPositionUsersDlg(const users_t& users, CWnd* pParent /*=NULL*/)
: CDialog(CPositionUsersDlg::IDD, pParent)
, m_bPositionUsers(FALSE)
, m_users(users)
{
}

CPositionUsersDlg::~CPositionUsersDlg()
{
    map_userbtn_t::iterator ite;
    for(ite = m_mapUserBtn.begin();
        ite != m_mapUserBtn.end();
        ite++)
    {
        delete (*ite).second;
    }
}

void CPositionUsersDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Control(pDX, IDOK, m_wndOK);
    DDX_Control(pDX, IDC_STATIC_ROOM, m_wndRoom);
    DDX_Control(pDX, IDC_STATIC_COOR, m_wndCoordinates);
    DDX_Control(pDX, IDC_CHECK_POSITIONING, m_wndPositionBtn);
    DDX_Check(pDX, IDC_CHECK_POSITIONING, m_bPositionUsers);
}


BEGIN_MESSAGE_MAP(CPositionUsersDlg, CDialog)
    ON_MESSAGE(WM_USERBUTTON_DRAGGING, OnUserButtonDragged)
    ON_BN_CLICKED(IDC_BUTTON_DEFAULT, OnBnClickedButtonDefault)
    ON_BN_CLICKED(IDC_CHECK_POSITIONING, OnBnClickedCheckPositioning)
END_MESSAGE_MAP()


// CPositionUsersDlg message handlers


BOOL CPositionUsersDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    TRANSLATE(*this, IDD);

    users_t::const_iterator ite;
    int i=0;
    for(ite=m_users.begin();ite != m_users.end();ite++)
    {
        if(ite->first != TT_GetMyUserID(ttInst))
        {
            CUserButton* pBtn = new CUserButton();
            CRect rect;
            rect.left = 20;
            rect.right = rect.left + 35;
            rect.top = 20;
            rect.bottom = rect.top + 20;
            pBtn->Create(GetDisplayName(ite->second), WS_CHILD|WS_VISIBLE|WS_TABSTOP, rect, this, 9001+i);  // give a unique ID (not strictly necessary)
            i++;
            pBtn->m_nBtnID = ite->second.nUserID;
            pBtn->SetFont(GetFont());

            m_mapUserBtn[ite->second.nUserID] = pBtn;
        }
    }

    PositionUsers();

    return TRUE;  // return TRUE unless you set the focus to a control
    // EXCEPTION: OCX Property Pages should return FALSE
}

LRESULT CPositionUsersDlg::OnUserButtonDragged(WPARAM wParam, LPARAM lParam)
{
    CRect client;
    GetClientRect(&client);
    map_userbtn_t::iterator ite = m_mapUserBtn.find(int(wParam));
    if( ite != m_mapUserBtn.end())
    {
        CUserButton* pBtn = (*ite).second;
        if(pBtn)
        {
            CRect rect;
            pBtn->GetWindowRect(&rect);
            ScreenToClient(&rect);
            CPoint point;
            GetCursorPos(&point);
            ScreenToClient(&point);
            int nWidth = rect.Width();
            int nHeight = rect.Height();

            rect.left = point.x - (nWidth / 2);
            rect.right = point.x + (nWidth / 2);
            if( rect.left < 0 ){ rect.left = 0; rect.right = nWidth; }
            if( rect.right > client.Width() ){ rect.right = client.right; rect.left = rect.right-nWidth; }

            rect.top = point.y - (nHeight / 2);
            rect.bottom = point.y + (nHeight / 2);
            if( rect.top < 0 ){ rect.top = 0; rect.bottom = nHeight; }
            if( rect.bottom > client.Height() ){ rect.bottom = client.bottom; rect.top = rect.bottom-nHeight; }

            pBtn->MoveWindow(rect.left, rect.top, rect.Width(), rect.Height());

            UpdatePosition(int(wParam), pBtn);
        }
    }
    else
        (*ite).second->EnableWindow(FALSE);//user has disconnected

    return TRUE;
}

void CPositionUsersDlg::UpdatePosition(int nUserID, CUserButton* pBtn)
{
    ASSERT(pBtn);
    CRect rect;
    CRect client;
    pBtn->GetWindowRect(&rect);
    ScreenToClient(&rect);
    m_wndRoom.GetWindowRect(&client);
    ScreenToClient(&client);

    CPoint center(client.left + client.Width()/2, client.top + client.Height()/2);
    CPoint btnPoint( rect.left+rect.Width()/2, rect.top+rect.Height()/2);

    float a = float(btnPoint.x - center.x);
    a /= 100.0f;
    float b = float(center.y - btnPoint.y);
    b /= 100.0f;
    float c = a*a+b*b;
    c = sqrt(c);

    CString s;
    s.Format(LoadText(IDS_POSUSERDISTCOORDO, _T("Distance: %.2fm\r\nCoordinates: (%.2f,%.2f)")), c, a, b);
    m_wndCoordinates.SetWindowText(s);

    TT_SetUserPosition(ttInst, nUserID, STREAMTYPE_VOICE, a, b, 0);
}

void CPositionUsersDlg::PositionUsers()
{
    map_userbtn_t::iterator ite;
    for(ite = m_mapUserBtn.begin();
        ite != m_mapUserBtn.end();    
        ite++)
    {
        PositionButton((*ite).second, (*ite).first);
    }
}

void CPositionUsersDlg::PositionButton(CUserButton* pBtn, int nUserID)
{
    ASSERT(pBtn);
    if(pBtn)
    {
        CRect rect;
        CRect client;
        m_wndRoom.GetWindowRect(&client);
        ScreenToClient(&client);

        User user;
        if(TT_GetUser(ttInst, nUserID, &user))
        {
            CPoint center(client.left + client.Width()/2, client.top + client.Height()/2);

            CRect btnRect;
            pBtn->GetWindowRect(btnRect);
            int nWidth = btnRect.Width();
            int nHeight = btnRect.Height();
            btnRect.left = LONG(center.x + (user.soundPositionVoice[0]*100.0f) - nWidth/2);
            btnRect.right = btnRect.left + nWidth;
            btnRect.top = LONG(center.y + (user.soundPositionVoice[1]*100.0f*(-1)) - nHeight/2);
            btnRect.bottom = btnRect.top + nHeight;
            pBtn->MoveWindow(btnRect.left, btnRect.top, btnRect.Width(), btnRect.Height());
        }
        else
        {
            pBtn->EnableWindow(FALSE);
        }
    }
}
void CPositionUsersDlg::OnBnClickedButtonDefault()
{
    BOOL bPos = (TT_GetFlags(ttInst) & CLIENT_SNDOUTPUT_AUTO3DPOSITION) != CLIENT_CLOSED;
    TT_Enable3DSoundPositioning(ttInst, TRUE);
    TT_AutoPositionUsers(ttInst);
    PositionUsers();
    TT_Enable3DSoundPositioning(ttInst, bPos);
}

void CPositionUsersDlg::MoveButton(CUserButton* pBtn, int x, int y)
{
    ASSERT(pBtn);
    if(pBtn)
    {
        CRect rect;
        pBtn->GetWindowRect(rect);
        ScreenToClient(rect);
        rect.left += x;
        rect.right += x;
        rect.top += y;
        rect.bottom += y;
        pBtn->MoveWindow(rect.left, rect.top, rect.Width(), rect.Height());

        map_userbtn_t::iterator ite = m_mapUserBtn.find(pBtn->m_nBtnID);
        UpdatePosition(pBtn->m_nBtnID, pBtn);
    }
}

BOOL CPositionUsersDlg::PreTranslateMessage(MSG* pMsg)
{
    if(pMsg->message == WM_KEYDOWN)
    {
        CWnd* pWnd = GetFocus();
        if(pWnd)
        {
            for(map_userbtn_t::iterator ite = m_mapUserBtn.begin();
                ite != m_mapUserBtn.end();
                ite++)
            {
                if(ite->second == pWnd)
                {
                    bool bHandled = true;
                    switch(pMsg->wParam)
                    {
                    case VK_UP :
                        MoveButton(ite->second, 0, -10);
                        break;
                    case VK_DOWN :
                        MoveButton(ite->second, 0, 10);
                        break;
                    case VK_LEFT :
                        MoveButton(ite->second, -10, 0);
                        break;
                    case VK_RIGHT :
                        MoveButton(ite->second, 10, 0);
                        break;
                    default :
                        bHandled = false;
                    }

                    if(bHandled)
                        return TRUE;
                }
            }
        }
    }

    return CDialog::PreTranslateMessage(pMsg);
}

void CPositionUsersDlg::OnBnClickedCheckPositioning()
{
    if(m_wndPositionBtn.GetCheck() == BST_CHECKED)
        TT_Enable3DSoundPositioning(ttInst, TRUE);
    else
        TT_Enable3DSoundPositioning(ttInst, FALSE);
}

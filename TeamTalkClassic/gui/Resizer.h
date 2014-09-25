/******************************************************************************\
$Copyright: (C)2001 Dmitry Kochin <dco@mail.ru>
$Workfile: Resizer.h $
\******************************************************************************/

// Resizer.h: interface for the CResizer class.
//
//////////////////////////////////////////////////////////////////////
// Class to correctly move child windows after parent window was resized
// Created: 06/07/01 by dukei@
//////////////////////////////////////////////////////////////////////

//An idea:
//
//Each child window side (left, top, right and bottom) is connected to a side of another window,
//so called relative window. It is tipically dialog window, that owns the child window.
//When dialog window is resized, child window sides are moved after the relative window,
//preserving the connections. 
//
//Typical usage:
//
// 1. CResizer m_resizer is member variable;
//
// 2. Add the following code to OnInitDialog() 
// (replacing control IDs to your specific ones).
// See array format description later in the comment
//  
//  static CResizer::CBorderInfo s_bi[] = {
//    {IDC_CONTROL_ID,      {CResizer::eFixed, IDC_MAIN, CResizer::eLeft}, //Left side
//                          {CResizer::eFixed, IDC_MAIN, CResizer::eTop},  //Top side
//                          {CResizer::eFixed, IDC_MAIN, CResizer::eLeft}, //Right side
//                          {CResizer::eFixed, IDC_MAIN, CResizer::eTop}}, //Bottom side
//    {IDC_STATIC_ID,       {CResizer::eFixed, IDC_MAIN, CResizer::eLeft}, 
//                          {CResizer::eFixed, IDC_MAIN, CResizer::eTop},  
//                          {CResizer::eFixed, IDC_MAIN, CResizer::eRight},
//                          {CResizer::eFixed, IDC_MAIN, CResizer::eTop}}, 
//  };
//  const nSize = sizeof(s_bi)/sizeof(s_bi[0]);
//  m_resizer.Init(m_hWnd, NULL, s_bi, nSize);
//
// 3. Add the following code to OnSize() handler
//
//  m_resizer.Move();
//
// 4. Everything should work now
//
// P.S. Data array format consists of one or more CBorderInfo structures, which contains
// moved control ID (first field) and four CBorder structures, 
// for left, top, right and bottom sides of moved control accordingly.
//
// The main difficulty is to understand CBorder structure, which has the following format:
// 
// {<how control side is connected to side of another window>, <another window id>, 
//                  <side of another window, to which a control side is connected>}
// 
//     CResizer::eFixed                                  CResizer::eLeft 
//     CResizer::eProportional  IDC_MAIN                 CResizer::eTop   
// or {CResizer::eWidth       , IDC_ANOTHER_CONTROL_ID , CResizer::eRight   }
//     CResizer::eHeight                                 CResizer::eBottom
//                                                       CResizer::eXCenter
//                                                       CResizer::eYCenter
//
// For example, {CResizer::eFixed, IDC_MAIN, CResizer::eLeft} means, that moved control side is
// on the fixed distance (CResizer::eFixed) from left side (CResizer::eLeft) of dialog window (IDC_MAIN)
//
// Another example: {CResizer::eProportional, IDC_CONTROL_ID, CResizer::eLeft} means, that
// moved control side preserves relation (proportionaly) (CResizer::eProportional)
// to the width (CResizer::eLeft or CResizer::eRight) of control IDC_CONTROL_ID.
//
// TIP: Resizer resizes controls in the order they are defined in the array, so
// <another window id> should always be defined (and, therefore, moved by the resizer) before
// it is used as relative window. Otherwise, resizer ASSERTs.

#pragma once

#define IDC_MAIN 0   //Parent dialog ID to be relative window

class CResizer  
{
public:
  enum EBorder{
    eFixed = 1,  //Given distance to specified window side
    eProportional, //Coordinate is changed proportionally to width/height of specified window
    eWidth,   //The width is preserved (relative window and window side are ignored)
    eHeight,  //The height is preserved (relative window and window side are ignored)
  };
  enum ESize{
    eLeft = 1,  //Relative to left side
    eTop,       //Top
    eRight,     //Right
    eBottom,    //Bottom
    eXCenter,   //The center of width
    eYCenter    //The center of height
  };
  struct CBorder{
    EBorder eType;  //Type of relation to relative control side
    int nRelID;     //Relative control
    ESize eRelType; //Side of relative control
  };
  struct CBorderInfo{
    int nID;         //Control ID
    CBorder left;    
    CBorder top;
    CBorder right;
    CBorder bottom;
  };
private:
  struct CControlInfo{
    RECT rcInitial; //initial control position;
    const CBorderInfo *pInfo;
  };
  struct CRectInfo{
    int nID;       //Control ID
    RECT rc;      //New control rect
    bool bVisible; //If control is visible
    bool bHide;    //If control should be hidden because it overlaps senior control
  };
  HWND m_wndParent;
  RECT m_rcInitial; //Initial window client area
  typedef CControlInfo * TInfo;
  typedef CRectInfo * TRectInfo;
  TInfo m_vInfo;
  int m_nSize; //Size of m_vInfo array
  mutable TRectInfo m_vRectInfo;
  mutable int m_nCachedSize; //Size of m_vRectInfo
protected:
  HWND GetDlgItem(int nID) const{return ::GetDlgItem(m_wndParent, nID);}
  void GetDlgItemRect(int nID, RECT &rc) const;
  void GetCachedDlgItemRect(int nID, RECT &rc) const;
  void GetInitialDlgItemRect(int nID, RECT &rc) const;
  void Move(int nIndex) const;
  int GetCoordinate(ESize eType, const RECT &rcInitial, const CBorder &border, const RECT &rc) const;
  static int GetRectCoord(ESize eType, const RECT &rc);
  static int GetRectSize(ESize eType, const RECT &rc);
  int GetRelativeCoord(const CBorder &border) const;
  int Find(int nID) const;
  int FindCached(int nID) const;
  void MoveAndHideOverlapped() const;
  void Clear();
public:
  //Initializes resizer
  bool Init(HWND hWndParent, LPCRECT rcInitial, const CBorderInfo *pBorders, int nSize);
  //Performs moving of controls
  void Move() const;
  //Just constructor
  CResizer();
  //Just destructor
  ~CResizer();
};


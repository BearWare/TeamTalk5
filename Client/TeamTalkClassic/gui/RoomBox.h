#pragma once


// CRoomBox

class CRoomBox : public CStatic
{
    DECLARE_DYNAMIC(CRoomBox)

public:
    CRoomBox();
    virtual ~CRoomBox();

protected:
    DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnPaint();
};



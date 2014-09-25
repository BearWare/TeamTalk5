#pragma once


// CVertSliderCtrl

class CMySliderCtrl : public CSliderCtrl
{
    DECLARE_DYNAMIC(CMySliderCtrl)

public:
    CMySliderCtrl();
    virtual ~CMySliderCtrl();
    void SetPos(int nPos);
    int GetPos( ) const;

protected:
    DECLARE_MESSAGE_MAP()
};



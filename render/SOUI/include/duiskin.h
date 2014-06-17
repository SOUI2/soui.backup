
#pragma once
#include "skinobj-i.h"
#include "res.mgr/duiimgpool.h"

namespace SOUI
{

// State Define
enum
{
    DuiWndState_Normal       = 0x00000000UL,
    DuiWndState_Hover        = 0x00000001UL,
    DuiWndState_PushDown     = 0x00000002UL,
    DuiWndState_Check        = 0x00000004UL,
    DuiWndState_Invisible    = 0x00000008UL,
    DuiWndState_Disable      = 0x00000010UL,
};

#define IIF_STATE2(the_state, normal_value, hover_value) \
    (((the_state) & DuiWndState_Hover) ? (hover_value) : (normal_value))

#define IIF_STATE3(the_state, normal_value, hover_value, pushdown_value) \
    (((the_state) & DuiWndState_PushDown) ? (pushdown_value) : IIF_STATE2(the_state, normal_value, hover_value))

#define IIF_STATE4(the_state, normal_value, hover_value, pushdown_value, disable_value) \
    (((the_state) & DuiWndState_Disable) ? (disable_value) : IIF_STATE3(the_state, normal_value, hover_value, pushdown_value))


//////////////////////////////////////////////////////////////////////////
class SOUI_EXP SSkinImgList: public ISkinObj
{
    SOUI_CLASS_NAME(SSkinImgList, "imglist")

public:
    SSkinImgList();
    virtual ~SSkinImgList();

    virtual void Draw(IRenderTarget *pRT, LPCRECT rcDraw, DWORD dwState,BYTE byAlpha);

    virtual SIZE GetSkinSize();

    virtual BOOL IgnoreState();
    
    virtual int GetStates();
    void    SetStates(int nStates){m_nStates=nStates;}


    void SetImage(IBitmap *pImg)
    {
        if(m_pImg) m_pImg->Release();
        m_pImg=pImg;
        if(m_pImg) m_pImg->AddRef();
    }

    IBitmap * GetImage()
    {
        return m_pImg;
    }

    void SetTile(BOOL bTile){m_bTile=bTile;}
    BOOL IsTile(){return m_bTile;}

    void SetVertical(BOOL bVertical){m_bVertical=bVertical;}
    BOOL IsVertical(){return m_bVertical;}
    
    
protected:
    IBitmap *m_pImg;
    int  m_nStates;
    BOOL m_bTile;
    BOOL m_bVertical;

    SOUI_ATTRS_BEGIN()
        ATTR_IMAGE("src", m_pImg, TRUE)
        ATTR_INT("tile", m_bTile, TRUE)
        ATTR_INT("vertical", m_bVertical, TRUE)
        ATTR_INT("states",m_nStates,TRUE)
    SOUI_ATTRS_END()
};

//////////////////////////////////////////////////////////////////////////
class SOUI_EXP SSkinImgFrame : public SSkinImgList
{
    SOUI_CLASS_NAME(SSkinImgFrame, "imgframe")

public:
    SSkinImgFrame();


    void SetMargin(const CRect rcMargin){m_rcMargin=rcMargin;}

    CRect GetMargin(){return m_rcMargin;}

    virtual void Draw(IRenderTarget *pRT, LPCRECT rcDraw, DWORD dwState,BYTE byAlpha);
protected:
    CRect m_rcMargin;

    SOUI_ATTRS_BEGIN()
        ATTR_INT("left", m_rcMargin.left, TRUE)
        ATTR_INT("top", m_rcMargin.top, TRUE)
        ATTR_INT("right", m_rcMargin.right, TRUE)
        ATTR_INT("bottom", m_rcMargin.bottom, TRUE)
        ATTR_INT("x", m_rcMargin.left=m_rcMargin.right, TRUE)
        ATTR_INT("y", m_rcMargin.top=m_rcMargin.bottom, TRUE)
    SOUI_ATTRS_END()
};

//////////////////////////////////////////////////////////////////////////
class SOUI_EXP SSkinButton : public ISkinObj
{
    SOUI_CLASS_NAME(SSkinButton, "button")

    enum{
        ST_NORMAL=0,
        ST_HOVER,
        ST_PUSHDOWN,
        ST_DISABLE,
    };

public:
    SSkinButton();

    virtual void Draw(IRenderTarget *pRT, LPCRECT rcDraw, DWORD dwState,BYTE byAlpha);

    virtual BOOL IgnoreState();

    virtual int GetStates();

    void SetColors(COLORREF crUp[4],COLORREF crDown[4],COLORREF crBorder);

protected:
    COLORREF m_crBorder;

    COLORREF    m_crUp[4];
    COLORREF    m_crDown[4];
public:
    SOUI_ATTRS_BEGIN()
        ATTR_COLOR("border", m_crBorder, TRUE)
        ATTR_COLOR("bgup", m_crUp[ST_NORMAL], TRUE)
        ATTR_COLOR("bgdown", m_crDown[ST_NORMAL], TRUE)
        ATTR_COLOR("bguphover", m_crUp[ST_HOVER], TRUE)
        ATTR_COLOR("bgdownhover", m_crDown[ST_HOVER], TRUE)
        ATTR_COLOR("bguppush", m_crUp[ST_PUSHDOWN], TRUE)
        ATTR_COLOR("bgdownpush", m_crDown[ST_PUSHDOWN], TRUE)
        ATTR_COLOR("bgupdisable", m_crUp[ST_DISABLE], TRUE)
        ATTR_COLOR("bgdowndisable", m_crDown[ST_DISABLE], TRUE)
    SOUI_ATTRS_END()
};

//////////////////////////////////////////////////////////////////////////

class SOUI_EXP SSkinGradation  : public ISkinObj
{
    SOUI_CLASS_NAME(SSkinGradation, "gradation")
public:
    SSkinGradation();

    virtual void Draw(IRenderTarget *pRT, LPCRECT prcDraw, DWORD dwState,BYTE byAlpha);
    
    void SetColorFrom(COLORREF crFrom)
    {
        m_crFrom=crFrom;
    }

    void SetColorTo(COLORREF crTo)
    {
        m_crTo=crTo;
    }

    void SetVertical(BOOL bVertical)
    {
        m_bVert=bVertical;
    }

protected:
    COLORREF m_crFrom;
    COLORREF m_crTo;
    BOOL m_bVert;

    SOUI_ATTRS_BEGIN()
        ATTR_COLOR("crFrom", m_crFrom, TRUE)
        ATTR_COLOR("crTo", m_crTo, TRUE)
        ATTR_INT("vert", m_bVert, TRUE)
    SOUI_ATTRS_END()
};


//////////////////////////////////////////////////////////////////////////
//
enum SBSTATE{
    SBST_NORMAL=0,    //正常状态
    SBST_HOVER,        //hover状态
    SBST_PUSHDOWN,    //按下状态
    SBST_DISABLE,    //禁用状态
    SBST_INACTIVE,    //失活状态,主要针对两端的箭头
};

#define MAKESBSTATE(sbCode,nState1,bVertical) MAKELONG((sbCode),MAKEWORD((nState1),(bVertical)))
#define SB_CORNOR        10
#define SB_THUMBGRIPPER    11    //滚动条上的可拖动部分

#define THUMB_MINSIZE    18

class SOUI_EXP SSkinScrollbar : public SSkinImgList
{
    SOUI_CLASS_NAME(SSkinScrollbar, "scrollbar")

public:

    SSkinScrollbar();

    virtual void Draw(IRenderTarget *pRT, LPCRECT prcDraw, DWORD dwState,BYTE byAlpha=0xff);

    //指示滚动条皮肤是否支持显示上下箭头
    virtual BOOL HasArrow(){return TRUE;}
    
    virtual int GetIdealSize(){
        if(!m_pImg) return 0;
        return m_pImg->Width()/9;
    }

protected:
    //返回源指定部分在原位图上的位置。
    CRect GetPartRect(int nSbCode, int nState,BOOL bVertical);
    int            m_nMargin;
    BOOL        m_bHasGripper;
    BOOL        m_bHasInactive;//有失活状态的箭头时，滚动条皮肤有必须有5行，否则可以是3行或者4行

    SOUI_ATTRS_BEGIN()
        ATTR_INT("margin",m_nMargin,FALSE)
        ATTR_INT("hasgripper",m_bHasGripper,FALSE)
        ATTR_INT("hasinactive",m_bHasInactive,FALSE)
    SOUI_ATTRS_END()
};

class SOUI_EXP SSkinMenuBorder : public SSkinImgFrame
{
    SOUI_CLASS_NAME(SSkinMenuBorder, "border")

public:

    SSkinMenuBorder():m_rcBorder(1,1,1,1)
    {

    }

    CRect GetMarginRect()
    {
        return m_rcBorder;
    }
protected:
    CRect        m_rcBorder;

    SOUI_ATTRS_BEGIN()
        ATTR_RECT("border",m_rcBorder,FALSE)
    SOUI_ATTRS_END()
};

}//namespace SOUI
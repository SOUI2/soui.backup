//////////////////////////////////////////////////////////////////////////
//   File Name: DuiSkinPool
// Description: DuiWindow Skin Definition
//     Creator: ZhangXiaoxuan
//     Version: 2009.4.22 - 1.0 - Create
//                2012.8.18   1.1   huangjianxiong
//////////////////////////////////////////////////////////////////////////

#pragma once
#include "duiskinbase.h"
#include "MemDC.h"

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

class SOUI_EXP CDuiSkinImgList: public CDuiSkinBase
{
    SOUI_CLASS_NAME(CDuiSkinImgList, "imglst")

public:
    CDuiSkinImgList();
    virtual ~CDuiSkinImgList();

    virtual void Draw(HDC dc, CRect rcDraw, DWORD dwState,BYTE byAlpha);

    virtual SIZE GetSkinSize();

    virtual BOOL IgnoreState();
    
    int GetStates();

    void    SetStates(int nStates){m_nStates=nStates;}

    virtual void SetWidth(LONG width) {
        m_lSubImageWidth=width;
        m_bVertical=FALSE;
        if(m_nStates==0)
        {
            if(m_pDuiImg)
                m_nStates=m_pDuiImg->Width()/m_lSubImageWidth;
            else
                m_nStates=1;
        }
    }

    void SetTile(BOOL bTile){m_bTile=bTile;}
    BOOL IsTile(){return m_bTile;}

    void SetVertical(BOOL bVertical){m_bVertical=bVertical;}
    BOOL IsVertical(){return m_bVertical;}
protected:
    virtual void OnAttributeFinish(pugi::xml_node xmlNode);
    virtual void PrepareCache(HDC hdc,CSize & sz);
    virtual void _Draw(HDC dc, CRect rcDraw, DWORD dwState,BYTE byAlpha);

    LONG m_lSubImageWidth;
    int  m_nStates;
    BOOL m_bTile;
    BOOL m_bVertical;

    BOOL m_bCache;
    CMemDC * m_memdc;    
    CSize  m_szTarget;

    SOUI_ATTRS_BEGIN()
    ATTR_INT("subwidth", m_lSubImageWidth, TRUE)
    ATTR_INT("tile", m_bTile, TRUE)
    ATTR_INT("vertical", m_bVertical, TRUE)
    ATTR_INT("states",m_nStates,TRUE)
    ATTR_INT("cache",m_bCache,TRUE)
    SOUI_ATTRS_END()
};

class SOUI_EXP CDuiSkinImgFrame : public CDuiSkinImgList
{
    SOUI_CLASS_NAME(CDuiSkinImgFrame, "imgframe")

public:
    CDuiSkinImgFrame();


    void SetMargin(const CRect rcMargin){m_rcMargin=rcMargin;}

    CRect GetMargin(){return m_rcMargin;}

    UINT  GetDrawPart() {return m_uDrawPart;}

    void SetDrawPart(UINT uDrawPart){m_uDrawPart=uDrawPart;}

    COLORREF GetBgColor(){return m_crBg;}

    void SetBgColor(COLORREF cr){m_crBg=cr;}
protected:
    virtual void _Draw(HDC dc, CRect rcDraw, DWORD dwState,BYTE byAlpha);
    virtual void OnAttributeFinish(pugi::xml_node xmlNode);

    CRect m_rcMargin;
    COLORREF m_crBg;
    UINT m_uDrawPart;
public:
    SOUI_ATTRS_BEGIN()
    ATTR_COLOR("crbg", m_crBg, TRUE)
    ATTR_INT("left", m_rcMargin.left, TRUE)
    ATTR_INT("top", m_rcMargin.top, TRUE)
    ATTR_INT("right", m_rcMargin.right, TRUE)
    ATTR_INT("bottom", m_rcMargin.bottom, TRUE)
    ATTR_HEX("part2", m_uDrawPart, TRUE)
    ATTR_ENUM_BEGIN("part", UINT, TRUE)
        ATTR_ENUM_VALUE("all", Frame_Part_All)
        ATTR_ENUM_VALUE("top", Frame_Part_Top)
        ATTR_ENUM_VALUE("middle", Frame_Part_Mid)
        ATTR_ENUM_VALUE("bottom", Frame_Part_Bottom)
        ATTR_ENUM_VALUE("left", Frame_Part_Left)
        ATTR_ENUM_VALUE("center", Frame_Part_Center)
        ATTR_ENUM_VALUE("right", Frame_Part_Right)
        ATTR_ENUM_VALUE("topleft", Frame_Part_TopLeft)
        ATTR_ENUM_VALUE("topcenter", Frame_Part_TopCenter)
        ATTR_ENUM_VALUE("topright", Frame_Part_TopRight)
        ATTR_ENUM_VALUE("midleft", Frame_Part_MidLeft)
        ATTR_ENUM_VALUE("midcenter", Frame_Part_MidCenter)
        ATTR_ENUM_VALUE("midright", Frame_Part_MidRight)
        ATTR_ENUM_VALUE("bottomleft", Frame_Part_BottomLeft)
        ATTR_ENUM_VALUE("bottomcenter", Frame_Part_BottomCenter)
        ATTR_ENUM_VALUE("bottomright", Frame_Part_BottomRight)
    ATTR_ENUM_END(m_uDrawPart)
    SOUI_ATTRS_END()
};


class SOUI_EXP CDuiSkinButton : public CDuiSkinBase
{
    SOUI_CLASS_NAME(CDuiSkinButton, "button")

    enum{
        ST_NORMAL=0,
        ST_HOVER,
        ST_PUSHDOWN,
        ST_DISABLE,
    };

public:
    CDuiSkinButton();

    virtual void Draw(HDC dc, CRect rcDraw, DWORD dwState,BYTE byAlpha);

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

class SOUI_EXP CDuiSkinGradation  : public CDuiSkinBase
{
    enum GRA_DIR
    {
        DIR_VERT=0,
        DIR_HORZ,
    };

    SOUI_CLASS_NAME(CDuiSkinGradation, "gradation")
public:
    CDuiSkinGradation();

    virtual void Draw(HDC dc, CRect rcDraw, DWORD dwState,BYTE byAlpha);
    
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
        m_uDirection=bVertical?DIR_VERT:DIR_HORZ;
    }

protected:
    COLORREF m_crFrom;
    COLORREF m_crTo;
    GRA_DIR     m_uDirection;
public:
    SOUI_ATTRS_BEGIN()
    ATTR_COLOR("from", m_crFrom, TRUE)
    ATTR_COLOR("to", m_crTo, TRUE)
    ATTR_ENUM_BEGIN("dir", GRA_DIR, TRUE)
        ATTR_ENUM_VALUE("horz", DIR_HORZ)
        ATTR_ENUM_VALUE("vert", DIR_VERT)
    ATTR_ENUM_END(m_uDirection)
    SOUI_ATTRS_END()
};

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

class SOUI_EXP CDuiScrollbarSkin : public CDuiSkinImgFrame
{
    SOUI_CLASS_NAME(CDuiScrollbarSkin, "scrollbar")

public:

    CDuiScrollbarSkin();

    virtual void Draw(HDC dc, CRect rcDraw, DWORD dwState,BYTE byAlpha=0xff);

    //指示滚动条皮肤是否支持显示上下箭头
    virtual BOOL HasArrow(){return TRUE;}
    virtual int GetIdealSize(){
        if(!m_pDuiImg) return 0;
        return m_pDuiImg->Width()/9;
    }

    SOUI_ATTRS_BEGIN()
        ATTR_INT("margin",m_nMargin,FALSE)
        ATTR_INT("hasgripper",m_bHasGripper,FALSE)
        ATTR_INT("hasinactive",m_bHasInactive,FALSE)
    SOUI_ATTRS_END()
protected:
    //返回源指定部分在原位图上的位置。
    CRect GetPartRect(int nSbCode, int nState,BOOL bVertical);
    int            m_nMargin;
    BOOL        m_bHasGripper;
    BOOL        m_bHasInactive;//有失活状态的箭头时，滚动条皮肤有必须有5行，否则可以是3行或者4行
};

class SOUI_EXP CDuiSkinMenuBorder : public CDuiSkinImgFrame
{
    SOUI_CLASS_NAME(CDuiSkinMenuBorder, "border")

public:

    CDuiSkinMenuBorder():m_rcBorder(1,1,1,1)
    {

    }

    CRect GetMarginRect()
    {
        return m_rcBorder;
    }
public:
    SOUI_ATTRS_BEGIN()
    ATTR_RECT("border",m_rcBorder,FALSE)
    SOUI_ATTRS_END()
protected:
    CRect        m_rcBorder;
};

}//namespace SOUI
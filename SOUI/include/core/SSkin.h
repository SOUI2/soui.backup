#pragma once

#include "SSkinObjBase.h"

namespace SOUI
{

// State Define
enum
{
    WndState_Normal       = 0x00000000UL,
    WndState_Hover        = 0x00000001UL,
    WndState_PushDown     = 0x00000002UL,
    WndState_Check        = 0x00000004UL,
    WndState_Invisible    = 0x00000008UL,
    WndState_Disable      = 0x00000010UL,
};

#define IIF_STATE2(the_state, normal_value, hover_value) \
    (((the_state) & WndState_Hover) ? (hover_value) : (normal_value))

#define IIF_STATE3(the_state, normal_value, hover_value, pushdown_value) \
    (((the_state) & WndState_PushDown) ? (pushdown_value) : IIF_STATE2(the_state, normal_value, hover_value))

#define IIF_STATE4(the_state, normal_value, hover_value, pushdown_value, disable_value) \
    (((the_state) & WndState_Disable) ? (disable_value) : IIF_STATE3(the_state, normal_value, hover_value, pushdown_value))


//////////////////////////////////////////////////////////////////////////
class SOUI_EXP SSkinImgList: public SSkinObjBase
{
    SOUI_CLASS_NAME(SSkinImgList, L"imglist")

public:
    SSkinImgList();
    virtual ~SSkinImgList();


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
    virtual void _Draw(IRenderTarget *pRT, LPCRECT rcDraw, DWORD dwState,BYTE byAlpha);
    HRESULT OnAttrImage(const SStringW & strValue,BOOL bLoading);

    IBitmap *m_pImg;
    int  m_nStates;
    BOOL m_bTile;
    BOOL m_bVertical;
    SOUI_ATTRS_BEGIN()
        ATTR_CUSTOM(L"src", OnAttrImage)    //skinObj引用的图片文件定义在uires.idx中的name属性。
        ATTR_INT(L"tile", m_bTile, TRUE)    //绘制是否平铺,0--位伸（默认），其它--平铺
        ATTR_INT(L"vertical", m_bVertical, TRUE)//子图是否垂直排列，0--水平排列(默认), 其它--垂直排列
        ATTR_INT(L"states",m_nStates,TRUE)  //子图数量,默认为1
    SOUI_ATTRS_END()
};

//////////////////////////////////////////////////////////////////////////
class SOUI_EXP SSkinImgFrame : public SSkinImgList
{
    SOUI_CLASS_NAME(SSkinImgFrame, L"imgframe")

public:
    SSkinImgFrame();


    void SetMargin(const CRect rcMargin){m_rcMargin=rcMargin;}

    CRect GetMargin(){return m_rcMargin;}

protected:
    virtual void _Draw(IRenderTarget *pRT, LPCRECT rcDraw, DWORD dwState,BYTE byAlpha);
    CRect m_rcMargin;

    SOUI_ATTRS_BEGIN()
        ATTR_INT(L"left", m_rcMargin.left, FALSE)        //九宫格左边距
        ATTR_INT(L"top", m_rcMargin.top, FALSE)          //九宫格上边距
        ATTR_INT(L"right", m_rcMargin.right, FALSE)      //九宫格右边距
        ATTR_INT(L"bottom", m_rcMargin.bottom, FALSE)    //九宫格下边距
        ATTR_INT(L"margin-x", m_rcMargin.left=m_rcMargin.right, FALSE)//九宫格左右边距
        ATTR_INT(L"margin-y", m_rcMargin.top=m_rcMargin.bottom, FALSE)//九宫格上下边距
        ATTR_RECT(L"margin" ,m_rcMargin,FALSE)          //九宫格4周
    SOUI_ATTRS_END()
};

//////////////////////////////////////////////////////////////////////////
class SOUI_EXP SSkinButton : public SSkinObjBase
{
    SOUI_CLASS_NAME(SSkinButton, L"button")

    enum{
        ST_NORMAL=0,
        ST_HOVER,
        ST_PUSHDOWN,
        ST_DISABLE,
    };

public:
    SSkinButton();


    virtual BOOL IgnoreState();

    virtual int GetStates();

    void SetColors(COLORREF crUp[4],COLORREF crDown[4],COLORREF crBorder);

protected:
    virtual void _Draw(IRenderTarget *pRT, LPCRECT rcDraw, DWORD dwState,BYTE byAlpha);
    COLORREF m_crBorder;

    COLORREF    m_crUp[4];
    COLORREF    m_crDown[4];

    SOUI_ATTRS_BEGIN()
        ATTR_COLOR(L"colorBorder", m_crBorder, TRUE)                //边框颜色
        ATTR_COLOR(L"colorUp", m_crUp[ST_NORMAL], TRUE)             //正常状态渐变起始颜色
        ATTR_COLOR(L"colorDown", m_crDown[ST_NORMAL], TRUE)         //正常状态渐变终止颜色
        ATTR_COLOR(L"colorUpHover", m_crUp[ST_HOVER], TRUE)         //浮动状态渐变起始颜色
        ATTR_COLOR(L"colorDownHover", m_crDown[ST_HOVER], TRUE)     //浮动状态渐变终止颜色
        ATTR_COLOR(L"colorUpPush", m_crUp[ST_PUSHDOWN], TRUE)       //下压状态渐变起始颜色
        ATTR_COLOR(L"colorDownPush", m_crDown[ST_PUSHDOWN], TRUE)   //下压状态渐变终止颜色
        ATTR_COLOR(L"colorUpDisable", m_crUp[ST_DISABLE], TRUE)     //禁用状态渐变起始颜色
        ATTR_COLOR(L"colorDownDisable", m_crDown[ST_DISABLE], TRUE) //禁用状态渐变终止颜色
    SOUI_ATTRS_END()
};

//////////////////////////////////////////////////////////////////////////

class SOUI_EXP SSkinGradation  : public SSkinObjBase
{
    SOUI_CLASS_NAME(SSkinGradation, L"gradation")
public:
    SSkinGradation();
    
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
    virtual void _Draw(IRenderTarget *pRT, LPCRECT prcDraw, DWORD dwState,BYTE byAlpha);

    COLORREF m_crFrom;
    COLORREF m_crTo;
    BOOL m_bVert;

    SOUI_ATTRS_BEGIN()
        ATTR_COLOR(L"colorFrom", m_crFrom, TRUE)    //渐变起始颜色
        ATTR_COLOR(L"colorTo", m_crTo, TRUE)        //渐变终止颜色
        ATTR_INT(L"vertical", m_bVert, TRUE)        //渐变方向,0--水平(默认), 1--垂直
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
    SOUI_CLASS_NAME(SSkinScrollbar, L"scrollbar")

public:

    SSkinScrollbar();

    //指示滚动条皮肤是否支持显示上下箭头
    virtual BOOL HasArrow(){return TRUE;}
    
    virtual int GetIdealSize(){
        if(!m_pImg) return 0;
        return m_pImg->Width()/9;
    }

protected:
    virtual void _Draw(IRenderTarget *pRT, LPCRECT prcDraw, DWORD dwState,BYTE byAlpha);
    //返回源指定部分在原位图上的位置。
    CRect GetPartRect(int nSbCode, int nState,BOOL bVertical);
    int         m_nMargin;
    BOOL        m_bHasGripper;
    BOOL        m_bHasInactive;//有失活状态的箭头时，滚动条皮肤有必须有5行，否则可以是3行或者4行

    SOUI_ATTRS_BEGIN()
        ATTR_INT(L"margin",m_nMargin,FALSE)             //边缘不拉伸大小
        ATTR_INT(L"hasGripper",m_bHasGripper,FALSE)     //滑块上是否有帮手(gripper)
        ATTR_INT(L"hasInactive",m_bHasInactive,FALSE)   //是否有禁用态
    SOUI_ATTRS_END()
};


}//namespace SOUI
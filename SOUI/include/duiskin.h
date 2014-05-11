//////////////////////////////////////////////////////////////////////////
//   File Name: DuiSkinPool
// Description: DuiWindow Skin Definition
//     Creator: ZhangXiaoxuan
//     Version: 2009.4.22 - 1.0 - Create
//				2012.8.18   1.1   huangjianxiong
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
    DUIOBJ_DECLARE_CLASS_NAME(CDuiSkinImgList, "imglst")

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
				m_nStates=m_pDuiImg->GetWidth()/m_lSubImageWidth;
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

    DUIWIN_DECLARE_ATTRIBUTES_BEGIN()
    DUIWIN_INT_ATTRIBUTE("subwidth", m_lSubImageWidth, TRUE)
    DUIWIN_INT_ATTRIBUTE("tile", m_bTile, TRUE)
	DUIWIN_INT_ATTRIBUTE("vertical", m_bVertical, TRUE)
	DUIWIN_INT_ATTRIBUTE("states",m_nStates,TRUE)
	DUIWIN_INT_ATTRIBUTE("cache",m_bCache,TRUE)
    DUIWIN_DECLARE_ATTRIBUTES_END()
};

class SOUI_EXP CDuiSkinImgFrame : public CDuiSkinImgList
{
    DUIOBJ_DECLARE_CLASS_NAME(CDuiSkinImgFrame, "imgframe")

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
    DUIWIN_DECLARE_ATTRIBUTES_BEGIN()
    DUIWIN_COLOR_ATTRIBUTE("crbg", m_crBg, TRUE)
    DUIWIN_INT_ATTRIBUTE("left", m_rcMargin.left, TRUE)
    DUIWIN_INT_ATTRIBUTE("top", m_rcMargin.top, TRUE)
    DUIWIN_INT_ATTRIBUTE("right", m_rcMargin.right, TRUE)
    DUIWIN_INT_ATTRIBUTE("bottom", m_rcMargin.bottom, TRUE)
	DUIWIN_HEX_ATTRIBUTE("part2", m_uDrawPart, TRUE)
    DUIWIN_ENUM_ATTRIBUTE("part", UINT, TRUE)
		DUIWIN_ENUM_VALUE("all", Frame_Part_All)
		DUIWIN_ENUM_VALUE("top", Frame_Part_Top)
		DUIWIN_ENUM_VALUE("middle", Frame_Part_Mid)
		DUIWIN_ENUM_VALUE("bottom", Frame_Part_Bottom)
		DUIWIN_ENUM_VALUE("left", Frame_Part_Left)
		DUIWIN_ENUM_VALUE("center", Frame_Part_Center)
		DUIWIN_ENUM_VALUE("right", Frame_Part_Right)
		DUIWIN_ENUM_VALUE("topleft", Frame_Part_TopLeft)
		DUIWIN_ENUM_VALUE("topcenter", Frame_Part_TopCenter)
		DUIWIN_ENUM_VALUE("topright", Frame_Part_TopRight)
		DUIWIN_ENUM_VALUE("midleft", Frame_Part_MidLeft)
		DUIWIN_ENUM_VALUE("midcenter", Frame_Part_MidCenter)
		DUIWIN_ENUM_VALUE("midright", Frame_Part_MidRight)
		DUIWIN_ENUM_VALUE("bottomleft", Frame_Part_BottomLeft)
		DUIWIN_ENUM_VALUE("bottomcenter", Frame_Part_BottomCenter)
		DUIWIN_ENUM_VALUE("bottomright", Frame_Part_BottomRight)
    DUIWIN_ENUM_END(m_uDrawPart)
    DUIWIN_DECLARE_ATTRIBUTES_END()
};


class SOUI_EXP CDuiSkinButton : public CDuiSkinBase
{
    DUIOBJ_DECLARE_CLASS_NAME(CDuiSkinButton, "button")

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

	COLORREF	m_crUp[4];
	COLORREF	m_crDown[4];
public:
    DUIWIN_DECLARE_ATTRIBUTES_BEGIN()
		DUIWIN_COLOR_ATTRIBUTE("border", m_crBorder, TRUE)
		DUIWIN_COLOR_ATTRIBUTE("bgup", m_crUp[ST_NORMAL], TRUE)
		DUIWIN_COLOR_ATTRIBUTE("bgdown", m_crDown[ST_NORMAL], TRUE)
		DUIWIN_COLOR_ATTRIBUTE("bguphover", m_crUp[ST_HOVER], TRUE)
		DUIWIN_COLOR_ATTRIBUTE("bgdownhover", m_crDown[ST_HOVER], TRUE)
		DUIWIN_COLOR_ATTRIBUTE("bguppush", m_crUp[ST_PUSHDOWN], TRUE)
		DUIWIN_COLOR_ATTRIBUTE("bgdownpush", m_crDown[ST_PUSHDOWN], TRUE)
		DUIWIN_COLOR_ATTRIBUTE("bgupdisable", m_crUp[ST_DISABLE], TRUE)
		DUIWIN_COLOR_ATTRIBUTE("bgdowndisable", m_crDown[ST_DISABLE], TRUE)
    DUIWIN_DECLARE_ATTRIBUTES_END()
};

class SOUI_EXP CDuiSkinGradation  : public CDuiSkinBase
{
	enum GRA_DIR
	{
		DIR_VERT=0,
		DIR_HORZ,
	};

    DUIOBJ_DECLARE_CLASS_NAME(CDuiSkinGradation, "gradation")
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
    DUIWIN_DECLARE_ATTRIBUTES_BEGIN()
    DUIWIN_COLOR_ATTRIBUTE("from", m_crFrom, TRUE)
    DUIWIN_COLOR_ATTRIBUTE("to", m_crTo, TRUE)
    DUIWIN_ENUM_ATTRIBUTE("dir", GRA_DIR, TRUE)
		DUIWIN_ENUM_VALUE("horz", DIR_HORZ)
		DUIWIN_ENUM_VALUE("vert", DIR_VERT)
    DUIWIN_ENUM_END(m_uDirection)
    DUIWIN_DECLARE_ATTRIBUTES_END()
};

enum SBSTATE{
	SBST_NORMAL=0,	//正常状态
	SBST_HOVER,		//hover状态
	SBST_PUSHDOWN,	//按下状态
	SBST_DISABLE,	//禁用状态
	SBST_INACTIVE,	//失活状态,主要针对两端的箭头
};

#define MAKESBSTATE(sbCode,nState1,bVertical) MAKELONG((sbCode),MAKEWORD((nState1),(bVertical)))
#define SB_CORNOR		10
#define SB_THUMBGRIPPER	11	//滚动条上的可拖动部分

#define THUMB_MINSIZE	18

class SOUI_EXP CDuiScrollbarSkin : public CDuiSkinImgFrame
{
    DUIOBJ_DECLARE_CLASS_NAME(CDuiScrollbarSkin, "scrollbar")

public:

    CDuiScrollbarSkin();

    virtual void Draw(HDC dc, CRect rcDraw, DWORD dwState,BYTE byAlpha=0xff);

	//指示滚动条皮肤是否支持显示上下箭头
	virtual BOOL HasArrow(){return TRUE;}
	virtual int GetIdealSize(){
		if(!m_pDuiImg) return 0;
		return m_pDuiImg->GetWidth()/9;
	}

    DUIWIN_DECLARE_ATTRIBUTES_BEGIN()
		DUIWIN_INT_ATTRIBUTE("margin",m_nMargin,FALSE)
		DUIWIN_INT_ATTRIBUTE("hasgripper",m_bHasGripper,FALSE)
		DUIWIN_INT_ATTRIBUTE("hasinactive",m_bHasInactive,FALSE)
    DUIWIN_DECLARE_ATTRIBUTES_END()
protected:
	//返回源指定部分在原位图上的位置。
	CRect GetPartRect(int nSbCode, int nState,BOOL bVertical);
    int			m_nMargin;
	BOOL		m_bHasGripper;
	BOOL		m_bHasInactive;//有失活状态的箭头时，滚动条皮肤有必须有5行，否则可以是3行或者4行
};

class SOUI_EXP CDuiSkinMenuBorder : public CDuiSkinImgFrame
{
    DUIOBJ_DECLARE_CLASS_NAME(CDuiSkinMenuBorder, "border")

public:

    CDuiSkinMenuBorder():m_rcBorder(1,1,1,1)
    {

    }

    CRect GetMarginRect()
    {
        return m_rcBorder;
    }
public:
    DUIWIN_DECLARE_ATTRIBUTES_BEGIN()
    DUIWIN_RECT_ATTRIBUTE("border",m_rcBorder,FALSE)
    DUIWIN_DECLARE_ATTRIBUTES_END()
protected:
    CRect		m_rcBorder;
};

}//namespace SOUI
#pragma once
#include "interface/slayout-i.h"
#include <sobject/sobject-state-impl.hpp>
#include <helper/SplitString.h>

namespace SOUI{

	class SouiLayoutParam: public SObjectImpl<TObjRefImpl<ILayoutParam>>
	{
		SOUI_CLASS_NAME(SouiLayoutParam,L"SouiLayoutParam")

		friend class SouiLayout;
	public:
		virtual bool IsMatchParent(ORIENTATION orientation) const;

		virtual bool IsSpecifiedSize(ORIENTATION orientation) const;

		virtual bool IsWrapContent(ORIENTATION orientation) const;

		virtual int GetSpecifiedSize(ORIENTATION orientation) const;

	public:
		bool IsOffsetRequired(ORIENTATION orientation) const;
        int  GetExtraSize(ORIENTATION orientation) const;
	protected:
		HRESULT OnAttrWidth(const SStringW & strValue,BOOL bLoading);

		HRESULT OnAttrHeight(const SStringW & strValue,BOOL bLoading);

		HRESULT OnAttrSize(const SStringW & strValue,BOOL bLoading);

		HRESULT OnAttrPos(const SStringW & strValue,BOOL bLoading);

		HRESULT OnAttrOffset(const SStringW & strValue,BOOL bLoading);

		SOUI_ATTRS_BEGIN()
            ATTR_CUSTOM(L"width",OnAttrWidth)
            ATTR_CUSTOM(L"height",OnAttrHeight)
			ATTR_CUSTOM(L"pos",OnAttrPos)
			ATTR_CUSTOM(L"size",OnAttrSize)
			ATTR_CUSTOM(L"offset",OnAttrOffset)
        SOUI_ATTRS_BREAK()

    protected:
        //将字符串描述的坐标转换成POSITION_ITEM
        BOOL StrPos2ItemPos(const SStringW &strPos,POSITION_ITEM & posItem);

        //解析在pos中定义的前两个位置
        BOOL ParsePosition12(const SStringW & pos1, const SStringW &pos2);

        //解析在pos中定义的后两个位置
        BOOL ParsePosition34(const SStringW & pos3, const SStringW &pos4);

	protected:
        int  nCount;                /**< 定义的坐标个数 */
        POSITION_ITEM pos[4];       /**< 由pos属性定义的值, nCount >0 时有效*/

		float fOffsetX,fOffsetY;    /**< 窗口坐标偏移量, x += fOffsetX * width, y += fOffsetY * height  */

		int  m_width;        /**<使用width属性定义的宽 nCount==0 时有效*/
		int  m_height;       /**<使用height属性定义的高 nCount==0 时有效*/
	};

	class SouiLayout: public SObjectImpl<TObjRefImpl<ILayout>>
	{
		SOUI_CLASS_NAME(SouiLayout,L"SouiLayout")

	public:
		SouiLayout(void);
		~SouiLayout(void);

		static HRESULT CreateLayoutParam(IObjRef ** ppObj);

        virtual bool IsParamAcceptable(ILayoutParam *pLayoutParam) const;

        virtual void LayoutChildren(SWindow * pParent);

        virtual ILayoutParam * CreateLayoutParam() const;

        virtual CSize MeasureChildren(SWindow * pParent,int nWidth,int nHeight) const;
    protected:
        struct WndPos{
            SWindow *pWnd;
            CRect    rc;
			bool     bWaitOffsetX;
			bool	 bWaitOffsetY;
        };

        void CalcPositionEx(SList<WndPos> *pListChildren,int nWidth,int nHeight) const;
        int CalcPostion(SList<WndPos> *pListChildren,int nWidth,int nHeight) const;

		int PositionItem2Value(SList<WndPos> *pLstChilds,SPOSITION position,const POSITION_ITEM &pos , int nMax,BOOL bX) const;
        
        int CalcChildLeft(SWindow *pWindow,SouiLayoutParam *pParam);
        int CalcChildRight(SWindow *pWindow,SouiLayoutParam *pParam);
        int CalcChildTop(SWindow *pWindow,SouiLayoutParam *pParam);
        int CalcChildBottom(SWindow *pWindow,SouiLayoutParam *pParam);


        BOOL IsWaitingPos( int nPos ) const;
		SWindow * GetRefSibling(SWindow *pCurWnd,int uCode);
        CRect GetWindowLayoutRect(SWindow *pWindow);
    };


}

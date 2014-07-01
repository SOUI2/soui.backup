#pragma once

#include "duiwnd.h"
#include "wtl.mini/duicoll.h"
namespace SOUI
{
#define DUIHDI_WIDTH               0x0001
#define DUIHDI_TEXT                0x0002
#define DUIHDI_SORTFLAG               0x0004
#define DUIHDI_LPARAM              0x0008
#define DUIHDI_ORDER               0x0010

    typedef enum _DUIHDSORTFLAG{
        ST_NULL=0,
        ST_UP,
        ST_DOWN,
    }DUIHDSORTFLAG;

    typedef struct _DUIHDITEM {
        UINT    mask; 
        int     cx; 
        LPTSTR  pszText; 
        int     cchTextMax; 
        DUIHDSORTFLAG stFlag;
        LPARAM  lParam; 
        UINT   state;
        int        iOrder;
    } DUIHDITEM,  *LPDUIHDITEM;


    class SOUI_EXP SHeaderCtrl: public SWindow
    {
        SOUI_CLASS_NAME(SHeaderCtrl, L"header")
    public:
        SHeaderCtrl(void);
        ~SHeaderCtrl(void);

        int InsertItem( int iItem,LPCTSTR pszText,int nWidth, DUIHDSORTFLAG stFlag,LPARAM lParam );
        BOOL GetItem(int iItem,DUIHDITEM *pItem);

        int GetItemCount(){return m_arrItems.GetCount();}
        int GetTotalWidth();
        int GetItemWidth(int iItem);

        BOOL DeleteItem(int iItem);
        void DeleteAllItems();

        SOUI_ATTRS_BEGIN()
            ATTR_SKIN(L"itemSkin",m_pSkinItem,FALSE)
            ATTR_SKIN(L"sortSkin",m_pSkinSort,FALSE)
            ATTR_INT(L"fixWidth",m_bFixWidth,FALSE)
            ATTR_INT(L"itemSwapEnable",m_bItemSwapEnable,FALSE)
            ATTR_INT(L"sortHeader",m_bSortHeader,FALSE)
        SOUI_ATTRS_END()
    protected:
        virtual BOOL CreateChildren(pugi::xml_node xmlNode);
        virtual BOOL OnDuiSetCursor(const CPoint &pt);

        void OnPaint(IRenderTarget * pRT);
        virtual void DrawItem(IRenderTarget * pRT,CRect rcItem,const LPDUIHDITEM pItem);
        CRect    GetItemRect(UINT iItem);
        //鼠标位置分析，返回一个DWORD，LOWORD为前面的项，HIWORD为后面的项，相同时代表在一个项的中间
        DWORD  HitTest(CPoint pt);
        void RedrawItem(int iItem);

        void DrawDraggingState(DWORD dwDragTo);

        BOOL IsItemHover(DWORD dwHitTest)
        {
            return dwHitTest!=-1 && LOWORD(dwHitTest)==HIWORD(dwHitTest);
        }

        HBITMAP CreateDragImage(UINT iItem);

        void OnLButtonDown(UINT nFlags,CPoint pt);
        void OnLButtonUp(UINT nFlags,CPoint pt);
        void OnMouseMove(UINT nFlags,CPoint pt);
        void OnMouseLeave();
        void OnDestroy();
        
        SOUI_MSG_MAP_BEGIN()
            MSG_WM_PAINT_EX(OnPaint)
            MSG_WM_LBUTTONDOWN(OnLButtonDown)
            MSG_WM_LBUTTONUP(OnLButtonUp)
            MSG_WM_MOUSEMOVE(OnMouseMove)
            MSG_WM_MOUSELEAVE(OnMouseLeave)
            MSG_WM_DESTROY(OnDestroy)
        SOUI_MSG_MAP_END()

        ISkinObj *    m_pSkinItem;
        ISkinObj *    m_pSkinSort;
        BOOL            m_bSortHeader;        //表头可以点击排序
        BOOL            m_bFixWidth;        //表项宽度固定开关
        BOOL            m_bItemSwapEnable;//允许拖动调整位置开关

        BOOL            m_bDragging;
        HBITMAP            m_hDragImg;
        CPoint            m_ptClick;
        DWORD            m_dwHitTest;
        DWORD            m_dwDragTo;
        
        int                m_nAdjItemOldWidth;//保存被拖动项的原始宽度
        SArray<DUIHDITEM> m_arrItems;
    };
}//end of namespace SOUI

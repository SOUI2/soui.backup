#pragma once

namespace SOUI
{
#define SHDI_WIDTH               0x0001
#define SHDI_TEXT                0x0002
#define SHDI_SORTFLAG            0x0004
#define SHDI_LPARAM              0x0008
#define SHDI_ORDER               0x0010
#define SHDI_VISIBLE             0x0020

#define CX_HDITEM_MARGIN    4
	/**
	* @enum      _SHDSORTFLAG
	* @brief     排序标志
	*
	* Describe   排序标志
	*/
	// 	enum SHDSORTFLAG008 {
	// 		ST_NULL = 0,
	// 		ST_UP,
	// 		ST_DOWN,
	// 	};

		/**
		* @struct    _SHDITEM
		* @brief     列表头项
		*
		* Describe   列表头项
		*/
	typedef struct SHDITEM008 {
		SHDITEM008() :mask(0), cx(0), stFlag(ST_NULL), iOrder(0), bVisible(true) {
		}
		UINT    mask;
		int cx;
		SHDSORTFLAG stFlag;
		int     iOrder;
		bool    bVisible;
	}*LPSHDITEM008;


	/**
	* @class     SHeaderCtrl008
	* @brief     表头控件
	*
	* Describe   表头控件
	*/
	class SHeaderItem;
	class /*SOUI_EXP*/ SHeaderCtrl008 : public SWindow//, public ITimelineHandler
	{
		SOUI_CLASS_NAME(SHeaderCtrl008, L"header008")
			friend class SHeaderItem;
	public:
		/**
		* SHeaderCtrl008::SHeaderCtrl008
		* @brief    构造函数
		*
		* Describe  构造函数
		*/
		SHeaderCtrl008(void);
		/**
		* SHeaderCtrl008::~SHeaderCtrl008
		* @brief    析构函数
		*
		* Describe  析构函数
		*/
		~SHeaderCtrl008(void);

		/**
		* SHeaderCtrl008::InsertItem
		* @brief    插入新项
		* @param    int iItem --  新项索引
		* @param    LPCTSTR pszText  --  新项标题
		* @param    int nWidth  -- 宽度
		* @param    SHDSORTFLAG stFlag -- 排序标志
		* @param    LPARAM lParam -- 附加参数
		* @return   返回int
		*
		* Describe  插入新项
		*/
		//int InsertItem(int iItem, LPCTSTR pszText, int nWidth, SHDSORTFLAG stFlag, LPARAM lParam);
		//int InsertItem(int iItem, LPCTSTR pszText, int nWidth, SLayoutSize::Unit unit, SHDSORTFLAG stFlag, LPARAM lParam);
		/**
		* SHeaderCtrl008::GetItem
		* @brief    获得新项
		* @param    int iItem  --  索引
		* @param    SHDITEM *pItem  -- 返回列表项结构
		* @return   返回BOOL
		*
		* Describe  获得新项
		*/
		BOOL GetItem(int iItem, SHDITEM008 *pItem);

		/**
		* SHeaderCtrl008::GetItemCount
		* @brief    获取列表项个数
		* @return   返回int
		*
		* Describe  获取列表项个数
		*/
		size_t GetItemCount() const { return m_arrItems.GetCount(); }
		/**
		* SHeaderCtrl008::GetTotalWidth
		* @brief    获得所有宽度
		* @return   返回int
		*
		* Describe  获得所有宽度
		*/
		int GetTotalWidth();

		/**
		* SHeaderCtrl008::DeleteItem
		* @brief    删除指定项
		* @param    int iItem  --  索引
		* @return   返回BOOL
		*
		* Describe  删除指定项
		*/
		BOOL DeleteItem(int iItem);

		/**
		* SHeaderCtrl008::DeleteAllItems
		* @brief    删除所有项
		*
		* Describe  获得新项
		*/
		void DeleteAllItems();

		void SetItemSort(int iItem, SHDSORTFLAG stFlag);

		void SetItemVisible(int iItem, bool visible);

		bool IsItemVisible(int iItem) const;

		SOUI_ATTRS_BEGIN()
			ATTR_SKIN(L"itemSkin", m_pSkinItem, FALSE)
			ATTR_SKIN(L"sortSkin", m_pSkinSort, FALSE)
			ATTR_INT(L"fixWidth", m_bFixWidth, FALSE)
			ATTR_INT(L"itemSwapEnable", m_bItemSwapEnable, FALSE)
			ATTR_INT(L"sortHeader", m_bSortHeader, FALSE)
		SOUI_ATTRS_END()
			
	protected:
//		virtual void OnNextFrame() override;
		int ChangeItemPos(SHeaderItem* pCurMove, CPoint ptCur);
		void ChangeItemSize(SHeaderItem*,CPoint ptCur);
		/**
		* SHeaderCtrl008::CreateChildren
		* @brief    创建新项
		* @param    pugi::xml_node xmlNode  -- xml配置文件
		*
		* Describe  创建新项
		*/
		virtual BOOL CreateChildren(pugi::xml_node xmlNode);

		bool IsLastItem(int iOrder);

		virtual void UpdateChildrenPosition()override;
		virtual CSize GetDesiredSize(LPCRECT pRcContainer);
			/**
			* SHeaderCtrl008::IsItemHover
			* @brief    判断是否Hover状态
			* @param    DWORD dwHitTest
			* @return   返回BOOL
			*
			* Describe  判断是否Hover状态
			*/
			BOOL IsItemHover(DWORD dwHitTest)
		{
			return dwHitTest != -1 && LOWORD(dwHitTest) == HIWORD(dwHitTest);
		}
// 			void StartAni();
// 			void StopAni();
		void OnDestroy();

		SOUI_MSG_MAP_BEGIN()
			MSG_WM_DESTROY(OnDestroy)
			SOUI_MSG_MAP_END()

		ISkinObj *    m_pSkinItem;  /**< 表头绘制Skin */
		ISkinObj *    m_pSkinSort;  /**< 排序标志Skin */
		BOOL          m_bSortHeader;      /**< 表头可以点击排序 */
		BOOL          m_bFixWidth;        /**< 表项宽度固定开关 */
		BOOL          m_bItemSwapEnable;  /**< 允许拖动调整位置开关 */

		BOOL            m_bDragging; /**< 正在拖动标志 */

		CPoint          m_ptClick;   /**< 当前点击坐标 */
		DWORD           m_dwHitTest; /**< 鼠标位置 */
		DWORD           m_dwDragTo;  /**< 拖放目标 */
		int             m_nAdjItemOldWidth;  /**< 保存被拖动项的原始宽度 */
		SArray<SHeaderItem*> m_arrItems;
	};
}
/**
 * Copyright (C) 2014-2050 SOUI团队
 * All rights reserved.
 * 
 * @file       SCalendar.h
 * @brief      日历时间控件
 * @version    v1.0      
 * @author     soui      
 * @date       2014-05-25
 * 
 * Describe    此类完成日历控件
 */

#pragma once
#include "core/SWnd.h"

namespace SOUI
{
    /**
     * @class      SCalendar
     * @brief      日历类
     * 
     * Describe    此类是日历的核心类 大部分函数都是静态函数
     */
    class SCalendar2 : public SWindow
    {
    public:
        SOUI_CLASS_NAME(SCalendar2, L"calendar2")
        
        /**
         * CCalendarCore::SCalendar
         * @brief    构造函数
         * @param    WORD iYear -- 年
         * @param    WORD iMonth -- 月
         * @param    WORD iDay -- 日
         *
         * Describe  构造函数
         */
        SCalendar2(WORD iYear, WORD iMonth, WORD iDay);

        /**
         * SCalendar::SCalendar
         * @brief    构造函数
         *
         * Describe  构造函数
         */
        SCalendar2();

    public:
        /**
         * SCalendar::GetYear
         * @brief    获得年
         *
         * Describe  获得年
         */
        WORD GetYear(){return m_iYear;}
        
        /**
         * SCalendar::GetMonth
         * @brief    获得月
         *
         * Describe  获得月
         */
        WORD GetMonth(){return m_iMonth;}

        /**
         * SCalendar::GetDay
         * @brief    获得天
         *
         * Describe  获得天
         */
        WORD GetDay(){return m_iDay;}
        
        /**
         * SCalendar::GetDate
         * @brief    获得日期
         * @param    WORD iYear -- 年
         * @param    WORD iMonth -- 月
         * @param    WORD iDay -- 日         
         *
         * Describe  获得日期
         */
        void GetDate(WORD &iYear, WORD &iMonth, WORD &iDay);

        /**
         * SCalendar::SetDate
         * @brief    设置日期
         * @param    WORD iYear -- 年
         * @param    WORD iMonth -- 月
         * @param    WORD iDay -- 日         
         *
         * Describe  设置日期
         */        
        BOOL SetDate(WORD iYear, WORD iMonth, WORD iDay);

    protected:
      /**
       * SCalendar::Init
       * @brief    初始化函数
       *
       * Describe  初始化函数
       */            
        void Init();

        /**
         * SCalendar::OnPaint
         * @brief    绘画消息
         * @param    IRenderTarget *pRT -- 绘制设备句柄
         *
         * Describe  此函数是消息响应函数
         */
        void OnPaint(IRenderTarget *pRT);
        
        /**
         * SCalendar::OnLButtonDown
         * @brief    鼠标左键抬起事件
         * @param    UINT nFlags -- 标志
         * @param    CPoint point -- 鼠标坐标
         *
         * Describe  此函数是消息响应函数
         */
        void OnLButtonDown(UINT nFlags, CPoint point);
        
        /**
         * SCalendar::OnMouseMove
         * @brief    鼠标移动事件
         * @param    UINT nFlags -- 标志
         * @param    CPoint point -- 鼠标坐标
         *
         * Describe  此函数是消息响应函数
         */
        void OnMouseMove(UINT nFlags,CPoint pt);
        
        /**
         * SCalendar::OnMouseLeave
         * @brief    鼠标离开事件
         *
         * Describe  此函数是消息响应函数
         */
        void OnMouseLeave();

        
        /**
         * SCalendar::Load
         * @brief    加载xml
         * @param    pugi::xml_node xmlNode -- xml节点    
         *
         * Describe  通过加载xml来构造窗口
         */
        virtual BOOL InitFromXml(pugi::xml_node xmlNode);
    protected:
        /**
         * SCalendar::GetDayRect
         * @brief    获得日期的坐标
         * @param    WORD iDay  -- 日期         
         *
         * Describe  根据日期所在的周以及星期几，来计算坐标
         */    
        CRect GetDayRect(WORD iDay);
        WORD HitTest(CPoint  pt);
	   bool  ShowSelectMonthYear(EventArgs *pArg); //显示个界面，选择年、月
	   bool  SelectPrevYear(EventArgs *pArg); //切换到去年
	   bool  SelectNextYear(EventArgs *pArg); //切换到明年
		void DrawBackGround(IRenderTarget *pRT);
		void	DrawBorder(IRenderTarget *pRT);
		void DrawHeader(IRenderTarget *pRT);
		void DrawFooter(IRenderTarget *pRT);

        /**
         * SCalendar::DrawTitle
         * @brief    绘制标题
         * @param    IRenderTarget *pRT -- 绘制设备句柄         
         *
         * Describe  返回只是该日期所在的坐标，采用CRect表示
         */    
        void DrawTitle(IRenderTarget *pRT);
        
        /**
         * SCalendar::DrawDate
         * @brief    绘制日期
         * @param    IRenderTarget *pRT -- 绘制设备句柄         
         *
         * Describe  绘制日期
         */            
        void DrawDate(IRenderTarget *pRT);

        /**
         * SCalendar::DrawDay
         * @brief    绘制日期--天
         * @param    IRenderTarget *pRT -- 绘制设备句柄         
         *
         * Describe  绘制日期--天
         */    
        void DrawDay(IRenderTarget *pRT,CRect & rcDay,WORD iDay );
        
        /**
         * SCalendar::RedrawDay
         * @brief    重新绘制日期--天
         * @param    CDCHandle dc -- 绘制设备句柄         
         *
         * Describe  重新绘制日期--天
         */    
        void RedrawDay(WORD iDay);

        /**
         * SCalendar::OnTodayClick
         * @brief    在日期---天的单击事件
         *
         * Describe  在日期---天的单击事件
         */            
        bool OnTodayClick(EventArgs *pArg);

        SOUI_ATTRS_BEGIN()
            ATTR_INT(L"titleHeight", m_nTitleHei, FALSE)			
            ATTR_INT(L"footerHeight", m_nFooterHei, FALSE)
            ATTR_COLOR(L"colorWeekend", m_crWeekend, FALSE)
            ATTR_COLOR(L"colorTitleBack", m_crTitleBack, FALSE)
			ATTR_COLOR(L"colorBorder", m_crBorder, FALSE)
            ATTR_COLOR(L"colorDay", m_crDay, FALSE)
            ATTR_SKIN(L"titleSkin", m_pTitleSkin, FALSE)
			
			//新增部分
            ATTR_STRINGW(L"daySkin", m_strDaySkin, FALSE)			
			ATTR_STRINGW(L"bkgSkin", m_strBkgSkin, FALSE)
			ATTR_STRINGW(L"headerTxtColor", m_strCRHeader, FALSE)
			ATTR_STRINGW(L"headerSkin", m_strHeaderSkin, FALSE)
			ATTR_INT(L"headerHeight", m_nHeaderHei, FALSE)
			ATTR_STRINGW(L"footerSkin", m_strFooterSkin, FALSE)
			ATTR_STRINGW(L"preMonSkin", m_strPreMonSkin, FALSE)
			ATTR_STRINGW(L"nextMonSkin", m_strNxtMonSkin, FALSE)
			ATTR_STRINGW(L"selectMonYearSkin", m_strSelMYSkin, FALSE)
			ATTR_STRINGW(L"MonYearSkin", m_strMYSkin, FALSE)
			ATTR_STRINGW(L"prevYearSkin", m_strPreYearSkin, FALSE)
			ATTR_STRINGW(L"nextYearSkin", m_strNxtYearSkin, FALSE)
			ATTR_STRINGW(L"sepMonYearSkin", m_strMonSepYearSkin, FALSE)

            ATTR_I18NSTRT(L"title-1", m_strTitle[0], FALSE)
            ATTR_I18NSTRT(L"title-2", m_strTitle[1], FALSE)
            ATTR_I18NSTRT(L"title-3", m_strTitle[2], FALSE)
            ATTR_I18NSTRT(L"title-4", m_strTitle[3], FALSE)
            ATTR_I18NSTRT(L"title-5", m_strTitle[4], FALSE)
            ATTR_I18NSTRT(L"title-6", m_strTitle[5], FALSE)
            ATTR_I18NSTRT(L"title-7", m_strTitle[6], FALSE)
        SOUI_ATTRS_END()

        SOUI_MSG_MAP_BEGIN()
            MSG_WM_PAINT_EX(OnPaint)
            MSG_WM_LBUTTONDOWN(OnLButtonDown)
            MSG_WM_MOUSEMOVE(OnMouseMove)
            MSG_WM_MOUSELEAVE(OnMouseLeave)
        SOUI_MSG_MAP_END()
    protected:
        int             m_nTitleHei;    /**< 表头高度 */
		int             m_nHeaderHei ;
        int             m_nFooterHei;   /**< 表尾高度 */	
        SStringW     m_strDaySkin;	
        ISkinObj    *m_pDaySkin;    /**< 日期项皮肤 */
		SStringW   m_strCRHeader; /*表头年月字体颜色*/
		SStringW   m_strHeaderSkin;  /**< 表头皮肤 */
        ISkinObj    *m_pTitleSkin;  /**< 表头 周标题 背景皮肤 */	 		
		SStringW    m_strBkgSkin; /*主背景皮肤*/
		SStringW    m_strFooterSkin; /*表尾皮肤*/
		SStringW    m_strPreMonSkin;  /**< 月份前翻按钮皮肤 */	 		
		SStringW   m_strNxtMonSkin;  /**< 月份后翻按钮皮肤 */	 		
		SStringW   m_strSelMYSkin;  /**< 选择年月按钮(下箭头)皮肤 */	 		
		SStringW   m_strMYSkin;  /**< 年月项皮肤 */	 		
		SStringW   m_strPreYearSkin;  /**< 年份前翻按钮皮肤 */	 		
		SStringW   m_strNxtYearSkin;  /**< 年份后翻按钮皮肤 */	 		
		SStringW   m_strMonSepYearSkin;  /**<月份年份分割线图片皮肤 */	 		
		
		
		COLORREF		m_crBorder; /*整体边框颜色*/
        COLORREF        m_crWeekend;    /**< 周末文字颜色 */
        COLORREF        m_crTitleBack;  /**< 表头背景色 */
        COLORREF        m_crDay;        /**< 选中日期颜色 */
        COLORREF        m_crDayBack;    /**< 选中日期背景颜色 */
        SStringT     m_strTitle[7];  /**< 表头文本 */

        WORD    m_iYear, m_iMonth, m_iDay; /**< 年月日 */
        int        m_iHoverDay;
		BOOL  m_beInited;
		UINT  m_selMonth,m_selYear;
    };
}//end of namespace


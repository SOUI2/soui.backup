/**
 * Copyright (C) 2014-2050 SOUI团队
 * All rights reserverd.
 * 
 * @file       DuiCalendar.h
 * @brief      日历时间控件
 * @version    v1.0      
 * @author     soui      
 * @date       2014-05-25
 * 
 * Describe    此类完成日历控件
 */

#pragma once
#include "duiwnd.h"

namespace SOUI
{
#define START_YEAR    1901
#define END_YEAR    2050
    /**
     * @class      CCalendarCore
     * @brief      日历核心类
     * 
     * Describe    此类是日历的核心类 大部分函数都是静态函数
     */
    class CCalendarCore
    {
    public:
        /**
         * CCalendarCore::IsLeapYear
         * @brief    判断iYear是不是闰年
         * @param    WORD iYear -- 待判断的年份
         * @return   TRUE -- 闰年  FALSE -- 非闰年
         *
         * Describe  判断是否是闰年  
         */        
        static BOOL IsLeapYear(WORD iYear)
        {
            return !(iYear%4)&&(iYear%100) || !(iYear%400);
        }

        /**
         * CCalendarCore::WeekDay
         * @brief    返回星期几
         * @param    WORD iYear -- 年
         * @param    WORD iMonth -- 月
         * @param    WORD iDay -- 日
         * @return   返回:0,1,2,3,4,5,6分别对应日、一、二、三、四、五、六
         *
         * Describe  输入年月日返回星期几 
         *           注意:有效范围是(1年1月1日 --- 65535年12月31日)
         */
        static WORD WeekDay(WORD iYear, WORD iMonth, WORD iDay);

        /**
         * CCalendarCore::MonthWeeks
         * @brief    返回指定月份的周数
         * @param    WORD iYear -- 年
         * @param    WORD iMonth -- 月
         * @return   周数
         *
         * Describe  输入年月返回本月有几周
         *           注意:有效范围是(1年1月1日 --- 65535年12月31日)
         */
        static WORD MonthWeeks(WORD iYear, WORD iMonth);
        
        /**
         * CCalendarCore::DayWeek
         * @brief    返回某天在当月的第几周
         * @param    WORD iYear -- 年
         * @param    WORD iMonth -- 月
         * @param    WORD iDay -- 日         
         * @return   周数
         *
         * Describe  输入年月日，返回这天在当月的第几周
         *           注意:有效范围是(1年1月1日 --- 65535年12月31日)
         */
        static WORD DayWeek(WORD iYear, WORD iMonth, WORD iDay);

        /**
         * CCalendarCore::MonthDays
         * @brief    返回指定月份的天数
         * @param    WORD iYear -- 年
         * @param    WORD iMonth -- 月
         * @return   返回该月的天数
         *
         * Describe  输入年月返回本月的天数 
         *           注意:有效范围是(1年1月 --- 65535年12月)
         */
        static WORD MonthDays(WORD iYear, WORD iMonth);

        /**
         * CCalendarCore::LunarMonthDays
         * @brief    指定阴历年月分返回该月的天数
         * @param    WORD iLunarYear -- 年
         * @param    WORD iLunarMonth -- 月
         * @return   返回该月的天数
         *
         * Describe  如果iLunarMonth为闰月，高字为第二个iLunarMonth月的天数，
         *           否则高字为0  注意:有效范围是(1901年1月---2050年12月)
         */
        static LONG LunarMonthDays(WORD iLunarYear, WORD iLunarMonth);

        /**
         * CCalendarCore::LunarYearDays
         * @brief    指定阴历年返回该年的总天数
         * @param    WORD iLunarYear -- 年
         * @return   返回该阴历年的总天数
         *
         * Describe  输入年份，返回该年阴历的总天数
         *           注意:有效范围是(1901年1月---2050年12月)
         */
        static WORD LunarYearDays(WORD iLunarYear);

        /**
         * CCalendarCore::LunarYearDays
         * @brief    指定阴历年返回该年的闰月月份
         * @param    WORD iLunarYear -- 年
         * @return   返回0表示当年没有闰月 否则是月份
         *
         * Describe  指定阴历年返回该年的闰月月份
         *           注意:有效范围是(1901年1月---2050年12月)
         */
        static WORD GetLeapMonth(WORD iLunarYear);

        /**
         * CCalendarCore::FormatLunarYear
         * @brief    格式化年份显示型式
         * @param    WORD iLunarYear -- 年
         * @param    TCHAR *pBuffer -- 输出参数，保存格式化后字符串
         *
         * Describe  指定阴历年返回采用干支纪年法
         */        
        static void FormatLunarYear(WORD  iYear, TCHAR *pBuffer);

        /**
         * CCalendarCore::FormatMonth
         * @brief    格式化月份显示型式
         * @param    WORD iMonth -- 月
         * @param    TCHAR *pBuffer -- 输出参数，保存格式化后字符串
         * @param    BOOL bLunar -- 是否是阴历 默认是TRUE
         *
         * Describe  如果是阴历，则一月返回的是"正月",否则就是"一月"
         */        
        static void FormatMonth(WORD iMonth, TCHAR *pBuffer, BOOL bLunar = TRUE);
        
        /**
         * CCalendarCore::FormatLunarDay
         * @brief    格式化日期显示型式
         * @param    WORD iDay -- 日
         * @param    TCHAR *pBuffer -- 输出参数，保存格式化后字符串
         *
         * Describe  默认是按照阴历返回，比如说"初六"
         */
        static void FormatLunarDay(WORD  iDay, TCHAR *pBuffer);

        /**
         * CCalendarCore::CalcDateDiff
         * @brief    返回公历某两个日期相差的天数
         * @param    WORD iEndYear  -- 结束年
         * @param    WORD iEndMonth  -- 结束月
         * @param    WORD iEndDay  -- 结束日
         * @param    WORD iStartYear  -- 开始年 默认值1901
         * @param    WORD iStartMonth  -- 开始月 默认值1
         * @param    WORD iStartDay  -- 开始日 默认值1
         * @return   返回天数
         *
         * Describe  计算两个日期之间的天数差
         */
        static LONG CalcDateDiff(WORD iEndYear, WORD iEndMonth, WORD iEndDay,
            WORD iStartYear = START_YEAR, 
            WORD iStartMonth =1, WORD iStartDay =1);

        /**
         * CCalendarCore::GetLunarDate
         * @brief    返回阴历日期以及阴历节气
         * @param    WORD iYear  -- 公历年
         * @param    WORD iMonth  -- 公历月
         * @param    WORD iDay  -- 公历日
         * @param    WORD &iLunarYear  -- 输出参数 阴历年 
         * @param    WORD &iLunarMonth  -- 输出参数 阴历月 
         * @param    WORD &iLunarDay  -- 输出参数 阴历日 
         * @return   返回节气标号 0 - 24 0不是节气
         *
         * Describe  输入公历日期，返回对应的阴历日期和节气 
         *           注意:有效范围是(1901年1月1日---2050年12月31日)
         */
        static WORD GetLunarDate(WORD iYear, WORD iMonth, WORD iDay,
            WORD &iLunarYear, WORD &iLunarMonth, WORD &iLunarDay);

    protected:
        /**
         * CCalendarCore::l_CalcLunarDate
         * @brief    返回阴历日期
         * @param    WORD iYear  -- 输出参数 阴历年 
         * @param    WORD iMonth  -- 输出参数 阴历月 
         * @param    WORD iDay  -- 输出参数 阴历日 
         * @param    LONG iSpanDays  -- 公历日期距离1901年1月1日天数
         *
         * Describe  输入公历日期，返回对应的阴历日期
         *           注意:有效范围是(1901年1月1日---2050年12月31日)
         */
        static void   l_CalcLunarDate(WORD &iYear, WORD &iMonth ,WORD &iDay, LONG iSpanDays);
        
        /**
         * CCalendarCore::l_GetLunarHolDay
         * @brief    阴历节气
         * @param    WORD iYear  -- 公历年 
         * @param    WORD iMonth  -- 公历月 
         * @param    WORD iDay  -- 公历日 
         * @return   0-24  0不是节气
         *
         * Describe  判断当天是否是24节气中一个 如果不是返回0否则返回对应编码
         */
        static WORD   l_GetLunarHolDay(WORD iYear, WORD iMonth, WORD iDay);
    };

    /**
     * @class      CDuiCalendar
     * @brief      日历类
     * 
     * Describe    此类是日历的核心类 大部分函数都是静态函数
     */
    class SCalendar : public SWindow
    {
    public:
        SOUI_CLASS_NAME(SCalendar, L"calendar")
        
        /**
         * CCalendarCore::CDuiCalendar
         * @brief    构造函数
         * @param    WORD iYear -- 年
         * @param    WORD iMonth -- 月
         * @param    WORD iDay -- 日
         *
         * Describe  构造函数
         */
        SCalendar(WORD iYear, WORD iMonth, WORD iDay);

        /**
         * CDuiCalendar::CDuiCalendar
         * @brief    构造函数
         *
         * Describe  构造函数
         */
        SCalendar();

    public:
        /**
         * CDuiCalendar::GetYear
         * @brief    获得年
         *
         * Describe  获得年
         */
        WORD GetYear(){return m_iYear;}
        
        /**
         * CDuiCalendar::GetMonth
         * @brief    获得月
         *
         * Describe  获得月
         */
        WORD GetMonth(){return m_iMonth;}

        /**
         * CDuiCalendar::GetDay
         * @brief    获得天
         *
         * Describe  获得天
         */
        WORD GetDay(){return m_iDay;}
        
        /**
         * CDuiCalendar::GetDate
         * @brief    获得日期
         * @param    WORD iYear -- 年
         * @param    WORD iMonth -- 月
         * @param    WORD iDay -- 日         
         *
         * Describe  获得日期
         */
        void GetDate(WORD &iYear, WORD &iMonth, WORD &iDay);

        /**
         * CDuiCalendar::SetDate
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
       * CDuiCalendar::Init
       * @brief    初始化函数
       *
       * Describe  初始化函数
       */            
        void Init();

        /**
         * CDuiCalendar::OnPaint
         * @brief    绘画消息
         * @param    IRenderTarget *pRT -- 绘制设备句柄
         *
         * Describe  此函数是消息响应函数
         */
        void OnPaint(IRenderTarget *pRT);
        
        /**
         * CDuiCalendar::OnLButtonDown
         * @brief    鼠标左键抬起事件
         * @param    UINT nFlags -- 标志
         * @param    CPoint point -- 鼠标坐标
         *
         * Describe  此函数是消息响应函数
         */
        void OnLButtonDown(UINT nFlags, CPoint point);
        
        /**
         * CDuiCalendar::OnMouseMove
         * @brief    鼠标移动事件
         * @param    UINT nFlags -- 标志
         * @param    CPoint point -- 鼠标坐标
         *
         * Describe  此函数是消息响应函数
         */
        void OnMouseMove(UINT nFlags,CPoint pt);
        
        /**
         * CDuiCalendar::OnMouseLeave
         * @brief    鼠标离开事件
         *
         * Describe  此函数是消息响应函数
         */
        void OnMouseLeave();

        
        /**
         * CDuiCalendar::Load
         * @brief    加载xml
         * @param    pugi::xml_node xmlNode -- xml节点    
         *
         * Describe  通过加载xml来构造窗口
         */
        virtual BOOL InitFromXml(pugi::xml_node xmlNode);
    protected:
        /**
         * CDuiCalendar::GetDayRect
         * @brief    获得日期的坐标
         * @param    WORD iDay  -- 日期         
         *
         * Describe  根据日期所在的周以及星期几，来计算坐标
         */    
        CRect GetDayRect(WORD iDay);
        WORD HitTest(CPoint  pt);

        /**
         * CDuiCalendar::DrawTitle
         * @brief    绘制标题
         * @param    IRenderTarget *pRT -- 绘制设备句柄         
         *
         * Describe  返回只是该日期所在的坐标，采用CRect表示
         */    
        void DrawTitle(IRenderTarget *pRT);
        
        /**
         * CDuiCalendar::DrawDate
         * @brief    绘制日期
         * @param    IRenderTarget *pRT -- 绘制设备句柄         
         *
         * Describe  绘制日期
         */            
        void DrawDate(IRenderTarget *pRT);

        /**
         * CDuiCalendar::DrawDay
         * @brief    绘制日期--天
         * @param    IRenderTarget *pRT -- 绘制设备句柄         
         *
         * Describe  绘制日期--天
         */    
        void DrawDay(IRenderTarget *pRT,CRect & rcDay,WORD iDay );
        
        /**
         * CDuiCalendar::RedrawDay
         * @brief    重新绘制日期--天
         * @param    CDCHandle dc -- 绘制设备句柄         
         *
         * Describe  重新绘制日期--天
         */    
        void RedrawDay(WORD iDay);

        /**
         * CDuiCalendar::OnTodayClick
         * @brief    在日期---天的单击事件
         *
         * Describe  在日期---天的单击事件
         */            
        bool OnTodayClick(EventArgs *pArg);

        SOUI_ATTRS_BEGIN()
            ATTR_INT(L"titleHeight", m_nTitleHei, FALSE)
            ATTR_INT(L"footerHeight", m_nFooterHei, FALSE)
            ATTR_COLOR(L"crWeekend", m_crWeekend, FALSE)
            ATTR_COLOR(L"crTitleBack", m_crTitleBack, FALSE)
            ATTR_COLOR(L"crDay", m_crDay, FALSE)
            ATTR_SKIN(L"daySkin", m_pDaySkin, FALSE)
            ATTR_SKIN(L"titleSkin", m_pTitleSkin, FALSE)
            ATTR_STRINGT(L"title-1", m_strTitle[0], FALSE)
            ATTR_STRINGT(L"title-2", m_strTitle[1], FALSE)
            ATTR_STRINGT(L"title-3", m_strTitle[2], FALSE)
            ATTR_STRINGT(L"title-4", m_strTitle[3], FALSE)
            ATTR_STRINGT(L"title-5", m_strTitle[4], FALSE)
            ATTR_STRINGT(L"title-6", m_strTitle[5], FALSE)
            ATTR_STRINGT(L"title-7", m_strTitle[6], FALSE)
        SOUI_ATTRS_END()

        SOUI_MSG_MAP_BEGIN()
            MSG_WM_PAINT_EX(OnPaint)
            MSG_WM_LBUTTONDOWN(OnLButtonDown)
            MSG_WM_MOUSEMOVE(OnMouseMove)
            MSG_WM_MOUSELEAVE(OnMouseLeave)
        SOUI_MSG_MAP_END()
    protected:
        int             m_nTitleHei;    /**< 表头高度 */
        int             m_nFooterHei;   /**< 表尾高度 */
        ISkinObj    *m_pDaySkin;    /**< 日期项皮肤 */
        ISkinObj    *m_pTitleSkin;  /**< 表头皮肤 */

        COLORREF        m_crWeekend;    /**< 周末文字颜色 */
        COLORREF        m_crTitleBack;  /**< 表头背景色 */
        COLORREF        m_crDay;        /**< 选中日期颜色 */
        COLORREF        m_crDayBack;    /**< 选中日期背景颜色 */
        SStringT     m_strTitle[7];  /**< 表头文本 */

        WORD    m_iYear, m_iMonth, m_iDay; /**< 年月日 */
        int        m_iHoverDay;

    };
}//end of namespace


/**
* Copyright (C) 2014-2050 
* All rights reserved.
* 
* @file       SwndLayout.h
* @brief      
* @version    v1.0      
* @author     SOUI group   
* @date       2014/08/02
* 
* Describe    SOUI的窗口布局模块
*/

#pragma once

#include "SwndLayout.h"
#include "Swnd.h"

namespace SOUI
{

    enum
    {
        POS_INIT=0x11000000,    //坐标的初始化值
        POS_WAIT=0x12000000,    //坐标的计算依赖于其它窗口的布局
    };

    class SWindow;
    
    class SWindowRepos
    {
    public:
        explicit SWindowRepos(SWindow *pWnd);
        ~SWindowRepos();
        SWindow * GetWindow(){return m_pWnd;}
    protected:
        SWindow * m_pWnd;
        CRect     m_rcWnd;
    };
    
    class SOUI_EXP SwndLayoutBuilder
    {
    public:
        
        static void InitLayoutState(CRect &rcWindow);

        static BOOL IsWaitingPos(int nPos);

        /**
         * CalcPosition
         * @brief    计算窗口坐标
         * @param   SWindow *pWnd --  待计算坐标的窗口指针
         * @param   const CRect & rcContainer --  容器位置
         * @param   CRect & rcWindow --  窗口矩形
         * @param   const SwndLayout * pSwnLayout -- 窗口布局
         * @return   int 需要等待计算的坐标数(<=4)
         *
         * Describe  每个窗口包含4个坐标，由于一个坐标可能依赖于其它兄弟窗口的布局，一次计算可能不能全部得到4个坐标
         */
        static int CalcPosition(SWindow *pWnd,const CRect & rcContainer,CRect & rcWindow, const SwndLayout * pSwnLayout=NULL);


        /**
         * CalcChildrenPosition
         * @brief    计算列表中子窗口的坐标
         * @param    SList<SWindow * > * pListChildren --  子窗口列表
         * @param    const CRect & rcContainer --  容器坐标
         * @return   BOOL TRUE-成功，FALSE-失败，可能由于布局依赖形成死锁
         *
         * Describe  
         */
        static BOOL CalcChildrenPosition(SList<SWindowRepos*> *pListChildren,const CRect & rcContainer);

    protected:
    
        /**
         * PositionItem2Value
         * @brief    将一个position_item解释为绝对坐标
         * @param    SWindow *pWindow -- 待计算坐标的窗口指针
         * @param    const POSITION_ITEM & pos --  一个位置定义的引用
         * @param    int nMin --  父窗口的范围
         * @param    int nMax --  父窗口的范围
         * @param    BOOL bX --  计算X坐标
         * @return   int 计算得到的坐标
         *
         * Describe  
         */
        static int PositionItem2Value(SWindow *pWindow,const POSITION_ITEM &pos,int nMin, int nMax,BOOL bX);


        /**
         * CalcSize
         * @brief    计算窗口大小
         * @param    SWindow *pWindow -- 待计算坐标的窗口指针
         * @param    const CRect & rcContainer --  容器位置
         * @param    const SwndLayout * pSwnLayout -- 窗口布局
         * @return   CSize 
         *
         * Describe  
         */
        static CSize CalcSize(SWindow *pWindow,const CRect & rcContainer,const SwndLayout * pSwndLayout);
        
        /**
         * GetWindowLayoutRect
         * @brief    获得一个窗口布局占用的位置
         * @param    SWindow * pWindow --  
         * @return   CRect 
         *
         * Describe  
         */
        static CRect GetWindowLayoutRect(SWindow *pWindow);
    };
}

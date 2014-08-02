/**
* Copyright (C) 2014-2050 
* All rights reserved.
* 
* @file       SThreadActiveWndMgr.h
* @brief      
* @version    v1.0      
* @author     SOUI group   
* @date       2014/08/02
* 
* Describe    活动的Real窗口管理模块
*/

#pragma once

#include "core/SSingletonMap.h"

namespace SOUI
{

    class SOUI_EXP SThreadActiveWndMgr: public SSingletonMap<SThreadActiveWndMgr,HWND,DWORD>
    {
    public:
        SThreadActiveWndMgr();

        virtual ~SThreadActiveWndMgr();

        static HWND SetActive(HWND hWnd);

        static HWND GetActive();

        static void EnterPaintLock();

        static void LeavePaintLock();

    protected:
        HWND _SetActive(HWND hWnd);
        HWND _GetActive();

    protected:

        CRITICAL_SECTION        m_lockMapActive;
        CRITICAL_SECTION        m_lockRepaint;
    };

}//namespace SOUI
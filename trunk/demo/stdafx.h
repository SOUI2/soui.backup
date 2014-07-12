// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//  are changed infrequently
//

#pragma once

#define SUPPORT_LUA     //打开SUPPORT_LUA来演示如何在SOUI中和LUA交互。
#define SUPPORT_WKE      //需要把wke.dll复制到bin目录才能打开该开关测试wkeWebkit

#include <souistd.h>
#include <core/SHostDialog.h>
#include <control/SMessageBox.h>
#include <control/souictrls.h>
#include <res.mgr/sobjdefattr.h>

#include "resource.h"	//APP资源

#ifdef SUPPORT_WKE
#include "../controls.extend/SWkeWebkit.h"
#endif

using namespace SOUI;

#ifdef _DEBUG
	#ifdef USING_ATL
	# pragma comment(lib, "soui_static_atl_d.lib")
	#elif DLL_SOUI
	# pragma comment(lib, "soui_d.lib")
	#else
	# pragma comment(lib, "soui_static_d.lib")
	#endif
#else
	#ifdef DLL_SOUI
	# pragma comment(lib, "soui.lib")
	#else
	# pragma comment(lib, "soui_static.lib")
	#endif

#endif



// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//  are changed infrequently
//

#pragma once

#if DLL_SOUI
#define SUPPORT_LUA     //打开SUPPORT_LUA来演示如何在SOUI中和LUA交互,LUA导出依赖DLL编译的SOUI，只有定义了DLL_SOUI才能打开这个开关
#endif
//#define SUPPORT_WKE      //需要把third-part/wke/wke.7z解压到bin目录才能打开该开关测试wkeWebkit

#include <souistd.h>
#include <core/SHostDialog.h>
#include <control/SMessageBox.h>
#include <control/souictrls.h>
#include <res.mgr/sobjdefattr.h>

#include "resource.h"	//APP资源

#ifdef SUPPORT_WKE
#include "../controls.extend/SWkeWebkit.h"
#endif

#include "../controls.extend/gif/SGifPlayer.h"


using namespace SOUI;

#ifdef _DEBUG
	# pragma comment(lib, "souid.lib")
#else
	# pragma comment(lib, "soui.lib")
#endif



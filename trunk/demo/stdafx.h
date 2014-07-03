// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//  are changed infrequently
//

#pragma once

#include <duistd.h>
#include <duihostwnd.h>
#include <duictrls.h>
#include <res.mgr/DuiCSS.h>

#if defined(DLL_SOUI) && defined(_DEBUG) && !defined(_WIN64)
// #define LUA_TEST 
#endif

#ifdef LUA_TEST 
#include "..\scriptModule\luaScriptModule\luaScriptModule\luaScriptModule.h"
#pragma comment(lib,"..\\scriptModule\\luaScriptModule\\lib\\luaScriptModule_d.lib")
#endif


#include "resource.h"	//APP×ÊÔ´

#include "../zlib/zconf.h"
#include "../zlib/zlib.h"

using namespace SOUI;

#ifdef _DEBUG

#if !defined(_WIN64)
// #pragma comment(lib,"zlib_d.lib")
#endif

	#ifdef USING_ATL
	# pragma comment(lib, "soui_static_atl_d.lib")
	#elif DLL_SOUI
	# pragma comment(lib, "soui_d.lib")
	#else
	# pragma comment(lib, "soui_static_d.lib")
	#endif
#else

#if !defined(_WIN64)
// #pragma comment(lib,"zlib.lib")
#endif

	#ifdef DLL_SOUI
	# pragma comment(lib, "soui.lib")
	#else
	# pragma comment(lib, "soui_static.lib")
	#endif

#endif



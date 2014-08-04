// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//  are changed infrequently
//

#pragma once

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




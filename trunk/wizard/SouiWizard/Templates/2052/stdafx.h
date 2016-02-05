// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//  are changed infrequently
//

#pragma once

#define  _CRT_SECURE_NO_WARNINGS
[!if DYNAMIC_SOUI]
#define	 DLL_SOUI
[!endif]	
#include <souistd.h>
#include <core/SHostDialog.h>
#include <control/SMessageBox.h>
#include <control/souictrls.h>
#include <res.mgr/sobjdefattr.h>
#include <com-cfg.h>
[!if CHECKBOX_SHELLNOTIFYICON]
#include "trayicon/SShellNotifyIcon.h"
[!endif]
#include "resource.h"
#include "res\resource.h"
using namespace SOUI;


#include "stdafx.h"

extern "C"
{
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
};

#include "../../lua_tinker/lua_tinker.h"

using namespace SOUI;

#include "exp_Basic.h"
#include "exp_app.h"
#include "exp_ScriptModule.h"
#include "exp_ResProvider.h"
#include "exp_msgbox.h"
#include "exp_Window.h"

BOOL SOUI_Export_Lua(lua_State *L)
{
	lua_tinker::init(L);
	BOOL bRet=TRUE;
	bRet=ExpLua_Basic(L);
	bRet=ExpLua_App(L);
	bRet=ExpLua_MessageBox(L);
	bRet=ExpLua_ScriptModule(L);
	bRet=ExpLua_ResProvider(L);
    bRet=ExpLua_Window(L);

	return bRet;
}
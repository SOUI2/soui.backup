#define LUA_LIB

#include "lua.h"

LUA_API int lua_getvernum()
{
	return LUA_VERSION_NUM;
}

LUA_API const char * lua_getverstring()
{
	return LUA_VERSION;
}
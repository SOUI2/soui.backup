#ifndef __IFACE_LUA_MODULE_H__
#define __IFACE_LUA_MODULE_H__
#pragma once

#ifdef _DEBUG
#define LUA_WRAPPER _T("liblua_wrapperd.dll")
#else
#define LUA_WRAPPER _T("liblua_wrapper.dll")
#endif

class ILuaModule {
public:
    virtual void *          LuaGetEngine() = 0;
    virtual bool            LuaInitialize() = 0;
    virtual void            LuaShutdown() = 0;
    virtual const char *    LuaIdentifier() = 0;
    virtual void            LuaExecuteFile(const char *lpszFile) = 0;
    virtual void            LuaExecuteBuffer(const char *lpBuffer, size_t sz) = 0;
    virtual void            LuaExecuteString(const char *str) = 0;
    virtual bool            LuaCall(const char *fun, SOUI::EventArgs *pArg) = 0;
};
#endif // !__IFACE_LUA_MODULE_H__
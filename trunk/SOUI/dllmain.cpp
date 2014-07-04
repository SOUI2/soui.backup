// dllmain.cpp : DllMain 的实现。

#include "souistd.h"

#if defined(USING_ATL) && defined(DLL_SOUI)
//只有使用ATL的动态库版本才需要DllMain

class CSOUIModule : public CAtlDllModuleT< CSOUIModule >
{
} _AtlModule;

// DLL 入口点
extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
    return _AtlModule.DllMain(dwReason, lpReserved); 
}

#endif
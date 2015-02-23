#include <core/SHostWnd.h>

SHostWnd * toHostWnd(SObject *pObj)
{
    if(!pObj->IsClass(SHostWnd::GetClassName()))
        return NULL;
    return (SHostWnd*)pObj;
}

BOOL ExpLua_HostWnd(lua_State *L)
{
	try{
		lua_tinker::class_add<SHostWnd>(L,"SHostWnd");
        lua_tinker::class_inh<SHostWnd,SWindow>(L);
		lua_tinker::class_def<SHostWnd>(L,"AnimateHostWindow",&SHostWnd::AnimateHostWindow);
		lua_tinker::class_def<SHostWnd>(L,"setTimeout",&SHostWnd::setTimeout);
		lua_tinker::class_def<SHostWnd>(L,"setInterval",&SHostWnd::setInterval);
		lua_tinker::class_def<SHostWnd>(L,"clearTimer",&SHostWnd::clearTimer);
        
        lua_tinker::def(L,"toHostWnd",toHostWnd);
		return TRUE;
	}catch(...)
	{
		return FALSE;
	}
}
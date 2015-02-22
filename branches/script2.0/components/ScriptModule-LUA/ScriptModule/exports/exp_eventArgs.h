#include <event/events.h>

EventTimer * toEventTimer(EventArgs *pEvt)
{
    return (EventTimer *)pEvt;
}

BOOL ExpLua_EventArgs(lua_State *L)
{
	try{
		lua_tinker::class_add<EventArgs>(L,"EventArgs");
		lua_tinker::class_def<EventArgs>(L,"GetEventID",&EventArgs::GetEventID);
		lua_tinker::class_mem<EventArgs>(L,"sender",&EventArgs::sender);
        lua_tinker::class_mem<EventArgs>(L,"idFrom",&EventArgs::idFrom);
        lua_tinker::class_mem<EventArgs>(L,"nameFrom",&EventArgs::nameFrom);
        
        lua_tinker::class_add<EventTimer>(L,"EventTimer");
        lua_tinker::class_inh<EventTimer,EventArgs>(L);
        lua_tinker::class_mem<EventTimer>(L,"uID",&EventTimer::uID);
        lua_tinker::def(L,"toEventTimer",toEventTimer);//ÀàÐÍ×ª»»

		return TRUE;
	}catch(...)
	{
		return FALSE;
	}
}
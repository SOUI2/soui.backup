#include <event/events.h>

BOOL ExpLua_EventArgs(lua_State *L)
{
	try{
		lua_tinker::class_add<EventArgs>(L,"EventArgs");
		lua_tinker::class_def<EventArgs>(L,"GetEventID",&EventArgs::GetEventID);
		lua_tinker::class_mem<EventArgs>(L,"sender",&EventArgs::sender);
        lua_tinker::class_mem<EventArgs>(L,"idFrom",&EventArgs::idFrom);
        lua_tinker::class_mem<EventArgs>(L,"nameFrom",&EventArgs::nameFrom);
        	
		return TRUE;
	}catch(...)
	{
		return FALSE;
	}
}
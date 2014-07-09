#pragma  once

#ifdef LUASCRIPTMODULE_EXPORTS
#define LUASCRIPTMODULE_API __declspec(dllexport)
#else
#define LUASCRIPTMODULE_API __declspec(dllimport)
#endif

#include <interface/SScriptModule-i.h>
#include <unknown/obj-ref-impl.hpp>
extern "C"
{
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
};

// 此类是从 luaScriptModule.dll 导出的
class SScriptModule_Lua : public SOUI::TObjRefImpl<SOUI::IScriptModule>
{
public:
	SScriptModule_Lua(void);

	~SScriptModule_Lua();

	virtual void * GetScriptEngine () {return d_state;}

	virtual	void	executeScriptFile(LPCSTR pszScriptFile);
    virtual void executeString(LPCSTR str);

	virtual	bool	executeScriptedEventHandler(LPCSTR handler_name,SOUI::EventArgs *pEvt);

    virtual LPCSTR getIdentifierString() const;

    virtual bool subscribeEvent(SOUI::SWindow* target, UINT uEvent, LPCSTR subscriber_name);
	virtual bool unsubscribeEvent(SOUI::SWindow* target, UINT uEvent, LPCSTR subscriber_name );
protected:
	lua_State * d_state;
};

extern "C" BOOL LUASCRIPTMODULE_API CreateScriptModule_Lua(SOUI::IScriptModule ** ppScript);
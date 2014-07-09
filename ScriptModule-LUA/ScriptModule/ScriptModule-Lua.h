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

	virtual int loadScriptFile(LPCSTR pszScriptFile);

	virtual int loadScriptString(LPCSTR pszScriptStr);

	virtual	void	executeScriptFile(LPCSTR pszScriptFile);


	/*!
	\brief
		Execute a scripted global 'event handler' function.  The function should take some kind of EventArgs like parameter
		that the concrete implementation of this function can create from the passed EventArgs based object.  The function
		should not return anything.

	\param handler_name
		String object holding the name of the scripted handler function.

	\param SOUI::EventArgs *pEvt
		Event Object

	\return
		- true if the event was handled.
		- false if the event was not handled.
	*/
	virtual	bool	executeScriptedEventHandler(LPCSTR handler_name,SOUI::EventArgs *pEvt);


    /*!
    \brief
        Execute script code contained in the given String object.

    \param str
        String object holding the valid script code that should be executed.

    \return
        Nothing.
    */
    virtual void executeString(LPCSTR str);



    /*!
    \brief
        Return identification string for the ScriptModule.  If the internal id string has not been
        set by the ScriptModule creator, a generic string of "Unknown scripting module" will be returned.

    \return
        String object holding a string that identifies the ScriptModule in use.
    */
    virtual LPCSTR getIdentifierString() const;

    /*!
    \brief
            Subscribes or unsubscribe the named Event to a scripted function

    \param target
            The target EventSet for the subscription.

    \param uEvent
            Event ID to subscribe to.

    \param subscriber_name
            String object containing the name of the script function that is to be subscribed to the Event.

    \return 
    */
    virtual bool subscribeEvent(SOUI::SWindow* target, UINT uEvent, LPCSTR subscriber_name);
	virtual bool unsubscribeEvent(SOUI::SWindow* target, UINT uEvent, LPCSTR subscriber_name );
protected:
	lua_State * d_state;
};

extern "C" BOOL LUASCRIPTMODULE_API CreateScriptModule_Lua(SOUI::IScriptModule ** ppScript);
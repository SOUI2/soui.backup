/***********************************************************************
    filename:     DUIScriptModule.h
    created:    2/4/2013
    author:        Jiaxiong Huang

    purpose:    Abstract class interface for scripting support
*************************************************************************/
#pragma once

#include "event/EventSubscriber.h"

// Start of SOUI namespace section
namespace SOUI
{
    class SWindow;
/*!
\brief
    Abstract interface required for all scripting support modules to be used with
    the SOUI system.
*/
class SOUI_EXP IScriptModule
{
public:
    virtual void * GetScriptEngine () = 0;

    virtual int loadScriptFile(LPCSTR pszScriptFile) = 0;

    virtual int loadScriptString(LPCSTR pszScriptStr) = 0;

    /*************************************************************************
        Abstract interface
    *************************************************************************/
    /*!
    \brief
        Execute a script file.

    \param pszScriptFile
        String object holding the filename of the script file that is to be executed
        
    */
    virtual    void    executeScriptFile(LPCSTR pszScriptFile)    = 0;


    /*!
    \brief
        Execute a scripted global 'event handler' function.  The function should take some kind of EventArgs like parameter
        that the concrete implementation of this function can create from the passed EventArgs based object.  The function
        should not return anything.

    \param handler_name
        String object holding the name of the scripted handler function.

    \param SWindow *pSender
        SWindow based object that should be passed, by any appropriate means, to the scripted function.

    \param LPDUINMHDR pNmhdr
        notify message 

    \return
        - true if the event was handled.
        - false if the event was not handled.
    */
    virtual    bool    executeScriptedEventHandler(LPCSTR handler_name, SWindow *pSender, LPSNMHDR pNmhdr)=0;


    /*!
    \brief
        Execute script code contained in the given String object.

    \param str
        String object holding the valid script code that should be executed.

    \return
        Nothing.
    */
    virtual void executeString(LPCSTR str) = 0;



    /*!
    \brief
        Return identification string for the ScriptModule.  If the internal id string has not been
        set by the ScriptModule creator, a generic string of "Unknown scripting module" will be returned.

    \return
        String object holding a string that identifies the ScriptModule in use.
    */
    virtual LPCSTR getIdentifierString() const = 0;

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
    virtual bool subscribeEvent(SWindow* target, UINT uEvent, LPCSTR subscriber_name) = 0;
    virtual bool unsubscribeEvent(SWindow* target, UINT uEvent, LPCSTR subscriber_name ) = 0;

};

} // End of  SOUI namespace section


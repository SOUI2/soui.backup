#pragma once

#define EVENT_MAP_BEGIN()                   \
    BOOL _HandleEvent(SOUI::EventArgs *pEvt)        \
    {                                           \
        UINT      uCode = pEvt->GetEventID();   \
 

#define EVENT_MAP_END()                     \
        return FALSE;                           \
    }                                           \
 
#define CHAIN_EVENT_MAP(ChainClass)         \
        if(ChainClass::_HandleEvent(pEvt))       \
            return TRUE;                         \
 

// BOOL OnEvent(EventArgs *pEvt)
#define EVENT_HANDLER(cd, func) \
    if(cd == uCode) \
{ \
    return func(pEvt); \
} 


// BOOL OnEvent(EventArgs *pEvt)
#define EVENT_ID_HANDLER(id, cd, func) \
    if(cd == uCode && id == pEvt->idFrom) \
    { \
        return func(pEvt); \
    }

// BOOL OnEvent(EventArgs *pEvt)
#define EVENT_NAME_HANDLER(name, cd, func) \
    if(cd == uCode && pEvt->nameFrom!= NULL && wcscmp(pEvt->nameFrom,name)==0) \
{ \
    return func(pEvt); \
}


// void OnCommand(EventArgs *pEvt)
#define EVENT_COMMAND(func)                                                     \
    if (EVT_CMD == uCode)                                                      \
    {                                                                               \
    func(pEvt);   \
    return TRUE;                                                                \
    }                                                                               \

// void OnCommand()
#define EVENT_ID_COMMAND(id, func)                                  \
    if (EVT_CMD == uCode && id == pEvt->idFrom)  \
    {                                                                       \
        func();                                                             \
        return TRUE;                                                        \
    }                                                                       \
 
// void OnCommand()
#define EVENT_ID_COMMAND_RANGE(idMin, idMax, func)                    \
    if (EVT_CMD == uCode && idMin <= pEvt->idFrom && idMax >= pEvt->idFrom )  \
    {                                                                            \
        func(pEvt->idFrom);                                   \
        return TRUE;                                                            \
    }                                                                            \

// void OnCommand()
#define EVENT_NAME_COMMAND(name, func)                                  \
    if (EVT_CMD == uCode && pEvt->nameFrom!= NULL && wcscmp(pEvt->nameFrom,name)==0)  \
    {                                                                       \
        func();                                                             \
        return TRUE;                                                        \
    }                                                                       \

 
// void OnContextMenu(CPoint pt)
#define EVENT_ID_CONTEXTMENU(id,func)                                      \
    if (EVT_CTXMENU == uCode && pEvt->idFrom==id)                          \
{                                                                               \
    func(((SOUI::EventCtxMenu*)pEvt)->pt);                                        \
    return TRUE;                                                                \
}                                                                               \


// void OnContextMenu(CPoint pt)
#define EVENT_NAME_CONTEXTMENU(name,func)                                      \
    if (EVT_CTXMENU == uCode && pEvt->nameFrom!= NULL && wcscmp(pEvt->nameFrom,name)==0) \
{                                                                               \
    func(((SOUI::EventCtxMenu*)pEvt)->pt);                                        \
    return TRUE;                                                                \
}                                                                               \


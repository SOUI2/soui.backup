
#ifdef DLL_SOUI
# ifdef SOUI_EXPORTS
# pragma message("dll export")
#  define SOUI_EXP __declspec(dllexport)
# else
#  define SOUI_EXP __declspec(dllimport)
# endif // SOUI_EXPORTS
#else
#define SOUI_EXP
#endif

// Change these values to use different versions
#define WINVER        0x0500
#define _WIN32_WINNT    0x0501
#define _WIN32_IE    0x0601
#define _RICHEDIT_VER    0x0200

#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NON_CONFORMING_SWPRINTFS


# pragma warning(disable:4661)
# pragma warning(disable:4251)
# pragma warning(disable:4100)    //unreferenced formal parameter
# pragma warning(disable:4355)

#include <Windows.h>
#include <CommCtrl.h>
#include <Shlwapi.h>
#include <OleCtl.h>
#include <tchar.h>
#include <stdio.h>

#include "duidef.h"

//export pugixml interface
#include "../pugixml/pugixml.hpp"

#include "DuiUtilities.h"

#ifdef USING_ATL
    #define _COLL_NS    ATL
    #include <atlbase.h>
    #include <atlapp.h>
    #include <atlmisc.h>
    #include <atlgdi.h>
    #include <atlstr.h>
    #include <atlcoll.h>
    #include <atlcomcli.h>
    #define CDuiComPtr    CComPtr
    #define CDuiComQIPtr    CComQIPtr

    #include "wtl.mini/duicrack.h"
    #define CDuiArray    CAtlArray
    #define CDuiList    CAtlList
    #define CDuiMap        CAtlMap
    #define CDuiStringA    CAtlStringA
    #define CDuiStringW CAtlStringW
    #define CDuiStringT CAtlString

    #include "wtl.mini/strcpcvt.h"

#else//ATL_FREE
    #define _WTYPES_NS SOUI
    #define _COLL_NS    SOUI
    #include "wtl.mini/duicrack.h"
    #include "wtl.mini/duimisc.h"
    #include "wtl.mini/duigdi.h"
    #include "wtl.mini/tstring.h" 
    #include "wtl.mini/strcpcvt.h"
    #include "wtl.mini/duicoll.h"
    #include "atl.mini/atldef.h"
    #include "atl.mini/duicomcli.h"

#endif//USING_ATL


#include "DuiAttrCrack.h"
#include "duifontpool.h"
#include "duiresprovider.h"
#include "duisystem.h"
#include "DuiMessageBox.h"

#pragma comment(lib,"Msimg32.lib")
#pragma comment(lib,"shlwapi.lib")
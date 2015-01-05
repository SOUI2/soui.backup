//SOUI组件配置

#pragma  once

#include <com-def.h>

#ifdef LIB_SOUI_COM
#pragma message("LIB_SOUI_COM")

#pragma comment(lib,"Usp10")
#pragma comment(lib,"opengl32")

#ifdef _DEBUG
#pragma comment(lib,"skiad")
#pragma comment(lib,"zlibd")
#ifdef DLL_CORE
#pragma comment(lib,"lua-51d")
#pragma comment(lib,"scriptmodule-luad")
#endif
#pragma comment(lib,"render-gdid")
#pragma comment(lib,"render-skiad")
#pragma comment(lib,"imgdecoder-wicd")
#pragma comment(lib,"translatord")
#pragma comment(lib,"resprovider-zipd")
#else
#pragma comment(lib,"skia")
#pragma comment(lib,"zlib")
#ifdef DLL_CORE
#pragma comment(lib,"lua-51")
#pragma comment(lib,"scriptmodule-lua")
#endif

#pragma comment(lib,"imgdecoder-wic")
#pragma comment(lib,"render-gdi")
#pragma comment(lib,"render-skia")
#pragma comment(lib,"translator")
#pragma comment(lib,"resprovider-zip")
#endif

namespace SOUI
{
    namespace IMGDECODOR_WIC
    {
        BOOL SCreateInstance(IObjRef **);
    }
    namespace RENDER_GDI
    {
        BOOL SCreateInstance(IObjRef **);
    }
    namespace RENDER_SKIA
    {
        BOOL SCreateInstance(IObjRef **);
    }
    namespace SCRIPT_LUA
    {
        BOOL SCreateInstance(IObjRef **);
    }
    namespace TRANSLATOR
    {
        BOOL SCreateInstance(IObjRef **);
    }
    namespace RESPROVIDER_ZIP
    {
        BOOL SCreateInstance(IObjRef **);
    }
}//end of soui

class SComMgr
{
public:
    BOOL CreateImgDecoder(IObjRef ** ppObj)
    {
        return SOUI::IMGDECODOR_WIC::SCreateInstance(ppObj);
    }

    BOOL CreateRender_GDI(IObjRef **ppObj)
    {
        return SOUI::RENDER_GDI::SCreateInstance(ppObj);
    }

    BOOL CreateRender_Skia(IObjRef **ppObj)
    {
        return SOUI::RENDER_SKIA::SCreateInstance(ppObj);
    }
    BOOL CreateScrpit_Lua(IObjRef **ppObj)
    {
        return SOUI::SCRIPT_LUA::SCreateInstance(ppObj);
    }

    BOOL CreateTranslator(IObjRef **ppObj)
    {
        return SOUI::TRANSLATOR::SCreateInstance(ppObj);
    }
    BOOL CreateResProvider_ZIP(IObjRef **ppObj)
    {
        return SOUI::RESPROVIDER_ZIP::SCreateInstance(ppObj);
    }
};

#else
	
#include <com-loader.hpp>

#ifdef _DEBUG
#define COM_IMGDECODER  _T("imgdecoder-pngd.dll")
#define COM_RENDER_GDI  _T("render-gdid.dll")
#define COM_RENDER_SKIA _T("render-skiad.dll")
#define COM_SCRIPT_LUA _T("scriptmodule-luad.dll")
#define COM_TRANSLATOR _T("translatord.dll")
#define COM_ZIPRESPROVIDER _T("resprovider-zipd.dll")
#else
#define COM_IMGDECODER  _T("imgdecoder-png.dll")
#define COM_RENDER_GDI  _T("render-gdi.dll")
#define COM_RENDER_SKIA _T("render-skia.dll")
#define COM_SCRIPT_LUA _T("scriptmodule-lua.dll")
#define COM_TRANSLATOR _T("translator.dll")
#define COM_ZIPRESPROVIDER _T("resprovider-zip.dll")
#endif

class SComMgr
{
public:
   BOOL CreateImgDecoder(IObjRef ** ppObj)
    {
        return imgDecLoader.CreateInstance(COM_IMGDECODER,ppObj);
    }
    
    BOOL CreateRender_GDI(IObjRef **ppObj)
    {
        return renderLoader.CreateInstance(COM_RENDER_GDI,ppObj);
    }

    BOOL CreateRender_Skia(IObjRef **ppObj)
    {
        return renderLoader.CreateInstance(COM_RENDER_SKIA,ppObj);
    }
    BOOL CreateScrpit_Lua(IObjRef **ppObj)
    {
        return scriptLoader.CreateInstance(COM_SCRIPT_LUA,ppObj);
    }

    BOOL CreateTranslator(IObjRef **ppObj)
    {
        return transLoader.CreateInstance(COM_TRANSLATOR,ppObj);
    }
    BOOL CreateResProvider_ZIP(IObjRef **ppObj)
    {
        return zipResLoader.CreateInstance(COM_ZIPRESPROVIDER,ppObj);
    }
protected:
    //SComLoader实现从DLL的指定函数创建符号SOUI要求的类COM组件。
    SComLoader imgDecLoader;
    SComLoader renderLoader;
    SComLoader transLoader;
    SComLoader scriptLoader;
    SComLoader zipResLoader;
};
#endif

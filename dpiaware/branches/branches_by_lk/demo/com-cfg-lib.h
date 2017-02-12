//LIB°æ±¾×é¼þÅäÖÃ

#pragma  once


#pragma comment(lib,"usp10")

#ifdef _DEBUG
#pragma comment(lib,"myskiad")
#pragma comment(lib,"freetyped")
#pragma comment(lib,"zlibd")
#ifdef DLL_SOUI
#pragma comment(lib,"lua-51d")
#pragma comment(lib,"scriptmodule-luad")
#endif
#pragma comment(lib,"render-gdid")
#pragma comment(lib,"imgdecoder-wicd")
#pragma comment(lib,"render-skiad")
#pragma comment(lib,"translatord")
#pragma comment(lib,"resprovider-zipd")
#else
#pragma comment(lib,"myskia")
#pragma comment(lib,"freetype")
#pragma comment(lib,"zlib")
#ifdef DLL_SOUI
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

class SComMgrLib
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
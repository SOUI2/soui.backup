// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the TRANSLATOR_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// TRANSLATOR_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef TRANSLATOR_EXPORTS
#define TRANSLATOR_API __declspec(dllexport)
#else
#define TRANSLATOR_API __declspec(dllimport)
#endif


#include "souicoll.h"
#include "string/tstring.h"
#include "string/strcpcvt.h"
#include "pugixml/pugixml.hpp"

namespace SOUI
{
    class SStrMap;
    class SStrMapEntry;

class TRANSLATOR_API SLang
{
    friend class STranslator;
public:
    SLang();
    ~SLang();
    BOOL Load(LPCTSTR pszFileName);
    BOOL LoadXML(pugi::xml_node xmlLang);
    SStringW GetLangName(){return m_strLang;}
protected:
    SStringW m_strLang;
    SArray<SStrMapEntry*> * m_arrEntry;
};

class TRANSLATOR_API STranslator
{
public:
    STranslator(void);
    ~STranslator(void);
    
    void InstallLang(SLang *pLang);
    void UninstallLang(SLang *pLang);
    
    void PushContext(const SStringW &strCtx);
    void PushContext(const SStringA &strCtx);
    SStringW PopContext();
    
    SStringW tr(const SStringW & str);
    SStringA tr(const SStringA & str);
protected:
    SList<SLang*> *m_lstLang;
    SList<SStringW> *m_ctxStack;
};


}

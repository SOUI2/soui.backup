// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the TRANSLATOR_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// TRANSLATOR_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.

#include <souicoll.h>
#include <unknown/obj-ref-impl.hpp>
#include <string/tstring.h>
#include <string/strcpcvt.h>
#include <pugixml/pugixml.hpp>
#include <interface/STranslator-i.h>

namespace SOUI
{
    class SStrMap;
    class SStrMapEntry;
    
    enum LANGDATA{
    LD_UNKNOWN=0,
    LD_XML,
    LD_COMPILEDFILE,
    LD_COMPILEDDATA,
    };
    
class SLang : public TObjRefImpl<ILang>
{
    friend class STranslator;
public:
    SLang();
    ~SLang();
   
    virtual BOOL Load(LPVOID pData,UINT uType);
    
    virtual SStringW name();
    virtual GUID     guid();
    virtual BOOL tr(const SStringW & strSrc,const SStringW & strCtx,SStringW & strRet);
protected:
    BOOL LoadFromXml(pugi::xml_node xmlLang);
    
    SStringW m_strLang;
    GUID     m_guid;
    SArray<SStrMapEntry*> * m_arrEntry;
};

class STranslator : public TObjRefImpl<ITranslator>
{
public:
    STranslator(void);
    ~STranslator(void);
    
    /*virtual */
    BOOL CreateLang(ILang ** ppLang);
    /*virtual */
    BOOL InstallLang(ILang *pLang);
    /*virtual */
    BOOL UninstallLang(REFGUID id);
    /*virtual */
    SStringW tr(const SStringW & strSrc,const SStringW & strCtx);
protected:
    SList<ILang*> *m_lstLang;
};

extern "C" __declspec(dllexport) BOOL SCreateInstance(IObjRef **ppTrans);

}

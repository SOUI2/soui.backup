#include "include\souistd.h"
#include "res.mgr\SUiDef.h"
#include "helper\SplitString.h"
#include "helper\mybuffer.h"

namespace SOUI{

	const static WCHAR KNodeUidef[]     = L"uidef" ;
	const static WCHAR KNodeString[]    = L"string";
	const static WCHAR KNodeColor[]     = L"color";
	const static WCHAR KNodeSkin[]      = L"skin";
	const static WCHAR KNodeStyle[]     = L"style";
	const static WCHAR KNodeObjAttr[]   = L"objattr";


	static pugi::xml_node GetSourceXmlNode(pugi::xml_node nodeRoot,pugi::xml_document &docInit,IResProvider *pResProvider, const wchar_t * pszName)
	{
		pugi::xml_node     nodeData = nodeRoot.child(pszName,false);
		if(nodeData)
		{
			pugi::xml_attribute attrSrc = nodeData.attribute(L"src",false);
			if(attrSrc)
			{//优先从src属性里获取数据
				SStringT strSrc = S_CW2T(attrSrc.value());
				SStringTList strList;
				if(2==ParseResID(strSrc,strList))
				{
					CMyBuffer<char> strXml;
					DWORD dwSize = pResProvider->GetRawBufferSize(strList[0],strList[1]);

					strXml.Allocate(dwSize);
					pResProvider->GetRawBuffer(strList[0],strList[1],strXml,dwSize);
					pugi::xml_parse_result result= docInit.load_buffer(strXml,strXml.size(),pugi::parse_default,pugi::encoding_utf8);
					if(result) nodeData = docInit.child(pszName);
				}
			}
		}
		return nodeData;
	}

	class SUiDefInfo : public TObjRefImpl<IUiDefInfo>
	{
	public:
		SUiDefInfo(IResProvider *pResProvide,LPCTSTR pszUidef);

		virtual SSkinPool * GetSkinPool() {return pSkinPool;}
		virtual SStylePool * GetStylePool(){return pStylePool;}
		virtual SNamedColor & GetNamedColor() {return namedColor;}
		virtual SNamedString & GetNamedString() {return namedString;}
	protected:

		CAutoRefPtr<SSkinPool>    pSkinPool;
		CAutoRefPtr<SStylePool>   pStylePool;

		SNamedColor   namedColor;
		SNamedString  namedString;
	};

	SUiDefInfo::SUiDefInfo(IResProvider *pResProvider,LPCTSTR pszUidef)
	{
		SStringTList strUiDef;
		if(2!=ParseResID(pszUidef,strUiDef))
		{
			SLOGFMTW(_T("warning!!!! Add ResProvider Error."));
		}

		DWORD dwSize=pResProvider->GetRawBufferSize(strUiDef[0],strUiDef[1]);
		if(dwSize==0)
		{
			SLOGFMTW(_T("warning!!!! uidef was not found in the specified resprovider"));
		}else
		{
			pugi::xml_document docInit;
			CMyBuffer<char> strXml;
			strXml.Allocate(dwSize);

			pResProvider->GetRawBuffer(strUiDef[0],strUiDef[1],strXml,dwSize);

			pugi::xml_parse_result result= docInit.load_buffer(strXml,strXml.size(),pugi::parse_default,pugi::encoding_utf8);

			if(!result)
			{//load xml failed
				SLOGFMTW(_T("warning!!! load uidef as xml document failed"));
			}else
			{//init named objects
				pugi::xml_node root = docInit.child(KNodeUidef,false);
				if(!root)
				{
					SLOGFMTW(_T("warning!!! \"uidef\" element is not the root element of uidef xml"));
				}else
				{
					//set default font
					pugi::xml_node xmlFont;
					xmlFont=root.child(L"font",false);
					if(xmlFont)
					{
						int nSize=xmlFont.attribute(L"size").as_int(12);
						BYTE byCharset=(BYTE)xmlFont.attribute(L"charset").as_int(DEFAULT_CHARSET);
						SFontPool::getSingleton().SetDefaultFont(S_CW2T(xmlFont.attribute(L"face").value()),nSize,byCharset);
					}

					//load named string
					{
						pugi::xml_document docData;
						pugi::xml_node     nodeData = GetSourceXmlNode(root,docData,pResProvider,KNodeString);
						if(nodeData)
						{
							namedString.Init(nodeData);
						}
					}

					//load named color
					{
						pugi::xml_document docData;
						pugi::xml_node     nodeData = GetSourceXmlNode(root,docData,pResProvider,KNodeColor);
						if(nodeData)
						{
							namedColor.Init(nodeData);
						}
					}

					//load named skin
					{
						pugi::xml_document docData;
						pugi::xml_node     nodeData = GetSourceXmlNode(root,docData,pResProvider,KNodeSkin);
						if(nodeData)
						{
							pSkinPool = new SSkinPool;
							pSkinPool->LoadSkins(nodeData);
							SSkinPoolMgr::getSingletonPtr()->PushSkinPool(pSkinPool);
						}
					}
					//load named style
					{
						pugi::xml_document docData;
						pugi::xml_node     nodeData = GetSourceXmlNode(root,docData,pResProvider,KNodeStyle);
						if(nodeData)
						{
							pStylePool = new SStylePool;
							pStylePool->Init(nodeData);
							SStylePoolMgr::getSingleton().PushStylePool(pStylePool);
						}
					}
					//load SWindow default attribute
					if(SObjDefAttr::getSingleton().IsEmpty())
					{//style只能加载一次
						pugi::xml_document docData;
						pugi::xml_node     nodeData = GetSourceXmlNode(root,docData,pResProvider,KNodeObjAttr);
						if(nodeData)
						{
							SObjDefAttr::getSingleton().Init(nodeData);
						}
					}

				}
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////

	template<> SUiDef * SSingleton<SUiDef>::ms_Singleton = NULL;

	SUiDef::SUiDef(void)
	{
	}

	SUiDef::~SUiDef(void)
	{
	}

	IUiDefInfo * SUiDef::CreateUiDefInfo(IResProvider *pResProvider, LPCTSTR pszUiDef)
	{
		return new SUiDefInfo(pResProvider,pszUiDef);
	}

}

#pragma once

#include "duiwndpanel.h"
#include "DuiSystem.h"
#include "mybuffer.h"

namespace DuiEngine
{

/*
使用另外一个资源XML来加载子元素

<dlgfile src=16 />
*/
class DUI_EXP CDuiDialogFile : public CDuiDialog
{
    DUIOBJ_DECLARE_CLASS_NAME(CDuiDialogFile, "dlgfile")

public:

    virtual BOOL Load(pugi::xml_node xmlNode)
    {
        if (!CDuiWindow::Load(xmlNode))
            return FALSE;

        CDuiStringT	strChildSrc=DUI_CA2T(xmlNode.attribute("src").value(),CP_UTF8);

        if (strChildSrc.IsEmpty())
            return FALSE;

        DuiResProviderBase *pRes=DuiSystem::getSingleton().GetResProvider();
        if(!pRes) return FALSE;

        DWORD dwSize=pRes->GetRawBufferSize(DUIRES_XML_TYPE,strChildSrc);
        if(dwSize==0) return FALSE;

        CMyBuffer<char> strXml;
        strXml.Allocate(dwSize);
        pRes->GetRawBuffer(DUIRES_XML_TYPE,strChildSrc,strXml,dwSize);

		pugi::xml_document xmlDoc;

		if(!xmlDoc.load_buffer_inplace(strXml,strXml.size(),pugi::parse_default,pugi::encoding_utf8))
		{
			DUIASSERT(FALSE);
			return FALSE;
		}

        return LoadChildren(xmlDoc.first_child());
    }

};

}//namespace DuiEngine
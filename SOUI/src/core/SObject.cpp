#include "souistd.h"
#include "core/sobject.h"
#include "res.mgr/SObjDefAttr.h"

namespace SOUI
{
    BOOL SObject::InitFromXml( pugi::xml_node xmlNode )
    {
        if(!xmlNode) return FALSE;
#ifdef _DEBUG
        {
            pugi::xml_writer_buff writer;
            xmlNode.print(writer,L"\t",pugi::format_default,pugi::encoding_utf16);
            m_strXml=SStringW(writer.buffer(),writer.size());
        }
#endif

        //检索并设置类的默认属性
        pugi::xml_node defAttr = GETCSS(GetObjectClass());
        if(defAttr)
        {
            //优先处理"class"属性
            pugi::xml_attribute attrClass=defAttr.attribute(L"class");
            if(attrClass)
            {
                defAttr.remove_attribute(L"class");
                SetAttribute(attrClass.name(), attrClass.value(), TRUE);
            }
            for (pugi::xml_attribute attr = defAttr.first_attribute(); attr; attr = attr.next_attribute())
            {
                SetAttribute(attr.name(), attr.value(), TRUE);
            }
            if(attrClass)
            {
                defAttr.prepend_copy(attrClass);
            }
        }

        //设置当前对象的属性

        //优先处理"class"属性
        pugi::xml_attribute attrClass=xmlNode.attribute(L"class");
        if(attrClass)
        {
            SetAttribute(attrClass.name(), attrClass.value(), TRUE);
            xmlNode.remove_attribute(attrClass);
        }
        for (pugi::xml_attribute attr = xmlNode.first_attribute(); attr; attr = attr.next_attribute())
        {
            SetAttribute(attr.name(), attr.value(), TRUE);
        }
        if(attrClass)
        {
            xmlNode.prepend_copy(attrClass);
        }
        //调用初始化完成接口
        OnInitFinished(xmlNode);
        return TRUE;
    }
}//end of namespace
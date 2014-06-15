#include "duistd.h"
#include "duiobject.h"

namespace SOUI
{


    BOOL SObject::Load( pugi::xml_node xmlNode )
    {
        if(!xmlNode) return FALSE;
#ifdef _DEBUG
        {
            pugi::xml_writer_buff writer;
            xmlNode.print(writer);
            m_strXml=DUI_CA2A(CDuiStringA(writer.buffer(),writer.size()),CP_UTF8);
        }
#endif

        //检索并设置类的默认属性
        pugi::xml_node defAttr = GETCSS(GetObjectClass());
        if(defAttr)
        {
            //优先处理"class"属性
            pugi::xml_attribute attrClass=defAttr.attribute("class");
            if(attrClass) SetAttribute(attrClass.name(), attrClass.value(), TRUE);
            for (pugi::xml_attribute attr = defAttr.first_attribute(); attr; attr = attr.next_attribute())
            {
                if(strcmp(attr.name(),"class")==0) continue;
                SetAttribute(attr.name(), attr.value(), TRUE);
            }
        }

        //设置当前对象的属性

        //优先处理"class"属性
        pugi::xml_attribute attrClass=xmlNode.attribute("class");
        if(attrClass) SetAttribute(attrClass.name(), attrClass.value(), TRUE);
        for (pugi::xml_attribute attr = xmlNode.first_attribute(); attr; attr = attr.next_attribute())
        {
            if(strcmp(attr.name(),"class")==0) continue;
            SetAttribute(attr.name(), attr.value(), TRUE);
        }
        OnAttributeFinish(xmlNode);
        return TRUE;
    }
}//end of namespace
#include "stdafx.h"
#include "ResManger.h"
#include "CDebug.h"
#include "helpapi.h"

ResManger::ResManger()
{
}


ResManger::~ResManger()
{
}

void ResManger::LoadUIResFromFile(SStringT strPath)
{
	m_strProPath = strPath.Mid(0, strPath.ReverseFind(_T('\\')));
	m_strUIResFile = strPath;

	LoadUIRes();
	LoadSkinFile();
	LoadStringFile();
	LoadColorFile();
	LoadStyleFile();
	LoadObjattrFile();
}

void ResManger::ReleaseUIRes()
{
	m_strUIResFile = m_strProPath = L"";

	m_mapResFile.RemoveAll();
	m_mapXmlFile.RemoveAll();

	m_strSkinFile = L"";
	m_strStringFile = L"";
	m_strColorFile = L"";
	m_strStyleFile = L"";
	m_strObjattrFile = L"";

	m_xmlDocSkin.reset();
	m_xmlDocString.reset();
	m_xmlDocColor.reset();
	m_xmlDocStyle.reset();
	m_xmlDocObjattr.reset();
}

void ResManger::SaveRes()
{
	m_xmlDocUiRes.save_file(m_strUIResFile);
	m_xmlDocSkin.save_file(m_strSkinFile);
// 	m_xmlDocString.save_file(m_strStringFile);
// 	m_xmlDocColor.save_file(m_strColorFile);
// 	m_xmlDocStyle.save_file(m_strStyleFile);
// 	m_xmlDocObjattr.save_file(m_strObjattrFile);
}

pugi::xml_node ResManger::GetResFirstNode(const SStringT tagname)
{
	pugi::xml_document * xmlDoc = &m_xmlDocSkin;
	if (tagname == _T("string"))
		xmlDoc = &m_xmlDocString;
	if (tagname == _T("color"))
		xmlDoc = &m_xmlDocColor;
	if (tagname == _T("style"))
		xmlDoc = &m_xmlDocStyle;
	if (tagname == _T("objattr"))
		xmlDoc = &m_xmlDocObjattr;

	pugi::xml_node xmlNode;
	if (xmlDoc->child(tagname))
	{
		xmlNode = xmlDoc->child(tagname).first_child();
	}
	else if (xmlDoc->child(L"UIDEF"))
	{
		xmlNode = xmlDoc->child(L"UIDEF").child(tagname).first_child();
	}
	return xmlNode;
}

void ResManger::LoadUIRes()
{
	if (!m_xmlDocUiRes.load_file(m_strUIResFile, pugi::parse_full))
	{
		CDebug::Debug(_T("加载uires文件失败"));
		return;
	}

	m_xmlNodeUiRes = m_xmlDocUiRes.root();

	pugi::xml_node xmlNode = m_xmlNodeUiRes.child(L"resource").first_child();
	GetSubNodes(xmlNode, L"");
	
/*  // 测试代码
	SPOSITION pos = m_mapXmlFile.GetStartPosition();
	while (pos)
	{
		auto aa = m_mapXmlFile.GetAt(pos);
		m_mapXmlFile.GetNext(pos);
	}
*/

	// 获取 Init.xml 文件名
	pugi::xml_node xmlNode_init = m_xmlNodeUiRes.child(L"resource").child(L"UIDEF").first_child();

	while (xmlNode_init)
	{
		if (xmlNode_init.type() == pugi::node_element)
		{
			break;
		}
		else
		{
			xmlNode_init = xmlNode_init.next_sibling();
		}
	}

	SStringW strPath;
	strPath = xmlNode_init.attribute(L"path").value();
	//while (xmlNode)
	//{
	//	SStringW str(L"XML_INIT");
	//	if (str.CompareNoCase(xmlNode.attribute(L"name").value()) == 0 )
	//	{
	//		strPath = xmlNode.attribute(L"path").value();
	//		break;
	//	}
	//	xmlNode = xmlNode.next_sibling();
	//}

	if (!strPath.IsEmpty())
	{
		m_strInitFile = m_strProPath + _T("\\") + strPath;
	}

	if (xmlNode.attribute(L"skin"))
	{
		m_strSkinFile = xmlNode.attribute(L"path").value();
	}
	if (xmlNode.attribute(L"string"))
	{
		m_strStringFile = xmlNode.attribute(L"path").value();
	}
	if (xmlNode.attribute(L"color"))
	{
		m_strColorFile = xmlNode.attribute(L"path").value();
	}
	if (xmlNode.attribute(L"style"))
	{
		m_strStyleFile = xmlNode.attribute(L"path").value();
	}
	if (xmlNode.attribute(L"objattr"))
	{
		m_strObjattrFile = xmlNode.attribute(L"path").value();
	}
}

void ResManger::GetSubNodes(pugi::xml_node& parentNode, SStringT parentNodeName)
{
	while (parentNode)
	{
		if (parentNode.type() == pugi::node_element)
		{
			SStringT strParentName = parentNode.name();
			if (parentNode.first_child() != NULL)
			{
				GetSubNodes(parentNode.first_child(), strParentName + L":");
			}
			else
			{
				if (strParentName == L"file")
				{
					SStringW strName, strPath;
					strName = parentNode.attribute(L"name").value();
					strPath = parentNode.attribute(L"path").value();
					if (!strName.IsEmpty() && !strPath.IsEmpty())
					{
						SStringT extname = GetFileExtname(strPath);
						if (extname.CompareNoCase(_T(".xml")) == 0)
						{
							m_mapXmlFile[parentNodeName + strName] = strPath;
						}
						else
						{
							m_mapResFile[parentNodeName + strName] = strPath;
						}
					}
				}
			}
		}
		parentNode = parentNode.next_sibling();
	}
}

// 删除资源类型名 如LAYOUT:sin_manm 将变成 sin_manm
SStringT ResManger::RemoveResTypename(const SStringT& resname)
{
	int nPos = resname.ReverseFind(':');
	if (nPos == -1)	nPos = 0;
	return resname.Mid(nPos + 1).Trim();
}

SStringT ResManger::GetResPathByName(const SStringT& resname)
{
	const SMap<SStringT, SStringT>::CPair * pFilePair = m_mapXmlFile.Lookup(resname);
	if (pFilePair == NULL)
	{
		pFilePair = m_mapResFile.Lookup(resname);
	}
	if (pFilePair == NULL)
		return _T("");

	return m_strProPath + _T("\\") + pFilePair->m_value;
}

void ResManger::LoadResFileEx(SStringT& filepath, pugi::xml_document& xmlDoc, SStringT tagname)
{
	if (!m_strInitFile.IsEmpty())
	{
		// 假设在Init.xml文件中定义了Skin
		if (filepath.IsEmpty())
			filepath = m_strInitFile;

		pugi::xml_parse_result result = xmlDoc.load_file(m_strInitFile, pugi::parse_full);
		if (result)
		{
			pugi::xml_node xmlNode1 = xmlDoc.child(L"UIDEF").child(tagname);
			if (xmlNode1.attribute(L"src"))
			{
				SStringT strSrc = xmlNode1.attribute(L"src").value();
				const SMap<SStringT, SStringT>::CPair * pFilePair = m_mapXmlFile.Lookup(strSrc);
				if (pFilePair == NULL)
				{
					SASSERT_FMTW(L"Locating filepath failed, src=%s", strSrc);
					return;
				}
				
				filepath = m_strProPath + _T("\\") + pFilePair->m_value;
				result = xmlDoc.load_file(filepath, pugi::parse_full);
				if (!result)
				{
					SStringT tmpstr;
					tmpstr.Format(_T("加载%s文件失败"), tagname);
					SMessageBox(NULL, _T("Resmgr"), tmpstr, MB_OK);
				}
			}
		}
	}
}

void ResManger::LoadSkinFile()
{
	LoadResFileEx(m_strSkinFile, m_xmlDocSkin, _T("skin"));
}

void ResManger::LoadStringFile()
{
	LoadResFileEx(m_strStringFile, m_xmlDocString, _T("string"));
}

void ResManger::LoadColorFile()
{
	LoadResFileEx(m_strColorFile, m_xmlDocColor, _T("color"));
}

void ResManger::LoadStyleFile()
{
	LoadResFileEx(m_strStyleFile, m_xmlDocStyle, _T("style"));
}

void ResManger::LoadObjattrFile()
{
	LoadResFileEx(m_strObjattrFile, m_xmlDocObjattr, _T("objattr"));
}

#pragma once


class ResManger
{
public:
	ResManger();
	~ResManger();

	void LoadUIResFromFile(SStringT strPath);
	void ReleaseUIRes();
	void SaveRes();

	pugi::xml_node GetResFirstNode(const SStringT tagname);

	void LoadUIRes();

	void LoadSkinFile();
	void LoadStringFile();
	void LoadColorFile();
	void LoadStyleFile();
	void LoadObjattrFile();

	void GetSubNodes(pugi::xml_node & parentNode, SStringT parentNodeName);

	static SStringT RemoveResTypename(const SStringT & resname);

	SStringT GetResPathByName(const SStringT & resname);

protected:
	void LoadResFileEx(SStringT & filepath, pugi::xml_document &xmlDoc, SStringT tagname);

public:
	SStringT m_strProPath;			// 加载资源文件的根目录
	SStringT m_strUIResFile;		// uires.idx 完整文件名
	SStringT m_strInitFile;			// Init.xml 完整文件名

	SStringT m_strSkinFile;			// skin完整文件名
	SStringT m_strStringFile;		// string完整文件名
	SStringT m_strColorFile;		// color完整文件名
	SStringT m_strStyleFile;		// Style完整文件名
	SStringT m_strObjattrFile;		// Objattr完整文件名

	pugi::xml_document m_xmlDocUiRes;			// uires.idx文件xml doc
	pugi::xml_node m_xmlNodeUiRes;				// uires.idx文件的根结点
	pugi::xml_document m_xmlDocSkin;			// skin定义文件xml doc
	pugi::xml_document m_xmlDocColor;			// Color定义文件xml doc
	pugi::xml_document m_xmlDocString;			// String定义文件xml doc
	pugi::xml_document m_xmlDocStyle;			// Style定义文件xml doc
	pugi::xml_document m_xmlDocObjattr;			// Objattr定义文件xml doc


	SMap<SStringT, SStringT> m_mapResFile;
	SMap<SStringT, SStringT> m_mapXmlFile;		// 所有XML文件信息
};


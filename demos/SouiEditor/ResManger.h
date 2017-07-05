#pragma once


class ResManger
{
public:
	ResManger();
	~ResManger();

	struct SkinItem
	{
		SStringT class_name;
		SStringT name;
		SStringT src;
		pugi::xml_node attrdoc;
		SkinItem() { ; }
		SkinItem(SStringT _classname, SStringT _name, SStringT _src, pugi::xml_node xmlnode)
		{
			class_name = _classname;
			name = _name;
			src = _src;
			attrdoc.append_copy(xmlnode);
		}
	};

	struct StyleItem
	{
		SStringT class_name;
		SStringT name;
		pugi::xml_node attrdoc;
		StyleItem() { ; }
		StyleItem(SStringT _classname, SStringT _name, pugi::xml_node xmlnode)
		{
			class_name = _classname;
			name = _name;
			attrdoc.append_copy(xmlnode);
		}
	};

	struct ValueItem
	{
		SStringT class_name;
		SStringT value;

		ValueItem() { ; }
		ValueItem(SStringT _classname, SStringT _value)
		{
			class_name = _classname;
			value = _value;
		}
	};


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

	SkinItem GetSkinByImg(SStringT srcimg);

	SStringA GetSkinAutos();

	SStringA GetStyleAutos();

	SStringA GetStringAutos();

	SStringA GetColorAutos();


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

	SMap<SStringT, SkinItem> m_mapSkins;		//所有定义的Skin项目
	SMap<SStringT, StyleItem> m_mapStyles;		//所有定义的Style项目
	SMap<SStringT, ValueItem> m_mapStrings;		//所有定义的String
	SMap<SStringT, ValueItem> m_mapColors;		//所有定义的Color
};


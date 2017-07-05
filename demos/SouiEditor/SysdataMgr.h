#pragma once


class CSysDataMgr
{
public:
	struct CtrlAttrItem
	{
		SStringT attrname;
		pugi::xml_node attrdoc;
		CtrlAttrItem(SStringT name, pugi::xml_node xmlnode)
		{
			attrname = name;
			attrdoc.append_copy(xmlnode);
		}
	};

	typedef SArray<CtrlAttrItem> CTRL_ATTR_VALUE;

	CSysDataMgr();
	~CSysDataMgr();

	// 读取系统数据, 控件属性表
	bool LoadSysData(LPCTSTR cfgDir);

	void InitProperty();

	void InitCtrlProperty(pugi::xml_node NodeCom, pugi::xml_node NodeCtrl, CTRL_ATTR_VALUE* arr_attr);

	// 获取控件名称列表
	SStringA GetCtrlAutos();

	// 获取指定控件的自动完成字串
	SStringA GetCtrlAttrAutos(SStringT ctrlname);

public:
	pugi::xml_document m_xmlDocProperty;	//property.xml文件doc


private:
	static int CtrlAttrCmp(const void * p1, const void*p2)
	{
		const CtrlAttrItem *tag1 = (const CtrlAttrItem*)p1;
		const CtrlAttrItem *tag2 = (const CtrlAttrItem*)p2;
		return tag1->attrname.Compare(tag2->attrname);
	}
	static int CtrlAttrCmpNoCase(const void * p1, const void*p2)
	{
		const CtrlAttrItem *tag1 = (const CtrlAttrItem*)p1;
		const CtrlAttrItem *tag2 = (const CtrlAttrItem*)p2;
		return tag1->attrname.CompareNoCase(tag2->attrname);
	}

	SStringT m_strConfigDir;

	// 控件属性
	SMap<SStringT, CTRL_ATTR_VALUE*> m_mapControl;
};
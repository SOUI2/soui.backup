#include "stdafx.h"
#include "SysdataMgr.h"
#include "CDebug.h"
#include <vector>
#include "helpapi.h"
#include "Global.h"

extern SStringT g_CurDir;

CSysDataMgr::CSysDataMgr()
{
}


CSysDataMgr::~CSysDataMgr()
{
}

bool CSysDataMgr::LoadSysData(LPCTSTR cfgDir)
{
	m_strConfigDir = cfgDir;
	InitProperty();

	return true;
}

void CSysDataMgr::InitProperty()   //初始化属性列表
{
	/*

	<通用样式>
	<id style="proptext" name ="窗口ID(id)" value="" />
	<name style="proptext" name ="窗口名称(name)" value="" />
	<skin style="proptext" name ="皮肤(skin)" value="" />
	</通用样式>

	<Button>
	<分组 name="基本">
	<id/>
	<name/>
	<skin/>
	<pos/>
	<size/>
	<offset/>
	</分组>

	<分组 name="拓展">
	<accel style="proptext" name ="快捷键(accel)" value="ctrl+alt+f9" />
	<animate style="propoption" name ="动画(animate)" value="0" options="无(0)|有(1)"/>
	</分组>

	</Button>
	*/


	pugi::xml_parse_result result = m_xmlDocProperty.load_file(g_CurDir + L"Config\\property.xml");
	if (!result)
	{
		CDebug::Debug(_T("InitProperty失败"));
	}

	pugi::xml_node NodeCom = m_xmlDocProperty.child(L"root").child(L"通用样式");
	pugi::xml_node NodeCtrlList = m_xmlDocProperty.child(L"root").child(L"属性列表");

	pugi::xml_node NodeCtrl = NodeCtrlList.first_child();  //NodeCtrl = Button节点
	while (NodeCtrl)
	{
		SStringT strCtrlname = NodeCtrl.name();
		m_mapControl.SetAt(strCtrlname, new CTRL_ATTR_VALUE());

		InitCtrlProperty(NodeCom, NodeCtrl, m_mapControl[strCtrlname]);

		NodeCtrl = NodeCtrl.next_sibling();
	}
}


void CSysDataMgr::InitCtrlProperty(pugi::xml_node NodeCom, pugi::xml_node NodeCtrl, CTRL_ATTR_VALUE* arr_attr)
{
	/*
	<通用样式>
	<id style="proptext" name ="窗口ID(id)" value="" />
	<name style="proptext" name ="窗口名称(name)" value="" />
	<skin style="proptext" name ="皮肤(skin)" value="" />
	</通用样式>

	<Button>
	<分组 name="基本">
	<id/>
	<name/>
	<skin/>
	<pos/>
	<size/>
	<offset/>
	</分组>

	<分组 name="拓展">
	<accel style="proptext" name ="快捷键(accel)" value="ctrl+alt+f9" />
	<animate style="propoption" name ="动画(animate)" value="0" options="无(0)|有(1)"/>
	</分组>

	</Button>

	<propgroup name="group1" description="desc of group1">
	<proptext name="text1.1" value="value 1.1">

	*/

	pugi::xml_node NodeChild = NodeCtrl.first_child();

	while (NodeChild)
	{
		SStringT nodeName = NodeChild.name();
		if (_wcsicmp(nodeName, L"分组") == 0)
		{
			NodeChild.set_name(L"propgroup");
			InitCtrlProperty(NodeCom, NodeChild, arr_attr);
		}
		else
		{
			if (!NodeChild.attribute(L"style"))
			{
				// 没有设置style的为通用属性, 从通用属性结点中获取信息
				SStringT strName = NodeChild.name();
				pugi::xml_node N = NodeCom.child(strName);
				pugi::xml_node NodeNew;

				// 用通用属性进行替换
				NodeNew = NodeChild.parent().insert_copy_before(N, NodeChild);
				NodeChild.parent().remove_child(NodeChild);

				NodeChild = NodeNew;
			}
			pugi::xml_node ctrl_node;
			ctrl_node.append_copy(NodeChild);
			arr_attr->Add(CtrlAttrItem(NodeChild.name(), ctrl_node));
		}

		NodeChild = NodeChild.next_sibling();
	}
}

SStringA CSysDataMgr::GetCtrlAutos()
{
	std::vector<SStringT> vecTemp;
	SPOSITION pos = m_mapControl.GetStartPosition();
	while (pos)
	{
		const SMap<SStringT, CTRL_ATTR_VALUE *>::CPair* item = m_mapControl.GetAt(pos);
		vecTemp.push_back(item->m_key);

		m_mapControl.GetNext(pos);
	}
	std::sort(vecTemp.begin(), vecTemp.end(), SortSStringNoCase);

	SStringT strAuto;
	std::vector<SStringT>::iterator it = vecTemp.begin();
	for (; it != vecTemp.end(); it++)
	{
		strAuto += *it + _T(" ");
	}
	strAuto.TrimRight(' ');

	SStringA str = S_CW2A(strAuto, CP_UTF8);
	return str;
}

SStringA CSysDataMgr::GetCtrlAttrAutos(SStringT ctrlname)
{
	ctrlname.MakeLower();
	if (ctrlname.CompareNoCase(_T("root")) == 0)
		ctrlname = _T("window");

	SMap<SStringT, CTRL_ATTR_VALUE*>::CPair* pNode = m_mapControl.Lookup(ctrlname);
	if (!pNode)
		return "";

	SStringT strAuto;
	CTRL_ATTR_VALUE* ctrl_attr = pNode->m_value;

	qsort(ctrl_attr->GetData(), ctrl_attr->GetCount(), sizeof(CtrlAttrItem), CtrlAttrCmpNoCase);
	for (int i = 0; i < ctrl_attr->GetCount(); i++)
	{
		if (ctrl_attr->GetAt(i).attrname.CompareNoCase(uiedit_SpecAttr) != 0)
			strAuto += ctrl_attr->GetAt(i).attrname + _T(" ");
	}

	strAuto.TrimRight(' ');
	SStringA str = S_CW2A(strAuto, CP_UTF8);
	return str;
}
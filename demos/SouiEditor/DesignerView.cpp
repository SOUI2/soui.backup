#include "stdafx.h"
#include "DesignerView.h"
#include "SMoveWnd.h"
#include "CNewGuid.h"
#include "helper\SplitString.h"
#include "Dialog/DlgSkinSelect.h"
#include "Dialog/DlgStyleManage.h"
#include "Dialog/DlgFontSelect.h"
#include "core/SWnd.h"
#include "MainDlg.h"
#include "adapter.h"
#include "Global.h"


#define  MARGIN 20

extern BOOL g_bHookCreateWnd;	//是否拦截窗口的建立

BOOL SDesignerView::NewLayout(SStringT strResName, SStringT strPath)
{
	SStringT strShortPath = strPath.Mid(m_strProPath.GetLength() + 1);

	pugi::xml_node xmlNode = m_xmlDocUiRes.child(_T("resource")).child(_T("LAYOUT"));

	if (xmlNode)
	{
		pugi::xml_node Child = xmlNode.append_child(_T("file"));
		Child.append_attribute(_T("name")).set_value(strResName);
		Child.append_attribute(_T("path")).set_value(strShortPath);

		m_xmlDocUiRes.save_file(m_strUIResFile);
	}

	return TRUE;
}


SDesignerView::SDesignerView(SHostDialog *pMainHost, SWindow *pContainer, STreeCtrl *pTreeXmlStruct)
{
	m_nSciCaretPos = 0;
	m_nState = 0;
	m_pMoveWndRoot = NULL;
	m_pRealWndRoot = NULL;
	m_pContainer = (SUIWindow*)pContainer;
	m_pMainHost = pMainHost;
	m_treeXmlStruct = pTreeXmlStruct;
	m_ndata = 0;
	g_nUIElmIndex = 0;
	m_treeXmlStruct->GetEventSet()->subscribeEvent(EVT_TC_SELCHANGED, Subscriber(&SDesignerView::OnTCSelChanged, this));

	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_file(g_CurDir + _T("Config\\Ctrl.xml"));
	if (!result)
	{
		Debug(_T("加载Ctrl.xml失败"));
		return;
	}

	pugi::xml_node node = doc.child(_T("root")).child(_T("容器控件")).first_child();
	while (node)
	{
		m_lstContainerCtrl.AddTail(node.name());
		node = node.next_sibling();
	}

	m_privateStylePool.Attach(new SStylePool);
	m_privateSkinPool.Attach(new SSkinPool);
}

BOOL SDesignerView::OpenProject(SStringT strFileName)
{
	m_xmlDocUiRes.load_file(strFileName, pugi::parse_full);

	m_strUIResFile = strFileName;
	TCHAR *s = strFileName.GetBuffer(strFileName.GetLength());

	PathRemoveFileSpec(s);
	SStringT strTemp(s);
	m_strProPath = strTemp;
	CAutoRefPtr<IResProvider>   pResProvider;

	CreateResProvider(RES_FILE, (IObjRef**)&pResProvider);
	if (!pResProvider->Init((LPARAM)s, 0))
	{
		Debug(_T("CreateResProvider失败"));
		return FALSE;
	}
	SApplication::getSingletonPtr()->AddResProvider(pResProvider, NULL);//param2 = null时不自动加载uidef

	SStringT strXMLInit;
	pugi::xml_node xmlNode = m_xmlDocUiRes.child(_T("resource")).child(_T("UIDEF")).child(_T("file"));

	if (xmlNode)
	{
		strXMLInit = xmlNode.attribute(_T("name")).as_string();
	}

	if (strXMLInit.IsEmpty())
	{
		strXMLInit = _T("xml_init");
	}

	//将皮肤中的uidef保存起来.
	m_pUiDef.Attach(SUiDef::getSingleton().CreateUiDefInfo(pResProvider, _T("uidef:") + strXMLInit));

	m_pOldUiDef = SUiDef::getSingleton().GetUiDef();

	return TRUE;
}

BOOL SDesignerView::CloseProject()
{
	m_strCurLayoutXmlFile.Empty();
	m_strCurFileEditor.Empty();
	m_strProPath = m_strUIResFile = L"";
	m_xmlDocUiRes.reset();
	UseEditorUIDef(false);


	// 清空私有样式池
	if (m_privateStylePool->GetCount())
	{
		m_privateStylePool->RemoveAll();
		GETSTYLEPOOLMGR->PopStylePool(m_privateStylePool);
	}
	// 清空私有皮肤池
	if (m_privateSkinPool->GetCount())
	{
		m_privateSkinPool->RemoveAll();
		GETSKINPOOLMGR->PopSkinPool(m_privateSkinPool);
	}
	m_pContainer->SSendMessage(WM_DESTROY);
	m_pContainer->Invalidate();
	m_pScintillaWnd->SendEditor(SCI_CLEARALL);
	m_pPropertyContainer->SSendMessage(WM_DESTROY);
	m_treeXmlStruct->RemoveAllItems();
	m_pRealWndRoot = NULL;
	m_pMoveWndRoot = NULL;
	m_nState = 0;
	m_ndata = 0;
	m_nSciCaretPos = 0;

	ShowNoteInSciwnd();

	return TRUE;
}

BOOL SDesignerView::InsertLayoutToMap(SStringT strFileName)
{
	SStringT FullFileName = m_strProPath + _T("\\") + strFileName;

	pugi::xml_document *xmlDoc1 = new pugi::xml_document();

	//if(!xmlDoc1->load_file(FullFileName,pugi::parse_default,pugi::encoding_utf8))
	if (!xmlDoc1->load_file(FullFileName, pugi::parse_full))
		return FALSE;

	//m_mapLayoutFile[strFileName] = xmlDoc1->first_child();
	m_mapLayoutFile[strFileName] = xmlDoc1->document_element();

	m_mapLayoutFile1[strFileName] = xmlDoc1;

	m_mapInclude1[strFileName] = new SMap<int, SStringT>;

	m_strCurLayoutXmlFile = strFileName;
	//RenameChildeWnd(xmlDoc1->first_child());
	RenameChildeWnd(xmlDoc1->root());
	m_strCurLayoutXmlFile.Empty();
	return TRUE;
}

BOOL SDesignerView::LoadLayout(SStringT strFileName)
{
	m_CurSelCtrl = NULL;
	m_ndata = 0;
	g_nUIElmIndex = 0;

	//设置uidef为当前皮肤的uidef
	UseEditorUIDef(false);

	m_defFont = SFontPool::getSingleton().GetFont(FF_DEFAULTFONT, 100);

	pugi::xml_node xmlroot;
	pugi::xml_node xmlnode;

	BOOL bIsInclude = FALSE;
	if (m_strCurLayoutXmlFile != strFileName)
		m_nSciCaretPos = 0;

	m_strCurLayoutXmlFile = strFileName;

	SMap<SStringT, pugi::xml_node>::CPair *p = m_mapLayoutFile.Lookup(strFileName);
	xmlroot = p->m_value;

	m_CurrentLayoutNode = xmlroot;

	if (xmlroot == NULL)
	{
		return TRUE;
	}

	if (S_CW2T(xmlroot.name()) != _T("SOUI"))
	{
		//include文件
		xmlnode = xmlroot;
		bIsInclude = TRUE;
	}
	else
	{
		//加载私有皮肤
		if (m_privateStylePool->GetCount())
		{
			m_privateStylePool->RemoveAll();
			GETSTYLEPOOLMGR->PopStylePool(m_privateStylePool);
		}
		BOOL ret = m_privateStylePool->Init(xmlroot.child(L"style"));
		if (ret)
		{
			GETSTYLEPOOLMGR->PushStylePool(m_privateStylePool);
		}
		if (m_privateSkinPool->GetCount())
		{
			m_privateSkinPool->RemoveAll();
			GETSKINPOOLMGR->PopSkinPool(m_privateSkinPool);
		}
		int skincount = m_privateSkinPool->LoadSkins(xmlroot.child(L"skin"));//从xmlNode加加载私有skin
		if (skincount)
		{
			GETSKINPOOLMGR->PushSkinPool(m_privateSkinPool);
		}

		xmlnode = xmlroot.child(L"root", false);
	}
	if (!xmlnode) return FALSE;

	m_pContainer->SSendMessage(WM_DESTROY);

	SStringW s1, s2;
	if (!bIsInclude)
	{
		int nWidth, nHeight;
		SStringT strSize(_T("size"));
		SStringT strWidth(_T("width"));
		SStringT strHeight(_T("height"));
		SStringT strMargin(_T("margin"));
		BOOL bHasSize = FALSE;

		pugi::xml_attribute attr = m_CurrentLayoutNode.first_attribute();
		while (attr)
		{
			// width height单独处理，解决margin的问题
			if (strSize.CompareNoCase(attr.name()) == 0)
			{
				//size属性
				SStringT strVal = attr.value();
				swscanf(strVal, L"%d,%d", &nWidth, &nHeight);

				bHasSize = TRUE;
			}
			else if (strWidth.CompareNoCase(attr.name()) == 0)
			{
				//width属性
				::StrToIntExW(attr.value(), STIF_SUPPORT_HEX, &nWidth);
			}
			else if (strHeight.CompareNoCase(attr.name()) == 0)
			{
				//height属性
				::StrToIntExW(attr.value(), STIF_SUPPORT_HEX, &nHeight);
			}
			else if (strMargin.CompareNoCase(attr.name()) == 0)
			{
				//忽略margin属性
			}
			else
			{
				s1.Format(L" %s=\"%s\" ", attr.name(), attr.value());
				s2 = s2 + s1;
			}
			attr = attr.next_attribute();
		}

		attr = xmlnode.first_attribute();
		while (attr)
		{
			s1.Format(L" %s=\"%s\" ", attr.name(), attr.value());
			s2 = s2 + s1;

			attr = attr.next_attribute();
		}

		SStringW strAttrSize;
		strAttrSize.Format(L" margin= \"%d\" width = \"%d\" height = \"%d\" ", MARGIN, nWidth + MARGIN * 2, nHeight + MARGIN * 2);

		s2 = L"<designerRoot pos=\"20, 20\" " + s2 + strAttrSize + L"></designerRoot>";

		//删除size 改成width height
		if (bHasSize)
		{
			m_CurrentLayoutNode.remove_attribute(_T("size"));

			pugi::xml_attribute attrWorH = m_CurrentLayoutNode.attribute(_T("width"));
			if (attrWorH)
			{
				attrWorH.set_value(nWidth);
			}

			attrWorH = m_CurrentLayoutNode.attribute(_T("height"));
			if (attrWorH)
			{
				attrWorH.set_value(nHeight);
			}
		}
	}
	else
	{
		//include文件
		int nWidth, nHeight;
		pugi::xml_attribute attrWorH = m_CurrentLayoutNode.attribute(_T("width"));

		if (attrWorH)
		{
			::StrToIntExW(attrWorH.value(), STIF_SUPPORT_HEX, &nWidth);
		}
		else
		{
			nWidth = 500;
			m_CurrentLayoutNode.append_attribute(_T("width")).set_value(nWidth);
		}

		attrWorH = m_CurrentLayoutNode.attribute(_T("height"));

		if (attrWorH)
		{
			::StrToIntExW(attrWorH.value(), STIF_SUPPORT_HEX, &nHeight);
		}
		else
		{
			nHeight = 500;
			m_CurrentLayoutNode.append_attribute(_T("height")).set_value(nHeight);
		}

		SStringW strAttrSize;
		strAttrSize.Format(L" margin= \"%d\" width = \"%d\" height = \"%d\" ", MARGIN, nWidth + MARGIN * 2, nHeight + MARGIN * 2);

		s2 = L"<designerRoot pos=\"20,20\" " + strAttrSize + L" colorBkgnd=\"#d0d0d0\"/>";
	}

	//wchar_t *s = L"<window pos=\"20,20,@500,@500\" colorBkgnd=\"#d0d0d0\"></window>";
	const wchar_t *s3 = L"<movewnd pos=\"20,20,@800,@500\" ></movewnd>";

	g_bHookCreateWnd = TRUE;

	////创建布局窗口的根窗口
	m_pRealWndRoot = (SDesignerRoot*)m_pContainer->CreateChildren(s2);
	m_pRealWndRoot->SetRootFont(m_defFont);

	m_pMoveWndRoot = (SMoveWnd *)m_pContainer->CreateChildren(s3);
	m_pMoveWndRoot->m_pRealWnd = m_pRealWndRoot;
	m_pMoveWndRoot->m_Desiner = this;

	int a = m_pMoveWndRoot->GetUserData();
	m_mapMoveRealWnd.RemoveAll();
	m_mapMoveRealWnd[m_pMoveWndRoot->m_pRealWnd] = m_pMoveWndRoot;

	if (!m_pRealWndRoot->CreateChildren(xmlnode))
	{
		g_bHookCreateWnd = FALSE;
		return FALSE;
	}

	CreateAllChildWnd(m_pRealWndRoot, m_pMoveWndRoot);
	g_bHookCreateWnd = FALSE;

	m_nState = 0;
	GetMoveWndRoot()->Click(0, CPoint(0, 0));

	m_treeXmlStruct->RemoveAllItems();
	InitXMLStruct(m_CurrentLayoutNode, STVI_ROOT);

	//恢复uidef为编辑器的uidef
	UseEditorUIDef(true);
	return TRUE;
}

void SDesignerView::CreateAllChildWnd(SUIWindow *pRealWnd, SMoveWnd *pMoveWnd)
{
	//view系列加上适配器
	if (pRealWnd->IsClass(SMCListView::GetClassNameW()))
	{
		CBaseMcAdapterFix *mcAdapter = new CBaseMcAdapterFix();
		((SMCListView*)pRealWnd)->SetAdapter(mcAdapter);
		mcAdapter->Release();
	}
	//listview(flex)需要重新处理，有空再来
	if (pRealWnd->IsClass(SListView::GetClassNameW()))
	{
		CBaseAdapterFix *listAdapter = new CBaseAdapterFix();
		((SListView*)pRealWnd)->SetAdapter(listAdapter);
		listAdapter->Release();
	}
	if (pRealWnd->IsClass(STileView::GetClassNameW()))
	{
		CBaseAdapterFix *listAdapter = new CBaseAdapterFix();
		((STileView*)pRealWnd)->SetAdapter(listAdapter);
		listAdapter->Release();
	}
	////得到第一个子窗口
	SUIWindow *pSibReal = (SUIWindow*)pRealWnd->GetWindow(GSW_FIRSTCHILD);
	for (; pSibReal; pSibReal = (SUIWindow*)pSibReal->GetWindow(GSW_NEXTSIBLING))
	{
		const wchar_t *s1 = L"<movewnd pos=\"0,0,@100,@100\" ></movewnd>";
		//创建布局窗口的根窗口
		SMoveWnd *pSibMove = (SMoveWnd *)pMoveWnd->CreateChildren(s1);
		pSibMove->m_pRealWnd = pSibReal;
		pSibMove->SetVisible(pSibReal->IsVisible());
		m_mapMoveRealWnd[pSibReal] = pSibMove;
		pSibMove->m_Desiner = this;

		CreateAllChildWnd(pSibReal, pSibMove);
	}
}

SDesignerView::~SDesignerView()
{
	m_pMoveWndRoot->m_pRealWnd->DestroyWindow();
	m_pMoveWndRoot->DestroyWindow();
	GETSTYLEPOOLMGR->PopStylePool(m_privateStylePool);
	GETSKINPOOLMGR->PopSkinPool(m_privateSkinPool);
}

BOOL SDesignerView::SaveAll()
{
	SMap<SStringT, pugi::xml_document*>::CPair *p;
	SStringT strFileName;
	SStringT FullFileName;
	pugi::xml_document *doc;
	pugi::xml_document DocSave;
	bool bRet = true;

	SPOSITION pos = m_mapLayoutFile1.GetStartPosition();

	while (pos)
	{
		p = m_mapLayoutFile1.GetNext(pos);
		strFileName = p->m_key;
		doc = p->m_value;

		pugi::xml_writer_buff writer;
		doc->print(writer, L"\t", pugi::format_default, pugi::encoding_utf16);
		SStringW *strxmlWnd = new SStringW(writer.buffer(), writer.size());

		if (DocSave.load_buffer(*strxmlWnd, wcslen(*strxmlWnd) * sizeof(wchar_t), pugi::parse_full, pugi::encoding_utf16))
		{
			pugi::xml_node NodeSave = DocSave.root();
			TrimXmlNodeTextBlank(DocSave.document_element());
			RemoveWndName(NodeSave, FALSE, strFileName);

			FullFileName = m_strProPath + _T("\\") + strFileName;
			if (!DocSave.save_file(FullFileName))
			{
				Debug(_T("保存文件失败：") + FullFileName);
				bRet = false;
			}
		}
		else
		{
			Debug(_T("保存失败：DocSave.load_buffer"));
			bRet = false;
		}

		delete strxmlWnd;
	}

	if (!m_xmlDocUiRes.save_file(m_strUIResFile))
	{
		Debug(_T("保存文件失败：") + m_strUIResFile);
		bRet = false;
	}

	if (bRet)
	{
		Debug(_T("保存成功"));
	}
	else
	{
		Debug(_T("保存失败"));
	}

	return TRUE;
}

//保存当前打开的布局文件
bool SDesignerView::SaveLayoutFile()
{
	if (m_strCurLayoutXmlFile.IsEmpty())
	{
		return false;
	}
	bool bRet = false;
	SStringT strFile = m_strCurLayoutXmlFile;
	SStringT strFileName;
	SStringT FullFileName;

	SMap<SStringT, pugi::xml_document*>::CPair *p = m_mapLayoutFile1.Lookup(strFile);
	strFileName = p->m_key;
	pugi::xml_document *doc = p->m_value;

	pugi::xml_writer_buff writer;
	doc->print(writer, L"\t", pugi::format_default, pugi::encoding_utf16);
	SStringW *strxmlWnd = new SStringW(writer.buffer(), writer.size());
	pugi::xml_document DocSave;
	if (DocSave.load_buffer(*strxmlWnd, wcslen(*strxmlWnd) * sizeof(wchar_t), pugi::parse_full, pugi::encoding_utf16))
	{
		pugi::xml_node NodeSave = DocSave.root();
		TrimXmlNodeTextBlank(DocSave.document_element());
		RemoveWndName(NodeSave, FALSE);
		FullFileName = m_strProPath + _T("\\") + strFileName;
		bRet = DocSave.save_file(FullFileName);
		if (!bRet)
		{
			Debug(_T("保存文件失败：") + FullFileName);
		}
	}
	else
	{
		Debug(_T("保存失败:DocSave.load_buffer失败"));
	}
	delete strxmlWnd;
	if (bRet)
	{
		bRet = m_xmlDocUiRes.save_file(m_strUIResFile);
		if (!bRet)
		{
			Debug(_T("保存失败:") + m_strUIResFile);
		}
	}
	return bRet;
}

//关闭当前打开的布局文件
BOOL SDesignerView::CloseLayoutFile()
{
	return TRUE;
}

//创建窗口
SMoveWnd* SDesignerView::CreateWnd(SUIWindow *pContainer, LPCWSTR pszXml)
{
	SWindow *pChild = pContainer->CreateChildren(pszXml);
	((SMoveWnd*)pChild)->m_Desiner = this;
	m_CurSelCtrl = (SMoveWnd*)pChild;
	return (SMoveWnd*)pChild;
}

void SDesignerView::RenameWnd(pugi::xml_node xmlNode, BOOL force)
{
	if (xmlNode.type() != pugi::node_element)
	{
		return;
	}

	pugi::xml_attribute xmlAttr = xmlNode.attribute(L"data", false);
	pugi::xml_attribute xmlAttr1 = xmlNode.attribute(L"uidesiner_data", false);

	SStringT strName = _T("item"); //不处理item节点
	if (strName.CompareNoCase(xmlNode.name()) == 0)
	{
		return;
	}

	//if (force)
	//{
	//	if (!xmlAttr)
	//	{

	//		xmlNode.append_attribute(L"data").set_value(GetIndexData());
	//	}else
	//	{
	//		xmlAttr.set_value(GetIndexData());
	//	}

	//	if (!xmlAttr1)
	//	{
	//		xmlNode.append_attribute(L"uidesiner_data").set_value(GetIndexData());
	//	}else
	//	{
	//		xmlAttr1.set_value(GetIndexData());
	//	}
	//}
	//else
	//{
	if (!xmlAttr)
	{
		xmlNode.append_attribute(L"data").set_value(GetIndexData());
		xmlNode.append_attribute(L"uidesiner_data").set_value(_T(""));
	}
	else
	{
		int data = xmlAttr.as_int();

		if (!xmlAttr1)
		{
			xmlNode.append_attribute(L"uidesiner_data").set_value(data);
		}
		else
		{
			xmlAttr1.set_value(data);
		}

		xmlAttr.set_value(GetIndexData());
	}
	//}
}

// 还原替换的include为原始内容
void SDesignerView::RemoveWndName(pugi::xml_node xmlNode, BOOL bClear, SStringT strFileName)
{
	pugi::xml_node NodeChild = xmlNode.first_child();

	pugi::xml_attribute attr, attr1;
	pugi::xml_document doc;

	while (NodeChild)
	{
		if (NodeChild.type() != pugi::node_element)
		{
			NodeChild = NodeChild.next_sibling();
			continue;
		}

		attr = NodeChild.attribute(L"uidesiner_data", false);
		attr1 = NodeChild.attribute(L"data", false);

		if (strFileName.IsEmpty())
		{
			strFileName = m_strCurLayoutXmlFile;
		}

		SMap<SStringT, SMap<int, SStringT>* >::CPair *p = m_mapInclude1.Lookup(strFileName);
		SMap<int, SStringT>* pMap;
		SMap<int, SStringT>::CPair *p1;
		if (p)
		{
			pMap = p->m_value;
			p1 = pMap->Lookup(attr1.as_int());
		}
		else
		{
			Debug(_T("替换include出错"));
		}

		if (p1)
		{	// 如果这个控件是include
			if (!doc.load_buffer(p1->m_value, wcslen(p1->m_value) * sizeof(wchar_t), pugi::parse_default, pugi::encoding_utf16))
			{
				Debug(_T("RemoveWndName出错了"));
			}
			else
			{
				pugi::xml_node nodeNew;
				nodeNew = NodeChild.parent().insert_copy_after(doc.first_child(), NodeChild);
				NodeChild.parent().remove_child(NodeChild);
				NodeChild = nodeNew.next_sibling();
				if (bClear)
				{
					pMap->RemoveKey(attr1.as_int());
				}
			}
		}
		else
		{
			if (attr && _wcsicmp(NodeChild.name(), L"item") != 0)
			{
				SStringT str = attr.value();
				if (str.IsEmpty())
				{
					NodeChild.remove_attribute(L"uidesiner_data");
					NodeChild.remove_attribute(L"data");
				}
				else
				{
					attr1.set_value(str);
					NodeChild.remove_attribute(L"uidesiner_data");
				}
			}

			if (_wcsicmp(NodeChild.name(), L"item") != 0)
			{
				RemoveWndName(NodeChild, bClear, strFileName);
			}

			NodeChild = NodeChild.next_sibling();
		}
	}

}

void SDesignerView::RenameChildeWnd(pugi::xml_node xmlNode)
{
	pugi::xml_node NodeChild = xmlNode.first_child();

	while (NodeChild)
	{
		if (NodeChild.type() != pugi::node_element)
		{
			NodeChild = NodeChild.next_sibling();
			continue;;
		}

		//替换Include 成一个window
		if (_wcsicmp(NodeChild.name(), L"include") == 0 && NodeChild.attribute(L"src"))
		{
			SStringT strInclude = NodeToStr(NodeChild);
			NodeChild.set_name(L"window");
			NodeChild.remove_attribute(L"src");
			NodeChild.append_attribute(L"pos").set_value(L"10,10,-10,-10");
			NodeChild.append_attribute(L"colorBkgnd").set_value(L"RGB(191,141,255)");
			NodeChild.text().set(strInclude);

			RenameWnd(NodeChild);

			SMap<SStringT, SMap<int, SStringT>* >::CPair *p = m_mapInclude1.Lookup(m_strCurLayoutXmlFile);
			if (p)
			{
				SMap<int, SStringT>* pMap = p->m_value;
				(*pMap)[NodeChild.attribute(L"data").as_int()] = strInclude;
			}
			else
			{
				Debug(_T("替换include出错"));
			}

			NodeChild = NodeChild.next_sibling();

			continue;
		}


		////判断NodeChild.name()类型的控件是否注册
  //      if (!static_cast<SWindowFactoryMgr*>(SApplication::getSingletonPtr())->HasKey(NodeChild.name()))
		//{
		//	RenameChildeWnd(NodeChild);

		//	NodeChild = NodeChild.next_sibling();
		//	continue;
		//}

		//将<button data = "1"/> 修改为
		//  <button data = "xxx" uidesiner_data = "1"/> 
		RenameWnd(NodeChild);
		RenameChildeWnd(NodeChild);

		NodeChild = NodeChild.next_sibling();
	}
}

void SDesignerView::RenameAllLayoutWnd()
{
	SPOSITION pos = m_mapLayoutFile.GetStartPosition();
	while (pos)
	{
		SMap<SStringT, pugi::xml_node>::CPair *p = m_mapLayoutFile.GetNext(pos);
		RenameChildeWnd(p->m_value);
	}
}

pugi::xml_node SDesignerView::FindNodeByAttr(pugi::xml_node NodeRoot, SStringT attrName, SStringT attrValue)
{
	pugi::xml_node NodeChild = NodeRoot.first_child();

	pugi::xml_attribute attr;
	pugi::xml_node NodeResult;

	while (NodeChild)
	{
		if (NodeChild.type() != pugi::node_element)
		{
			NodeChild = NodeChild.next_sibling();
			continue;
		}

		if (_wcsicmp(NodeChild.name(), _T("item")) == 0)
		{
			NodeChild = NodeChild.next_sibling();
			continue;
		}

		attr = NodeChild.attribute(attrName, false);
		if (attr)
		{
			if (0 == attrValue.CompareNoCase(attr.value()))
			{
				return NodeChild;
			}
		}

		NodeResult = FindNodeByAttr(NodeChild, attrName, attrValue);
		if (NodeResult)
		{
			return NodeResult;
		}

		NodeChild = NodeChild.next_sibling();
	}

	return NodeResult;
}


void SDesignerView::Debug(pugi::xml_node xmlNode)
{
	pugi::xml_writer_buff writer;
	xmlNode.print(writer, L"\t", pugi::format_default, pugi::encoding_utf16);
	SStringW *strDebug = new SStringW(writer.buffer(), writer.size());
	SMessageBox(NULL, *strDebug, _T(""), MB_OK);
	delete strDebug;
}

void SDesignerView::Debug(SStringT str)
{
	SMessageBox(NULL, str, _T(""), MB_OK);
}

SStringT SDesignerView::Debug1(pugi::xml_node xmlNode)
{
	pugi::xml_writer_buff writer;
	xmlNode.print(writer, L"\t", pugi::format_default, pugi::encoding_utf16);
	SStringW *strDebug = new SStringW(writer.buffer(), writer.size());
	SStringT strtemp = *strDebug;
	delete strDebug;
	return strtemp;
}

SStringT SDesignerView::NodeToStr(pugi::xml_node xmlNode)
{
	pugi::xml_writer_buff writer;
	xmlNode.print(writer, L"\t", pugi::format_default, pugi::encoding_utf16);
	SStringW *strDebug = new SStringW(writer.buffer(), writer.size());
	SStringT str(*strDebug);
	delete strDebug;
	return str;
}

// 响应各事件, 选中相应元素
void SDesignerView::SetCurrentCtrl(pugi::xml_node xmlNode, SMoveWnd *pWnd)
{
	m_xmlNode = xmlNode;

	m_CurSelCtrl = pWnd;

	m_pContainer->Invalidate();

	m_treeXmlStruct->GetEventSet()->unsubscribeEvent(EVT_TC_SELCHANGED, Subscriber(&SDesignerView::OnTCSelChanged, this));
	GoToXmlStructItem(m_CurSelCtrl->m_pRealWnd->GetUserData(), m_treeXmlStruct->GetRootItem());
	m_treeXmlStruct->GetEventSet()->subscribeEvent(EVT_TC_SELCHANGED, Subscriber(&SDesignerView::OnTCSelChanged, this));
}


void SDesignerView::UpdatePosToXmlNode(SUIWindow *pRealWnd, SMoveWnd* pMoveWnd)
{
	if (m_CurSelCtrl == m_pMoveWndRoot)
	{
		//SwndLayout *pLayout = pRealWnd->GetLayout();
		SouiLayoutParam *pSouiLayoutParam = pRealWnd->GetLayoutParamT<SouiLayoutParam>();

		CRect r;
		pMoveWnd->GetWindowRect(r);
		m_CurrentLayoutNode.attribute(_T("height")).set_value((int)pSouiLayoutParam->GetSpecifiedSize(Vert).fSize - MARGIN * 2);
		m_CurrentLayoutNode.attribute(_T("width")).set_value((int)pSouiLayoutParam->GetSpecifiedSize(Horz).fSize - MARGIN * 2);

		return;
	}

	SStringT s;
	s.Format(_T("%d"), pRealWnd->GetUserData());
	pugi::xml_node xmlNode = FindNodeByAttr(m_CurrentLayoutNode, L"data", s);
	if (!xmlNode)
	{
		return;
	}

	pugi::xml_attribute attrPos, attrSize, attrOffset, attrPos2type, attrWidth, attrHeight;

	SStringW strTemp;
	attrPos = xmlNode.attribute(L"pos");
	attrSize = xmlNode.attribute(L"size");
	attrOffset = xmlNode.attribute(L"offset");
	attrPos2type = xmlNode.attribute(L"pos2type");
	attrWidth = xmlNode.attribute(L"width");
	attrHeight = xmlNode.attribute(L"height");

	if (pRealWnd->GetLayoutParam()->IsClass(SouiLayoutParam::GetClassName()))
	{
		SouiLayoutParam *pSouiLayoutParam = pRealWnd->GetLayoutParamT<SouiLayoutParam>();
		SouiLayoutParamStruct *pSouiLayoutParamStruct = (SouiLayoutParamStruct*)pSouiLayoutParam->GetRawData();

		if (attrSize)
		{
			strTemp.Format(_T("%d%s, %d%s"),
				(int)pSouiLayoutParam->GetSpecifiedSize(Horz).fSize,
				UnitToStr(pSouiLayoutParam->GetSpecifiedSize(Horz).unit),
				(int)pSouiLayoutParam->GetSpecifiedSize(Vert).fSize,
				UnitToStr(pSouiLayoutParam->GetSpecifiedSize(Vert).unit));

			attrSize.set_value(strTemp);
		}

		if (attrPos2type)
		{
			//删除Pos2Type,改成attrOffset
			xmlNode.remove_attribute(L"pos2type");
			if (!attrOffset)
			{
				xmlNode.append_attribute(L"offset");
				attrOffset = xmlNode.attribute(L"offset");
			}
		}

		if (attrOffset)
		{
			strTemp.Format(_T("%g, %g"), pSouiLayoutParamStruct->fOffsetX, pSouiLayoutParamStruct->fOffsetY);
			attrOffset.set_value(strTemp);
		}

		if (attrPos)
		{
			if (pSouiLayoutParamStruct->nCount == 2)
			{
				strTemp = GetPosFromLayout(pSouiLayoutParam, 0) + _T(",");
				strTemp = strTemp + GetPosFromLayout(pSouiLayoutParam, 1);
				attrPos.set_value(strTemp);
			}
			else if (pSouiLayoutParamStruct->nCount == 4)
			{
				strTemp = GetPosFromLayout(pSouiLayoutParam, 0) + _T(",");
				strTemp = strTemp + GetPosFromLayout(pSouiLayoutParam, 1) + _T(",");
				strTemp = strTemp + GetPosFromLayout(pSouiLayoutParam, 2) + _T(",");
				strTemp = strTemp + GetPosFromLayout(pSouiLayoutParam, 3);
				attrPos.set_value(strTemp);
			}
		}

	}
	else
	{
		SLinearLayoutParam *pSLinearLayoutParam = pRealWnd->GetLayoutParamT<SLinearLayoutParam>();
		if (attrSize)
		{
			strTemp.Format(_T("%d%s, %d%s"),
				(int)pSLinearLayoutParam->GetSpecifiedSize(Horz).fSize,
				UnitToStr(pSLinearLayoutParam->GetSpecifiedSize(Horz).unit),
				(int)pSLinearLayoutParam->GetSpecifiedSize(Vert).fSize,
				UnitToStr(pSLinearLayoutParam->GetSpecifiedSize(Vert).unit));
			attrSize.set_value(strTemp);
		}

		if (attrWidth)
		{
			strTemp.Format(_T("%d%s"),
				(int)pSLinearLayoutParam->GetSpecifiedSize(Horz).fSize,
				UnitToStr(pSLinearLayoutParam->GetSpecifiedSize(Horz).unit));
			attrWidth.set_value(strTemp);
		}
		if (attrHeight)
		{
			strTemp.Format(_T("%d"),
				(int)pSLinearLayoutParam->GetSpecifiedSize(Vert).fSize,
				UnitToStr(pSLinearLayoutParam->GetSpecifiedSize(Vert).unit));
			attrHeight.set_value(strTemp);
		}
	}

	SetCurrentCtrl(xmlNode, pMoveWnd);
}

SStringW SDesignerView::GetPosFromLayout(SouiLayoutParam *pLayoutParam, INT nPosIndex)
{
	SouiLayoutParamStruct *pSouiLayoutParamStruct = (SouiLayoutParamStruct*)pLayoutParam->GetRawData();

	POS_INFO PI;

	switch (nPosIndex)
	{
	case 0:
		PI = pSouiLayoutParamStruct->posLeft;
		break;
	case 1:
		PI = pSouiLayoutParamStruct->posTop;
		break;
	case 2:
		PI = pSouiLayoutParamStruct->posRight;
		break;
	case 3:
	default:
		PI = pSouiLayoutParamStruct->posBottom;
	}

	SStringW strPos;
	switch (PI.pit)
	{
	case PIT_NULL:
		strPos = L"";        //无效定义
		break;
	case PIT_NORMAL:        //锚点坐标
		strPos = L"";
		break;
	case PIT_CENTER:        //参考父窗口中心点,以"|"开始
		strPos = L"|";
		break;
	case PIT_PERCENT:       //指定在父窗口坐标的中的百分比,以"%"开始
		strPos = L"%";
		break;
	case PIT_PREV_NEAR:     //参考前一个兄弟窗口与自己近的边,以"["开始
		strPos = L"[";
		break;
	case PIT_NEXT_NEAR:     //参考下一个兄弟窗口与自己近的边,以"]"开始
		strPos = L"]";
		break;
	case PIT_PREV_FAR:     //参考前一个兄弟窗口与自己远的边,以"{"开始
		strPos = L"{";
		break;
	case PIT_NEXT_FAR:      //参考下一个兄弟窗口与自己远的边,以"}"开始
		strPos = L"}";
		break;
	case PIT_SIZE:          //指定窗口的宽或者高,以"@"开始
		strPos = L"@";
		break;
	case PIT_SIB_LEFT:      //兄弟结点的left,用于X
		if (0 == nPosIndex)
		{
			strPos = strPos.Format(L"sib.left@%d:", PI.nRefID);
		}
		else
		{
			strPos = strPos.Format(L"sib.top@%d:", PI.nRefID);
		}
		break;

		//case PIT_SIB_TOP:      //兄弟结点的top，与left相同，用于Y
		//	break;

	case PIT_SIB_RIGHT:      //兄弟结点的right,用于X 
		if (2 == nPosIndex)
		{
			strPos = strPos.Format(L"sib.right@%d:", PI.nRefID);
		}
		else
		{
			strPos = strPos.Format(L"sib.bottom@%d:", PI.nRefID);
		}
		break;

		//case PIT_SIB_BOTTOM:      //兄弟结点的bottom,与right相同,用于Y 
		//	break;

	default:
		break;
	}

	if (PI.cMinus == -1)
	{
		strPos = strPos + L"-";
	}
	SStringW strTemp;
	int n = (int)PI.nPos.fSize;
	strTemp.Format(L"%d%s", n, UnitToStr(PI.nPos.unit));
	strPos = strPos + strTemp;
	return strPos;
}

void SDesignerView::BindXmlcodeWnd(SWindow *pXmlTextCtrl)
{
	m_pScintillaWnd = (CScintillaWnd*)pXmlTextCtrl->GetUserData();
	if (m_pScintillaWnd)
	{
		m_pScintillaWnd->SetSaveCallback((SCIWND_FN_CALLBACK)&CMainDlg::OnScintillaSave);

		ShowNoteInSciwnd();
	}
}

void SDesignerView::ShowNoteInSciwnd()
{
	SStringW strDebug = L"\r\n\r\n\r\n\t\t\t修改代码后按Ctrl+S可在窗体看到变化";

	SStringA str = S_CW2A(strDebug, CP_UTF8);
	m_pScintillaWnd->SendMessage(SCI_ADDTEXT, str.GetLength(),
		reinterpret_cast<LPARAM>((LPCSTR)str));
	m_pScintillaWnd->SetDirty(false);
	m_pScintillaWnd->SendMessage(SCI_SETREADONLY, 1, 0);
}


void SDesignerView::InitProperty(SWindow *pPropertyContainer)   //初始化属性列表
{
	m_pPropertyContainer = pPropertyContainer;
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

	SStringW s = L"<propgrid name=\"NAME_UIDESIGNER_PROPGRID_MAIN\" pos=\"0, 0, -4, -4\" switchSkin=\"skin_prop_switch\"                \
		nameWidth=\"150\" orderType=\"group\" itemHeight=\"26\" ColorGroup=\"RGB(96,112,138)\"  ColorBorder=\"#FFFFFF50\"                                        \
		ColorItemSel=\"rgb(234,128,16)\" colorItemSelText=\"#FF0000\" EditBkgndColor=\"rgb(87,104,132)\"                                \
		autoWordSel=\"1\"> <cmdbtnstyle skin=\"_skin.sys.btn.normal\" colorText=\"RGB(96,112,138)\">...</cmdbtnstyle> </propgrid>";


	pugi::xml_document m_xmlDocProperty;

	pugi::xml_parse_result result = m_xmlDocProperty.load_file(g_CurDir + L"Config\\property.xml");
	if (!result)
	{
		Debug(_T("InitProperty失败"));
	}

	pugi::xml_node NodeCom = m_xmlDocProperty.child(L"root").child(L"通用样式");
	pugi::xml_node NodeCtrlList = m_xmlDocProperty.child(L"root").child(L"属性列表");

	//hostwnd节点处理
	pugi::xml_node NodeCtrl = NodeCtrlList.child(_T("hostwnd")).first_child();
	m_lstSouiProperty.RemoveAll();
	pugi::xml_node NodeCtrlChild = NodeCtrl.first_child();
	while (NodeCtrlChild)
	{
		m_lstSouiProperty.AddTail(NodeCtrlChild.name());
		NodeCtrlChild = NodeCtrlChild.next_sibling();
	}

	NodeCtrl = NodeCtrl.next_sibling();
	m_lstRootProperty.RemoveAll();
	NodeCtrlChild = NodeCtrl.first_child();
	while (NodeCtrlChild)
	{
		m_lstRootProperty.AddTail(NodeCtrlChild.name());
		NodeCtrlChild = NodeCtrlChild.next_sibling();
	}

	NodeCtrl = NodeCtrlList.first_child();  //NodeCtrl = Button节点
	while (NodeCtrl)
	{
		InitCtrlProperty(NodeCom, NodeCtrl);

		SStringT strName = NodeCtrl.name();
		NodeCtrl.set_name(L"groups");

		pugi::xml_document *doc = new pugi::xml_document();

		if (!doc->load_buffer(s, wcslen(s) * sizeof(wchar_t), pugi::parse_default, pugi::encoding_utf16))
		{
			Debug(_T("InitProperty失败1"));
		}

		doc->child(L"propgrid").append_copy(NodeCtrl);

		m_mapCtrlProperty[strName.MakeLower()] = doc;

		NodeCtrl = NodeCtrl.next_sibling();
	}
}


void SDesignerView::InitCtrlProperty(pugi::xml_node NodeCom, pugi::xml_node NodeCtrl)
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
		if (_wcsicmp(NodeChild.name(), L"分组") == 0)
		{
			NodeChild.set_name(L"propgroup");
			InitCtrlProperty(NodeCom, NodeChild);
		}
		else
		{
			if (!NodeChild.attribute(L"style"))
			{
				SStringT strName = NodeChild.name();
				pugi::xml_node N = NodeCom.child(strName);
				pugi::xml_node NodeNew;

				NodeNew = NodeChild.parent().insert_copy_before(N, NodeChild);
				NodeChild.parent().remove_child(NodeChild);

				NodeChild = NodeNew;
			}
			NodeChild.append_attribute(L"name2").set_value(NodeChild.name());

			NodeChild.set_name(NodeChild.attribute(L"style").value());
			NodeChild.remove_attribute(L"style");
		}

		NodeChild = NodeChild.next_sibling();
	}

}

void SDesignerView::CreatePropGrid(SStringT strCtrlType)
{
	//if (m_strCurrentCtrlType.CompareNoCase(strCtrlType) != 0)
	//{
	m_pPropgrid = (SPropertyGrid *)m_pPropertyContainer->GetWindow(GSW_FIRSTCHILD);
	if (m_pPropgrid)
	{
		//这是一个坑啊，要先这样才不报错，因为点按钮的时候，PropGrid并没有失去焦点，
		//没有执行Killfocus操作销毁Edit,在DestroyChild以后PropGrid已经销毁了，这时候在执行PropGrid中edit的killfocus会报错		
		m_pPropgrid->GetEventSet()->unsubscribeEvent(EventPropGridValueChanged::EventID, Subscriber(&SDesignerView::OnPropGridValueChanged, this));
		m_pPropgrid->SetFocus();

		m_pPropertyContainer->DestroyChild(m_pPropgrid);
		m_pPropgrid = NULL;
	}

	SStringT strTemp;
	strTemp = m_CurrentLayoutNode.name();

	if (strCtrlType.CompareNoCase(_T("hostwnd")) == 0 && strTemp.CompareNoCase(_T("SOUI")) != 0)
	{   //include 文件

		strCtrlType = _T("include");
		//return;
	}

	SMap<SStringT, pugi::xml_document*>::CPair *p = m_mapCtrlProperty.Lookup(strCtrlType.MakeLower());
	if (p)
	{
		pugi::xml_document *tempDoc = p->m_value;

		m_pPropertyContainer->CreateChildren(tempDoc->root());

		m_pPropgrid = (SPropertyGrid *)m_pPropertyContainer->GetWindow(GSW_FIRSTCHILD);
		m_pPropertyContainer->Invalidate();

		m_pPropgrid->GetEventSet()->subscribeEvent(EventPropGridValueChanged::EventID, Subscriber(&SDesignerView::OnPropGridValueChanged, this));
		m_pPropgrid->GetEventSet()->subscribeEvent(EventPropGridItemClick::EventID, Subscriber(&SDesignerView::OnPropGridItemClick, this));
		m_pPropgrid->GetEventSet()->subscribeEvent(EventPropGridItemActive::EventID, Subscriber(&SDesignerView::OnPropGridItemActive, this));
	}

	m_strCurrentCtrlType = strCtrlType;

	((CMainDlg*)m_pMainHost)->m_edtDesc->SetWindowText(_T(""));

	//}
}


void SDesignerView::UpdatePropGrid(pugi::xml_node xmlNode)
{
	if (m_pPropgrid == NULL)
	{
		return;
	}

	m_pPropgrid->ClearAllGridItemValue();

	if (xmlNode == m_CurrentLayoutNode && _wcsicmp(xmlNode.name(), _T("SOUI")) == 0)
	{
		pugi::xml_attribute xmlAttr = xmlNode.first_attribute();

		while (xmlAttr)
		{
			SStringT str = xmlAttr.name();
			IPropertyItem *pItem = m_pPropgrid->GetGridItem(str.MakeLower());
			if (pItem)
			{
				if (str.CompareNoCase(_T("data")) == 0)
				{
					pItem->SetStringOnly(xmlNode.attribute(L"uidesiner_data").value());
				}
				else
				{
					SStringT s = xmlAttr.value();
					pItem->SetStringOnly(xmlAttr.value());
				}
			}

			xmlAttr = xmlAttr.next_attribute();
		}

		xmlAttr = xmlNode.child(_T("root")).first_attribute();
		while (xmlAttr)
		{
			SStringT str = xmlAttr.name();
			IPropertyItem *pItem = m_pPropgrid->GetGridItem(str.MakeLower());
			if (pItem)
			{
				if (str.CompareNoCase(_T("data")) == 0)
				{
					pItem->SetStringOnly(xmlNode.attribute(L"uidesiner_data").value());
				}
				else
				{
					pItem->SetStringOnly(xmlAttr.value());
				}
			}
			xmlAttr = xmlAttr.next_attribute();
		}
	}
	else
	{
		pugi::xml_attribute xmlAttr = xmlNode.first_attribute();

		IPropertyItem *pItem = m_pPropgrid->GetGridItem(uiedit_SpecAttr);
		if (pItem)
		{
			SStringT strTemp = xmlNode.text().get();
			strTemp.TrimBlank();
			pItem->SetStringOnly(strTemp);
		};

		while (xmlAttr)
		{
			SStringT str = xmlAttr.name();
			IPropertyItem *pItem = m_pPropgrid->GetGridItem(str.MakeLower());
			if (pItem)
			{
				if (str.CompareNoCase(_T("data")) == 0)
				{
					pItem->SetStringOnly(xmlNode.attribute(L"uidesiner_data").value());
				}
				else
				{
					pItem->SetStringOnly(xmlAttr.value());
				}
			}

			xmlAttr = xmlAttr.next_attribute();
		}
	}

	m_pPropgrid->Invalidate();
}

// 通过属性窗口调整窗口
bool SDesignerView::OnPropGridValueChanged(EventArgs *pEvt)
{
	pugi::xml_node xmlNode;
	BOOL bRoot = FALSE;

	IPropertyItem* pItem = ((EventPropGridValueChanged*)pEvt)->pItem;
	SStringT attr_name = pItem->GetName2();  //属性名：pos skin name id 等等
	SStringT attr_value = pItem->GetString();   //属性的值

	if (attr_name.IsEmpty())
	{
		return false;
	}

	//如果当前选择的是布局根窗口，需要特殊处理
	if (m_CurSelCtrl == m_pMoveWndRoot)
	{
		SPOSITION pos = m_lstRootProperty.GetHeadPosition();
		while (pos)
		{
			SStringT strTemp = m_lstRootProperty.GetNext(pos);
			if (strTemp.CompareNoCase(attr_name) == 0)
			{
				xmlNode = m_xmlNode.child(_T("root"));
				bRoot = TRUE;
				break;
			}
		}

		if (!bRoot)
		{
			SPOSITION pos = m_lstSouiProperty.GetHeadPosition();
			while (pos)
			{
				SStringT strTemp = m_lstSouiProperty.GetNext(pos);
				if (strTemp.CompareNoCase(attr_name) == 0)
				{
					xmlNode = m_xmlNode;
					bRoot = TRUE;
					break;
				}
			}
		}
	}
	else
	{
		xmlNode = m_xmlNode;
	}

	if (attr_name.CompareNoCase(uiedit_SpecAttr) == 0)
	{
		xmlNode.text().set(attr_value);
	}

	//SWindow * pWnd = ((SMoveWnd*)m_CurSelCtrl)->m_pRealWnd->GetParent();
	pugi::xml_attribute attr = xmlNode.attribute(attr_name);
	if (attr)
	{
		if (attr_value.IsEmpty())
		{
			if (attr_name.CompareNoCase(_T("data")) == 0)
			{
				xmlNode.attribute(_T("uidesiner_data")).set_value(_T(""));
			}
			else
			{
				xmlNode.remove_attribute(attr_name);
			}
		}
		else
		{
			if (attr_name.CompareNoCase(_T("data")) == 0)
			{
				xmlNode.attribute(_T("uidesiner_data")).set_value(attr_value);
			}
			else
			{
				attr.set_value(attr_value);
			}
		}
	}
	else
	{
		if ((!attr_value.IsEmpty()) && (attr_name.CompareNoCase(uiedit_SpecAttr) != 0))
		{
			xmlNode.append_attribute(attr_name).set_value(attr_value);
		}
	}

	/*SwndLayout *pLayout = pWnd->GetLayout();
	pLayout->Clear();

	pWnd->SetAttribute(s, s1);

	if (m_xmlNode.attribute(L"pos"))
	{
		pLayout->InitPosFromString(m_xmlNode.attribute(L"pos").value());
	}

	if (m_xmlNode.attribute(L"pos2type"))
	{
		pLayout->InitOffsetFromPos2Type(m_xmlNode.attribute(L"pos2type").value());
	}

	if (m_xmlNode.attribute(L"offset"))
	{
		pLayout->InitOffsetFromString(m_xmlNode.attribute(L"offset").value());
	}

	if (m_xmlNode.attribute(L"size"))
	{
		pLayout->InitSizeFromString(m_xmlNode.attribute(L"size").value());
	}*/


	int data = ((SMoveWnd*)m_CurSelCtrl)->m_pRealWnd->GetUserData();

	ReLoadLayout();

	if (bRoot)
	{
		m_CurSelCtrl = m_pMoveWndRoot;
	}
	else
	{
		SWindow *pRealWnd = FindChildByUserData(m_pRealWndRoot, data);

		SMap<SWindow*, SMoveWnd*>::CPair *p = m_mapMoveRealWnd.Lookup(pRealWnd);
		if (p)
		{
			m_CurSelCtrl = p->m_value;
		}
		else
		{
			m_pMoveWndRoot->Click(0, CPoint(0, 0));
		}
	}
	AddCodeToEditor(NULL);

	return true;
}

void SDesignerView::RefreshRes()
{
	m_xmlDocUiRes.load_file(m_strUIResFile, pugi::parse_full);

	CAutoRefPtr<IResProvider>   pResProvider1;
	TCHAR *s = m_strProPath.GetBuffer(m_strProPath.GetLength());

	IResProvider*	pResProvider = SApplication::getSingletonPtr()->GetMatchResProvider(_T("UIDEF"), _T("XML_INIT"));

	SApplication::getSingletonPtr()->RemoveResProvider(pResProvider);

	CreateResProvider(RES_FILE, (IObjRef**)&pResProvider1);
	if (!pResProvider1->Init((LPARAM)s, 0))
	{
		Debug(_T("ResProvider初始化失败"));
		return;
	}
	SApplication::getSingletonPtr()->AddResProvider(pResProvider1, NULL);

	SStringT strXMLInit;
	pugi::xml_node xmlNode = m_xmlDocUiRes.child(_T("resource")).child(_T("UIDEF")).child(_T("file"));

	if (xmlNode)
	{
		strXMLInit = xmlNode.attribute(_T("name")).as_string();
	}

	if (strXMLInit.IsEmpty())
	{
		strXMLInit = _T("xml_init");
	}

	//将皮肤中的uidef保存起来.
	m_pUiDef.Attach(SUiDef::getSingleton().CreateUiDefInfo(pResProvider1, _T("uidef:") + strXMLInit));
}

bool SDesignerView::OnPropGridItemClick(EventArgs *pEvt)
{
	EventPropGridItemClick *pEvent = (EventPropGridItemClick*)pEvt;
	IPropertyItem* pItem = pEvent->pItem;
	SStringT strType = pEvent->strType;

	if (strType.CompareNoCase(_T("skin")) == 0)
	{
		SDlgSkinSelect DlgSkin(_T("layout:UIDESIGNER_XML_SKIN_SELECT"), pItem->GetString(), m_strUIResFile);
		DlgSkin.m_pResFileManger = &((CMainDlg*)m_pMainHost)->m_UIResFileMgr;
		if (DlgSkin.DoModal(m_pMainHost->m_hWnd) == IDOK)
		{
			SStringT s1 = pItem->GetString();   //属性的值

			if (s1.CompareNoCase(DlgSkin.m_strSkinName) != 0)
			{
				RefreshRes();
				pItem->SetString(DlgSkin.m_strSkinName);
				m_pPropgrid->Invalidate();
				//ReLoadLayout();
			}
		}
		//调用皮肤对话框
	}
	else if (strType.CompareNoCase(_T("font")) == 0)
	{
		//调用字体对话框
		SDlgFontSelect DlgFont(pItem->GetString(), this);
		if (DlgFont.DoModal(m_pMainHost->m_hWnd) == IDOK)
		{
			pItem->SetString(DlgFont.m_strFont);
			m_pPropgrid->Invalidate();
		}
	}
	else if (strType.CompareNoCase(_T("class")) == 0)
	{

	}

	return true;
}

BOOL SDesignerView::ReLoadLayout(BOOL bClearSel)
{
	if (bClearSel)
		m_CurSelCtrl = NULL;

	m_ndata = 0;
	g_nUIElmIndex = 0;
	//设置uidef为当前皮肤的uidef
	UseEditorUIDef(false);

	pugi::xml_node xmlnode;
	BOOL bIsInclude = FALSE;

	if (m_CurrentLayoutNode == NULL)
	{
		return TRUE;
	}

	if (S_CW2T(m_CurrentLayoutNode.name()) != _T("SOUI"))
	{
		xmlnode = m_CurrentLayoutNode;
		bIsInclude = TRUE;
	}
	else
	{
		////加载私有皮肤
		//if (m_privateStylePool->GetCount())
		//{
		//	m_privateStylePool->RemoveAll();
		//	GETSTYLEPOOLMGR->PopStylePool(m_privateStylePool);
		//}
		//BOOL ret=m_privateStylePool->Init(m_CurrentLayoutNode.child(L"style"));
		//if (ret)
		//{
		//	GETSTYLEPOOLMGR->PushStylePool(m_privateStylePool);
		//}
		//if (m_privateSkinPool->GetCount())
		//{
		//	m_privateSkinPool->RemoveAll();
		//	GETSKINPOOLMGR->PopSkinPool(m_privateSkinPool);
		//}
		//int skincount=m_privateSkinPool->LoadSkins(m_CurrentLayoutNode.child(L"skin"));//从xmlNode加加载私有skin
		//if (skincount)
		//{
		//	GETSKINPOOLMGR->PushSkinPool(m_privateSkinPool);
		//}

		xmlnode = m_CurrentLayoutNode.child(L"root", false);
	}
	if (!xmlnode) return FALSE;


	SWindow *pChild = m_pContainer->GetWindow(GSW_FIRSTCHILD);
	while (pChild)
	{
		SWindow *pNext = pChild->GetWindow(GSW_NEXTSIBLING);
		pChild->DestroyWindow();
		pChild = pNext;
	}


	SStringW s1, s2;
	if (!bIsInclude)
	{
		int nWidth, nHeight;
		SStringT strSize(_T("size"));
		SStringT strWidth(_T("width"));
		SStringT strHeight(_T("height"));
		SStringT strMargin(_T("margin"));

		pugi::xml_attribute attr = m_CurrentLayoutNode.first_attribute();
		while (attr)
		{
			// width height单独处理，解决margin的问题
			if (strSize.CompareNoCase(attr.name()) == 0)
			{
				//size属性
				SStringT strVal = attr.value();
				swscanf(strVal, L"%d,%d", &nWidth, &nHeight);
			}
			else if (strWidth.CompareNoCase(attr.name()) == 0)
			{
				//width属性
				::StrToIntExW(attr.value(), STIF_SUPPORT_HEX, &nWidth);
			}
			else if (strHeight.CompareNoCase(attr.name()) == 0)
			{
				//height属性
				::StrToIntExW(attr.value(), STIF_SUPPORT_HEX, &nHeight);
			}
			else if (strMargin.CompareNoCase(attr.name()) == 0)
			{
				//忽略margin属性

			}
			else
			{
				s1.Format(L" %s=\"%s\" ", attr.name(), attr.value());
				s2 = s2 + s1;
			}
			attr = attr.next_attribute();
		}

		attr = xmlnode.first_attribute();
		while (attr)
		{
			s1.Format(L" %s=\"%s\" ", attr.name(), attr.value());
			s2 = s2 + s1;

			attr = attr.next_attribute();
		}

		SStringW strAttrSize;
		strAttrSize.Format(L" margin= \"%d\" width = \"%d\" height = \"%d\" ", MARGIN, nWidth + MARGIN * 2, nHeight + MARGIN * 2);

		s2 = L"<designerRoot pos=\"20,20\" " + s2 + strAttrSize + L"></designerRoot>";
	}
	else
	{
		int nWidth, nHeight;

		pugi::xml_attribute attrWorH = m_CurrentLayoutNode.attribute(_T("width"));

		if (attrWorH)
		{
			::StrToIntExW(attrWorH.value(), STIF_SUPPORT_HEX, &nWidth);
		}
		else
		{
			nWidth = 500;
			m_CurrentLayoutNode.append_attribute(_T("width")).set_value(nWidth);
		}

		attrWorH = m_CurrentLayoutNode.attribute(_T("height"));

		if (attrWorH)
		{
			::StrToIntExW(attrWorH.value(), STIF_SUPPORT_HEX, &nHeight);
		}
		else
		{
			nHeight = 500;
			m_CurrentLayoutNode.append_attribute(_T("height")).set_value(nHeight);
		}

		SStringW strAttrSize;
		strAttrSize.Format(L" margin= \"%d\" width = \"%d\" height = \"%d\" ", MARGIN, nWidth + MARGIN * 2, nHeight + MARGIN * 2);


		s2 = L"<designerRoot pos=\"20,20\" " + strAttrSize + L" colorBkgnd=\"#d0d0d0\"/>";

		//s2 = L"<window pos=\"20,20,-20,-20\" colorBkgnd=\"#d0d0d0\"></window>";
	}

	//wchar_t *s = L"<window pos=\"20,20,@500,@500\" colorBkgnd=\"#d0d0d0\"></window>";
	const wchar_t *s3 = L"<movewnd pos=\"20,20,@500,@500\" ></movewnd>";

	g_bHookCreateWnd = TRUE;
	////创建布局窗口的根窗口
	m_pRealWndRoot = (SDesignerRoot*)m_pContainer->CreateChildren(s2);
	m_pRealWndRoot->SetRootFont(m_defFont);

	m_pMoveWndRoot = (SMoveWnd *)m_pContainer->CreateChildren(s3);
	m_pMoveWndRoot->m_pRealWnd = m_pRealWndRoot;

	m_mapMoveRealWnd.RemoveAll();
	m_mapMoveRealWnd[m_pMoveWndRoot->m_pRealWnd] = m_pMoveWndRoot;

	m_pMoveWndRoot->m_Desiner = this;

	if (!m_pRealWndRoot->CreateChildren(xmlnode))
	{
		g_bHookCreateWnd = FALSE;
		return FALSE;
	}

	CreateAllChildWnd(m_pRealWndRoot, m_pMoveWndRoot);
	g_bHookCreateWnd = FALSE;

	m_treeXmlStruct->RemoveAllItems();
	InitXMLStruct(m_CurrentLayoutNode, STVI_ROOT);

	//恢复uidef为编辑器的uidef
	UseEditorUIDef(true);

	return TRUE;
}


BOOL SDesignerView::bIsContainerCtrl(SStringT strCtrlName) //判断控件是否是容器控件
{
	SStringT s;
	SPOSITION pos = m_lstContainerCtrl.GetHeadPosition();
	while (pos)
	{
		s = m_lstContainerCtrl.GetNext(pos);
		if (s.CompareNoCase(strCtrlName) == 0)
		{
			return TRUE;
		}
	}
	return FALSE;
}

void SDesignerView::SaveEditorCaretPos()
{
	m_nSciCaretPos = m_pScintillaWnd->SendMessage(SCI_GETCURRENTPOS);
}

void SDesignerView::RestoreEditorCaretPos()
{
	m_pScintillaWnd->SendMessage(SCI_GOTOPOS, m_nSciCaretPos);
}

void SDesignerView::AddCodeToEditor(CScintillaWnd* _pSciWnd)  //复制xml代码到代码编辑器
{
	CScintillaWnd* pSciWnd = m_pScintillaWnd;
	if (_pSciWnd)
		pSciWnd = _pSciWnd;

	m_strCurFileEditor = m_strCurLayoutXmlFile;

	pSciWnd->SendMessage(SCI_SETREADONLY, 0, 0);
	pSciWnd->SendMessage(SCI_CLEARALL, 0, 0);

	SStringA str;

	pugi::xml_document doc;
	SStringT strNodeName(_T("include"));

	if (m_xmlNode == m_CurrentLayoutNode && strNodeName.CompareNoCase(m_xmlNode.name()) == 0)
	{
		doc.append_copy(m_xmlNode);
	}
	else
		if (m_xmlNode == m_CurrentLayoutNode)
		{
			doc.append_copy(m_xmlNode.child(_T("root")));
		}
		else
		{
			doc.append_copy(m_xmlNode);
		}


	RemoveWndName(doc.root(), FALSE);
	//Debug(doc.root());
	TrimXmlNodeTextBlank(doc.document_element());

	pugi::xml_writer_buff writer;
	doc.document_element().print(writer, L"\t", pugi::format_default, pugi::encoding_utf16);
	SStringW *strDebug = new SStringW(writer.buffer(), writer.size());

	str = S_CW2A(*strDebug, CP_UTF8);
	pSciWnd->SendMessage(SCI_ADDTEXT, str.GetLength(),
		reinterpret_cast<LPARAM>((LPCSTR)str));
	pSciWnd->SetDirty(false);
	pSciWnd->SetFocus();
	RestoreEditorCaretPos();

	delete strDebug;
}

// 把代码编辑器修改的结果重新加载, 更新布局窗口
void SDesignerView::GetCodeFromEditor(CScintillaWnd* _pSciWnd)//从代码编辑器获取xml代码
{
	if (m_strCurLayoutXmlFile.IsEmpty())
	{
		return;
	}

	if (m_strCurFileEditor.CompareNoCase(m_strCurLayoutXmlFile) != 0)
	{
		return;
	}

	CScintillaWnd* pSciWnd = m_pScintillaWnd;
	if (_pSciWnd)
		pSciWnd = _pSciWnd;

	int n = pSciWnd->SendMessage(SCI_GETTEXT, 0, 0);
	if (n == 0)
	{
		return;
	}

	char *chText = new char[n];

	pSciWnd->SendMessage(SCI_GETTEXT, n, (LPARAM)chText);

	SStringA s(chText);
	SStringW s1 = S_CA2W(s, CP_UTF8);
	delete chText;

	pugi::xml_document doc;
	if (!doc.load_buffer(s1, wcslen(s1) * sizeof(wchar_t), pugi::parse_full, pugi::encoding_utf16))
	{
		Debug(_T("XML有语法错误！"));
		return;
	}

	SaveEditorCaretPos();

	RenameChildeWnd(doc.root());
	TrimXmlNodeTextBlank(doc.document_element());

	BOOL bRoot = FALSE;

	SStringT strNodeName(_T("include"));

	if (m_xmlNode == m_CurrentLayoutNode && strNodeName.CompareNoCase(m_xmlNode.name()) == 0)
	{
		pugi::xml_node xmlPNode;
		xmlPNode = m_xmlNode.parent();
		pugi::xml_node xmlNode = xmlPNode.insert_copy_after(doc.root().first_child(), m_xmlNode);

		xmlPNode.remove_child(m_xmlNode);
		m_xmlNode = xmlNode;

		SStringT aa = Debug1(m_xmlNode);
		m_CurrentLayoutNode = m_xmlNode;
	}
	else
		if (m_xmlNode == m_CurrentLayoutNode && strNodeName.CompareNoCase(m_xmlNode.name()) != 0)
		{
 			pugi::xml_node fristNode = m_xmlNode.child(_T("root"));
 			pugi::xml_node xmlNode = m_xmlNode.insert_copy_after(doc.root().first_child(), fristNode);
 			m_xmlNode.remove_child(fristNode);
			bRoot = TRUE;
		}
		else
		{
			pugi::xml_node xmlPNode;
			xmlPNode = m_xmlNode.parent();
			pugi::xml_node xmlNode = xmlPNode.insert_copy_after(doc.root().first_child(), m_xmlNode);

			xmlPNode.remove_child(m_xmlNode);
			m_xmlNode = xmlNode;
		}


	// 	ReLoadLayout(FALSE);
	// 	m_pMoveWndRoot->Click(0, CPoint(0, 0));

	// 先记下原来选的控件是第几个顺序的控件, 再进行重布局
	int data = ((SMoveWnd*)m_CurSelCtrl)->GetUserData();
	ReLoadLayout();

	if (bRoot)
	{
		m_CurSelCtrl = m_pMoveWndRoot;
	}
	else
	{
		SMoveWnd *pMoveWnd = (SMoveWnd*)FindChildByUserData(m_pMoveWndRoot, data);

		if (pMoveWnd)
		{
			m_CurSelCtrl = pMoveWnd;
			AddCodeToEditor(NULL);
		}
		else
		{
			m_pMoveWndRoot->Click(0, CPoint(0, 0));
		}
	}
}

void SDesignerView::SetSelCtrlNode(pugi::xml_node xmlNode)
{
	m_nState = 1;

	pugi::xml_writer_buff writer;
	xmlNode.print(writer, L"\t", pugi::format_default, pugi::encoding_utf16);
	SStringW *strxmlWnd = new SStringW(writer.buffer(), writer.size());

	if (m_xmlSelCtrlDoc.load_buffer(*strxmlWnd, wcslen(*strxmlWnd) * sizeof(wchar_t), pugi::parse_default, pugi::encoding_utf16))
	{
		m_xmlSelCtrlNode = m_xmlSelCtrlDoc.first_child();

		delete strxmlWnd;
	}
	else
	{
		Debug(_T("选择控件异常"));
	}

	return;
}


void SDesignerView::NewWnd(CPoint pt, SMoveWnd *pM)
{
	BOOL bIsInclude = FALSE;

	m_xmlNode = m_xmlSelCtrlNode.first_child();

	//替换Include
	if (_wcsicmp(m_xmlNode.name(), L"include") == 0 && m_xmlNode.attribute(L"src"))
	{
		bIsInclude = TRUE;
		SStringT strInclude = NodeToStr(m_xmlNode);
		m_xmlNode.set_name(L"window");
		m_xmlNode.remove_attribute(L"src");
		m_xmlNode.append_attribute(L"pos").set_value(L"10,10,-10,-10");
		m_xmlNode.append_attribute(L"colorBkgnd").set_value(L"RGB(191,141,255)");
		m_xmlNode.text().set(strInclude);

		RenameWnd(m_xmlNode, TRUE);

		SMap<SStringT, SMap<int, SStringT>* >::CPair *p = m_mapInclude1.Lookup(m_strCurLayoutXmlFile);
		if (p)
		{
			SMap<int, SStringT>* pMap = p->m_value;
			(*pMap)[m_xmlNode.attribute(L"data").as_int()] = strInclude;
		}
		else
		{
			Debug(_T("替换include出错"));
		}
	}
	else
	{
		//重命名控件
		RenameWnd(m_xmlNode, TRUE);
		RenameChildeWnd(m_xmlNode);
	}

	UseEditorUIDef(false);

	SWindow* pRealWnd;
	SMoveWnd* pMoveWnd;

	if (pM->m_pRealWnd == m_pRealWndRoot)
	{
		pRealWnd = pM->m_pRealWnd;
		pMoveWnd = pM;
	}
	else
	{
		SStringT s;
		s.Format(_T("%d"), pM->m_pRealWnd->GetUserData());
		pugi::xml_node xmlNodeRealWnd = FindNodeByAttr(m_CurrentLayoutNode, L"data", s);
		if (bIsContainerCtrl(xmlNodeRealWnd.name()))
		{
			pRealWnd = pM->m_pRealWnd;
			pMoveWnd = pM;
		}
		else
		{
			pRealWnd = pM->m_pRealWnd->GetParent();
			pMoveWnd = (SMoveWnd*)pM->GetParent();
		}
	}

	//有margin的情况
	SwndStyle &style = pRealWnd->GetStyle();
	int nMarginLeft = 0;
	int nMarginTop = 0;
	CRect rcMargin = style.GetMargin();
	nMarginLeft = rcMargin.left;
	nMarginTop = rcMargin.top;
	pt.x -= nMarginLeft;
	pt.y -= nMarginTop;

	if (!bIsInclude)
	{
		CRect rect;
		pMoveWnd->GetWindowRect(rect);

		SStringT strPos;
		/*strPos.Format(_T("%d,%d"), pt.x - rect.left, pt.y - rect.top);*/

		//8 对齐
		int x, y;
		x = (pt.x - rect.left) / 8 * 8;
		y = (pt.y - rect.top) / 8 * 8;
		if (x < 0)
		{
			x = 0;
		}

		if (y < 0)
		{
			y = 0;
		}
		strPos.Format(_T("%d,%d"), x, y);

		if (!m_xmlNode.attribute(L"pos"))
		{
			m_xmlNode.append_attribute(L"pos");
		}
		m_xmlNode.attribute(L"pos").set_value(strPos);
		if (m_xmlNode.attribute(L"size"))
		{
			SStringT strSize;
			strSize = m_xmlNode.attribute(L"size", false).value();
		}
	}
	SStringT strMoveWnd;
	strMoveWnd = _T("<movewnd pos=\"0,0,@120,@30\"></movewnd>");
	pugi::xml_writer_buff writer;
	//m_Desiner->m_xmlNode.print(writer,L"\t",pugi::format_default,pugi::encoding_utf16);
	m_xmlNode.print(writer, L"\t", pugi::format_default, pugi::encoding_utf16);
	SStringW *strxmlWnd = new SStringW(writer.buffer(), writer.size());
	pRealWnd->CreateChildren(*strxmlWnd);
	SWindow *Wnd = pRealWnd->GetWindow(GSW_LASTCHILD);
	SMoveWnd *Wnd1 = (SMoveWnd *)CreateWnd(pMoveWnd, strMoveWnd);
	Wnd1->m_pRealWnd = (SUIWindow*)Wnd;
	m_mapMoveRealWnd[Wnd] = Wnd1;
	CreateAllChildWnd((SUIWindow*)Wnd, Wnd1);
	//找到m_realWnd控件对应的xml节点
	if (pRealWnd == m_pRealWndRoot)
	{
		SStringT s(_T("SOUI"));
		pugi::xml_node firstNode = m_CurrentLayoutNode;
		if (s.CompareNoCase(firstNode.name()) == 0)
		{
			firstNode = firstNode.child(_T("root"));
		}
		SetCurrentCtrl(firstNode.append_copy(m_xmlNode), Wnd1);
		//m_Desiner->m_xmlNode = firstNode.append_copy(m_Desiner->m_xmlNode);
	}
	else
	{
		//找到m_realWnd控件对应的xml节点
		SStringT s;
		s.Format(_T("%d"), pRealWnd->GetUserData());
		pugi::xml_node xmlNodeRealWnd = FindNodeByAttr(m_CurrentLayoutNode, L"data", s);
		//将新创建的控件写入父控件的xml节点
		SetCurrentCtrl(xmlNodeRealWnd.append_copy(m_xmlNode), Wnd1);
		//m_Desiner->m_xmlNode = xmlNodeRealWnd.append_copy(m_Desiner->m_xmlNode);
	}

	UseEditorUIDef(true);

	pRealWnd->RequestRelayout();
	pRealWnd->Invalidate();
	m_nState = 0;
	delete strxmlWnd;
}

int SDesignerView::InitXMLStruct(pugi::xml_node xmlNode, HSTREEITEM item)
{
	if (!xmlNode)
	{
		return 0;
	}

	int count = 0;
	pugi::xml_node NodeSib = xmlNode;
	while (NodeSib)
	{
		if (NodeSib.type() != pugi::node_element)
		{
			NodeSib = NodeSib.next_sibling();
			continue;
		}

		int data = 0;
		if (NodeSib.attribute(_T("data")))
		{
			data = NodeSib.attribute(_T("data")).as_int();
		}

		SStringT strNodeName = NodeSib.name();
		if (strNodeName.IsEmpty())
		{
			NodeSib = NodeSib.next_sibling();
			continue;
		}

		count++;
		HSTREEITEM itemChild = m_treeXmlStruct->InsertItem(strNodeName, item);
		m_treeXmlStruct->SetItemData(itemChild, data);

		count += InitXMLStruct(NodeSib.first_child(), itemChild);
		NodeSib = NodeSib.next_sibling();
	}
	if (item == STVI_ROOT)
	{
		SStringT tmpstr;
		tmpstr.Format(_T("窗口总数: %d"), count);
		((CMainDlg*)m_pMainHost)->m_textNodenum->SetWindowText(tmpstr);
	}
	m_treeXmlStruct->Invalidate();
	return count;
}



BOOL SDesignerView::GoToXmlStructItem(int  data, HSTREEITEM item)
{
	HSTREEITEM SibItem = item;

	while (SibItem)
	{
		int data1 = m_treeXmlStruct->GetItemData(SibItem);

		if (data1 == data)
		{
			m_treeXmlStruct->SelectItem(SibItem);
			m_treeXmlStruct->Invalidate();
			return TRUE;
		}

		HSTREEITEM ChildItem = m_treeXmlStruct->GetChildItem(SibItem);

		BOOL Result = GoToXmlStructItem(data, ChildItem);
		if (Result)
		{
			return TRUE;
		}

		SibItem = m_treeXmlStruct->GetNextSiblingItem(SibItem);
	}
	return FALSE;
}

// 响应窗口结构中点击选中界面元素
bool SDesignerView::OnTCSelChanged(EventArgs *pEvt)
{
	if (!m_pContainer->GetParent()->IsVisible())
	{   //先这样写吧，有时间再改
		return true;
	}

	EventTCSelChanged *evt = (EventTCSelChanged*)pEvt;
	HSTREEITEM item = m_treeXmlStruct->GetSelectedItem();

	int data = m_treeXmlStruct->GetItemData(item);

	SStringT s;
	s.Format(_T("%d"), data);
	pugi::xml_node xmlNode = FindNodeByAttr(m_CurrentLayoutNode, L"data", s);

	if (!xmlNode)
	{
		return true;
	}

	SWindow *wnd = FindChildByUserData(m_pRealWndRoot, data);
	if (wnd)
	{
		SMap<SWindow*, SMoveWnd*>::CPair *p = m_mapMoveRealWnd.Lookup(wnd);
		if (p)
		{
			m_xmlNode = xmlNode;
			m_CurSelCtrl = p->m_value;

			m_CurSelCtrl->Click(0, CPoint(0, 0));
		}
	}

	return true;
}


void SDesignerView::DeleteCtrl()
{
	if (m_xmlNode == m_CurrentLayoutNode)
	{
		return;
	}
	else
	{
		m_xmlNode.parent().remove_child(m_xmlNode);

		//Debug(m_CurrentLayoutNode);
		GetMoveWndRoot()->Click(0, CPoint(0, 0));
		ReLoadLayout();
		m_nState = 0;

	}
}

void SDesignerView::Preview()
{
	//SMap<SWindow*, SMoveWnd*>::CPair *p = m_mapMoveRealWnd.GetNext();

	SMoveWnd *wnd;

	m_pMoveWndRoot->SetVisible(FALSE);

	m_pMoveWndRoot->GetParent()->Invalidate();

	//SPOSITION pos = m_mapMoveRealWnd.GetStartPosition();
	//while (pos)
	//{
	//	SMap<SWindow*, SMoveWnd*>::CPair *p = m_mapMoveRealWnd.GetNext(pos);
	//	wnd = p->m_value;
	//	wnd->SetVisible(FALSE);
	//}

}

void SDesignerView::unPreview()
{
	ReLoadLayout();
	m_nState = 0;
	GetMoveWndRoot()->Click(0, CPoint(0, 0));
	m_pMoveWndRoot->GetParent()->Invalidate();
}

void SDesignerView::ShowZYGLDlg()
{
	if (m_strUIResFile.IsEmpty())
	{
		return;
	}
	SDlgSkinSelect DlgSkin(_T("layout:UIDESIGNER_XML_SKIN_SELECT"), _T(""), m_strUIResFile, FALSE);
	DlgSkin.m_pResFileManger = &((CMainDlg*)m_pMainHost)->m_UIResFileMgr;
	if (DlgSkin.DoModal(m_pMainHost->m_hWnd) == IDOK)
	{
		RefreshRes();
	}
}

void SDesignerView::ShowYSGLDlg()
{
	if (m_strUIResFile.IsEmpty())
	{
		return;
	}
	SDlgStyleManage dlg(_T(""), m_strUIResFile, FALSE);
	dlg.m_pResFileManger = &((CMainDlg*)m_pMainHost)->m_UIResFileMgr;
	if (dlg.DoModal(m_pMainHost->m_hWnd) == IDOK)
	{
		RefreshRes();
	}
}

void SDesignerView::ShowMovWndChild(BOOL bShow, SMoveWnd* pMovWnd)
{
	if (bShow)
	{
		for (; pMovWnd; pMovWnd = (SMoveWnd*)pMovWnd->GetWindow(GSW_NEXTSIBLING))
		{
			SWindow* pRealWnd = pMovWnd->m_pRealWnd;
			pMovWnd->SetVisible(pRealWnd->IsVisible());
			ShowMovWndChild(bShow, (SMoveWnd*)pMovWnd->GetWindow(GSW_FIRSTCHILD));
		}
	}
	else
	{
		for (; pMovWnd; pMovWnd = (SMoveWnd*)pMovWnd->GetWindow(GSW_NEXTSIBLING))
		{
			pMovWnd->SetVisible(FALSE);
			ShowMovWndChild(bShow, (SMoveWnd*)pMovWnd->GetWindow(GSW_FIRSTCHILD));
		}
	}
}

int SDesignerView::GetIndexData()
{
	m_ndata = m_ndata + 1;
	return m_ndata;
}

SWindow* SDesignerView::FindChildByUserData(SWindow* pWnd, int data)
{
	SWindow *pChild = pWnd->GetWindow(GSW_FIRSTCHILD);
	while (pChild)
	{
		int child_data = pChild->GetUserData();
		if (child_data == data)
			return pChild;
		pChild = pChild->GetWindow(GSW_NEXTSIBLING);
	}

	pChild = pWnd->GetWindow(GSW_FIRSTCHILD);
	while (pChild)
	{
		SWindow *pChildFind = FindChildByUserData(pChild, data);
		if (pChildFind) return pChildFind;
		pChild = pChild->GetWindow(GSW_NEXTSIBLING);
	}

	return NULL;
}


void SDesignerView::TrimXmlNodeTextBlank(pugi::xml_node xmlNode)
{
	if (!xmlNode)
	{
		return;
	}

	pugi::xml_node NodeSib = xmlNode;
	while (NodeSib)
	{
		if (NodeSib.type() != pugi::node_element)
		{
			NodeSib = NodeSib.next_sibling();
			continue;
		}

		SStringT strText = NodeSib.text().get();
		strText.TrimBlank();
		if (!strText.IsEmpty())
		{
			NodeSib.text().set(strText);
		}

		TrimXmlNodeTextBlank(NodeSib.first_child());
		NodeSib = NodeSib.next_sibling();
	}
}


bool SDesignerView::OnPropGridItemActive(EventArgs *pEvt)
{
	EventPropGridItemActive *pEvent = (EventPropGridItemActive*)pEvt;
	IPropertyItem* pItem = pEvent->pItem;

	SStringT strDesc = pItem->GetDescription();
	SStringT strName = pItem->GetName1();

	if (strDesc.IsEmpty())
	{
		((CMainDlg*)m_pMainHost)->m_edtDesc->SetWindowText(strName);
	}
	else
	{

		((CMainDlg*)m_pMainHost)->m_edtDesc->SetWindowText(strDesc);
	}

	return true;
}

void SDesignerView::UseEditorUIDef(bool bYes) //使用编辑器自身的UIDef还是使用所打开的工程的UIDef
{
	if (bYes)
	{
		SUiDef::getSingleton().SetUiDef(m_pOldUiDef);
	}
	else
	{
		SUiDef::getSingleton().SetUiDef(m_pUiDef);
	}
}

SStringT SDesignerView::UnitToStr(int nUnit)
{
	//	px=0,dp,dip,sp
	switch (nUnit)
	{
	case 0:
		return _T("");
	case 1:
		return _T("dp");
	case 2:
		return _T("dip");
	case 3:
		return _T("sp");
	default:
		return _T("");
	}
}
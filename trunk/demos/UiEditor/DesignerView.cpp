#include "stdafx.h"
#include "DesignerView.h"
#include "SMoveWnd.h"
#include "CNewGuid.h"
#include "helper\SplitString.h"
#include "DlgSkinSelect.h"
#include "DlgStyleManage.h"
#include "core/SWnd.h"
#include "MainDlg.h"
#include "DlgFontSelect.h"

#define  MARGIN 20

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
	m_nState = 0;
	m_pMoveWndRoot = NULL;
	m_pRealWndRoot = NULL;
	m_pContainer = pContainer;
	m_pMainHost = pMainHost;
	m_treeXmlStruct = pTreeXmlStruct;
	m_ndata = 0;

	m_treeXmlStruct->GetEventSet()->subscribeEvent(EVT_TC_SELCHANGED,Subscriber(&SDesignerView::OnTCSelChanged,this));


	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_file(_T("Config\\Ctrl.xml"));
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
	m_xmlDocUiRes.load_file(strFileName);



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
	SApplication::getSingletonPtr()->AddResProvider(pResProvider);

	return TRUE;
}


BOOL SDesignerView::InsertLayoutToMap(SStringT strFileName)
{
	SStringT FullFileName = m_strProPath + _T("\\") + strFileName;

	pugi::xml_document *xmlDoc1 = new pugi::xml_document();

	//if(!xmlDoc1->load_file(FullFileName,pugi::parse_default,pugi::encoding_utf8))
	if(!xmlDoc1->load_file(FullFileName))
		return FALSE;

	m_mapLayoutFile[strFileName] = xmlDoc1->first_child();

	m_mapLayoutFile1[strFileName] = xmlDoc1;  

	m_mapInclude1[strFileName]= new SMap<int, SStringT>;


	m_strCurFile = strFileName;
	RenameChildeWnd(xmlDoc1->first_child());
	m_strCurFile.Empty();
	return TRUE;
}

BOOL SDesignerView::LoadLayout(SStringT strFileName)
{

	pugi::xml_node xmlroot;
	pugi::xml_node xmlnode;

	BOOL bIsInclude = FALSE;

	m_strCurFile = strFileName;

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
		BOOL ret=m_privateStylePool->Init(xmlroot.child(L"style"));
		if (ret)
		{
			GETSTYLEPOOLMGR->PushStylePool(m_privateStylePool);
		}
		if (m_privateSkinPool->GetCount())
		{
			m_privateSkinPool->RemoveAll();
			GETSKINPOOLMGR->PopSkinPool(m_privateSkinPool);
		}
		int skincount=m_privateSkinPool->LoadSkins(xmlroot.child(L"skin"));//从xmlNode加加载私有skin
		if (skincount)
		{
			GETSKINPOOLMGR->PushSkinPool(m_privateSkinPool);
		}

		xmlnode=xmlroot.child(L"root", false);
	}
	if(!xmlnode) return FALSE;


	m_pContainer->SSendMessage(WM_DESTROY);
	/*
	SWindow *pChild = m_pContainer->GetWindow(GSW_FIRSTCHILD);
	while(pChild)
	{
		SWindow *pNext = pChild->GetWindow(GSW_NEXTSIBLING);
		pChild->DestroyWindow();
		pChild = pNext;
	}
	*/


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
				swscanf(strVal,L"%d,%d",&nWidth,&nHeight);

				bHasSize = TRUE;
			}else if (strWidth.CompareNoCase(attr.name()) == 0)
			{     
				//width属性
				::StrToIntExW(attr.value(),STIF_SUPPORT_HEX,&nWidth);               
			}else if (strHeight.CompareNoCase(attr.name()) == 0)
			{  
				//height属性
				::StrToIntExW(attr.value(),STIF_SUPPORT_HEX,&nHeight);
			}else if (strMargin.CompareNoCase(attr.name()) == 0)
			{  
				//忽略margin属性

			}else
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

		s2 = L"<window pos=\"20,20\" " +  s2 + strAttrSize + L"></window>";


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


	}else
	{
		//include文件

		int nWidth, nHeight;

		pugi::xml_attribute attrWorH = m_CurrentLayoutNode.attribute(_T("width"));

		if (attrWorH)
		{
			::StrToIntExW(attrWorH.value(),STIF_SUPPORT_HEX,&nWidth);
		}else
		{
			nWidth = 500;
			m_CurrentLayoutNode.append_attribute(_T("width")).set_value(nWidth);
		}

		attrWorH = m_CurrentLayoutNode.attribute(_T("height"));

		if (attrWorH)
		{
			::StrToIntExW(attrWorH.value(),STIF_SUPPORT_HEX,&nHeight);
		}else
		{
			nHeight = 500;
			m_CurrentLayoutNode.append_attribute(_T("height")).set_value(nHeight);
		}

		SStringW strAttrSize;
		strAttrSize.Format(L" margin= \"%d\" width = \"%d\" height = \"%d\" ", MARGIN, nWidth + MARGIN * 2, nHeight + MARGIN * 2);


    	s2 = L"<window pos=\"20,20\" " + strAttrSize + L" colorBkgnd=\"#d0d0d0\"></window>";
	}

	//wchar_t *s = L"<window pos=\"20,20,@500,@500\" colorBkgnd=\"#d0d0d0\"></window>";
	wchar_t *s3 = L"<movewnd pos=\"20,20,@500,@500\" ></movewnd>";

	////创建布局窗口的根窗口

	m_pRealWndRoot = m_pContainer->CreateChildren(s2);
 	m_pMoveWndRoot = (SMoveWnd *)m_pContainer->CreateChildren(s3);
 	m_pMoveWndRoot->m_pRealWnd = m_pRealWndRoot;
 	m_pMoveWndRoot->m_Desiner = this;	


	m_mapMoveRealWnd.RemoveAll();
	m_mapMoveRealWnd[m_pMoveWndRoot->m_pRealWnd] = m_pMoveWndRoot;



	if (!m_pRealWndRoot->CreateChildren(xmlnode))
	{
		return FALSE;
	}

	CreateAllChildWnd(m_pRealWndRoot, m_pMoveWndRoot);

	m_nState = 0;
	GetMoveWndRoot()->Click(0, CPoint(0,0));

	m_treeXmlStruct->RemoveAllItems();
	InitXMLStruct(m_CurrentLayoutNode, STVI_ROOT);
	return TRUE;
}

void SDesignerView::CreateAllChildWnd(SWindow *pRealWnd, SMoveWnd *pMoveWnd)
{
	////得到第一个子窗口
	SWindow *pSibReal = pRealWnd->GetWindow(GSW_FIRSTCHILD);
	for (; pSibReal; pSibReal = pSibReal->GetWindow(GSW_NEXTSIBLING))
	{
		wchar_t *s1 = L"<movewnd pos=\"0,0,@100,@100\" ></movewnd>";

		//创建布局窗口的根窗口
		SMoveWnd *pSibMove = (SMoveWnd *)pMoveWnd->CreateChildren(s1);
		pSibMove->m_pRealWnd = pSibReal;
		pSibMove->SetVisible(pSibReal->IsVisible());

		m_mapMoveRealWnd[pSibReal] = pSibMove;

		pSibMove->m_Desiner = this;	

		CreateAllChildWnd(pSibReal, pSibMove);


	}
}

void SDesignerView::UpdateAllLayout(SMoveWnd* pMoveWndRoot)
{
	int b = pMoveWndRoot->GetChildrenCount();
	////得到第一个子窗口
	SMoveWnd *pSibMove = (SMoveWnd*)pMoveWndRoot->GetWindow(GSW_FIRSTCHILD);
	for (; pSibMove; pSibMove = (SMoveWnd*)pSibMove->GetWindow(GSW_NEXTSIBLING))
	{

		//更新两个窗口的布局
		SwndLayout *pMoveWndLayout = pSibMove->GetLayout();
		SwndLayout *pRealWndLayout = pSibMove->m_pRealWnd->GetLayout();

		pMoveWndLayout->uPositionType = pRealWndLayout->uPositionType;
		pMoveWndLayout->nCount = pRealWndLayout->nCount;

		for (int i = 0; i < 4; i++)
		{
			pMoveWndLayout->pos[i].cMinus = pRealWndLayout->pos[i].cMinus;
			pMoveWndLayout->pos[i].nPos = pRealWndLayout->pos[i].nPos;
			pMoveWndLayout->pos[i].nRefID = pRealWndLayout->pos[i].nRefID;
			pMoveWndLayout->pos[i].pit = pRealWndLayout->pos[i].pit;
		}
		pMoveWndLayout->fOffsetX = pRealWndLayout->fOffsetX;
		pMoveWndLayout->fOffsetY = pRealWndLayout->fOffsetY;
		pMoveWndLayout->SetWidth(pRealWndLayout->GetSpecifySize(PD_X));
		pMoveWndLayout->SetHeight(pRealWndLayout->GetSpecifySize(PD_Y));

		UpdateAllLayout(pSibMove);


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


	SPOSITION pos = m_mapLayoutFile1.GetStartPosition();

	while (pos)
	{
		p = m_mapLayoutFile1.GetNext(pos);
		strFileName = p->m_key;
		doc = p->m_value;

		pugi::xml_writer_buff writer;
		doc->print(writer,L"\t",pugi::format_default,pugi::encoding_utf16);
		SStringW *strxmlWnd= new SStringW(writer.buffer(),writer.size());

		if(DocSave.load_buffer(*strxmlWnd,wcslen(*strxmlWnd)*sizeof(wchar_t),pugi::parse_default,pugi::encoding_utf16)) 
		{
			pugi::xml_node NodeSave = DocSave.first_child();
			TrimXmlNodeTextBlank(NodeSave);
			RemoveWndName(NodeSave, FALSE, strFileName);

			FullFileName = m_strProPath + _T("\\") + strFileName;
			DocSave.save_file(FullFileName);
		}else
		{
			Debug(_T("保存文件失败：") + FullFileName);
		}
		delete strxmlWnd;

	}

	m_xmlDocUiRes.save_file(m_strUIResFile);
	Debug(_T("保存成功"));


	return TRUE;
}

//保存当前打开的布局文件
BOOL SDesignerView::SaveLayoutFile()
{
	if (m_strCurFile.IsEmpty())
	{
		return FALSE;
	}

	SStringT strFile = m_strCurFile;
	SStringT strFileName;
	SStringT FullFileName;

	SMap<SStringT, pugi::xml_document*>::CPair *p = m_mapLayoutFile1.Lookup(strFile);
	strFileName = p->m_key;
	pugi::xml_document *doc = p->m_value;

	pugi::xml_writer_buff writer;
	doc->print(writer,L"\t",pugi::format_default,pugi::encoding_utf16);
	SStringW *strxmlWnd= new SStringW(writer.buffer(),writer.size());


	pugi::xml_document DocSave;
	if(DocSave.load_buffer(*strxmlWnd,wcslen(*strxmlWnd)*sizeof(wchar_t),pugi::parse_default,pugi::encoding_utf16))
	{
		pugi::xml_node NodeSave = DocSave.root();
		TrimXmlNodeTextBlank(NodeSave);
	    RemoveWndName(NodeSave, FALSE);

		FullFileName = m_strProPath + _T("\\") + strFileName;
		DocSave.save_file(FullFileName);
	}else
	{
		Debug(_T("保存失败"));
	}

	delete strxmlWnd;

	m_xmlDocUiRes.save_file(m_strUIResFile);

	return TRUE;
}

//关闭当前打开的布局文件
BOOL SDesignerView::CloseLayoutFile()
{
	return TRUE;
}

//创建窗口
SMoveWnd* SDesignerView::CreateWnd(SWindow *pContainer,LPCWSTR pszXml)
{


	SWindow *pChild = pContainer->CreateChildren(pszXml);
	((SMoveWnd*)pChild)->m_Desiner = this;
	m_CurSelCtrl = (SMoveWnd*)pChild;
	return (SMoveWnd*)pChild;
}

void SDesignerView::RenameWnd(pugi::xml_node xmlNode, BOOL force)
{
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
		}else
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



void SDesignerView::RemoveWndName(pugi::xml_node xmlNode, BOOL bClear, SStringT strFileName)
{
	pugi::xml_node NodeChild = xmlNode.first_child();

	pugi::xml_attribute attr, attr1;
	pugi::xml_document doc;

	while (NodeChild)
	{  
		attr = NodeChild.attribute(L"uidesiner_data",false);
		attr1 = NodeChild.attribute(L"data", false);
		
		if (strFileName.IsEmpty())
		{
			strFileName = m_strCurFile;
		}

		SMap<SStringT, SMap<int, SStringT>* >::CPair *p = m_mapInclude1.Lookup(strFileName);
		SMap<int, SStringT>* pMap;
		SMap<int, SStringT>::CPair *p1;
		if (p)
		{
			pMap = p->m_value;
			p1 = pMap->Lookup(attr1.as_int());



		}else
		{
			Debug(_T("替换include出错"));
		}



		

		if (p1)
		{//如果这个控件是include

			if(!doc.load_buffer(p1->m_value,wcslen(p1->m_value)*sizeof(wchar_t),pugi::parse_default,pugi::encoding_utf16)) 
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

		}else
		{
			if (attr && _wcsicmp(NodeChild.name(),L"item") !=0 )
			{

				SStringT str;
				str = attr.value();
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

			if (_wcsicmp(NodeChild.name(),L"item")!=0)
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
		//替换Include
		if(_wcsicmp(NodeChild.name(),L"include")==0 && NodeChild.attribute(L"src"))
		{
			SStringT strInclude = NodeToStr(NodeChild);
			NodeChild.set_name(L"window");
			NodeChild.remove_attribute(L"src");
			NodeChild.append_attribute(L"pos").set_value(L"10,10,-10,-10");
			NodeChild.append_attribute(L"colorBkgnd").set_value(L"RGB(191,141,255)");
			NodeChild.append_attribute(L"colorBkgnd").set_value(L"RGB(191,141,255)");
			NodeChild.text().set(strInclude);

			RenameWnd(NodeChild);
		

			SMap<SStringT, SMap<int, SStringT>* >::CPair *p = m_mapInclude1.Lookup(m_strCurFile);
			if (p)
			{
				SMap<int, SStringT>* pMap = p->m_value;
				(*pMap)[NodeChild.attribute(L"data").as_int()] = strInclude;
			}else
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
	xmlNode.print(writer,L"\t",pugi::format_default,pugi::encoding_utf16);
	SStringW *strDebug= new SStringW(writer.buffer(),writer.size());
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
	xmlNode.print(writer,L"\t",pugi::format_default,pugi::encoding_utf16);
	SStringW *strDebug= new SStringW(writer.buffer(),writer.size());
	SStringT strtemp = *strDebug;
	delete strDebug;
	return strtemp;
}

SStringT SDesignerView::NodeToStr(pugi::xml_node xmlNode)
{
	pugi::xml_writer_buff writer;
	xmlNode.print(writer,L"\t",pugi::format_default,pugi::encoding_utf16);
	SStringW *strDebug= new SStringW(writer.buffer(),writer.size());
    SStringT str(*strDebug);
	delete strDebug;
	return str;
}


void SDesignerView::SetCurrentCtrl(pugi::xml_node xmlNode, SMoveWnd *pWnd)
{
	//m_xmlDoc.remove_child(m_xmlDoc.first_child());
 //   m_xmlDoc.append_copy(xmlNode);
	//m_xmlNode = m_xmlDoc.first_child();
	m_xmlNode= xmlNode;
	m_CurSelCtrl = pWnd;
	
	m_treeXmlStruct->GetEventSet()->unsubscribeEvent(EVT_TC_SELCHANGED,Subscriber(&SDesignerView::OnTCSelChanged,this));
	GoToXmlStructItem(m_CurSelCtrl->m_pRealWnd->GetUserData(), m_treeXmlStruct->GetRootItem());
	m_treeXmlStruct->GetEventSet()->subscribeEvent(EVT_TC_SELCHANGED,Subscriber(&SDesignerView::OnTCSelChanged,this));
}


void SDesignerView::UpdatePosToXmlNode(SWindow *pRealWnd, SMoveWnd* pMoveWnd)
{
	if (m_CurSelCtrl == m_pMoveWndRoot)
	{
		SwndLayout *pLayout = pRealWnd->GetLayout();
		CRect r;
		pMoveWnd->GetWindowRect(r);
		m_CurrentLayoutNode.attribute(_T("height")).set_value(pLayout->GetSpecifySize(PD_Y) - MARGIN*2);
		m_CurrentLayoutNode.attribute(_T("width")).set_value(pLayout->GetSpecifySize(PD_X) - MARGIN*2);

		return;
	}


	SwndLayout *pLayout = pRealWnd->GetLayout();

	
	SStringT s;
	s.Format(_T("%d"), pRealWnd->GetUserData());
	pugi::xml_node xmlNode = FindNodeByAttr(m_CurrentLayoutNode, L"data", s);
	if(!xmlNode)
	{
		return;
	}

	pugi::xml_attribute attrPos, attrSize, attrOffset, attrPos2type;

	SStringW strTemp;
	attrPos = xmlNode.attribute(L"pos");
	attrSize = xmlNode.attribute(L"size");
	attrOffset = xmlNode.attribute(L"offset");
	attrPos2type = xmlNode.attribute(L"pos2type");

	if (attrSize)
	{
		strTemp.Format(_T("%d, %d"), pLayout->GetSpecifySize(PD_X), pLayout->GetSpecifySize(PD_Y));
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
		strTemp.Format(_T("%g, %g"), pLayout->fOffsetX, pLayout->fOffsetY);
		attrOffset.set_value(strTemp);
	}

	if (attrPos)
	{   
		if (pLayout->nCount == 2)
		{
			strTemp = GetPosFromLayout(pLayout, 0) + _T(",");
			strTemp = strTemp + GetPosFromLayout(pLayout, 1);
			attrPos.set_value(strTemp);
		}else if (pLayout->nCount == 4)
		{
			strTemp = GetPosFromLayout(pLayout, 0) + _T(",");
			strTemp = strTemp + GetPosFromLayout(pLayout, 1) + _T(",");
			strTemp = strTemp + GetPosFromLayout(pLayout, 2) + _T(",");
			strTemp = strTemp + GetPosFromLayout(pLayout, 3);
			attrPos.set_value(strTemp);
		}
	}

	SetCurrentCtrl(xmlNode, pMoveWnd);

}

SStringW SDesignerView::GetPosFromLayout(SwndLayout *pLayout, INT nPosIndex)
{
	SStringW strPos;

	switch (pLayout->pos[nPosIndex].pit)
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
			strPos = strPos.Format(L"sib.left@%d:", pLayout->pos[nPosIndex].nRefID);
		}
		else
		{
			strPos = strPos.Format(L"sib.top@%d:", pLayout->pos[nPosIndex].nRefID);
		}

		break;

		//case PIT_SIB_TOP:      //兄弟结点的top，与left相同，用于Y
		//	break;

	case PIT_SIB_RIGHT:      //兄弟结点的right,用于X 
		if (2 == nPosIndex)
		{
			strPos = strPos.Format(L"sib.right@%d:", pLayout->pos[nPosIndex].nRefID);
		}
		else
		{
			strPos = strPos.Format(L"sib.bottom@%d:", pLayout->pos[nPosIndex].nRefID);
		}

		break;

		//case PIT_SIB_BOTTOM:      //兄弟结点的bottom,与right相同,用于Y 
		//	break;

	default:
		break;
	}

	if (pLayout->pos[nPosIndex].cMinus == -1)
	{
		strPos = strPos + L"-";
	}
	SStringW strTemp;
	int n = (int)pLayout->pos[nPosIndex].nPos;
	strTemp.Format(L"%d", n);
	strPos = strPos + strTemp;
	return strPos;
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

	SStringW s = L"<propgrid name=\"NAME_UIDESIGNER_PROPGRID_MAIN\" pos=\"0,0,-4,-4\" switchSkin=\"SKIN_UIDESIGNER_PROPGRID_SWITCH\"                      \
		nameWidth=\"100\" orderType=\"group\"   itemHeight=\"30\"   ColorGroup=\"RGB(96,112,138)\"                                          \
		ColorItemSel=\"rgb(234,128,16)\" colorItemSelText=\"#FF0000\" EditBkgndColor=\"rgb(87,104,132)\"                                    \
		autoWordSel=\"1\"> <cmdbtnstyle skin=\"_skin.sys.btn.normal\" colorText=\"RGB(96,112,138)\">...</cmdbtnstyle> </propgrid>";



	pugi::xml_document m_xmlDocProperty;

	pugi::xml_parse_result result = m_xmlDocProperty.load_file(L"Config\\property.xml");
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

		if(!doc->load_buffer(s,wcslen(s)*sizeof(wchar_t),pugi::parse_default,pugi::encoding_utf16))
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
		}else
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
				
				m_pPropgrid->SetFocus();


				m_pPropertyContainer->DestroyChild(m_pPropgrid);
				m_pPropgrid = NULL;
			}

			SStringT strTemp;
			strTemp = m_CurrentLayoutNode.name();

			if (strCtrlType.CompareNoCase(_T("hostwnd")) == 0 && strTemp.CompareNoCase(_T("SOUI")) !=0 )
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
				

				m_pPropgrid-> GetEventSet()->subscribeEvent(EventPropGridValueChanged::EventID,Subscriber(&SDesignerView::OnPropGridValueChanged,this));
				m_pPropgrid-> GetEventSet()->subscribeEvent(EventPropGridItemClick::EventID,Subscriber(&SDesignerView::OnPropGridItemClick,this));
				m_pPropgrid-> GetEventSet()->subscribeEvent(EventPropGridItemActive::EventID,Subscriber(&SDesignerView::OnPropGridItemActive,this));


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
					}else
					{
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
					}else
					{
						pItem->SetStringOnly(xmlAttr.value());
					}

				}

				xmlAttr = xmlAttr.next_attribute();
			}

	}else
	{
		pugi::xml_attribute xmlAttr = xmlNode.first_attribute();

		IPropertyItem *pItem = m_pPropgrid->GetGridItem(_T("UiEdit_windowText"));
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
				}else
				{
					pItem->SetStringOnly(xmlAttr.value());
				}



			}

			xmlAttr = xmlAttr.next_attribute();
		}
	}

	m_pPropgrid->Invalidate();
}


bool SDesignerView::OnPropGridValueChanged( EventArgs *pEvt )
{
	pugi:: xml_node xmlNode;
	BOOL bRoot = FALSE;

	IPropertyItem* pItem = ((EventPropGridValueChanged*)pEvt)->pItem;
	SStringT s = pItem->GetName2();  //属性名：pos skin name id 等等

	SStringT s1 = pItem->GetString();   //属性的值

	if (s.IsEmpty())
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
			if (strTemp.CompareNoCase(s) == 0)
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
				if (strTemp.CompareNoCase(s) == 0)
				{
					xmlNode = m_xmlNode;
				    bRoot = TRUE;
					break;
				}

			}
		}



	}else
	{
		xmlNode = m_xmlNode;
	}






	if (s.CompareNoCase(_T("UiEdit_windowText")) == 0)
	{
		xmlNode.text().set(s1);
	}

	SWindow * pWnd = ((SMoveWnd*)m_CurSelCtrl)->m_pRealWnd->GetParent();

	pugi::xml_attribute attr = xmlNode.attribute(s);
	if (attr)
	{
		if (s1.IsEmpty())
		{
			if (s.CompareNoCase(_T("data")) == 0)
			{
				xmlNode.attribute(_T("uidesiner_data")).set_value(_T(""));

			}else
			{
			  xmlNode.remove_attribute(s);
			}
		}else
		{
			if (s.CompareNoCase(_T("data")) == 0)
			{
				xmlNode.attribute(_T("uidesiner_data")).set_value(s1);

			}else
			{
				attr.set_value(s1);
			}
		}
	}
	else
	{
		if ((!s1.IsEmpty()) && (s.CompareNoCase(_T("UiEdit_windowText")) != 0))
		{
			xmlNode.append_attribute(s).set_value(s1);
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
	}else
	{
		SWindow *pRealWnd =FindChildByUserData(m_pRealWndRoot, data);

		SMap<SWindow*,SMoveWnd*>::CPair *p = m_mapMoveRealWnd.Lookup(pRealWnd);
		if (p)
		{	

			m_CurSelCtrl = p->m_value;
		}else
		{
			Debug(_T("出错了"));
		}
	}


	return true;
}

void SDesignerView::RefreshRes()
{
	m_xmlDocUiRes.load_file(m_strUIResFile);


	CAutoRefPtr<IResProvider>   pResProvider1;
	TCHAR *s = m_strProPath.GetBuffer(m_strProPath.GetLength());

    IResProvider*	pResProvider = SApplication::getSingletonPtr()->GetMatchResProvider(_T("UIDEF"),_T("XML_INIT"));

	SApplication::getSingletonPtr()->RemoveResProvider(pResProvider);

	CreateResProvider(RES_FILE, (IObjRef**)&pResProvider1);
	if (!pResProvider1->Init((LPARAM)s, 0))
	{
		Debug(_T("ResProvider初始化失败")); 
		return ;
	}
	SApplication::getSingletonPtr()->AddResProvider(pResProvider1);
}


bool SDesignerView::OnPropGridItemClick( EventArgs *pEvt )
{

	EventPropGridItemClick *pEvent = (EventPropGridItemClick*)pEvt;
	IPropertyItem* pItem = pEvent->pItem;
	SStringT strType = pEvent->strType;



	if (strType.CompareNoCase(_T("skin")) == 0)
	{
		SDlgSkinSelect DlgSkin(_T("layout:UIDESIGNER_XML_SKIN_SELECT"), pItem->GetString(), m_strUIResFile);
		if (DlgSkin.DoModal(m_pMainHost->m_hWnd) == IDOK)
		{
			SStringT s1 = pItem->GetString();   //属性的值

			if (s1.CompareNoCase(DlgSkin.m_strSkinName) != 0)
			{
				RefreshRes();
				pItem->SetString(DlgSkin.m_strSkinName);
				//ReLoadLayout();
			}

		}
		
	    

		//调用皮肤对话框
	}
	else if (strType.CompareNoCase(_T("font")) == 0)
	{
	
		//调用字体对话框
			SDlgFontSelect DlgFont(pItem->GetString());
			if (DlgFont.DoModal(m_pMainHost->m_hWnd) == IDOK)
			{
				pItem->SetString(DlgFont.m_strFont);
			}



	}	else if (strType.CompareNoCase(_T("class")) == 0)
	{

	}



	return true;
}

BOOL SDesignerView::ReLoadLayout()
{

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

		xmlnode=m_CurrentLayoutNode.child(L"root", false);
	}
	if(!xmlnode) return FALSE;


	SWindow *pChild = m_pContainer->GetWindow(GSW_FIRSTCHILD);
	while(pChild)
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
				swscanf(strVal,L"%d,%d",&nWidth,&nHeight);
			}else if (strWidth.CompareNoCase(attr.name()) == 0)
			{     
				//width属性
				::StrToIntExW(attr.value(),STIF_SUPPORT_HEX,&nWidth);               
			}else if (strHeight.CompareNoCase(attr.name()) == 0)
			{  
				//height属性
				::StrToIntExW(attr.value(),STIF_SUPPORT_HEX,&nHeight);
			}else if (strMargin.CompareNoCase(attr.name()) == 0)
			{  
				//忽略margin属性

			}else
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

		s2 = L"<window pos=\"20,20\" " +  s2 + strAttrSize + L"></window>";

	}else
	{
		int nWidth, nHeight;

		pugi::xml_attribute attrWorH = m_CurrentLayoutNode.attribute(_T("width"));

		if (attrWorH)
		{
			::StrToIntExW(attrWorH.value(),STIF_SUPPORT_HEX,&nWidth);
		}else
		{
			nWidth = 500;
			m_CurrentLayoutNode.append_attribute(_T("width")).set_value(nWidth);
		}

		attrWorH = m_CurrentLayoutNode.attribute(_T("height"));

		if (attrWorH)
		{
			::StrToIntExW(attrWorH.value(),STIF_SUPPORT_HEX,&nHeight);
		}else
		{
			nHeight = 500;
			m_CurrentLayoutNode.append_attribute(_T("height")).set_value(nHeight);
		}

		SStringW strAttrSize;
		strAttrSize.Format(L" margin= \"%d\" width = \"%d\" height = \"%d\" ", MARGIN, nWidth + MARGIN * 2, nHeight + MARGIN * 2);


		s2 = L"<window pos=\"20,20\" " + strAttrSize + L" colorBkgnd=\"#d0d0d0\"></window>";

			//s2 = L"<window pos=\"20,20,-20,-20\" colorBkgnd=\"#d0d0d0\"></window>";
	}

	//wchar_t *s = L"<window pos=\"20,20,@500,@500\" colorBkgnd=\"#d0d0d0\"></window>";
	wchar_t *s3 = L"<movewnd pos=\"20,20,@500,@500\" ></movewnd>";

	////创建布局窗口的根窗口

	m_pRealWndRoot = m_pContainer->CreateChildren(s2);
	m_pMoveWndRoot = (SMoveWnd *)m_pContainer->CreateChildren(s3);
	m_pMoveWndRoot->m_pRealWnd = m_pRealWndRoot;

	m_mapMoveRealWnd.RemoveAll();
	m_mapMoveRealWnd[m_pMoveWndRoot->m_pRealWnd] = m_pMoveWndRoot;



	m_pMoveWndRoot->m_Desiner = this;	

	if (!m_pRealWndRoot->CreateChildren(xmlnode))
	{
		return FALSE;
	}

	CreateAllChildWnd(m_pRealWndRoot, m_pMoveWndRoot);

	m_treeXmlStruct->RemoveAllItems();
	InitXMLStruct(m_CurrentLayoutNode, STVI_ROOT);

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


void SDesignerView::AddCodeToEditor(CScintillaWnd* pSciWnd)  //复制xml代码到代码编辑器
{
	m_strCurFileEditor = m_strCurFile;

	pSciWnd->SendMessage(SCI_CLEARALL, 0, 0);

	SStringA str;

	pugi::xml_document doc;
	SStringT strNodeName(_T("include"));

	if (m_xmlNode == m_CurrentLayoutNode && strNodeName.CompareNoCase(m_xmlNode.name()) == 0)
	{
		doc.append_copy(m_xmlNode);
	}else
	if (m_xmlNode == m_CurrentLayoutNode)
	{
		doc.append_copy(m_xmlNode.child(_T("root")));
	}else
	{
		doc.append_copy(m_xmlNode);
	}

	//Debug(doc.root().first_child());

	RemoveWndName(doc.root(), TRUE);
	TrimXmlNodeTextBlank(doc.root());


	pugi::xml_writer_buff writer;
	doc.first_child().print(writer,L"\t",pugi::format_default,pugi::encoding_utf16);
	SStringW *strDebug= new SStringW(writer.buffer(),writer.size());

	str=S_CW2A(*strDebug,CP_UTF8);
	pSciWnd->SendMessage(SCI_ADDTEXT, str.GetLength(),
		reinterpret_cast<LPARAM>((LPCSTR)str));
	delete strDebug;
}


void SDesignerView::GetCodeFromEditor(CScintillaWnd* pSciWnd)//从代码编辑器获取xml代码
{
	if (m_strCurFile.IsEmpty())
	{
		return;
	}

	if (m_strCurFileEditor.CompareNoCase(m_strCurFile) != 0)
	{
		return;
	}
	

	int n = pSciWnd->SendMessage(SCI_GETTEXT, 0,
		0);

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
	if(!doc.load_buffer(s1,wcslen(s1)*sizeof(wchar_t),pugi::parse_default,pugi::encoding_utf16))
	{
		Debug(_T("XML有语法错误！"));
		return;
	}



	RenameChildeWnd(doc.root());
	TrimXmlNodeTextBlank(doc.root());



	BOOL bRoot = FALSE;

	SStringT strNodeName(_T("include"));

	if (m_xmlNode == m_CurrentLayoutNode && strNodeName.CompareNoCase(m_xmlNode.name()) == 0)
	{
		pugi::xml_node xmlPNode;
		xmlPNode = m_xmlNode.parent();
		pugi::xml_node xmlNode = xmlPNode.insert_copy_after(doc.root().first_child(), m_xmlNode);

		xmlPNode.remove_child(m_xmlNode);
		m_xmlNode = xmlNode;
		m_CurrentLayoutNode = m_xmlNode;
	}else
	if (m_xmlNode == m_CurrentLayoutNode && strNodeName.CompareNoCase(m_xmlNode.name()) != 0)
	{
			pugi::xml_node fristNode = m_xmlNode.child(_T("root"));
			pugi::xml_node xmlNode = m_xmlNode.insert_copy_after(doc.root().first_child(), fristNode);
			m_xmlNode.remove_child(fristNode);
			bRoot = TRUE;

	}else
	{
			pugi::xml_node xmlPNode;
			xmlPNode = m_xmlNode.parent();
			pugi::xml_node xmlNode = xmlPNode.insert_copy_after(doc.root().first_child(), m_xmlNode);

			xmlPNode.remove_child(m_xmlNode);
			m_xmlNode = xmlNode;
	}


	ReLoadLayout();

	m_pMoveWndRoot->Click(0,CPoint(0,0));

	//SStringT strName = ((SMoveWnd*)m_CurSelCtrl)->m_pRealWnd->GetName();

	//ReLoadLayout();

	//if (bRoot)
	//{
	//	m_CurSelCtrl = m_pMoveWndRoot;
	//}else
	//{
	//	SWindow *pRealWnd =m_pRealWndRoot->FindChildByName2<SWindow>(strName);

	//	SMap<SWindow*,SMoveWnd*>::CPair *p = m_mapMoveRealWnd.Lookup(pRealWnd);
	//	if (p)
	//	{	

	//		m_CurSelCtrl = p->m_value;
	//	}else
	//	{
	//		Debug(_T("出错了"));
	//	}
	//}

	


}

void SDesignerView::SetSelCtrlNode(pugi::xml_node xmlNode)
{
	m_nState = 1;

	pugi::xml_writer_buff writer;
	xmlNode.print(writer,L"\t",pugi::format_default,pugi::encoding_utf16);
	SStringW *strxmlWnd= new SStringW(writer.buffer(),writer.size());

	if(m_xmlSelCtrlDoc.load_buffer(*strxmlWnd,wcslen(*strxmlWnd)*sizeof(wchar_t),pugi::parse_default,pugi::encoding_utf16)) 
	{

		m_xmlSelCtrlNode = m_xmlSelCtrlDoc.first_child();

		delete strxmlWnd;
	}else
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
	if(_wcsicmp(m_xmlNode.name(),L"include")==0 && m_xmlNode.attribute(L"src"))
	{
		bIsInclude = TRUE;
		SStringT strInclude = NodeToStr(m_xmlNode);
		m_xmlNode.set_name(L"window");
		m_xmlNode.remove_attribute(L"src");
		m_xmlNode.append_attribute(L"pos").set_value(L"10,10,-10,-10");
		m_xmlNode.append_attribute(L"colorBkgnd").set_value(L"RGB(191,141,255)");
		m_xmlNode.append_attribute(L"colorBkgnd").set_value(L"RGB(191,141,255)");
		m_xmlNode.text().set(strInclude);

		RenameWnd(m_xmlNode, TRUE);


		SMap<SStringT, SMap<int, SStringT>* >::CPair *p = m_mapInclude1.Lookup(m_strCurFile);
		if (p)
		{
			SMap<int, SStringT>* pMap = p->m_value;
			(*pMap)[m_xmlNode.attribute(L"data").as_int()] = strInclude;
		}else
		{
			Debug(_T("替换include出错"));
		}


	}else
	{

		//重命名控件
		RenameWnd(m_xmlNode, TRUE);
		RenameChildeWnd(m_xmlNode);

	}



	SWindow* pRealWnd;
	SMoveWnd* pMoveWnd;

	if(pM->m_pRealWnd == m_pRealWndRoot)
	{
		pRealWnd = pM->m_pRealWnd;
		pMoveWnd = pM;
	}else
	{
		SStringT s;
		s.Format(_T("%d"), pM->m_pRealWnd->GetUserData());
		pugi::xml_node xmlNodeRealWnd = FindNodeByAttr(m_CurrentLayoutNode, L"data", s);
		if (bIsContainerCtrl(xmlNodeRealWnd.name()))
		{
			pRealWnd = pM->m_pRealWnd;
			pMoveWnd = pM;
		}else
		{
			pRealWnd = pM->m_pRealWnd->GetParent();
			pMoveWnd = (SMoveWnd*)pM->GetParent();
		}
	}


	if (!bIsInclude)
	{
		CRect rect;
		pMoveWnd->GetWindowRect(rect);

		SStringT strPos;
		/*strPos.Format(_T("%d,%d"), pt.x - rect.left, pt.y - rect.top);*/

		//8 对齐
		strPos.Format(_T("%d,%d"), (pt.x - rect.left)/8*8, (pt.y - rect.top)/8*8);

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
	m_xmlNode.print(writer,L"\t",pugi::format_default,pugi::encoding_utf16);
	SStringW *strxmlWnd= new SStringW(writer.buffer(),writer.size());



	pRealWnd->CreateChildren(*strxmlWnd);
	SWindow *Wnd = pRealWnd->GetWindow(GSW_LASTCHILD);
	SMoveWnd *Wnd1 = (SMoveWnd *)CreateWnd(pMoveWnd, strMoveWnd);
	Wnd1->m_pRealWnd = Wnd;

	m_mapMoveRealWnd[Wnd] = Wnd1;
	CreateAllChildWnd(Wnd, Wnd1);


	//找到m_realWnd控件对应的xml节点
	if(pRealWnd == m_pRealWndRoot)
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

	m_nState = 0;


	delete strxmlWnd;
}

void SDesignerView::InitXMLStruct(pugi::xml_node xmlNode, HSTREEITEM item)
{
	if (!xmlNode)
	{
		return;
	}

	pugi::xml_node NodeSib = xmlNode;
	while (NodeSib)
	{
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

		HSTREEITEM itemChild = m_treeXmlStruct->InsertItem(strNodeName, item);  
		m_treeXmlStruct->SetItemData(itemChild, data);  


		InitXMLStruct(NodeSib.first_child(), itemChild);
		NodeSib = NodeSib.next_sibling();

	}
	m_treeXmlStruct->Invalidate();
}



BOOL SDesignerView::GoToXmlStructItem(int  data, HSTREEITEM item)
{
	    HSTREEITEM SibItem = item;

		while (SibItem)
		{
			int data1  = m_treeXmlStruct->GetItemData(SibItem);

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


bool SDesignerView::OnTCSelChanged(EventArgs *pEvt)
{
	if (!m_pContainer->GetParent()->IsVisible())
	{   //先这样写吧，有时间再改
		return true;
	}

	EventTCSelChanged *evt = (EventTCSelChanged*)pEvt;

	HSTREEITEM item =  m_treeXmlStruct->GetSelectedItem();

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

			m_CurSelCtrl->Click(0,CPoint(0,0));
		}


	}



	return true;
}


void SDesignerView::DeleteCtrl()
{
	if (m_xmlNode == m_CurrentLayoutNode)
	{
		return;
	}else
	{
		m_xmlNode.parent().remove_child(m_xmlNode);

		//Debug(m_CurrentLayoutNode);
		ReLoadLayout();
		m_nState = 0;
		GetMoveWndRoot()->Click(0, CPoint(0,0));
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
	GetMoveWndRoot()->Click(0, CPoint(0,0));
	m_pMoveWndRoot->GetParent()->Invalidate();

}

void SDesignerView::ShowZYGLDlg()
{
	if (m_strUIResFile.IsEmpty())
	{
		return;
	}
	SDlgSkinSelect DlgSkin(_T("layout:UIDESIGNER_XML_SKIN_SELECT"), _T(""), m_strUIResFile, FALSE);
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
	}else
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
	while(pChild)
	{
		if (pChild->GetUserData() == data)
			return pChild;
		pChild = pChild->GetWindow(GSW_NEXTSIBLING);
	}

	pChild = pWnd->GetWindow(GSW_FIRSTCHILD);
	while(pChild)
	{
		SWindow *pChildFind=FindChildByUserData(pChild,data);
		if(pChildFind) return pChildFind;
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


bool SDesignerView::OnPropGridItemActive( EventArgs *pEvt )
{
	EventPropGridItemActive *pEvent = (EventPropGridItemActive*)pEvt;
	IPropertyItem* pItem = pEvent->pItem;

	SStringT strDesc = pItem->GetDescription();
	SStringT strName = pItem->GetName1();

	if (strDesc.IsEmpty())
	{
		((CMainDlg*)m_pMainHost)->m_edtDesc->SetWindowText(strName);
	}else
	{

		((CMainDlg*)m_pMainHost)->m_edtDesc->SetWindowText(strDesc);
	}

	return true;
}
#include "stdafx.h"
#include "DlgSkinSelect.h"
#include "helper/SplitString.h"
#include "CDebug.h"

#include "helper\mybuffer.h"
#include "DlgNewSkin.h"
#include "DlgInput.h"
#include "propgrid/SPropertyGrid.h"
#include "ResManger.h"
#include "helpapi.h"

extern SStringT g_CurDir;

namespace SOUI
{
	SDlgSkinSelect::SDlgSkinSelect(LPCTSTR pszXmlName, SStringT strSkinName, SStringT strPath, BOOL bGetSkin) :SHostDialog(pszXmlName)
	{
		m_strSkinName = strSkinName;
		//m_xmlNodeUiRes = m_xmlDocUiRes.append_copy(xmlNode);
		//m_xmlNodeUiRes = xmlNode;
		//CDebug::Debug(m_xmlDocUiRes);
		m_pResFileManger = NULL;
		m_strProPath = strPath.Mid(0, strPath.ReverseFind(_T('\\')));
		m_strUIResFile = strPath;
		m_bGetSkin = bGetSkin;
	}

	//TODO:消息映射
	void SDlgSkinSelect::OnClose()
	{
		SHostDialog::OnCancel();
	}

	void SDlgSkinSelect::OnMaximize()
	{
		SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE);
	}

	void SDlgSkinSelect::OnRestore()
	{
		SendMessage(WM_SYSCOMMAND, SC_RESTORE);
	}

	void SDlgSkinSelect::OnMinimize()
	{
		SendMessage(WM_SYSCOMMAND, SC_MINIMIZE);
	}

	void SDlgSkinSelect::OnSize(UINT nType, CSize size)
	{
		SetMsgHandled(FALSE);

		SWindow *pBtnMax = FindChildByName(L"btn_max");
		SWindow *pBtnRestore = FindChildByName(L"btn_restore");
		if (!pBtnMax || !pBtnRestore) return;

		if (nType == SIZE_MAXIMIZED)
		{
			pBtnRestore->SetVisible(TRUE);
			pBtnMax->SetVisible(FALSE);
		}
		else if (nType == SIZE_RESTORED)
		{
			pBtnRestore->SetVisible(FALSE);
			pBtnMax->SetVisible(TRUE);
		}
	}

	void SDlgSkinSelect::OnOK()
	{
		if (m_bGetSkin)
		{
			if (m_lbResType->GetCurSel() == 0)
			{

				if (m_lbRes->GetCurSel() < 0)
				{
					CDebug::Debug(_T("请选择其中一项系统资源"));
					return;
				}

				m_strSkinName = GetLBCurSelText(m_lbRes);
			}
			else
			{
				if (m_lbSkin->GetCurSel() < 0)
				{
					CDebug::Debug(_T("请选择皮肤"));
					return;
				}

				SStringTList strList;
				SplitString(GetLBCurSelText(m_lbSkin), _T(':'), strList);
				if (strList.GetCount() != 2)
				{
					CDebug::Debug(_T("皮肤名不正确!"));
					return;
				}

				strList.GetAt(1).Trim();

				m_strSkinName = strList[1];
			}
		}

		Save();

		SHostDialog::OnOK();
	}

	void SDlgSkinSelect::Save()
	{
		m_pResFileManger->SaveRes();
	}

	BOOL SDlgSkinSelect::OnInitDialog(HWND wndFocus, LPARAM lInitParam)
	{
		m_lbResType = (SListBox *)FindChildByName(L"NAME_UIDESIGNER_LB_ZYLX");
		m_lbRes = (SListBox *)FindChildByName(L"NAME_UIDESIGNER_LB_ZY");
		m_lbSkin = (SListBox *)FindChildByName(L"NAME_UIDESIGNER_LB_SKIN");

		m_pEdit = (SEdit *)FindChildByName(L"NAME_UIDESIGNER_edit_SEARCH");

		m_imgView = (SImgCanvas *)FindChildByName(L"NAME_UIDESIGNER_IMG_VIEW");
		m_txtImageSize = (SStatic *)FindChildByName(L"sel_image_size");

		LRESULT lr = m_pEdit->SSendMessage(EM_SETEVENTMASK, 0, ENM_CHANGE);
		m_pEdit->GetEventSet()->subscribeEvent(EventRENotify::EventID, Subscriber(&SDlgSkinSelect::OnReNotify, this));

		m_wndGridContainer = (SWindow *)FindChildByName(L"NAME_UIDESIGNER_GRID_CONTAINER");

		m_lbResType->GetEventSet()->subscribeEvent(EVT_LB_SELCHANGED, Subscriber(&SDlgSkinSelect::OnLbResTypeSelChanged, this));
		m_lbRes->GetEventSet()->subscribeEvent(EVT_LB_SELCHANGED, Subscriber(&SDlgSkinSelect::OnLbResSelChanged, this));
		m_lbSkin->GetEventSet()->subscribeEvent(EVT_LB_SELCHANGED, Subscriber(&SDlgSkinSelect::OnLbSkinSelChanged, this));

		m_pResFileManger->LoadUIResFromFile(m_strUIResFile);

		LoadSysSkin();
		InitResType();
		LoadSkinProperty();

		if (!m_strSkinName.IsEmpty())
		{
			//如果传进了的皮肤名不为空，定位到皮肤
			GoToSkin();
		}

		return TRUE;
	}

	void SDlgSkinSelect::InitResType()
	{
		m_lbResType->AddString(_T("内置皮肤"));
		m_lbResType->AddString(_T("所有图片"));
		m_lbResType->AddString(_T("所有颜色"));

		pugi::xml_node xmlNode = m_pResFileManger->m_xmlNodeUiRes.child(L"resource").first_child();

		while (xmlNode)
		{
			if (xmlNode.type() != pugi::node_element)
			{
				xmlNode = xmlNode.next_sibling();
				continue;
			}

			m_lbResType->AddString(xmlNode.name());

			xmlNode = xmlNode.next_sibling();
		}
	}

	void SDlgSkinSelect::LoadSysSkin()
	{
		pugi::xml_parse_result resutl = m_xmlDocSysSkin.load_file(g_CurDir + L"Config\\theme_sys_res\\sys_xml_skin.xml");
		if (!resutl)
		{
			SMessageBox(NULL, _T("加载SysSkin.xml文件失败"), _T("加载SysSkin.xml文件失败"), MB_OK);
			return;
		}
	}

	bool SDlgSkinSelect::OnLbResTypeSelChanged(EventArgs *pEvtBase)
	{
		DestroyGrid();
		EventLBSelChanged *pEvt = (EventLBSelChanged*)pEvtBase;
		SListBox *listbox = (SListBox*)pEvt->sender;
		pugi::xml_node xmlNode;

		if (pEvt->nNewSel == 0)
		{
			m_lbRes->DeleteAll();
			m_mapSysSkin.RemoveAll();

			xmlNode = m_xmlDocSysSkin.child(L"skin").first_child();
			while (xmlNode)
			{
				if (xmlNode.type() != pugi::node_element)
				{
					xmlNode = xmlNode.next_sibling();
					continue;
				}

				SStringT *strData = new SStringT(xmlNode.name());

				m_lbRes->AddString(xmlNode.attribute(L"name").value(), -1, (LPARAM)strData);

				if (xmlNode.attribute(L"src"))
				{
					SStringTList strlist;
					SStringT s = xmlNode.attribute(L"src").value();
					SplitString(s, _T(':'), strlist);

					if (strlist.GetCount() == 2)
					{
						m_mapSysSkin[xmlNode.attribute(L"name").value()] = strlist[1] + _T(".png");
					}
				}

				xmlNode = xmlNode.next_sibling();
			}
		}
		else if (pEvt->nNewSel == 1)
		{	// 所有图片资源
			m_lbRes->DeleteAll();

			SPOSITION pos = m_pResFileManger->m_mapResFile.GetStartPosition();
			while (pos)
			{
				const SMap<SStringT, SStringT>::CPair * item = m_pResFileManger->m_mapResFile.GetAt(pos);
				SStringT *strData = new SStringT(item->m_key);

				m_lbRes->AddString(item->m_value, -1, (LPARAM)strData);

				m_pResFileManger->m_mapResFile.GetNext(pos);
			}
		}
		else if (pEvt->nNewSel == 2)
		{	// 所有颜色
			m_lbRes->DeleteAll();

			xmlNode = m_pResFileManger->GetResFirstNode(_T("color"));
			while (xmlNode)
			{
				if (xmlNode.type() != pugi::node_element)
				{
					xmlNode = xmlNode.next_sibling();
					continue;
				}

				SStringT s2, s3;
				s2 = xmlNode.name();
				s3 = xmlNode.attribute(L"value").value();

				SStringT *strData = new SStringT(xmlNode.attribute(L"src").value());
				m_lbRes->AddString(s2 + _T(":  ") + s3, -1, (LPARAM)strData);

				xmlNode = xmlNode.next_sibling();
			}
		}
		else
		{
			SStringT strText;
			listbox->GetText(pEvt->nNewSel, strText);
			m_lbRes->DeleteAll();
			xmlNode = m_pResFileManger->m_xmlNodeUiRes.child(L"resource").child(strText).first_child();
			while (xmlNode)
			{
				if (xmlNode.type() != pugi::node_element)
				{
					xmlNode = xmlNode.next_sibling();
					continue;
				}
				SStringT *strData = new SStringT(xmlNode.attribute(L"name").value());
				m_lbRes->AddString(xmlNode.attribute(L"path").value(), -1, (LPARAM)strData);

				xmlNode = xmlNode.next_sibling();
			}
		}

		m_lbSkin->DeleteAll();
		return true;
	}

	bool SDlgSkinSelect::OnLbResSelChanged(EventArgs *pEvtBase)
	{
		// 清除旧结果
		DestroyGrid();
		m_lbSkin->DeleteAll();
		m_imgView->Clear();
		m_txtImageSize->SetWindowText(L"");

		EventLBSelChanged *pEvt = (EventLBSelChanged*)pEvtBase;
		SListBox *listbox = (SListBox*)pEvt->sender;
		pugi::xml_node xmlNode;
		if (pEvt->nNewSel == -1)
		{
			return false;
		}
		SStringT *s = (SStringT *)listbox->GetItemData(pEvt->nNewSel);

		SStringT strTemp(*s);

		if (m_lbResType->GetCurSel() == 0)
		{
			m_lbSkin->AddString(strTemp + _T(":  ") + GetLBCurSelText(m_lbRes));

			ShowImage();
			m_lbRes->SetFocus();
			return true;
		}
		if (m_lbResType->GetCurSel() == 2)
		{
			SStringT strColor = GetLBCurSelText(m_lbRes);
			strColor = ResManger::RemoveResTypename(strColor);
			m_imgView->SetAttribute(_T("colorBkgnd"), strColor);
			return true;
		}

		SStringT strResName = GetLBCurSelText(m_lbResType);

		if (strTemp.Find(_T(":")) == -1)
			strTemp = strResName + _T(":") + strTemp;

		xmlNode = m_pResFileManger->GetResFirstNode(_T("skin"));

		while (xmlNode)
		{
			if (xmlNode.type() != pugi::node_element)
			{
				xmlNode = xmlNode.next_sibling();
				continue;
			}

			if (strTemp.CompareNoCase(xmlNode.attribute(L"src").value()) == 0)
			{
				SStringT s2, s3;
				s2 = xmlNode.name();
				s3 = xmlNode.attribute(L"name").value();

				SStringT *strData = new SStringT(xmlNode.attribute(L"src").value());

				m_lbSkin->AddString(s2 + _T(":  ") + s3, -1, (LPARAM)strData);
			}

			xmlNode = xmlNode.next_sibling();
		}

		if (m_lbSkin->GetCount() > 0)
		{
			m_lbSkin->SetCurSel(0);
			EventLBSelChanged evt(m_lbSkin);
			evt.nOldSel = 0;
			evt.nNewSel = 0;
			m_lbSkin->FireEvent(evt);
		}
		m_lbRes->SetFocus();

		ShowImage();
		return true;
	}

	bool SDlgSkinSelect::OnLbSkinSelChanged(EventArgs *pEvtBase)
	{
		if (m_lbResType->GetCurSel() == 0)
		{
			return true;
		}

		EventLBSelChanged *pEvt = (EventLBSelChanged*)pEvtBase;
		SListBox *listbox = (SListBox*)pEvt->sender;
		pugi::xml_node xmlNode;

		int n = m_lbSkin->GetCurSel();
		if (n < 0)
		{
			return true;
		}

		SStringT *sSrc = (SStringT *)m_lbSkin->GetItemData(n);

		SStringT strSrc(*sSrc);

		SStringT strSkinName;

		m_lbSkin->GetText(n, strSkinName);

		SStringTList strLst;
		SplitString(strSkinName, _T(':'), strLst);

		SStringT strSkinType = strLst.GetAt(0).Trim();
		SStringT strSkin = strLst.GetAt(1).Trim();

		xmlNode = m_pResFileManger->GetResFirstNode(_T("skin"));
		while (xmlNode)
		{
			if (xmlNode.type() != pugi::node_element)
			{
				xmlNode = xmlNode.next_sibling();
				continue;
			}
			if (xmlNode.attribute(L"name"))
			{
				if (strSkinType.CompareNoCase(xmlNode.name()) == 0)
				{
					if (strSkin.CompareNoCase(xmlNode.attribute(L"name").value()) == 0 &&
						strSrc.CompareNoCase(xmlNode.attribute(L"src").value()) == 0)
					{
						m_xmlNodeCurSkin = xmlNode;
					}
				}
			}

			xmlNode = xmlNode.next_sibling();
		}

		DestroyGrid();

		xmlNode = m_xmlDocSkinProperty.child(L"root").child(strSkinType);
		if (xmlNode)
		{
			if (xmlNode)
			{
				m_wndGridContainer->CreateChildren(xmlNode);
				m_pgGrid = (SPropertyGrid *)m_wndGridContainer->GetWindow(GSW_FIRSTCHILD);
				m_wndGridContainer->Invalidate();
				m_pgGrid->GetEventSet()->subscribeEvent(EventPropGridValueChanged::EventID, Subscriber(&SDlgSkinSelect::OnPropGridValueChanged, this));
				UpdatePropGrid();
			}
		}

		return true;
	}


	void SDlgSkinSelect::LoadSkinProperty()
	{
		pugi::xml_parse_result resutl = m_xmlDocSkinProperty.load_file(g_CurDir + L"Config\\SkinProperty.xml");
		if (!resutl)
		{
			SMessageBox(NULL, _T("加载SkinProperty.xml文件失败"), _T("加载SkinProperty.xml文件失败"), MB_OK);
			return;
		}
	}

	void SDlgSkinSelect::UpdatePropGrid()
	{
		if (m_pgGrid == NULL)
		{
			return;
		}

		m_pgGrid->ClearAllGridItemValue();

		pugi::xml_attribute xmlAttr = m_xmlNodeCurSkin.first_attribute();

		while (xmlAttr)
		{
			SStringT str = xmlAttr.name();
			IPropertyItem *pItem = m_pgGrid->GetGridItem(str.MakeLower());
			if (pItem)
			{
				pItem->SetStringOnly(xmlAttr.value());
			}

			xmlAttr = xmlAttr.next_attribute();
		}

		m_pgGrid->Invalidate();
	}


	void SDlgSkinSelect::OnZYLXNew()
	{
		SStringT strName;
		SDlgInput dlg;
		if (IDOK != dlg.DoModal(m_hWnd))
		{
			return;
		}

		strName = dlg.m_strValue;
		pugi::xml_node xmlNode = m_pResFileManger->m_xmlNodeUiRes.child(L"resource");
		if (xmlNode.child(strName))
		{
			CDebug::Debug(_T("该资源类型已存在！"));
			return;
		}

		xmlNode = xmlNode.append_child(strName);
		int n = m_lbResType->AddString(strName);
		SelectLBItem(m_lbResType, n);
	}

	void SDlgSkinSelect::OnZYLXDel()
	{
		if (m_lbResType->GetCurSel() < 0)
		{
			CDebug::Debug(_T("请选择资源类型"));
			return;
		}

		if (m_lbResType->GetCurSel() == 0)
		{
			CDebug::Debug(_T("不能选择内置皮肤类型"));
			return;
		}

		if (m_lbRes->GetCount() > 0)
		{
			CDebug::Debug(_T("请先删除资源"));
			return;
		}

		pugi::xml_node xmlNode = m_pResFileManger->m_xmlNodeUiRes.child(L"resource").child(GetLBCurSelText(m_lbResType));
		if (xmlNode)
		{
			xmlNode.parent().remove_child(xmlNode);
			m_lbResType->DeleteString(m_lbResType->GetCurSel());
			/*m_lbResType->Invalidate();*/
			return;
		}
	}
	
	void SDlgSkinSelect::OnZYNew()
	{
		if (m_lbResType->GetCurSel() < 0)
		{
			CDebug::Debug(_T("请选择资源类型"));
			return;
		}

		SStringT strResType = GetLBCurSelText(m_lbResType);
		if (m_lbResType->GetCurSel() <= 2)
		{
			CDebug::Debug(_T("不能选择") + strResType + _T("类型"));
			return;
		}

		CFileDialogEx OpenDlg(TRUE, NULL, NULL, 6, _T("所有文件 (*.*)\0*.*\0\0"));
		if (IDOK == OpenDlg.DoModal())
		{
			SStringT strFileName = OpenDlg.m_szFileName;
			int n = strFileName.Find(m_strProPath);
			if (n != 0)
			{
				SMessageBox(NULL, _T("请将资源放到uires目录下"), _T("提示"), MB_OK);
				return;
			}

			SStringT strFile = strFileName.Mid(m_strProPath.GetLength() + 1);

			pugi::xml_node xmlNode = m_pResFileManger->m_xmlNodeUiRes.child(L"resource").child(strResType).first_child();
			pugi::xml_node xmlNewNode;

			while (xmlNode)
			{
				if (xmlNode.type() != pugi::node_element)
				{
					xmlNode = xmlNode.next_sibling();
					continue;
				}

				if (strFile.CompareNoCase(xmlNode.attribute(L"path").value()) == 0)
				{
					break;
				}

				xmlNode = xmlNode.next_sibling();
			}

			if (!xmlNode)
			{
				// 如果该不存在该类型的资源，则添加	
				xmlNewNode = m_pResFileManger->m_xmlNodeUiRes.child(L"resource").child(strResType).append_child(L"file");

				SStringT strResName = GetFilename(strFile);
				strResName.Replace(_T("\\"), _T("_"));
				strResName.Replace(_T("."), _T("_"));

				xmlNewNode.append_attribute(L"name").set_value(strResName);
				xmlNewNode.append_attribute(L"path").set_value(strFile);

				SStringT *strData = new SStringT(strResName);
				m_lbRes->AddString(strFile, -1, (LPARAM)strData);
				//CDebug::Debug(xmlNewNode);
			}

			//定位到资源
			{
				SStringT strResText;
				for (int i = 0; i < m_lbRes->GetCount(); i++)
				{
					m_lbRes->GetText(i, strResText);
					if (strResText.Compare(strFile) != 0)
						continue;

					m_lbRes->SetCurSel(i);

					EventLBSelChanged evt(m_lbRes);
					evt.nOldSel = 0;
					evt.nNewSel = i;
					m_lbRes->FireEvent(evt);
					m_lbRes->EnsureVisible(i);

					if (m_lbSkin->GetCount() > 0)
					{
						m_lbSkin->SetCurSel(0);
						EventLBSelChanged evt1(m_lbSkin);
						evt1.nOldSel = 0;
						evt1.nNewSel = 0;
						m_lbSkin->FireEvent(evt1);
					}
					m_lbRes->SetFocus();

					break;
				}	// end for
			}
		}
	}
	void SDlgSkinSelect::OnZYDel()
	{
		if (m_lbResType->GetCurSel() == 0)
		{
			CDebug::Debug(_T("内置资源不能删除"));
			return;
		}

		SStringT strResText = GetLBCurSelText(m_lbRes);
		if (strResText.IsEmpty())
		{
			return;
		}

		SStringT strResType = GetLBCurSelText(m_lbResType);
		if (strResType.IsEmpty())
		{
			return;
		}

		if (m_lbResType->GetCurSel() <= 2)
		{
			CDebug::Debug(_T("不能选择") + strResType + _T("类型"));
			return;
		}

		int nResult = SMessageBox(NULL, _T("确定要删除资源吗?"), _T("提示"), MB_OKCANCEL);
		if (nResult != 1)
		{
			return;
		}

		if (m_lbSkin->GetCount() > 0)
		{
			SMessageBox(NULL, _T("请先移除皮肤"), _T("提示"), MB_OK);
			return;
		}

		pugi::xml_node xmlNode = m_pResFileManger->m_xmlNodeUiRes.child(L"resource").child(strResType).first_child();
		while (xmlNode)
		{
			if (xmlNode.type() != pugi::node_element)
			{
				xmlNode = xmlNode.next_sibling();
				continue;
			}
			if (strResText.CompareNoCase(xmlNode.attribute(L"path").value()) == 0)
			{

				pugi::xml_node xmlNodeP = xmlNode.parent();
				xmlNodeP.remove_child(xmlNode);
				m_lbRes->DeleteString(m_lbRes->GetCurSel());
				return;
			}
			xmlNode = xmlNode.next_sibling();
		}

	}

	//新建皮肤
	void SDlgSkinSelect::OnSkinNew()
	{
		if (m_lbResType->GetCurSel() == 0)
		{
			CDebug::Debug(_T("内置皮肤不能修改"));
			return;
		}

		if (m_lbRes->GetCurSel() < 0)
		{
			CDebug::Debug(_T("请先选择资源"));
			return;
		}

		if (m_lbSkin->GetCount() > 0)
		{
			CDebug::Debug(_T("一个资源只能对应一个皮肤"));
			return;
		}

		SStringT strSkinTypeName;
		SDlgNewSkin DlgNewSkin(_T("layout:UIDESIGNER_XML_NEW_SKIN"));
		if (IDOK == DlgNewSkin.DoModal(m_hWnd))
		{
			strSkinTypeName = DlgNewSkin.m_strSkinName;
		}
		else
		{
			return;
		}

		pugi::xml_node xmlNode;
		xmlNode = m_pResFileManger->GetResFirstNode(_T("skin"));

		SStringT strSrc;
		strSrc = GetLBCurSelText(m_lbResType) + _T(":");
		SStringT *s = (SStringT *)m_lbRes->GetItemData(m_lbRes->GetCurSel());
		strSrc = strSrc + *s;
		SStringT skinName = _T("skin_");
		skinName += *s;
		//判断当前资源已被其他皮肤引用
		pugi::xml_node NodeTemp = xmlNode.find_child_by_attribute(_T("src"), strSrc);
		if (NodeTemp)
		{
			CDebug::Debug(_T("当前资源已被其他皮肤引用:") + CDebug::Debug1(NodeTemp));
			return;
		}

		xmlNode = xmlNode.parent().append_child(strSkinTypeName);

		xmlNode.append_attribute(_T("name")).set_value(skinName);
		xmlNode.append_attribute(_T("src")).set_value(strSrc);

		SStringT *strData = new SStringT(strSrc);
		int n = m_lbSkin->AddString(strSkinTypeName + _T(": ") + skinName, -1, (LPARAM)strData);
		SelectLBItem(m_lbSkin, n);
	}

	//删除皮肤
	void SDlgSkinSelect::OnSkinDel()
	{
		if (m_lbResType->GetCurSel() == 0)
		{
			CDebug::Debug(_T("内置皮肤不能删除"));
			return;
		}

		if (m_lbSkin->GetCurSel() < 0)
		{
			return;
		}

		int nResult = SMessageBox(NULL, _T("确定要删除皮肤吗?"), _T("提示"), MB_OKCANCEL);
		if (nResult != 1)
		{
			return;
		}

		SStringTList strList;
		SplitString(GetLBCurSelText(m_lbSkin), _T(':'), strList);

		for (int i = 0; i < strList.GetCount(); i++)
		{
			strList.GetAt(i).TrimBlank();
		}

		pugi::xml_node xmlNode;
		xmlNode = m_pResFileManger->GetResFirstNode(_T("skin"));

		while (xmlNode)
		{
			if (xmlNode.type() != pugi::node_element)
			{
				xmlNode = xmlNode.next_sibling();
				continue;
			}

			if (strList[0].CompareNoCase(xmlNode.name()) == 0)
			{
				pugi::xml_attribute attr = xmlNode.attribute(_T("name"));
				if (attr)
				{
					if (strList[1].CompareNoCase(attr.value()) == 0)
					{
						xmlNode.parent().remove_child(xmlNode);
						SelectLBItem(m_lbRes, m_lbRes->GetCurSel());
						break;
					}
				}
			}

			xmlNode = xmlNode.next_sibling();
		}
	}

	void SDlgSkinSelect::DestroyGrid()
	{
		m_pgGrid = (SPropertyGrid *)m_wndGridContainer->GetWindow(GSW_FIRSTCHILD);
		if (m_pgGrid)
		{
			m_pgGrid->SetFocus();

			//Debug(tempDoc->root().first_child());

			m_wndGridContainer->DestroyChild(m_pgGrid);
			m_pgGrid = NULL;
		}
		m_imgView->SetAttribute(_T("colorBkgnd"), _T("@color/imgprviewbg"));
	}


	SStringT SDlgSkinSelect::GetLBCurSelText(SListBox * lb)
	{
		SStringT s(_T(""));
		int n = lb->GetCurSel();

		if (n < 0)
		{
			return s;
		}

		lb->GetText(n, s);
		return s;
	}


	void SDlgSkinSelect::SelectLBItem(SListBox * lb, int nIndex)
	{
		//优先查找系统皮肤
		lb->SetCurSel(nIndex);
		EventLBSelChanged evt(lb);
		evt.nOldSel = lb->GetCurSel();
		evt.nNewSel = nIndex;
		lb->FireEvent(evt);
		lb->EnsureVisible(nIndex);
	}


	bool SDlgSkinSelect::OnReNotify(EventArgs *pEvt)
	{
		//过滤edit事件通知
		if (m_lbResType->GetCurSel() < 0)
		{
			return true;
		}

		EventRENotify *pReEvt = (EventRENotify*)pEvt;
		if (pReEvt->iNotify == EN_CHANGE)
		{
			SStringT strValue = m_pEdit->GetWindowText();
			m_lbRes->DeleteAll();

			if (m_lbResType->GetCurSel() == 0)
			{
				pugi::xml_node xmlNode = m_xmlDocSysSkin.child(L"skin").first_child();
				while (xmlNode)
				{
					if (xmlNode.type() != pugi::node_element)
					{
						xmlNode = xmlNode.next_sibling();
						continue;
					}
					SStringT strPath = xmlNode.attribute(L"name").value();

					if (strPath.Find(strValue) >= 0)
					{
						SStringT *strData = new SStringT(xmlNode.name());

						m_lbRes->AddString(xmlNode.attribute(L"name").value(), -1, (LPARAM)strData);
					}

					xmlNode = xmlNode.next_sibling();
				}
			}
			else
			{
				pugi::xml_node  xmlNode = m_pResFileManger->m_xmlNodeUiRes.child(L"resource").child(GetLBCurSelText(m_lbResType)).first_child();
				while (xmlNode)
				{
					if (xmlNode.type() != pugi::node_element)
					{
						xmlNode = xmlNode.next_sibling();
						continue;
					}
					SStringT strPath = xmlNode.attribute(L"path").value();

					if (strPath.Find(strValue) >= 0)
					{
						SStringT *strData = new SStringT(xmlNode.attribute(L"name").value());
						m_lbRes->AddString(strPath, -1, (LPARAM)strData);
					}

					xmlNode = xmlNode.next_sibling();
				}
			}

		}
		return true;
	}


	void SDlgSkinSelect::GoToSkin()
	{
		//优先查找系统资源
		pugi::xml_node xmlNode = m_xmlDocSysSkin.child(L"skin").first_child();
		while (xmlNode)
		{
			if (xmlNode.type() != pugi::node_element)
			{
				xmlNode = xmlNode.next_sibling();
				continue;
			}
			if (m_strSkinName.CompareNoCase(xmlNode.attribute(L"name").value()) == 0)
			{
				SelectLBItem(m_lbResType, 0);
				SStringT strResText;

				for (int i = 0; i < m_lbRes->GetCount(); i++)
				{
					m_lbRes->GetText(i, strResText);
					if (m_strSkinName.CompareNoCase(strResText) == 0)
					{
						SelectLBItem(m_lbRes, i);
						return;
					}
				}

				return;
			}

			xmlNode = xmlNode.next_sibling();

		}


		//if (m_xmlDocSysSkin.child(L"skin").child(m_strSkinName))
		//{
		//	//优先查找系统皮肤

		//	SelectLBItem(m_lbResType, 0);
		//	SStringT strResText;

		//	for (int i = 0; i < m_lbRes->GetCount(); i++)
		//	{
		//		m_lbRes->GetText(i, strResText);
		//		if (m_strSkinName.CompareNoCase(strResText) == 0)
		//		{
		//			SelectLBItem(m_lbRes, i);
		//			break;
		//		}
		//	}

		//}else
		{
			pugi::xml_node xmlNode;
			SStringT strSrc;

			xmlNode = m_pResFileManger->GetResFirstNode(_T("skin"));

			while (xmlNode)
			{
				if (xmlNode.type() != pugi::node_element)
				{
					xmlNode = xmlNode.next_sibling();
					continue;
				}
				if (m_strSkinName.CompareNoCase(xmlNode.attribute(L"name").value()) == 0)
				{
					strSrc = xmlNode.attribute(L"src").value();
					break;
				}

				xmlNode = xmlNode.next_sibling();
			}

			if (!xmlNode)
			{
				CDebug::Debug(m_strSkinName + _T("找不到!"));
				return;
			}


			SStringTList strList;
			SplitString(strSrc, _T(':'), strList);

			for (int i = 0; i < strList.GetCount(); i++)
			{
				strList.GetAt(i).TrimBlank();
			}

			int n = GetLbIndexFromText(m_lbResType, strList[0]);
			if (n == -1)
			{
				CDebug::Debug(_T("未知的资源类型!"));
				return;
			}

			SelectLBItem(m_lbResType, n);

			SStringT strPath;

			xmlNode = m_pResFileManger->m_xmlNodeUiRes.child(L"resource").child(strList[0]).first_child();
			while (xmlNode)
			{
				if (xmlNode.type() != pugi::node_element)
				{
					xmlNode = xmlNode.next_sibling();
					continue;
				}

				if (strList[1].CompareNoCase(xmlNode.attribute(L"name").value()) == 0)
				{
					strPath = xmlNode.attribute(L"path").value();

					break;
				}

				xmlNode = xmlNode.next_sibling();
			}

			if (!xmlNode)
			{
				CDebug::Debug(_T("未知的资源名称"));
				return;
			}

			n = GetLbIndexFromText(m_lbRes, strPath);
			if (n == -1)
			{
				CDebug::Debug(_T("未知的资源!"));
				return;
			}

			SelectLBItem(m_lbRes, n);

			//n = GetLbIndexFromText(m_lbRes, strList[1]);
			//if (n == -1)
			//{
			// CDebug::Debug(_T("未知的皮肤!"));
			// return;
			//}

			//SelectLBItem(m_lbRes)
		}
	}


	int SDlgSkinSelect::GetLbIndexFromText(SListBox *lb, SStringT strText)
	{
		int n = -1;
		if (lb->GetCount() == 0)
		{
			return -1;
		}

		SStringT strLbText;
		for (int i = 0; i < lb->GetCount(); i++)
		{
			lb->GetText(i, strLbText);
			if (strLbText.CompareNoCase(strText) == 0)
			{
				n = i;
				break;
			}
		}

		return n;
	}


	void SDlgSkinSelect::ShowImage()
	{
		SStringT strImgname = GetLBCurSelText(m_lbRes);
		if (m_lbResType->GetCurSel() == 0)
		{
			SMap<SStringT, SStringT>::CPair *p = m_mapSysSkin.Lookup(strImgname);
			if (!p)
			{
				return;
			}

			strImgname = g_CurDir + _T("Config\\theme_sys_res\\") + p->m_value;
		}
		else
		{
			strImgname = m_strProPath + _T("\\") + strImgname;
		}

		m_imgView->Clear();
		m_imgView->AddFile(strImgname);
		CSize imgSize = m_imgView->GetImgInfo();
		SStringT strSize;
		strSize.Format(L"%d * %d", imgSize.cx, imgSize.cy);
		m_txtImageSize->SetWindowText(strSize);
	}

	bool SDlgSkinSelect::OnPropGridValueChanged(EventArgs *pEvt)
	{
		if (m_lbResType->GetCurSel() == 0)
		{
			return true;
		}

		pugi::xml_node xmlNode;

		IPropertyItem* pItem = ((EventPropGridValueChanged*)pEvt)->pItem;
		SStringT s = pItem->GetName2();  //属性名：pos skin name id 等等

		SStringT s1 = pItem->GetString();   //属性的值

		//修改name的时候判断是否存在
		if (s.CompareNoCase(_T("name")) == 0)
		{
			pugi::xml_attribute attrScale = m_xmlNodeCurSkin.attribute(_T("scale"));
			SStringT strScale = _T("");
			if (attrScale)
			{
				strScale = attrScale.value();
			}

			if (ChekSkin(s1, strScale))
			{
				SStringT strError;
				strError = strError.Format(_T("已经存在皮肤名:%s Scale:%s的皮肤"), s1, strScale);
				CDebug::Debug(strError);
				return false;
			}
		}

		//修改scale的时候判断是否存在
		if (s.CompareNoCase(_T("scale")) == 0)
		{
			pugi::xml_attribute attrName = m_xmlNodeCurSkin.attribute(_T("name"));
			SStringT strName = _T("");
			if (attrName)
			{
				strName = attrName.value();
			}

			if (ChekSkin(strName, s1))
			{
				SStringT strError;
				strError = strError.Format(_T("已经存在皮肤名:%s Scale:%s的皮肤"), strName, s1);
				CDebug::Debug(strError);
				return false;
			}
		}

		if (s.IsEmpty())
		{
			return false;
		}


		xmlNode = m_xmlNodeCurSkin;


		pugi::xml_attribute attr = xmlNode.attribute(s);
		if (attr)
		{
			if (s1.IsEmpty())
			{

				xmlNode.remove_attribute(s);
			}
			else
			{
				attr.set_value(s1);
			}
		}
		else
		{
			if (!s1.IsEmpty())
			{
				xmlNode.append_attribute(s).set_value(s1);
			}
		}



		return true;
	}


	bool SDlgSkinSelect::ChekSkin(SStringT strName, SStringT strScale)
	{
		pugi::xml_node xmlNode;
		xmlNode = m_pResFileManger->GetResFirstNode(_T("skin"));

		while (xmlNode)
		{
			if (xmlNode.type() != pugi::node_element)
			{
				xmlNode = xmlNode.next_sibling();
				continue;
			}

			if (xmlNode == m_xmlNodeCurSkin)
			{
				xmlNode = xmlNode.next_sibling();
				continue;
			}

			if (xmlNode.attribute(L"name"))
			{
				if (strName.CompareNoCase(xmlNode.attribute(L"name").value()) == 0)
				{
					SStringT strScale1 = _T("");
					if (xmlNode.attribute(L"scale"))
					{
						strScale1 = xmlNode.attribute(L"scale").value();
					}
					if (strScale.CompareNoCase(strScale1) == 0)
					{
						return true;
					}
				}
			}

			xmlNode = xmlNode.next_sibling();
		}

		return false;
	}

}



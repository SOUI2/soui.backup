#pragma once
#include <Dbghelp.h>
#pragma comment(lib,"Dbghelp.lib")
#include <helper/SAdapterBase.h>
struct ImportTableItemData
{	
	bool bGroup;//是否是一个分组
	SStringT strIdx;//函数索引
	SStringT strName;//函数名
};
class CImportTableTreeViewAdapter :public STreeAdapterBase<ImportTableItemData>
{
public:	
	CImportTableTreeViewAdapter() {}
	~CImportTableTreeViewAdapter() {}
	
	virtual void getView(SOUI::HTREEITEM loc, SWindow * pItem, pugi::xml_node xmlTemplate)
	{
		ItemInfo & ii = m_tree.GetItemRef((HSTREEITEM)loc);
		int itemType = getViewType(loc);
		if (pItem->GetChildrenCount() == 0)
		{
			switch (itemType)
			{
			case 0:xmlTemplate = xmlTemplate.child(L"item_group");
				break;
			case 1:xmlTemplate = xmlTemplate.child(L"item_data");
				break;
			}
			pItem->InitFromXml(xmlTemplate);
		}
		if (itemType == 0)
		{			
			SToggle *pSwitch = pItem->FindChildByName2<SToggle>(L"tgl_switch");
			
			pSwitch->SetVisible(ii.data.bGroup);
			pSwitch->SetToggle(IsItemExpanded(loc));
			pItem->FindChildByName(L"txt_dll_name")->SetWindowText(ii.data.strName);
			pItem->GetEventSet()->subscribeEvent(EVT_CMD, Subscriber(&CImportTableTreeViewAdapter::OnGroupPanleClick, this));//OnGroupPanleClick
		}
		else {
			SWindow * pWnd = pItem->FindChildByName2<SWindow>(L"txt_red");
			if (pWnd) pWnd->SetWindowText(S_CW2T(ii.data.strName));			
			pItem->FindChildByName(L"txt_idx")->SetWindowTextW(ii.data.strIdx);
			pItem->FindChildByName(L"txt_fun_name")->SetWindowTextW(ii.data.strName);
		}
	}

	bool OnItemPanleClick(EventArgs *pEvt)
	{
		SItemPanel *pItemPanel = sobj_cast<SItemPanel>(pEvt->sender);
		SASSERT(pItemPanel);
		pItemPanel->ModifyState(WndState_Check, 0);
		STreeView *pTreeView = (STreeView*)pItemPanel->GetContainer();
		SOUI::HTREEITEM loc = (SOUI::HTREEITEM)pItemPanel->GetItemIndex();
		if (pTreeView)
		{
			pTreeView->SetSel(loc, TRUE);
		}
		return true;
	}

	int Updata(LPBYTE lpBaseAddress)
	{
		m_tree.DeleteAllItems();
		notifyBranchChanged(ITEM_ROOT);
		int i, j;
		
		if (lpBaseAddress == NULL)
		{
			return -1;
		}
		PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)lpBaseAddress;
		PIMAGE_NT_HEADERS32 pNtHeaders = (PIMAGE_NT_HEADERS32)(lpBaseAddress + pDosHeader->e_lfanew);
		//测试一下是不是一个有效的PE文件
		if (pDosHeader->e_magic != IMAGE_DOS_SIGNATURE || IMAGE_NT_SIGNATURE != pNtHeaders->Signature)
		{			
			return -1;
		}

		if (pNtHeaders->FileHeader.Machine == IMAGE_FILE_MACHINE_I386)
		{
			//导入表的rva
			DWORD Rva_import_table = pNtHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
			if (Rva_import_table == 0)
			{
				return -1;
			}
			PIMAGE_IMPORT_DESCRIPTOR pImportTable = (PIMAGE_IMPORT_DESCRIPTOR)ImageRvaToVa(
				pNtHeaders,
				lpBaseAddress,
				Rva_import_table,
				NULL
			);
			IMAGE_IMPORT_DESCRIPTOR null_iid;
			IMAGE_THUNK_DATA null_thunk;
			memset(&null_iid, 0, sizeof(null_iid));
			memset(&null_thunk, 0, sizeof(null_thunk));
			//每个元素代表了一个引入的DLL。
			ImportTableItemData data;
			for (i = 0; memcmp(pImportTable + i, &null_iid, sizeof(null_iid)) != 0; i++)
			{
				//LPCSTR: 就是 const char*
				LPCSTR szDllName = (LPCSTR)ImageRvaToVa(
					pNtHeaders, lpBaseAddress,
					pImportTable[i].Name, //DLL名称的RVA
					NULL);
				data.bGroup = TRUE;
				data.strName = S_CA2T(szDllName);
				HSTREEITEM hDll = InsertItem(data);
				// 		SetItemExpanded(hRoot, FALSE);			
				//IMAGE_TRUNK_DATA 数组（IAT：导入地址表）前面
				PIMAGE_THUNK_DATA32 pThunk = (PIMAGE_THUNK_DATA32)ImageRvaToVa(
					pNtHeaders, lpBaseAddress,
					pImportTable[i].OriginalFirstThunk,
					NULL);
				int iFunCount = 0;
				for (j = 0; memcmp(pThunk + j, &null_thunk, sizeof(null_thunk)) != 0; j++)
				{
					//这里通过RVA的最高位判断函数的导入方式，
					//如果最高位为1，按序号导入，否则按名称导入
					if (pThunk[j].u1.AddressOfData & IMAGE_ORDINAL_FLAG32)
					{
						data.bGroup = false;
						data.strIdx.Format(L"%ld", pThunk[j].u1.AddressOfData & 0xffff);
						data.strName = L"按序号导入";
						InsertItem(data, hDll);
					}
					else
					{
						//按名称导入，我们再次定向到函数序号和名称
						//注意其地址不能直接用，因为仍然是RVA！
						PIMAGE_IMPORT_BY_NAME pFuncName = (PIMAGE_IMPORT_BY_NAME)ImageRvaToVa(
							pNtHeaders, lpBaseAddress,
							pThunk[j].u1.AddressOfData,
							NULL);
						data.bGroup = false;
						data.strIdx.Format(L"%ld", pFuncName->Hint);
						data.strName = S_CA2T((char*)pFuncName->Name);
						InsertItem(data, hDll);
					}
					iFunCount = j + 1;
				}
				SStringT strFunCount;
				strFunCount.Format(L"(%d)", iFunCount);
				m_tree.GetItemRef(hDll).data.strName += strFunCount;
			}
			notifyBranchChanged(ITvAdapter::ITEM_ROOT);
		}
		if (pNtHeaders->FileHeader.Machine == IMAGE_FILE_MACHINE_IA64 ||
			pNtHeaders->FileHeader.Machine == IMAGE_FILE_MACHINE_AMD64)
		{
			//PIMAGE_NT_HEADERS64 pNtHeaders64 = (PIMAGE_NT_HEADERS64)(lpBaseAddress + pDosHeader->e_lfanew);
			//导入表的rva
			DWORD Rva_import_table = ((PIMAGE_NT_HEADERS64)pNtHeaders)->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;

			if (Rva_import_table == 0)
			{
				return -1;
			}
			PIMAGE_IMPORT_DESCRIPTOR pImportTable = (PIMAGE_IMPORT_DESCRIPTOR)ImageRvaToVa(
				pNtHeaders,
				lpBaseAddress,
				Rva_import_table,
				NULL
			);
			IMAGE_IMPORT_DESCRIPTOR null_iid;
			//64位和32的不同之处
			IMAGE_THUNK_DATA64 null_thunk;
			memset(&null_iid, 0, sizeof(null_iid));
			memset(&null_thunk, 0, sizeof(null_thunk));
			//每个元素代表了一个引入的DLL。
			ImportTableItemData data;
			for (i = 0; memcmp(pImportTable + i, &null_iid, sizeof(null_iid)) != 0; i++)
			{
				//LPCSTR: 就是 const char*
				LPCSTR szDllName = (LPCSTR)ImageRvaToVa(
					pNtHeaders, lpBaseAddress,
					pImportTable[i].Name, //DLL名称的RVA
					NULL);
				data.bGroup = TRUE;
				data.strName = S_CA2T(szDllName);
				HSTREEITEM hDll = InsertItem(data);
				// 		SetItemExpanded(hRoot, FALSE);			
				//IMAGE_TRUNK_DATA 数组（IAT：导入地址表）前面
				PIMAGE_THUNK_DATA64 pThunk = (PIMAGE_THUNK_DATA64)ImageRvaToVa(
					pNtHeaders, lpBaseAddress,
					pImportTable[i].OriginalFirstThunk,
					NULL);
				int iFunCount = 0;
				for (j = 0; memcmp(pThunk + j, &null_thunk, sizeof(null_thunk)) != 0; j++)
				{
					//这里通过RVA的最高位判断函数的导入方式，
					//如果最高位为1，按序号导入，否则按名称导入
					if (pThunk[j].u1.AddressOfData & IMAGE_ORDINAL_FLAG64)
					{
						data.bGroup = false;
						data.strIdx.Format(L"%ld", IMAGE_ORDINAL64(pThunk[j].u1.AddressOfData));
						data.strName = L"按序号导入";
						InsertItem(data, hDll);
					}
					else
					{
						//按名称导入，我们再次定向到函数序号和名称
						//注意其地址不能直接用，因为仍然是RVA！
						PIMAGE_IMPORT_BY_NAME pFuncName = (PIMAGE_IMPORT_BY_NAME)ImageRvaToVa(
							pNtHeaders, lpBaseAddress,
							pThunk[j].u1.AddressOfData,
							NULL);
						data.bGroup = false;
						data.strIdx.Format(L"%ld", pFuncName->Hint);
						data.strName = S_CA2T((char*)pFuncName->Name);
						InsertItem(data, hDll);
					}
					iFunCount = j + 1;
				}
				SStringT strFunCount;
				strFunCount.Format(L"(%d)", iFunCount);
				m_tree.GetItemRef(hDll).data.strName += strFunCount;
			}
			notifyBranchChanged(ITvAdapter::ITEM_ROOT);
		}		
		
		return 0;
	}
	
	void HandleTreeViewContextMenu(CPoint pt, SItemPanel * pItem, HWND hWnd)
	{
		if (pItem)
		{
			SOUI::HTREEITEM loc = (SOUI::HTREEITEM)pItem->GetItemIndex();
			ItemInfo & ii = m_tree.GetItemRef((HSTREEITEM)loc);
			if (ii.data.bGroup)
			{
				SMenuEx menu;
				menu.LoadMenu(_T("smenuex:menuex_group"));
				int iCmd = menu.TrackPopupMenu(TPM_RETURNCMD, pt.x, pt.y, hWnd);
				ImportTableItemData data;
				switch (iCmd)
				{
				case 1:  //添加分组
				{
					SOUI::HTREEITEM hParent = GetParentItem(loc);					
					data.bGroup = true;
					data.strName = L"添加测试组";
					SOUI::HTREEITEM hItem=InsertItem(data,hParent,loc);
					notifyBranchChanged(hParent);
				}break;
				case 2:  //删除分组
				{
					SOUI::HTREEITEM hParent = GetParentItem(loc);
					DeleteItem(loc);
					notifyBranchChanged(hParent);
				}
				break;
				case 3:  //分组下添加一个子项
				{									
					data.bGroup = false;
					data.strIdx = L"9527";
					data.strName = L"添加测试子项";
					SOUI::HTREEITEM hItem = InsertItem(data, loc);
					notifyBranchChanged(loc);
				}
				break;
				case 4:  //分组下添加一个子分组
				{
					data.bGroup = true;
					data.strIdx = L"9527";
					data.strName = L"添加测试子项";
					SOUI::HTREEITEM hItem = InsertItem(data, loc);
					notifyBranchChanged(loc);
				}
				break;
				}
			}
			else
			{
				SMenuEx menu;
				menu.LoadMenu(_T("smenuex:menuex_item"));
				int iCmd = menu.TrackPopupMenu(TPM_RETURNCMD, pt.x, pt.y, hWnd);
				ImportTableItemData data;
				switch (iCmd)
				{
				case 1:
				{					
					SOUI::HTREEITEM hParent = GetParentItem(loc);
					data.bGroup = false;
					data.strIdx = L"008";
					data.strName = L"添加测试子项";
					SOUI::HTREEITEM hItem = InsertItem(data, hParent, loc);
					notifyBranchChanged(hParent);
				}
				break;
				case 2:  //删除
				{
					SOUI::HTREEITEM hParent = GetParentItem(loc);
					DeleteItem(loc);
					notifyBranchChanged(hParent);
				}break;
				}
			}
		}	
		else
		{
			SMenuEx menu;
			menu.LoadMenu(_T("smenuex:menuex_none"));
			int iCmd = menu.TrackPopupMenu(TPM_RETURNCMD, pt.x, pt.y, hWnd);
			switch (iCmd)
			{
			case 1:  //添加分组
			{
				ImportTableItemData data;
				data.bGroup = true;
				data.strName = L"空白添加测试组";
				SOUI::HTREEITEM hItem = InsertItem(data);
				notifyBranchChanged(ITEM_ROOT);
			}			
			}
		}
	}

	bool OnGroupPanleClick(EventArgs *pEvt)
	{
		SItemPanel *pItem = sobj_cast<SItemPanel>(pEvt->sender);
		SToggle *pSwitch = pItem->FindChildByName2<SToggle>(L"tgl_switch");

		SOUI::HTREEITEM loc = (SOUI::HTREEITEM)pItem->GetItemIndex();
		ExpandItem(loc, ITvAdapter::TVC_TOGGLE);
		pSwitch->SetToggle(IsItemExpanded(loc));
		return true;
	}

	bool OnSwitchClick(EventArgs *pEvt)
	{
		SToggle *pToggle = sobj_cast<SToggle>(pEvt->sender);
		SASSERT(pToggle);
		SItemPanel *pItem = sobj_cast<SItemPanel>(pToggle->GetRoot());
		SASSERT(pItem);
		SOUI::HTREEITEM loc = (SOUI::HTREEITEM)pItem->GetItemIndex();
		ExpandItem(loc, ITvAdapter::TVC_TOGGLE);
		return true;
	}

	virtual int getViewType(SOUI::HTREEITEM hItem) const
	{
		ItemInfo & ii = m_tree.GetItemRef((HSTREEITEM)hItem);
		if (ii.data.bGroup) return 0;
		else return 1;
	}
	
	virtual int getViewTypeCount() const
	{
		return 2;
	}
	
};
#pragma once
#include "ScintillaWnd.h"
#include "Scintilla.h"
#include "layout/SouiLayout.h"
#include "layout/SLinearLayout.h"
#include "SDesignerRoot.h"

namespace SOUI
{
	class SMoveWnd;
	class SPropertyGrid;

	class SDesignerView
	{
	private:
		CAutoRefPtr<SStylePool> m_privateStylePool; /**<局部style pool*/
		CAutoRefPtr<SSkinPool>  m_privateSkinPool;  /**<局部skin pool*/
	public:
		SDesignerView(SHostDialog *pMainHost, SWindow *pContainer, STreeCtrl *pTreeXmlStruct);
		~SDesignerView();

		SMoveWnd* m_CurSelCtrl;  //用户当前选择的控件
		SList<SWindow*> m_CopyList;

		//保存当前打开的布局文件
		bool SaveLayoutFile();
		BOOL SaveAll();

		//关闭当前打开的布局文件
		BOOL CloseLayoutFile();

		// 打开工程
		BOOL OpenProject(SStringT strFileName);
		BOOL CloseProject();
		

		BOOL InsertLayoutToMap(SStringT);

		BOOL NewLayout(SStringT strResName, SStringT strPath);  //新建dialog 或include布局

		BOOL LoadLayout(SStringT strFileName);   //加载布局

		//创建Root窗口
		SMoveWnd* CreateWnd(SUIWindow *pContainer, LPCWSTR pszXml);
		void CreateAllChildWnd(SUIWindow *pRealWnd, SMoveWnd *pMoveWnd);

		//重命名每一个控件的名字
		void RenameChildeWnd(pugi::xml_node xmlNode);
		void RenameWnd(pugi::xml_node xmlNode, BOOL force = FALSE);
		void RenameAllLayoutWnd();
		void RemoveWndName(pugi::xml_node xmlNode, BOOL bClear, SStringT strFileName = _T(""));

		void UpdatePosToXmlNode(SUIWindow *pRealWnd, SMoveWnd* pMoveWnd);//移动控件的同时将控件位置写入xml节点

		//调试用
		void Debug(pugi::xml_node);
		void Debug(SStringT str);
		SStringT Debug1(pugi::xml_node);

		SStringT NodeToStr(pugi::xml_node xmlNode);
		SStringT NodeToStr(pugi::xml_document xmlNode);

		void SetCurrentCtrl(pugi::xml_node xmlNode, SMoveWnd *pWnd); //设置当前选中的控件

		SStringW GetPosFromLayout(SouiLayoutParam *pLayoutParam, INT nPosIndex);

		//通过控件的属性值找到该控件对应的xml节点
		pugi::xml_node FindNodeByAttr(pugi::xml_node NodeRoot, SStringT attrName, SStringT attrValue);

		void BindXmlcodeWnd(SWindow *pXmlTextCtrl);   //绑定界面代码编辑窗口

		void ShowNoteInSciwnd();		

		void InitProperty(SWindow *pPropertyContainer);   //初始化属性列表
		void InitCtrlProperty(pugi::xml_node NodeCom, pugi::xml_node NodeCtrl);

		void CreatePropGrid(SStringT strCtrlType);
		void UpdatePropGrid(pugi::xml_node xmlNode);

		bool OnPropGridValueChanged(EventArgs *pEvt);
		bool OnPropGridItemClick(EventArgs *pEvt);
		bool OnPropGridItemActive(EventArgs *pEvt);
		bool OnTCSelChanged(EventArgs *pEvt);

		BOOL ReLoadLayout(BOOL bClearSel=FALSE);
		BOOL bIsContainerCtrl(SStringT strCtrlName); //判断控件是否是容器控件

		void SaveEditorCaretPos();

		void RestoreEditorCaretPos();

		SMoveWnd* GetMoveWndRoot() { return m_pMoveWndRoot; };
		SWindow* GetRealWndRoot() { return m_pRealWndRoot; };

		void AddCodeToEditor(CScintillaWnd* pSciWnd);  //复制xml代码到代码编辑器
		void GetCodeFromEditor(CScintillaWnd* pSciWnd);//从代码编辑器获取xml代码

		void SetSelCtrlNode(pugi::xml_node xmlNode);

		void NewWnd(CPoint pt, SMoveWnd *pM);

		int InitXMLStruct(pugi::xml_node xmlNode, HSTREEITEM item);
		BOOL GoToXmlStructItem(int data, HSTREEITEM item);

		void DeleteCtrl();
		void Preview();
		void unPreview();

		void RefreshRes();
		void ShowZYGLDlg();
		void ShowYSGLDlg();

		void ShowMovWndChild(BOOL bShow, SMoveWnd* pMovWnd);

		int GetIndexData();

		SWindow* FindChildByUserData(SWindow* pWnd, int data);

		void TrimXmlNodeTextBlank(pugi::xml_node xmlNode);

		void UseEditorUIDef(bool bYes);  //使用编辑器自身的UIDef还是使用所打开的工程的UIDef
		SStringT UnitToStr(int nUnit);

	public:
		CAutoRefPtr<IFont> m_defFont;

		SDesignerRoot *m_pRealWndRoot;       //布局容器窗口;

		SMoveWnd  *m_pMoveWndRoot; //布局窗口的根窗口

		int		 m_nSciCaretPos;		//代码编辑窗口光标位置

		BOOL     m_bChange;    //文件是否被修改,如果被修改需要保存
		BOOL     m_bPage;      //是否为页文件,否则为窗口文件
		CPoint   m_downPt;     //按下的位置
		INT      m_nState;     //是否正在进行创建控件的鼠标动作； 1:是；0:否

		SStringW *m_strxml;
		pugi::xml_node m_xmlNode;   //当前选中的控件的xmlnode 
		pugi::xml_document m_xmlSelCtrlDoc;//鼠标选择控件列表要创建的控件的xml
		pugi::xml_node m_xmlSelCtrlNode;  //鼠标选择控件列表要创建的控件的xml

		SMap<SStringT, pugi::xml_node> m_mapLayoutFile;
		SMap<SStringT, pugi::xml_document *> m_mapLayoutFile1;
		//SMap<int, SStringT> m_mapInclude;    //保存include节点
		SMap<SStringT, SMap<int, SStringT>* > m_mapInclude1;    //保存每一个布局文件对应的include map;

		SMap<SWindow*, SMoveWnd*> m_mapMoveRealWnd;

		SStringT m_strUIResFile;   //C:\demos\MyTest\uires\uires.idx
		SStringT m_strProPath;     //C:\demos\MyTest\uires\

		SStringT m_strCurLayoutXmlFile; //当前打开的窗体文件名  xml\main.xml
		SStringT m_strCurFileEditor; //当前代码编辑器打开代码对应的文件  xml\main.xml

		//CAutoRefPtr<IResProvider>   pResProvider;
		SUIWindow *m_pContainer;  //所有布局窗口根的容器 
		pugi::xml_node m_CurrentLayoutNode;

		SMap<SStringT, pugi::xml_document*> m_mapCtrlProperty;//所有控件的属性列表 <Button, xmlnode> <Check, xmlNode>
		SWindow *m_pPropertyContainer;     //属性面板父窗口

		CScintillaWnd *m_pScintillaWnd;	//XML代码编辑窗口

		SStringT m_strCurrentCtrlType; //当前选中的控件类型 "button" "check"等等
		SPropertyGrid *m_pPropgrid;    //属性面板

		SHostDialog* m_pMainHost;

		pugi::xml_document m_xmlDocUiRes;  //uires文件

		SList<SStringT> m_lstSouiProperty;   //SOUI节点的属性列表  在property.xml hostwnd节点中定义
		SList<SStringT> m_lstRootProperty;   //Root节点的属性列表 
		SList<SStringT> m_lstContainerCtrl;  //容器控件列表，非容器控件，上面不能摆放控件 在Ctrl.xml中定义

		STreeCtrl *m_treeXmlStruct; //显示xml文档结构的tree控件

		int m_ndata; //这个值用来标识xmlnode的每一个节点，节点属性为data,xmlnode的这个属性值是唯一的;

		CAutoRefPtr<IUiDefInfo> m_pUiDef;  //加载工程的UIdef

		CAutoRefPtr<IUiDefInfo>  m_pOldUiDef;//编辑器自身的UiDef
	};
}
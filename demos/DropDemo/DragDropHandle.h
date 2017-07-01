#pragma once
#include "oleidl.h"
#include "ShObjIdl.h"
#include <string>
#include <vector>

/*
#define	DROPEFFECT_NONE	( 0 )    

#define	DROPEFFECT_COPY	( 1 )

#define	DROPEFFECT_MOVE	( 2 )

#define	DROPEFFECT_LINK	( 4 )
*/
typedef DWORD					DROPEFFECT;


//这是  主窗口 的接口  主窗口 继承该接口  然后就可以做出处理了。
/*我还是建议在 主窗口继承这个  而不是分发到具体控件
因为拖放 一般都伴随着业务逻辑.
放在窗口的基类里  基类判断 具体哪个子控件 后 写个再写到 虚函数  窗口类重载*/
class IDropTargetIF
{
public:
	//************************************
	// Method:    OnDragEnter  判断是否可以接受一个拖操作，以及接受之后的效果
	// FullName:  IDropTargetIF::OnDragEnter
	// Access:    virtual public 
	// Returns:   HRESULT 返回 S_FALSE 表示不能拖放   S_OK 表示可以 
	// Qualifier:
	// Parameter: IDataObject * pDataObject  /指向源数据对象的接口指针 剪切板 数据结构 
	// Parameter: DWORD dwKeyState				// 当前键盘修饰符的状态 按键 状态  和 MK_LBUTTON宏 一样的值
	// Parameter: const POINT & point				屏幕的坐标 需要自己转换
	//************************************
	virtual HRESULT OnDragEnter(IDataObject* pDataObject, DWORD dwKeyState, const POINT& point) PURE;
	
	//************************************
	// Method:    OnDragOver  提供通过DoDragDrop函数执行的目标反馈
	// FullName:  IDropTargetIF::OnDragOver
	// Access:    virtual public 
	// Returns:   DROPEFFECT 返回 DROPEFFECT_COPY 类似的值  用来显示 图标类型  有移动 复制 等 
	// Qualifier:
	// Parameter: IDataObject * pDataObject		剪切板 数据结构
	// Parameter: DWORD dwKeyState					按键 状态  和 MK_LBUTTON宏 一样的值
	// Parameter: const POINT & point					屏幕的坐标 需要自己转换  检测具体 在哪个控件上
	// Parameter: std::wstring & szMessage			拖放图标右边的操作类型名称。比如 复制到 或上传到
	// Parameter: std::wstring & szInsert				拖放图标右边的目标名称 。比如 123文件夹   
	//************************************
	virtual DROPEFFECT OnDragOver(IDataObject* pDataObject,
		DWORD dwKeyState,
		const POINT& point, 
		std::wstring& szMessage,
		std::wstring& szInsert) PURE;
	// 数据放进目标窗口
	virtual BOOL OnDrop(IDataObject* pDataObject, DWORD dwKeyState, const POINT& point)PURE;
	//导致一个drop目标挂起它的返回行为
	virtual void OnDragLeave() {}
};


class DropTargetEx : public IDropTarget
{
public:
	DropTargetEx(IDropTargetIF* iDropTargetIF);
	~DropTargetEx(void);  
	bool DragDropRegister(HWND hWnd, DWORD AcceptKeyState = MK_LBUTTON);  
	bool DragDropRevoke(HWND hWnd);  
	HRESULT	STDMETHODCALLTYPE QueryInterface(REFIID riid, __RPC__deref_out void **ppvObject);  
	ULONG		STDMETHODCALLTYPE AddRef();  
	ULONG		STDMETHODCALLTYPE Release();  
	//进入  
	HRESULT	STDMETHODCALLTYPE DragEnter(__RPC__in_opt IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, __RPC__inout DWORD *pdwEffect);  
	//移动  
	HRESULT	STDMETHODCALLTYPE DragOver(DWORD grfKeyState, POINTL pt, __RPC__inout DWORD *pdwEffect);  
	//离开  
	HRESULT	STDMETHODCALLTYPE DragLeave();  
	//释放  
	HRESULT	STDMETHODCALLTYPE Drop(__RPC__in_opt IDataObject *pDataObj, DWORD grfKeyState, POINTL pt, __RPC__inout DWORD *pdwEffect);  

private:
	//设置拖放过程中的描述 信息
	bool SetDropDescription(DROPIMAGETYPE nImageType, LPCWSTR lpszText, LPCWSTR lpszInsert=NULL);
private:  
	HWND								m_hWnd;  
	IDropTargetHelper*				m_piDropHelper;  
	DWORD								m_dwAcceptKeyState;  
	IDataObject*						m_pDataObj;
	IDropTargetIF*					m_pIDropTargetHandle;
protected:
	
};



/////////////////////////////// 拖放 操作类 ///////////////////////////////////////////
class DataObjectEx : public IDataObject
{
public:
	// IUnknown members
	HRESULT STDMETHODCALLTYPE QueryInterface (REFIID riid, void ** ppvObject);
	ULONG   STDMETHODCALLTYPE AddRef (void);
	ULONG   STDMETHODCALLTYPE Release (void);

	// IDataObject members
	HRESULT STDMETHODCALLTYPE GetData (FORMATETC* pFormatEtc, STGMEDIUM* pMedium);
	HRESULT STDMETHODCALLTYPE GetDataHere (FORMATETC* pFormatEtc, STGMEDIUM* pMedium);
	HRESULT STDMETHODCALLTYPE QueryGetData (FORMATETC *pFormatEtc);
	HRESULT STDMETHODCALLTYPE GetCanonicalFormatEtc (FORMATETC* pFormatEct, FORMATETC* pFormatEtcOut);
	HRESULT STDMETHODCALLTYPE SetData (FORMATETC* pFormatEtc, STGMEDIUM* pMedium,  BOOL fRelease);
	HRESULT STDMETHODCALLTYPE EnumFormatEtc (DWORD dwDirection, IEnumFORMATETC** ppEnumFormatEtc);
	HRESULT STDMETHODCALLTYPE DAdvise (FORMATETC* pFormatEtc, DWORD advf, IAdviseSink*, DWORD *);
	HRESULT STDMETHODCALLTYPE DUnadvise (DWORD dwConnection);
	HRESULT STDMETHODCALLTYPE EnumDAdvise (IEnumSTATDATA** ppEnumAdvise);

	// Constructor / Destructor
	DataObjectEx();
	~DataObjectEx();
public:
	bool CacheSingleFileAsHdrop(LPCTSTR lpszFilePath);			//设置单个文件 到剪切板
	// 开始 拖放 
	DROPEFFECT DoDragDrop(DROPEFFECT dwEffect);
	bool SetDragImage(HBITMAP hBitmap, POINT* pPoint=NULL, COLORREF clr=GetSysColor(COLOR_WINDOW));
	bool SetDragImageWindow(HWND hWnd, POINT* pPoint=NULL);

private:
	int	 FindFmtEtc(const FORMATETC* pFormatEtc);
	void CopyMedium(STGMEDIUM* pMedDest, STGMEDIUM* pMedSrc, FORMATETC* pFmtSrc);
private:
	std::vector<FORMATETC*>			m_vctFormatEtc;
	std::vector<STGMEDIUM*>			m_vctStgMedium;
	IDragSourceHelper*						m_pDragSourceHelper;
	IDragSourceHelper2*					m_pDragSourceHelper2;		// Drag image helper 2 (SetFlags function)
};

//
class EnumFormatEtcEx : public IEnumFORMATETC
{
public:
	EnumFormatEtcEx(const std::vector<FORMATETC>& arrFE);
	EnumFormatEtcEx(const std::vector<FORMATETC*>& arrFE);
	//IUnknown members
	STDMETHOD(QueryInterface)(REFIID riid, void FAR* FAR* ppv);
	STDMETHOD_(ULONG, AddRef)(void);
	STDMETHOD_(ULONG, Release)(void);

	//IEnumFORMATETC members
	STDMETHOD(Next)(ULONG ulCelt, LPFORMATETC lpFormatEtc, ULONG FAR* pCeltFetched);
	STDMETHOD(Skip)(ULONG ulCelt);
	STDMETHOD(Reset)(void);
	STDMETHOD(Clone)(IEnumFORMATETC FAR* FAR* ppCloneEnumFormatEtc);

private:
	volatile LONG								m_ulRefCount;
	std::vector<FORMATETC>	m_vctFmtEtc;
	UINT									m_nCurrent;
};

//////////////////////////////////////////////////////////////////////////
class DropSourceEx : public IDropSource
{
public:
	DropSourceEx();
	HRESULT STDMETHODCALLTYPE QueryInterface    (REFIID riid, void ** ppvObject);
	ULONG   STDMETHODCALLTYPE AddRef            (void);
	ULONG   STDMETHODCALLTYPE Release           (void);
	//
	// IDropSource members
	//
	HRESULT STDMETHODCALLTYPE QueryContinueDrag (BOOL fEscapePressed, DWORD grfKeyState);
	HRESULT STDMETHODCALLTYPE GiveFeedback      (DWORD dwEffect);
private:
	bool SetDragImageCursor(DROPEFFECT dwEffect);
private:
	friend class DataObjectEx;
	bool					m_bSetCursor;		// internal flag set when Windows cursor must be set
	LPDATAOBJECT	m_pIDataObj;			// set by DataObjectEx to its IDataObject
};

//帮助类
class DragDropHelper
{
public:
	static bool GetGlobalData(LPDATAOBJECT pIDataObj, LPCTSTR lpszFormat, FORMATETC& formatEtc, STGMEDIUM& stgMedium);
	static DWORD	GetGlobalDataDWord(LPDATAOBJECT pIDataObj, LPCTSTR lpszFormat);

	static bool ClearDescription(DROPDESCRIPTION* pDropDescription);
	static DROPIMAGETYPE DropEffectToDropImage(DROPEFFECT dwEffect);
	static HGLOBAL GetGlobalData(LPDATAOBJECT pIDataObj, CLIPFORMAT cfFormat, LPFORMATETC lpFormatEtc=NULL);
};
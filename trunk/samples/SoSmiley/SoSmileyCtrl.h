#ifndef SoSmileyCtrl_h__
#define SoSmileyCtrl_h__

// SoSmileyCtrl.h : Declaration of the CSoSmileyCtrl
#pragma once
#include "resource.h"       // main symbols
#include <atlctl.h>

interface IRichEditOle;

//ITimerHandler
[
    object,
    uuid(8A0E5678-792F-439b-AEDD-E8D4AB602040),
    nonextensible,
    helpstring("ITimerHandler Interface"),
    pointer_default(unique)
]
__interface  ITimerHandler : IUnknown{
    [id(1), helpstring("method OnTimer")] HRESULT OnTimer([in]HDC hdc);
    [id(2), helpstring("method Clear")] HRESULT Clear();
    [id(3), helpstring("method GetRect")] HRESULT GetRect([out]LPRECT pRect);
};

//ISmileySource
[
    object,
    uuid(E9FFF8D9-7585-42ce-B6CE-33336283994D),
    nonextensible,
    helpstring("ISmileySource Interface"),
    pointer_default(unique)
]
__interface  ISmileySource : IUnknown{
    [id(1), helpstring("method Stream_Load")] HRESULT Stream_Load([in] LPSTREAM pStm);
    [id(2), helpstring("method Stream_Save")] HRESULT Stream_Save([in] LPSTREAM pStm);
    [id(3), helpstring("method LoadFromID")] HRESULT LoadFromID([in]UINT uID);
    [id(4), helpstring("method LoadFromFile")] HRESULT LoadFromFile([in]LPCWSTR pszFilePath);
    [id(5), helpstring("method GetID")] HRESULT GetID([out] UINT *pID);
    [id(6), helpstring("method GetFile")] HRESULT GetFile([out] BSTR * bstrFile);
    [id(7), helpstring("method GetFrameCount")] HRESULT GetFrameCount([out] int *pFrameCount);
    [id(8), helpstring("method GetFrameDelay")] HRESULT GetFrameDelay([in] int iFrame, [out] int *pFrameDelay);
    [id(9), helpstring("method GetSize")] HRESULT GetSize([out] LPSIZE pSize);
    [id(10), helpstring("method Draw")] HRESULT Draw([in] HDC hdc,[in] LPCRECT pRect,[in] int iFrame);
};

//ISmileyHost
[
    object,
    uuid(0F3687B9-333F-48a4-9001-C994455B430C),
    nonextensible,
    helpstring("ISmileyHost Interface"),
    pointer_default(unique)
]
__interface  ISmileyHost : IUnknown{
    [id(1), helpstring("method SendMessage")] HRESULT SendMessage([in] UINT uMsg,[in] WPARAM wParam,[in] LPARAM lParam,[out] LRESULT *pRet);
    [id(2), helpstring("method GetHostRect")] HRESULT GetHostRect([out] LPRECT prcHost);
    [id(3), helpstring("method InvalidateRect")] HRESULT InvalidateRect([in] LPCRECT pRect);
    [id(4), helpstring("method CreateSource")] HRESULT CreateSource([in,out] ISmileySource **ppSource);
    [id(5), helpstring("method SetTimer")] HRESULT SetTimer([in] ITimerHandler * pTimerHander,[in] int nInterval);
    [id(6), helpstring("method KillTimer")] HRESULT KillTimer([in] ITimerHandler * pTimerHander);
    [id(7), helpstring("method OnTimer")] HRESULT OnTimer([in] int nInterval);
    [id(8), helpstring("method ClearTimer")] HRESULT ClearTimer();
};

// ISoSmileyCtrl
[
    object,
    uuid(3286141B-C87F-4052-B6A2-376391DCDAF6),
    nonextensible,
    helpstring("ISoSmileyCtrl Interface"),
    pointer_default(unique)
]
__interface  ISoSmileyCtrl: IUnknown{
    [id(1), helpstring("method Insert2Richedit")] HRESULT Insert2Richedit([in] DWORD_PTR ole);
    [id(2), helpstring("method SetSource")] HRESULT SetSource([in] ISmileySource * pSource);
    [id(3), helpstring("method GetSource")] HRESULT GetSource([out] ISmileySource ** ppSource);
};


//CSoSmileyCtrl
[
	coclass,
	control,
	threading(apartment),
	vi_progid("SoSmiley.SoSmileyCtrl"),
	progid("SoSmiley.SoSmileyCtrl.1"),
	version(1.0),
	uuid("D29E0BDE-CFDA-4b93-929A-877AB4557BD8"),
	helpstring("SoSmileyCtrl Class"),
	registration_script("control.rgs")
]

class ATL_NO_VTABLE CSoSmileyCtrl :
	public CComControl<CSoSmileyCtrl>,
    public IOleObjectImpl<CSoSmileyCtrl>,
    public IOleInPlaceObjectWindowlessImpl<CSoSmileyCtrl>,
	public IViewObjectExImpl<CSoSmileyCtrl>,
	public IPersistStreamInitImpl<CSoSmileyCtrl>,
	public IPersistStorageImpl<CSoSmileyCtrl>,
	public IOleControlImpl<CSoSmileyCtrl>,
    public ITimerHandler,
    public ISoSmileyCtrl
{
public:

	DECLARE_OLEMISC_STATUS(OLEMISC_RECOMPOSEONRESIZE |
	                        OLEMISC_CANTLINKINSIDE |
		                    OLEMISC_INSIDEOUT |
		                    OLEMISC_ACTIVATEWHENVISIBLE |
		                    OLEMISC_SETCLIENTSITEFIRST
		                    )

		BEGIN_COM_MAP(CSoSmileyCtrl)
			COM_INTERFACE_ENTRY(ITimerHandler)
			COM_INTERFACE_ENTRY(ISoSmileyCtrl)
			COM_INTERFACE_ENTRY(IOleControl)
			COM_INTERFACE_ENTRY(IOleObject)
			COM_INTERFACE_ENTRY(IPersistStreamInit)
			COM_INTERFACE_ENTRY2(IPersist, IPersistStreamInit)
			COM_INTERFACE_ENTRY(IPersistStorage)
		END_COM_MAP()

        BEGIN_PROP_MAP(CSoSmileyCtrl)
        END_PROP_MAP()

		BEGIN_MSG_MAP(CSoSmileyCtrl)
			CHAIN_MSG_MAP(CComControl<CSoSmileyCtrl>)
			DEFAULT_REFLECTION_HANDLER()
		END_MSG_MAP()

		DECLARE_PROTECT_FINAL_CONSTRUCT()

public:
    CSoSmileyCtrl();
    ~CSoSmileyCtrl();

public://ISoSmileyCtrl
    STDMETHOD(Insert2Richedit)(DWORD_PTR ole);
    STDMETHOD(SetSource)(ISmileySource * pSource);    
    STDMETHOD(GetSource)(ISmileySource ** ppSource);    
    
public://IOleObject
    STDMETHOD(SetClientSite)(IOleClientSite *pClientSite);
public://IPersistStreamInitImpl
    HRESULT IPersistStreamInit_Load(LPSTREAM pStm, const ATL_PROPMAP_ENTRY* pMap);
    HRESULT IPersistStreamInit_Save(LPSTREAM pStm, BOOL fClearDirty, const ATL_PROPMAP_ENTRY* pMap);
    HRESULT FireViewChange();
    HRESULT	OnDraw(ATL_DRAWINFO& di);
public://IPersistStorage
    STDMETHOD(Load)(IStorage* pStorage);//重载IPersistStorage::Load，初始化完成后写入几个标志位

public://ITimerHander
    STDMETHOD(OnTimer)(HDC hdc);
    STDMETHOD(Clear)();;
    STDMETHOD(GetRect)(LPRECT pRect){memcpy(pRect,&m_rcPos,sizeof(RECT));return S_OK;};
    
private:
    void UpdateSmiley(HDC hdc);
    void UpdateSmileyFlag();
    DWORD GetSmileyFlag(IRichEditOle *ole,int iFirst,int iLast);
    
    CComPtr<ISmileyHost>    m_pSmileyHost;
    CComPtr<ISmileySource>  m_pSmileySource;
    int                     m_iFrameIndex;
    DWORD                   m_dwID;
    
    DWORD                   m_dwDrawFlag;
};

#endif // SoSmileyCtrl_h__
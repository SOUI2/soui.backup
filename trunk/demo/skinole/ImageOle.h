#pragma once

#include <richole.h>
#include <core/simplewnd.h>
#include <control/SRichEdit.h>

class __declspec(uuid("{8C96DAB8-0525-44e4-B026-0FB76E05BB05}")) CImageOle : public IOleObject
                                                                           , public IViewObject2
                                                                           , public SOUI::ITimelineHandler
{
private:
	CImageOle(SOUI::SRichEdit *pRichedit);
	~CImageOle(void);

public:
	// IUnknown接口
	virtual HRESULT WINAPI QueryInterface(REFIID iid, void ** ppvObject);
	virtual ULONG   WINAPI AddRef(void);
	virtual ULONG   WINAPI Release(void);

	// IOleObject接口
	virtual HRESULT WINAPI SetClientSite(IOleClientSite *pClientSite);
	virtual HRESULT WINAPI GetClientSite(IOleClientSite **ppClientSite);
	virtual HRESULT WINAPI SetHostNames(LPCOLESTR szContainerApp, LPCOLESTR szContainerObj);
	virtual HRESULT WINAPI Close(DWORD dwSaveOption);
	virtual HRESULT WINAPI SetMoniker(DWORD dwWhichMoniker, IMoniker *pmk);
	virtual HRESULT WINAPI GetMoniker(DWORD dwAssign, DWORD dwWhichMoniker, IMoniker **ppmk);
	virtual HRESULT WINAPI InitFromData(IDataObject *pDataObject, BOOL fCreation, DWORD dwReserved);
	virtual HRESULT WINAPI GetClipboardData(DWORD dwReserved, IDataObject **ppDataObject);
	virtual HRESULT WINAPI DoVerb(LONG iVerb, LPMSG lpmsg, IOleClientSite *pActiveSite, LONG lindex, HWND hwndParent, LPCRECT lprcPosRect);
	virtual HRESULT WINAPI EnumVerbs(IEnumOLEVERB **ppEnumOleVerb);
	virtual HRESULT WINAPI Update(void);
	virtual HRESULT WINAPI IsUpToDate(void);
	virtual HRESULT WINAPI GetUserClassID(CLSID *pClsid);
	virtual HRESULT WINAPI GetUserType(DWORD dwFormOfType, LPOLESTR *pszUserType);
	virtual HRESULT WINAPI SetExtent(DWORD dwDrawAspect, SIZEL *psizel);
	virtual HRESULT WINAPI GetExtent(DWORD dwDrawAspect, SIZEL *psizel);
	virtual HRESULT WINAPI Advise(IAdviseSink *pAdvSink, DWORD *pdwConnection);
	virtual HRESULT WINAPI Unadvise(DWORD dwConnection);
	virtual HRESULT WINAPI EnumAdvise(IEnumSTATDATA **ppenumAdvise);
	virtual HRESULT WINAPI GetMiscStatus(DWORD dwAspect, DWORD *pdwStatus);
	virtual HRESULT WINAPI SetColorScheme(LOGPALETTE *pLogpal);

	// IViewObject接口
	virtual HRESULT WINAPI Draw(DWORD dwDrawAspect, LONG lindex, void *pvAspect, DVTARGETDEVICE *ptd,
		HDC hdcTargetDev, HDC hdcDraw, LPCRECTL lprcBounds, LPCRECTL lprcWBounds,
		BOOL ( WINAPI *pfnContinue )(ULONG_PTR dwContinue), ULONG_PTR dwContinue);
	virtual HRESULT WINAPI GetColorSet(DWORD dwDrawAspect, LONG lindex, void *pvAspect, 
		DVTARGETDEVICE *ptd, HDC hicTargetDev, LOGPALETTE **ppColorSet);
	virtual HRESULT WINAPI Freeze(DWORD dwDrawAspect, LONG lindex, void *pvAspect, DWORD *pdwFreeze);
	virtual HRESULT WINAPI Unfreeze(DWORD dwFreeze);
	virtual HRESULT WINAPI SetAdvise(DWORD aspects, DWORD advf, IAdviseSink *pAdvSink);
	virtual HRESULT WINAPI GetAdvise(DWORD *pAspects, DWORD *pAdvf, IAdviseSink **ppAdvSink);

	// IViewObject2接口
	virtual HRESULT WINAPI GetExtent(DWORD dwDrawAspect, LONG lindex, DVTARGETDEVICE *ptd, LPSIZEL lpsizel);

	virtual void OnNextFrame();

	void SetSkinObj(ISkinObj *pSkin);
    
    static CImageOle * CreateObject(SOUI::SRichEdit *pRichedit);
protected:

	volatile LONG m_ulRef;
	
	SOUI::CAutoRefPtr<IOleClientSite> m_pOleClientSite;
	SOUI::CAutoRefPtr<IAdviseSink> m_pAdvSink;
    SOUI::CAutoRefPtr<SOUI::ISkinObj> m_pSkin;

	SOUI::SRichEdit *m_pRichedit;
	
	int		m_iFrame;
	int		m_nTimePass;	//过去的时间
	int		m_nTimeDelay;	//一个动画帧需要的时间
};

class __declspec(uuid("{C733506E-7539-4a88-9B7F-334356B28C62}")) SRichEditOleCallback_Impl : public IRichEditOleCallback
{
public:
	static SRichEditOleCallback_Impl * CreateObject(SOUI::SRichEdit *pRicheditCtrl);
    
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID iid, void ** ppvObject);
    virtual ULONG STDMETHODCALLTYPE AddRef();
    virtual ULONG STDMETHODCALLTYPE Release();


    virtual HRESULT STDMETHODCALLTYPE GetNewStorage(LPSTORAGE* lplpstg);

    virtual HRESULT STDMETHODCALLTYPE GetInPlaceContext(LPOLEINPLACEFRAME FAR *lplpFrame,
        LPOLEINPLACEUIWINDOW FAR *lplpDoc, LPOLEINPLACEFRAMEINFO lpFrameInfo);
    virtual HRESULT STDMETHODCALLTYPE ShowContainerUI(BOOL fShow);
    virtual HRESULT STDMETHODCALLTYPE QueryInsertObject(LPCLSID lpclsid, LPSTORAGE lpstg, LONG cp);
    virtual HRESULT STDMETHODCALLTYPE DeleteObject(LPOLEOBJECT lpoleobj);
    virtual HRESULT STDMETHODCALLTYPE QueryAcceptData(LPDATAOBJECT lpdataobj, CLIPFORMAT FAR *lpcfFormat,
        DWORD reco, BOOL fReally, HGLOBAL hMetaPict);
    virtual HRESULT STDMETHODCALLTYPE ContextSensitiveHelp(BOOL fEnterMode);
    virtual HRESULT STDMETHODCALLTYPE GetClipboardData(CHARRANGE FAR *lpchrg, DWORD reco, LPDATAOBJECT FAR *lplpdataobj);
    virtual HRESULT STDMETHODCALLTYPE GetDragDropEffect(BOOL fDrag, DWORD grfKeyState, LPDWORD pdwEffect);
    virtual HRESULT STDMETHODCALLTYPE GetContextMenu(WORD seltyp, LPOLEOBJECT lpoleobj, CHARRANGE FAR *lpchrg,
        HMENU FAR *lphmenu);
        
private:
	SRichEditOleCallback_Impl(SOUI::SRichEdit *pRicheditCtrl);
    
    ~SRichEditOleCallback_Impl()
    {
    
    }
    
    int m_iNumStorages;
    CAutoRefPtr<IStorage> m_pStorage;
	SOUI::SRichEdit *m_pRicheditCtrl;

    DWORD m_dwRef;
};

BOOL RichEdit_SetOleCallback(SOUI::SRichEdit *pRicheditCtrl);
BOOL RichEdit_InsertSkin(SOUI::SRichEdit *pRicheditCtrl, SOUI::ISkinObj *pSkin);
BOOL RichEdit_InsertImage(SOUI::SRichEdit *pRicheditCtrl, LPCTSTR lpszFileName);

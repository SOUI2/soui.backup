#pragma once
#include "RichEditObj.h"
#include "core\SwndContainerImpl.h"
#include <atlcomcli.h>
#include <oleidl.h>
#include <objidl.h>
#include <richole.h>

//////////////////////////////////////////////////////////////////////////
//
class OleTimerHandler : public ITimelineHandler
{
public:

    BOOL RegisterHandler( ITimelineHandler *pHandler )
    {
        SPOSITION pos= m_lstHandler.Find(pHandler);
        if(pos) return FALSE;
        m_lstHandler.AddTail(pHandler);
        return TRUE;
    }

    BOOL UnregisterHandler( ITimelineHandler *pHandler )
    {
        SPOSITION pos= m_lstHandler.Find(pHandler);
        if(!pos) return FALSE;
        m_lstHandler.RemoveAt(pos);
        return TRUE;
    }

    void OnNextFrame()
    {
        /*
         * 如果有大量的定时器注册上来,每次都要拷贝,很费CPU,特意把以下两句去掉
         * 注意在OnNextFrame里面不能注册和取消定时器,否则循环会出错
        */
        //SList<ITimelineHandler*> lstCopy;
        //CopyList(m_lstHandler,lstCopy);

        SPOSITION pos=m_lstHandler.GetHeadPosition();
        while(pos)
        {
            ITimelineHandler * pHandler = m_lstHandler.GetNext(pos);
            pHandler->OnNextFrame();
        }
    }

protected:
    SList<ITimelineHandler*>    m_lstHandler;
};

//////////////////////////////////////////////////////////////////////////
// OleWindow 
//
class OleWindow : public SwndContainerImpl
{
public:
    OleWindow();
    virtual ~OleWindow();

    //
    // ISwndContainer methods
    //
    BOOL            OnFireEvent(EventArgs &evt);
    CRect           GetContainerRect();
    HWND            GetHostHwnd();
    const SStringW& GetTranslatorContext();
    IRenderTarget*  OnGetRenderTarget(const CRect & rc,DWORD gdcFlags);
    void            OnReleaseRenderTarget(IRenderTarget * pRT,const CRect &rc,DWORD gdcFlags);
    void            OnRedraw(const CRect &rc);
    BOOL            IsTranslucent() const;
    BOOL            IsSendWheel2Hover() const;
    BOOL            OnCreateCaret(SWND swnd,HBITMAP hBmp,int nWidth,int nHeight) {return FALSE;}
    BOOL            OnShowCaret(BOOL bShow) {return FALSE;}
    BOOL            OnSetCaretPos(int x,int y) {return FALSE;}
    BOOL            UpdateWindow();
    BOOL            RegisterTimelineHandler(ITimelineHandler *pHandler);
    BOOL            UnregisterTimelineHandler(ITimelineHandler *pHandler);
    SMessageLoop *  GetMsgLoop();
    IScriptModule * GetScriptModule() { return NULL; }

    //
    // internal public methods
    //
    void        SetHostRichEdit(IRichEditObjHost *);
    LRESULT     HandleEvent(UINT msg, WPARAM wParam, LPARAM lParam);
    void        SetOleWindowRect(CRect rcWindow)    { m_rcOleWindow = rcWindow; }
    void        SetDelayDraw(BOOL bDelayDraw)       { m_bDelayDraw  = bDelayDraw; }

protected:
    void        RealDraw(CRect rc);

private:
    ISwndContainer*         m_pHostContainer;
    IRichEditObjHost *      m_pHostRichEdit;
    int                     m_nWindowID;   

    CRect                   m_rcOleWindow;      // ole在richedit里面的位置   
    BOOL                    m_bDelayDraw;       // 对于GIF，需要设置延迟刷新

    static OleTimerHandler  m_timerHandler;     // 定时器
    static BOOL             m_bTiemrRegistered;
};

//////////////////////////////////////////////////////////////////////////
// RichEditOleBase 
//
class RichEditOleBase : public RichEditObj
    ,public IOleObject
    ,public IViewObject2
{
public:
    RichEditOleBase();
    virtual ~RichEditOleBase();

    //
    // IUnknown meghods
    //
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void ** ppvObject);
    ULONG   STDMETHODCALLTYPE AddRef(void);
    ULONG   STDMETHODCALLTYPE Release(void);

    //
    // IOleObject methods
    //
    HRESULT STDMETHODCALLTYPE SetHostNames(LPCOLESTR, LPCOLESTR)        { return S_OK; }
    HRESULT STDMETHODCALLTYPE Close(DWORD dwSaveOption)                 { return S_OK; }
    HRESULT STDMETHODCALLTYPE SetMoniker(DWORD dwWhich, IMoniker *pmk)  { return E_NOTIMPL;	}
    HRESULT STDMETHODCALLTYPE GetMoniker(DWORD, DWORD, IMoniker **ppmk) { return E_NOTIMPL;	}
    HRESULT STDMETHODCALLTYPE InitFromData(IDataObject *, BOOL, DWORD)  { return E_NOTIMPL;	}
    HRESULT STDMETHODCALLTYPE GetClipboardData(DWORD, IDataObject **)   { return E_NOTIMPL; }
    HRESULT STDMETHODCALLTYPE DoVerb(LONG iVerb, LPMSG lpmsg, IOleClientSite *pActiveSite,LONG lindex, HWND hwndParent, LPCRECT lprcPosRect)
                                                                        { return E_NOTIMPL; }
    HRESULT STDMETHODCALLTYPE EnumVerbs(IEnumOLEVERB **ppEnumOleVerb)   { return E_NOTIMPL; }
    HRESULT STDMETHODCALLTYPE Update( void)                             { return E_NOTIMPL; }
    HRESULT STDMETHODCALLTYPE IsUpToDate( void)                         { return E_NOTIMPL;	}
    HRESULT STDMETHODCALLTYPE GetUserType(DWORD, LPOLESTR *pszUserType) { return E_NOTIMPL; }
    HRESULT STDMETHODCALLTYPE EnumAdvise(IEnumSTATDATA **ppenumAdvise)  { return E_NOTIMPL; }
    HRESULT STDMETHODCALLTYPE GetMiscStatus(DWORD , DWORD *pdwStatus)   { return E_NOTIMPL;	}
    HRESULT STDMETHODCALLTYPE SetColorScheme(LOGPALETTE *pLogpal)       { return E_NOTIMPL;	}

    //以下接口需要实现
    HRESULT STDMETHODCALLTYPE GetUserClassID(CLSID *pClsid);
    HRESULT STDMETHODCALLTYPE Advise(IAdviseSink *pAdvSink,DWORD *pdwConnection);
    HRESULT STDMETHODCALLTYPE Unadvise(DWORD dwConnection);
    HRESULT STDMETHODCALLTYPE SetClientSite(IOleClientSite *pClientSite);
    HRESULT STDMETHODCALLTYPE GetClientSite(IOleClientSite **ppClientSite);
    HRESULT STDMETHODCALLTYPE GetExtent(DWORD dwDrawAspect, SIZEL *psizel);
    HRESULT STDMETHODCALLTYPE SetExtent(DWORD dwDrawAspect, SIZEL *psizel) ;

    //
    // IViewObject2 methods
    //
    HRESULT STDMETHODCALLTYPE GetColorSet( DWORD, LONG, void *, DVTARGETDEVICE *,  HDC, LOGPALETTE **)    
                                                                            { return E_NOTIMPL; }
    HRESULT STDMETHODCALLTYPE Freeze( DWORD , LONG , void *, DWORD *)       { return S_OK; }
    HRESULT STDMETHODCALLTYPE Unfreeze(DWORD dwFreeze)                      { return S_OK; }

    // 以下接口需要实现
    HRESULT STDMETHODCALLTYPE SetAdvise( DWORD aspects, DWORD advf, IAdviseSink *pAdvSink);
    HRESULT STDMETHODCALLTYPE GetAdvise(DWORD *pAspects, DWORD *pAdvf, IAdviseSink **ppAdvSink);
    HRESULT STDMETHODCALLTYPE Draw(
        DWORD dwDrawAspect, LONG lindex,  
        void *pvAspect, 
        DVTARGETDEVICE *ptd, 
        HDC hdcTargetDev,
        HDC hdcDraw, 
        LPCRECTL lprcBounds,
        LPCRECTL lprcWBounds,
        BOOL ( STDMETHODCALLTYPE *pfnContinue )(ULONG_PTR dwContinue), 
        ULONG_PTR dwContinue);

    HRESULT STDMETHODCALLTYPE GetExtent(
        DWORD dwDrawAspect, 
        LONG lindex, 
        DVTARGETDEVICE *ptd, 
        LPSIZEL lpsizel);

    //
    // RichEditObj methods
    //
    void            SetDirty(BOOL bDirty);
    LRESULT         ProcessMessage(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    BOOL            InsertIntoHost(IRichEditObjHost * pHost);
    BOOL            OnUpdateToolTip(CPoint pt, SwndToolTipInfo &tipInfo);
    BOOL            NeedToProcessMessage() {return TRUE; }

    // internal  methods
    virtual BOOL    InitOleWindow(IRichEditObjHost* pHost);
    virtual void    PreInsertObject(REOBJECT& object){}
    void            CalculateExtentSize(const SIZE& sizeNature);

protected:
    HRESULT         InsertOleObject(IRichEditObjHost*);
    void            InvertBorder(HDC hdc, LPRECT lprcBorder);

    SOUI_ATTRS_BEGIN()
        ATTR_INT(L"selectable", m_bCanSelect, FALSE)
    SOUI_ATTRS_END()

protected:
    //ULONG                       m_dwRef;
    CComPtr<IOleClientSite>     m_spClientSite;
    CComPtr<IOleAdviseHolder>   m_spOleAdviseHolder;
    CComPtr<IAdviseSink>        m_spAdviseSink;
    SIZEL                       m_sizeExtent;
    OleWindow                   m_oleWindow;
    SIZE                        m_sizeNatural;
    GUID                        m_guidOle;
    SStringW                    m_strXmlLayout;
    BOOL                        m_bCanSelect;
};

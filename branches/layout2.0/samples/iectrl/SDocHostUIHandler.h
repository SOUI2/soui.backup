#pragma once

#include <atl.mini/SComHelper.h>

namespace SOUI
{
    class SJsCallCppIDispatch : public SUnknownImpl<IDispatch>
    {
    public:
        SJsCallCppIDispatch(void){}
        ~SJsCallCppIDispatch(void){}

        COM_INTERFACE_BEGIN()
            COM_INTERFACE(IDispatch)
        COM_INTERFACE_END()

        //IDispatch
        STDMETHODIMP GetTypeInfoCount(UINT* pctinfo);
        STDMETHODIMP GetTypeInfo(/* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo** ppTInfo);
        STDMETHODIMP GetIDsOfNames(
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        STDMETHODIMP Invoke(
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS  *pDispParams,
            /* [out] */ VARIANT  *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
    };

    class SDocHostUIHandler : public SUnknownImpl<IDocHostUIHandler>
    {
    public:
        SDocHostUIHandler(void);
        ~SDocHostUIHandler(void);

        COM_INTERFACE_BEGIN()
            COM_INTERFACE(IDocHostUIHandler)
        COM_INTERFACE_END()


        virtual STDMETHODIMP ShowContextMenu(DWORD dwID, POINT *ppt, IUnknown *pcmdtReserved, IDispatch *pdispReserved);

        virtual STDMETHODIMP GetHostInfo(DOCHOSTUIINFO *pInfo);

        virtual STDMETHODIMP ShowUI(DWORD dwID, IOleInPlaceActiveObject *pActiveObject, IOleCommandTarget *pCommandTarget, IOleInPlaceFrame *pFrame, IOleInPlaceUIWindow *pDoc);

        virtual STDMETHODIMP HideUI(void);

        virtual STDMETHODIMP UpdateUI(void);

        virtual STDMETHODIMP EnableModeless(BOOL fEnable);

        virtual STDMETHODIMP OnDocWindowActivate(BOOL fActivate);

        virtual STDMETHODIMP OnFrameWindowActivate(BOOL fActivate);

        virtual STDMETHODIMP ResizeBorder(LPCRECT prcBorder, IOleInPlaceUIWindow *pUIWindow, BOOL fRameWindow);

        virtual STDMETHODIMP TranslateAccelerator(LPMSG lpMsg, const GUID *pguidCmdGroup, DWORD nCmdID);

        virtual STDMETHODIMP GetOptionKeyPath(__out LPOLESTR *pchKey, DWORD dw);

        virtual STDMETHODIMP GetDropTarget(IDropTarget *pDropTarget, IDropTarget **ppDropTarget);

        virtual STDMETHODIMP GetExternal(IDispatch **ppDispatch);

        virtual STDMETHODIMP TranslateUrl(DWORD dwTranslate, __in __nullterminated OLECHAR *pchURLIn, __out OLECHAR **ppchURLOut);

        virtual STDMETHODIMP FilterDataObject(IDataObject *pDO, IDataObject **ppDORet);

    protected:
        SComPtr<IDispatch> m_disp;
    };

    VARIANT ExecuteScript(IWebBrowser2 *pWebBrowser, const SStringW & fun,SArray<SStringW> & params );
}

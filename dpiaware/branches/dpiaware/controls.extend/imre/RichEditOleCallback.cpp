#include "stdafx.h"
#include "RichEditOleCallback.h"

//////////////////////////////////////////////////////////////////////////
// RichEditOleCallback

RichEditOleCallback::RichEditOleCallback()
    :m_dwRef(1)
    ,m_iStorage(0)
{
    HRESULT hResult = ::StgCreateDocfile(NULL,
        STGM_TRANSACTED | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE| STGM_DELETEONRELEASE,
        0, &m_stg );

    if ( m_stg == NULL || hResult != S_OK )
    {
//         AfxThrowOleException( hResult );
    }
}

RichEditOleCallback::~RichEditOleCallback()
{
}

HRESULT RichEditOleCallback::GetNewStorage(LPSTORAGE* ppStg)
{
    WCHAR tName[150];
    swprintf(tName, L"REStorage_%d", ++m_iStorage);

    if(m_iStorage%100 == 0)
    {//每100个对象提交一次,避免创建stream or storage由于内存不足而失败
        m_stg->Commit(STGC_DEFAULT);
    }

    HRESULT hr = m_stg->CreateStorage(tName, 
        STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE ,
        0, 0, ppStg );    
    if(FAILED(hr) && (hr & E_OUTOFMEMORY))
    {//失败后向storage提交后重试
        m_stg->Commit(STGC_DEFAULT);
        hr = m_stg->CreateStorage(tName, 
            STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE ,
            0, 0, ppStg );    
    }

    return hr;
}

HRESULT RichEditOleCallback::QueryInterface(REFIID iid, void ** ppvObject)
{
    HRESULT hr = S_OK;
    *ppvObject = NULL;

    if ( iid == IID_IUnknown ||
        iid == IID_IRichEditOleCallback )
    {
        *ppvObject = this;
        AddRef();
    }else
    {
        hr = E_NOINTERFACE;
    }

    return hr;
}

ULONG RichEditOleCallback::AddRef()
{
    return ++m_dwRef;
}

ULONG RichEditOleCallback::Release()
{
    if ( --m_dwRef == 0 )
    {
        delete this;
        return 0;
    }

    return m_dwRef;
}

HRESULT RichEditOleCallback::GetInPlaceContext(
    LPOLEINPLACEFRAME FAR *lplpFrame,
    LPOLEINPLACEUIWINDOW FAR *lplpDoc, 
    LPOLEINPLACEFRAMEINFO lpFrameInfo)
{
    return S_OK;
}

HRESULT RichEditOleCallback::ShowContainerUI(BOOL fShow)
{
    return S_OK;
}

HRESULT RichEditOleCallback::QueryInsertObject(
    LPCLSID lpclsid, 
    LPSTORAGE lpstg, 
    LONG cp)
{
    return S_OK;
}

HRESULT RichEditOleCallback::DeleteObject(LPOLEOBJECT lpoleobj)
{
    return S_OK;
}

HRESULT RichEditOleCallback::GetClipboardData(
    CHARRANGE FAR *lpchrg, 
    DWORD reco, 
    LPDATAOBJECT FAR *lplpdataobj)
{
    /*演示自定义剪贴板格式的复制
    if(RECO_COPY == reco || RECO_CUT == reco)
    {
        wchar_t * pBuf = new WCHAR[lpchrg->cpMax - lpchrg->cpMin +1];
        TEXTRANGE txtRng;
        txtRng.chrg = *lpchrg;
        txtRng.lpstrText = pBuf;
        m_pRichedit->SSendMessage(EM_GETTEXTRANGE,0,(LPARAM)&txtRng);
        pBuf[lpchrg->cpMax - lpchrg->cpMin] =0;
        
        int  strBytes=  (lpchrg->cpMax - lpchrg->cpMin +1) * 2;  
        HGLOBAL hG = GlobalAlloc(GMEM_DDESHARE, strBytes);  
        void* pBuffer = GlobalLock(hG);  
        {  
            memcpy(pBuffer, pBuf, strBytes);  
            GlobalUnlock(hG);  
        }  
        delete []txtRng.lpstrText;

        FORMATETC fmt;  
        fmt.cfFormat = KCF_SMILEY;  
        fmt.dwAspect = DVASPECT_CONTENT;  
        fmt.lindex = -1;  
        fmt.ptd = NULL;  
        fmt.tymed = TYMED_HGLOBAL;  

        STGMEDIUM stg;  
        stg.tymed = TYMED_HGLOBAL;  
        stg.hGlobal = hG;  
        stg.pUnkForRelease = NULL;  

        HRESULT hr =CreateDataObject(&fmt,&stg,1,lplpdataobj);


        return hr;
    }
    */
    return E_NOTIMPL;
}

HRESULT RichEditOleCallback::QueryAcceptData(
    LPDATAOBJECT lpdataobj, 
    CLIPFORMAT FAR *lpcfFormat,
    DWORD reco, 
    BOOL fReally, 
    HGLOBAL hMetaPict)
{
    if(!fReally) return S_OK;

    /*演示自定义剪贴板格式的粘贴
    if(RECO_DROP == reco || RECO_PASTE == reco)
    {
        FORMATETC fmt;  
        fmt.cfFormat = KCF_SMILEY;  
        fmt.dwAspect = DVASPECT_CONTENT;  
        fmt.lindex = -1;  
        fmt.ptd = NULL;  
        fmt.tymed = TYMED_HGLOBAL;  
        //如果KCF_SMILEY 剪贴板格式可用  
        if (SUCCEEDED(lpdataobj->QueryGetData(&fmt)) )
        {  
            STGMEDIUM stg;  
            HRESULT hr = lpdataobj->GetData(&fmt, &stg);  

            int nSize = GlobalSize(stg.hGlobal);  
            void* pBuffer = GlobalLock(stg.hGlobal);  
            {  

                STRACE(L"QueryAcceptData:%s",(LPWSTR)pBuffer);
                GlobalUnlock(stg.hGlobal);  
            }  
            return S_OK;  
        } 
    }
    */
    return E_NOTIMPL;
}

HRESULT RichEditOleCallback::ContextSensitiveHelp(BOOL fEnterMode)
{
    return S_OK;
}

HRESULT RichEditOleCallback::GetDragDropEffect(
    BOOL fDrag, 
    DWORD grfKeyState, 
    LPDWORD pdwEffect)
{
    return S_OK;
}

HRESULT RichEditOleCallback::GetContextMenu(
    WORD seltyp, 
    LPOLEOBJECT lpoleobj, 
    CHARRANGE FAR *lpchrg,
    HMENU FAR *lphmenu)
{
    return S_OK;
}

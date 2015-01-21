// SoSmileyCtrl.cpp : Implementation of CSoSmileyCtrl
#include "stdafx.h"
#include "SoSmileyCtrl.h"
#include <atltypes.h>
#include <TOM.h>
#include <richedit.h>
#include <richole.h>


DWORD _GenerateID()
{
    static DWORD s_id=1;
    return s_id++;
}

// CCSmiley

CSoSmileyCtrl::CSoSmileyCtrl():m_iFrameIndex(0)
{
    m_dwID = _GenerateID();
}

CSoSmileyCtrl::~CSoSmileyCtrl()
{

}

STDMETHODIMP CSoSmileyCtrl::Insert2Richedit(DWORD_PTR pole)
{
    if(!m_pSmileySource) return E_FAIL;

    IRichEditOle *ole = (IRichEditOle*)pole;
    
    HRESULT hr;

    //insert this to host
    CComPtr<IOleObject>	pOleObject;
    CComPtr<IOleClientSite> pClientSite;  

    // Get site
    ole->GetClientSite(&pClientSite);
    ATLASSERT(pClientSite != NULL);
    
    CComPtr<IRichEditOleCallback> pCallback;
    hr=ole->QueryInterface(IID_IRichEditOleCallback,(void**)&pCallback);
    if(!SUCCEEDED(hr)) return E_FAIL;
    
    //get the IOleObject
    hr = QueryInterface(IID_IOleObject, (void**)&pOleObject);
    if (FAILED(hr))
    {
        return	 E_FAIL;
    }

    //to insert into richedit, you need a struct of REOBJECT
    REOBJECT reobject;
    ZeroMemory(&reobject, sizeof(REOBJECT));

    reobject.cbStruct = sizeof(REOBJECT);	

    CLSID clsid;
    pOleObject->GetUserClassID(&clsid);
    
    //get storage
    pCallback->GetNewStorage(&reobject.pstg);
    
    //set clsid
    reobject.clsid = clsid;
    //can be selected
    reobject.cp = REO_CP_SELECTION;
    //content, but not static
    reobject.dvaspect = DVASPECT_CONTENT;
    //goes in the same line of text line
    reobject.dwFlags = REO_BELOWBASELINE;
    reobject.dwUser = (DWORD)m_dwID;
    //the very object
    reobject.poleobj = pOleObject;
    //client site contain the object
    reobject.polesite = pClientSite;

    SIZEL sizel={0};
    reobject.sizel = sizel;

    hr = pOleObject->SetClientSite(pClientSite);
    
    if(SUCCEEDED(hr))
    {
        hr = ole->InsertObject(&reobject);

        FireViewChange();
    }
    
    if(reobject.pstg)
    {
        reobject.pstg->Release();
    }
    
    return hr;
}


STDMETHODIMP CSoSmileyCtrl::SetSource(ISmileySource * pSource)
{
    m_pSmileySource = pSource;
    if(m_pSmileySource)
    {
        //更新表情大小
        SIZE sz;
        m_pSmileySource->GetSize(&sz);

        HDC hDC = ::GetDC(NULL);
        m_sizeExtent.cx = ::MulDiv(sz.cx, 2540, GetDeviceCaps(hDC, LOGPIXELSX));
        m_sizeExtent.cy = ::MulDiv(sz.cy, 2540, GetDeviceCaps(hDC, LOGPIXELSY));
        ::ReleaseDC(NULL, hDC);
    }

    return S_OK;
}

STDMETHODIMP CSoSmileyCtrl::OnTimer()
{
    int cFrame = 1;
    m_pSmileySource->GetFrameCount(&cFrame);
    m_iFrameIndex++;
    m_iFrameIndex %= cFrame;
    FireViewChange();
    return S_OK;
}


HRESULT CSoSmileyCtrl::IPersistStreamInit_Load(LPSTREAM pStm, const ATL_PROPMAP_ENTRY* pMap)
{
    ATLASSERT(!m_pSmileySource);
    ATLASSERT(m_pSmileyHost);
    CComPtr<ISmileySource> pSource;
    HRESULT hr =m_pSmileyHost->CreateSource(&pSource);
    if(!SUCCEEDED(hr)) return hr;
    hr = pSource->Stream_Load(pStm);
    if(!SUCCEEDED(hr)) return hr;
    SetSource(pSource);

    hr = IPersistStreamInitImpl<CSoSmileyCtrl>::IPersistStreamInit_Load( pStm, pMap);

    if(!SUCCEEDED(hr)) return hr;
    FireViewChange();
    return hr;
}

HRESULT CSoSmileyCtrl::IPersistStreamInit_Save(LPSTREAM pStm, BOOL fClearDirty, const ATL_PROPMAP_ENTRY* pMap)
{
    if(!m_pSmileySource) return E_FAIL;
    HRESULT hr = m_pSmileySource->Stream_Save(pStm);
    if(!SUCCEEDED(hr)) return hr;
    return IPersistStreamInitImpl<CSoSmileyCtrl>::IPersistStreamInit_Save( pStm, fClearDirty, pMap);
}

HRESULT CSoSmileyCtrl::FireViewChange()
{
    if (m_bInPlaceActive)
    {
        // Active
        if (m_hWndCD != NULL)
            ::InvalidateRect(m_hWndCD, NULL, TRUE); // Window based
        else if (m_bWndLess && m_spInPlaceSite != NULL)
            m_spInPlaceSite->InvalidateRect(NULL, TRUE); // Windowless
    }
    else if (!m_pSmileyHost)
    {
        SendOnViewChange(DVASPECT_CONTENT);
    }
    else
    {
        UpdateSmiley();
    }
    return S_OK;
}

HRESULT CSoSmileyCtrl::OnDraw(ATL_DRAWINFO& di)
{
    if(!m_pSmileySource) return E_FAIL;
    RECT rc={di.prcBounds->left,di.prcBounds->top,di.prcBounds->right,di.prcBounds->bottom};
    m_pSmileySource->Draw(di.hdcDraw,&rc,m_iFrameIndex);
    
    int nDelay=0;
    HRESULT hr = m_pSmileySource->GetFrameDelay(m_iFrameIndex,&nDelay);
    if(SUCCEEDED(hr) && m_pSmileyHost)
    {
        m_pSmileyHost->SetTimer(this,nDelay);
    }
    return S_OK;    
}


LONG GetOleCP(IRichEditOle *pOle, int iOle)
{
    REOBJECT reobj={0};
    reobj.cbStruct=sizeof(REOBJECT);
    pOle->GetObject(iOle,&reobj,REO_GETOBJ_NO_INTERFACES);
    return reobj.cp;
}

//find first Ole Object in char range of [cpMin,cpMax)
int FindFirstOleInrange(IRichEditOle *pOle, int iBegin,int iEnd,int cpMin,int cpMax)
{
    if(iBegin==iEnd) return -1;

    int iMid = (iBegin + iEnd)/2;

    LONG cp = GetOleCP(pOle,iMid);

    if(cp < cpMin)
    {
        return FindFirstOleInrange(pOle,iMid+1,iEnd,cpMin,cpMax);
    }else if(cp >= cpMax)
    {
        return FindFirstOleInrange(pOle,iBegin,iMid,cpMin,cpMax);
    }else
    {
        int iRet = iMid;
        while(iRet>iBegin)
        {
            cp = GetOleCP(pOle,iRet-1);
            if(cp<cpMin) break;
            iRet --;
        }
        return iRet;
    }
}

//find Last Ole Object in char range of [cpMin,cpMax)
int FindLastOleInrange(IRichEditOle *pOle, int iBegin,int iEnd,int cpMin,int cpMax)
{
    if(iBegin==iEnd) return -1;

    int iMid = (iBegin + iEnd)/2;

    LONG cp = GetOleCP(pOle,iMid);

    if(cp < cpMin)
    {
        return FindLastOleInrange(pOle,iMid+1,iEnd,cpMin,cpMax);
    }else if(cp >= cpMax)
    {
        return FindLastOleInrange(pOle,iBegin,iMid,cpMin,cpMax);
    }else
    {
        int iRet = iMid;
        while(iRet<(iEnd-1))
        {
            cp = GetOleCP(pOle,iRet+1);
            if(cp>=cpMax) break;
            iRet ++;
        }
        return iRet;
    }
}

#define  MAX_FULL_SMILEIES   50     //超过MAX_FULL_SMILEIES个表情可见时不计算表情位置，直接全屏刷新。

void CSoSmileyCtrl::UpdateSmiley()
{
    if(!m_pSmileyHost) return ;
    CComPtr<IRichEditOle>  ole;
    LRESULT lMsgRet = 0;
    m_pSmileyHost->SendMessage(EM_GETOLEINTERFACE, 0, (LPARAM)&ole,&lMsgRet);
    if (!lMsgRet) return;

    //获得可见字符范围
    m_pSmileyHost->SendMessage(EM_GETFIRSTVISIBLELINE,0,0,&lMsgRet);

    int iFirstLine = (int)lMsgRet;
    RECT rcView;
    m_pSmileyHost->SendMessage(EM_GETRECT,0,(LPARAM)&rcView,&lMsgRet);
    POINT pt={rcView.right+1,rcView.bottom-2};

    m_pSmileyHost->SendMessage(EM_LINEINDEX,iFirstLine,0,&lMsgRet);
    LONG cpFirst = (LONG)lMsgRet;
    m_pSmileyHost->SendMessage(EM_CHARFROMPOS,0,(LPARAM)&pt,&lMsgRet);
    LONG cpLast  = (LONG)lMsgRet;

    //采用两分法查找在可见范围中的OLE对象
    int nCount=ole->GetObjectCount();

    BOOL bFind = FALSE;

    int iFirstVisibleOle = FindFirstOleInrange(ole,0,nCount,cpFirst,cpLast);
    if(iFirstVisibleOle==-1) return;
    int iLastVisibleOle = FindLastOleInrange(ole,iFirstVisibleOle,nCount,cpFirst,cpLast);
    
    if(iLastVisibleOle - iFirstVisibleOle >= MAX_FULL_SMILEIES)
    {
        m_pSmileyHost->InvalidateRect(NULL);
    }else
    {
        RECT rcSmiley={0};
        if(GetSmileyPos(ole,iFirstVisibleOle,iLastVisibleOle,&rcSmiley))
        {
            rcSmiley.top-=2;
            m_pSmileyHost->InvalidateRect(&rcSmiley);
        }
    }
}


BOOL CSoSmileyCtrl::GetSmileyPos(IRichEditOle *ole,int iFirst,int iLast,LPRECT pRect)
{
    ATLASSERT(m_pSmileyHost);
    
    BOOL bFind=FALSE;    
    for(int i=iFirst;i<=iLast;i++)
    {
        REOBJECT reobj={0};
        reobj.cbStruct=sizeof(REOBJECT);
        HRESULT hr=ole->GetObject(i,&reobj,REO_GETOBJ_NO_INTERFACES);
        if(FAILED(hr)) break;
        
        if (reobj.clsid==__uuidof(CSoSmileyCtrl) && reobj.dwUser==m_dwID)
        {
            long left, bottom;
            HRESULT res;
            ITextDocument * iDoc=NULL;
            ITextRange *iRange=NULL;

            ole->QueryInterface(__uuidof(ITextDocument),(void**)&iDoc);
            if (!iDoc) break;

            iDoc->Range(reobj.cp, reobj.cp, &iRange);

            BOOL bBottom=TRUE;
            if (reobj.dwFlags&REO_BELOWBASELINE)
                res=iRange->GetPoint(TA_BOTTOM|TA_LEFT, &left, &bottom);
            else
                res=iRange->GetPoint(TA_BASELINE|TA_LEFT, &left, &bottom);
            if (res!=S_OK) //object is out of screen let do normal fireview change
            {
                res=iRange->GetPoint(TA_TOP|TA_LEFT, &left, &bottom);
                bBottom = FALSE;
            }

            iRange->Release();
            iDoc->Release();

            if (res!=S_OK) //object is out of screen let do normal fireview change
            {
                memset(pRect,0,sizeof(RECT));
                break;
            }

            SIZE sz;
            m_pSmileySource->GetSize(&sz);

            DWORD nom=1, den=1;
            LRESULT lMsgRet =0;
            m_pSmileyHost->SendMessage( EM_GETZOOM, (WPARAM)&nom, (LPARAM)&den,&lMsgRet);
            if(lMsgRet && nom && den)
            {
                sz.cx = sz.cx * nom / den;
                sz.cy = sz.cy * nom / den;
            }
            CRect rcHost;
            m_pSmileyHost->GetHostRect(&rcHost);


            CPoint pt(left,bottom-(bBottom?sz.cy:0));
            pt -= rcHost.TopLeft();

            CRect rc(pt,sz);
            memcpy(pRect,rc,sizeof(RECT));
            bFind = TRUE;

            break;            
        }
    }
    return bFind;
}

HRESULT CSoSmileyCtrl::Load( IStorage* pStorage )
{
    //向RichEditFlags字段中写入3个REOBJECT标志。分析了Richedit20的wince源代码后发现对象初始化完成插入richedit前会从"RichEditFlags"流中读3个标志位
    CComPtr<IStream> spStream;
    static LPCOLESTR vszRichEditFlags = OLESTR("RichEditFlags");
    HRESULT hr = pStorage->CreateStream(vszRichEditFlags,
        STGM_WRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE,
        0, 0, &spStream);
    if (FAILED(hr)) return hr;
    DWORD dwFlag=REO_BELOWBASELINE,dwUser=m_dwID,dwAspect=DVASPECT_CONTENT;
    spStream->Write(&dwFlag,sizeof(dwFlag),NULL);
    spStream->Write(&dwUser,sizeof(dwUser),NULL);
    spStream->Write(&dwAspect,sizeof(dwAspect),NULL);

    return IPersistStorageImpl<CSoSmileyCtrl>::Load(pStorage);
}

STDMETHODIMP CSoSmileyCtrl::SetClientSite(IOleClientSite *pClientSite)
{
    if(pClientSite) 
    {
        CComPtr<IRichEditOleCallback> pCallback;
        HRESULT hr=pClientSite->QueryInterface(IID_IRichEditOleCallback,(void**)&pCallback);
        if(FAILED(hr)) return E_FAIL;

        //从callback中获取host
        hr = pCallback->QueryInterface(__uuidof(ISmileyHost),(LPVOID*)&m_pSmileyHost);
        if(FAILED(hr)) return E_FAIL;
    }
    
    return __super::SetClientSite(pClientSite);
}

// RichEditOle.cpp : implementation file
//
#include "stdafx.h"
#include "RichEditOle.h"
#include <gdialpha.h>
#include <GdiPlus.h>
#pragma comment(lib,"gdiplus")


HRESULT GetSmileyHost(SRichEdit * pRichedit,ISmileyHost ** ppHost)
{
    SComPtr<IRichEditOle> ole;
    if(!pRichedit->SSendMessage(EM_GETOLEINTERFACE,0,(LPARAM)(void**)&ole))
    {
        return S_FALSE;
    }
    SComPtr<IRichEditOleCallback> pCallback;
    HRESULT hr = ole->QueryInterface(IID_IRichEditOleCallback,(LPVOID*)&pCallback);
    if(SUCCEEDED(hr))
    {
        hr = pCallback->QueryInterface(__uuidof(ISmileyHost),(LPVOID*)ppHost);
    }
    return hr;
}

//////////////////////////////////////////////////////////////////////////
//  ImageItem

using namespace Gdiplus;

float FitSize(CSize& InSize, CSize& ImageSize)
{
    float scaleX= (InSize.cx>0) ? (float)InSize.cx/ImageSize.cx : 0;
    float scaleY= (InSize.cy>0)? (float)InSize.cy/ImageSize.cy : 0;
    float scale=1;
    if (scaleX&&scaleY) 
        scale=min(scaleX, scaleY);
    else if ( scaleX||scaleY )
        scale = scaleX ? scaleX : scaleY;
    InSize.cx=(INT)(ImageSize.cx * scale);
    InSize.cy=(INT)(ImageSize.cy * scale);
    return scale;
}

HBITMAP CreateGDIBitmap(HDC hdc, int nWid,int nHei )
{
    BITMAPINFO bmi;
    memset(&bmi, 0, sizeof(bmi));
    bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth       = nWid;
    bmi.bmiHeader.biHeight      = -nHei; // top-down image 
    bmi.bmiHeader.biPlanes      = 1;
    bmi.bmiHeader.biBitCount    = 32;
    bmi.bmiHeader.biCompression = BI_RGB;
    bmi.bmiHeader.biSizeImage   = 0;

    HBITMAP hBmp=CreateDIBSection(hdc,&bmi,DIB_RGB_COLORS,NULL,0,0);
    return hBmp;
}

HBITMAP GetBitmapFromFile(const SStringW& strFilename, 
                       int& m_nFrameCount, CSize& m_FrameSize, CSize& ImageSize,
                       UINT* &m_pFrameDelays )
{
    Gdiplus::Bitmap * bmpSrc= new Gdiplus::Bitmap((LPCWSTR)strFilename);    
    if (!bmpSrc) return NULL;
    GUID   pageGuid = FrameDimensionTime;
    // Get the number of frames in the first dimension.
    m_nFrameCount = max(1, bmpSrc->GetFrameCount(&pageGuid));


    CSize imSize(bmpSrc->GetWidth(),bmpSrc->GetHeight());
    m_FrameSize=ImageSize;
    float scale=FitSize(m_FrameSize,imSize);
    
    HDC hdc = GetDC(NULL);
    HDC hMemDC = CreateCompatibleDC(hdc);
    HBITMAP hBmp = CreateGDIBitmap(hdc,m_FrameSize.cx*m_nFrameCount, m_FrameSize.cy);
    SelectObject(hMemDC,hBmp);

    Graphics *g = new Gdiplus::Graphics(hMemDC);

    ImageAttributes attr;
    if ( scale!=1 ) 
    {
        g->SetInterpolationMode(InterpolationModeHighQualityBicubic);
        g->SetPixelOffsetMode(PixelOffsetModeHighQuality);            
        if ( scale<1 )
        {
            attr.SetGamma((REAL)1.2,ColorAdjustTypeBitmap); //some darker to made sharpen
        }
    }
    g->Clear(Color(0));
    if (m_nFrameCount>1)
    {
        m_pFrameDelays=new UINT[m_nFrameCount];
        int nSize = bmpSrc->GetPropertyItemSize(PropertyTagFrameDelay);
        // Allocate a buffer to receive the property item.
        PropertyItem* pDelays = (PropertyItem*) new char[nSize];
        bmpSrc->GetPropertyItem(PropertyTagFrameDelay, nSize, pDelays);
        for (int i=0; i<m_nFrameCount; i++)
        {
            GUID pageGuid = FrameDimensionTime;
            bmpSrc->SelectActiveFrame(&pageGuid, i);
            Rect rect(i*m_FrameSize.cx,0,m_FrameSize.cx, m_FrameSize.cy);
            if (scale>=1 )
                g->DrawImage(bmpSrc,rect,0,0,bmpSrc->GetWidth(),bmpSrc->GetHeight(), UnitPixel/*, &attr*/);
            else
            {
                Bitmap bm2(bmpSrc->GetWidth(),bmpSrc->GetHeight(), PixelFormat32bppARGB);
                Graphics g2(&bm2);
                g2.DrawImage(bmpSrc,Rect(0,0,bm2.GetWidth(),bm2.GetHeight()),0,0,bmpSrc->GetWidth(),bmpSrc->GetHeight(), UnitPixel);
                g->DrawImage(&bm2,rect,0,0,bm2.GetWidth(),bm2.GetHeight(), UnitPixel, &attr);
            }
            m_pFrameDelays[i]=10*max(((int*) pDelays->value)[i], 10);
        }   
        delete [] pDelays;
    }
    else
    {
        Rect rect(0,0,m_FrameSize.cx, m_FrameSize.cy);
        g->DrawImage(bmpSrc,rect,0,0,bmpSrc->GetWidth(),bmpSrc->GetHeight(), UnitPixel, &attr);
        m_pFrameDelays=NULL;
    }
    ImageSize=CSize(bmpSrc->GetWidth(),bmpSrc->GetHeight());
    delete g;
    delete bmpSrc;
    DeleteDC(hMemDC);
    ReleaseDC(NULL,hdc);
    return hBmp;
}


ImageItem::ImageItem() : 
m_nRef( 0 ), 
m_hMemDC( NULL ),
m_nFrameCount( 0 ),
m_pFrameDelays( NULL )
{

}
ImageItem::~ImageItem()
{
    if(m_hMemDC)
    {
        HBITMAP hBmp = (HBITMAP)::GetCurrentObject(m_hMemDC,OBJ_BITMAP);
        DeleteDC(m_hMemDC);
        DeleteObject(hBmp);
        m_hMemDC=NULL;
    }
    if ( m_pFrameDelays ) delete [] m_pFrameDelays;
}

BOOL ImageItem::LoadImageFromFile(const SStringW& strFilename, int nHeight)
{
    ATLASSERT(m_hMemDC == NULL);
    m_imgid.m_nHeight=nHeight;
    m_imgid.m_strFilename=strFilename;

    CSize ImageSize(0, nHeight);
    HBITMAP hBmp = GetBitmapFromFile(strFilename, m_nFrameCount, m_FrameSize, ImageSize, m_pFrameDelays );
    if(!hBmp) return FALSE;
    HDC hDC = ::GetDC(NULL);
    m_hMemDC = CreateCompatibleDC(hDC);
    ::SelectObject(m_hMemDC,hBmp);
    ReleaseDC(NULL,hDC);
    return TRUE;
}

int ImageItem::Release()
{
    int nRef = --m_nRef;
    if (nRef < 1)
    {
        delete this;
    }
    return nRef;
}

void ImageItem::Draw( HDC hdc,LPCRECT pRect,int iFrame )
{
    if(m_hMemDC)
    {
        BLENDFUNCTION bf={ AC_SRC_OVER,0,0xFF,AC_SRC_ALPHA};
        AlphaBlend(hdc,pRect->left,pRect->top,pRect->right-pRect->left,pRect->bottom-pRect->top,
                m_hMemDC,m_FrameSize.cx*iFrame,0,m_FrameSize.cx,m_FrameSize.cy,
                bf);
    }
}
/////////////////////////////////////////////////////////////////////////////
ULONG_PTR   CSmileySource::_gdiPlusToken    = 0;
CSmileySource::IMAGEPOOL CSmileySource::_imgPool;


bool CSmileySource::GdiplusStartup( void )
{
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;

    return Gdiplus::Ok == Gdiplus::GdiplusStartup(&_gdiPlusToken, &gdiplusStartupInput, NULL);
}

void CSmileySource::GdiplusShutdown( void )
{
    if (_gdiPlusToken != 0)
    {
        Gdiplus::GdiplusShutdown(_gdiPlusToken);
        _gdiPlusToken = 0;
    }
}

CSmileySource::CSmileySource():m_pImg(NULL),m_cRef(0)
{

}

CSmileySource::~CSmileySource()
{
    if(m_pImg)
    {
        ImageID id = m_pImg->GetImageID();
        if(m_pImg->Release()==0)
        {
            _imgPool.erase(id);
        }
    }
}


HRESULT CSmileySource::Stream_Load( /* [in] */ LPSTREAM pStm )
{
    int nFileLen=0;
    pStm->Read(&nFileLen,4,NULL);
    wchar_t *pszFileName=new wchar_t[nFileLen+1];
    pStm->Read(pszFileName,nFileLen*2,NULL);
    pszFileName[nFileLen]=0;
    int nHeight = 0;
    pStm->Read(&nHeight,4,NULL);

    HRESULT hr = Init((WPARAM)pszFileName,(LPARAM)nHeight);
    delete []pszFileName;
    return hr;
}

HRESULT CSmileySource::Stream_Save( /* [in] */ LPSTREAM pStm )
{
    if(!m_pImg) return E_FAIL;
    ImageID id = m_pImg->GetImageID();
    int nFileLen = id.m_strFilename.GetLength();
    pStm->Write(&nFileLen,4,NULL);
    pStm->Write((LPCWSTR)id.m_strFilename,nFileLen*2,NULL);
    pStm->Write(&id.m_nHeight,4,NULL);

    return S_OK;
}

HRESULT CSmileySource::Init( /* [in] */ WPARAM wParam, /* [in] */ LPARAM lParam )
{
    ImageID imgid;
    imgid.m_strFilename = (wchar_t*)wParam;
    imgid.m_nHeight = (int)lParam;
    
    if(m_pImg)
    {
        if(!m_pImg->IsEqual(imgid))
        {//设置新图
            ImageID oldID = m_pImg->GetImageID();
            if(m_pImg->Release() ==0 )
                _imgPool.erase(oldID);
            m_pImg = NULL;
        }else
        {//相同的图，直接返回
            return S_OK;
        }
    }
    IMAGEPOOL::iterator it = _imgPool.find(imgid);
    if(it==_imgPool.end())
    {//在pool中没有找到
        ImageItem *pImg = new ImageItem;
        if(!pImg->LoadImageFromFile(imgid.m_strFilename,imgid.m_nHeight))
        {
            delete pImg;
            return E_INVALIDARG;
        }
        _imgPool[imgid] = pImg;
        m_pImg = pImg;
    }else
    {
        m_pImg = it->second;
    }
    m_pImg->AddRef();
    return S_OK;
}

HRESULT CSmileySource::GetFrameCount( /* [out] */ int *pFrameCount )
{
    if(!m_pImg) return E_FAIL;
    *pFrameCount = m_pImg->GetFrameCount();
    return S_OK;
}

HRESULT CSmileySource::GetFrameDelay( /* [in] */ int iFrame, /* [out] */ int *pFrameDelay )
{
    if(!m_pImg || m_pImg->GetFrameCount()<=1) return E_FAIL;
    if(iFrame>=(int)m_pImg->GetFrameCount()) return E_INVALIDARG;
    *pFrameDelay = m_pImg->GetFrameDelays()[iFrame];
    return S_OK;
}


HRESULT CSmileySource::GetSize( /* [out] */ LPSIZE pSize )
{
    if(!m_pImg) return E_FAIL;
    *pSize = m_pImg->GetFrameSize();
    return S_OK;
}

HRESULT CSmileySource::Draw( /* [in] */ HDC hdc, /* [in] */ LPCRECT pRect , int iFrameIndex)
{
    if(!m_pImg) return E_FAIL;
    m_pImg->Draw(hdc,pRect,iFrameIndex);
    return S_OK;
}

ISmileySource * CSmileySource::CreateInstance()
{
    ISmileySource *pRet = new CSmileySource;
    pRet->AddRef();
    return pRet;
}

//////////////////////////////////////////////////////////////////////////
//  CSmileyHost
CSmileyHost::CSmileyHost() :m_pHost(0),m_cRef(1),m_cTime(0)
{

}

CSmileyHost::~CSmileyHost()
{
}

void CSmileyHost::ClearTimer()
{
    SPOSITION pos = m_lstTimerInfo.GetHeadPosition();
    while(pos)
    {
        TIMERINFO *pTi = m_lstTimerInfo.GetNext(pos);
        pTi->pHandler->Clear();
        pTi->pHandler->Release();
        delete pTi;
    }
    m_lstTimerInfo.RemoveAll();
}

HRESULT STDMETHODCALLTYPE CSmileyHost::SetTimer( /* [in] */ ITimerHandler * pTimerHander, /* [in] */ int nInterval )
{
    SPOSITION pos = m_lstTimerInfo.GetHeadPosition();
    while(pos)
    {
        if(m_lstTimerInfo.GetNext(pos)->pHandler == pTimerHander) return S_FALSE;
    } 

    m_lstTimerInfo.AddTail(new TIMERINFO(pTimerHander,nInterval));
    pTimerHander->AddRef();
        
    return S_OK;
}

#define INTERVAL    2
HRESULT STDMETHODCALLTYPE  CSmileyHost::OnTimer( int nInterval )
{
    if(++m_cTime<INTERVAL) return S_OK;
    m_cTime=0;
    
    //找到所有到时间的定时器,防止在执行定时器时插入新定时器，需要先查找再执行。
    TIMERHANDLER_LIST lstDone;
    SPOSITION pos = m_lstTimerInfo.GetHeadPosition();
    while(pos)
    {
        SPOSITION pos2 = pos;
        TIMERINFO *pti = m_lstTimerInfo.GetNext(pos);
        pti->nInterval -= nInterval*INTERVAL;
        if(pti->nInterval <= 0)
        {
            lstDone.AddTail(pti);
            m_lstTimerInfo.RemoveAt(pos2);
        }
    }
    if(lstDone.IsEmpty()) return S_OK;

    //计算出刷新区域
    CAutoRefPtr<IRegion> rgn;
    GETRENDERFACTORY->CreateRegion(&rgn);;
    RECT rcSmiley;
    pos = lstDone.GetHeadPosition();
    while(pos)
    {
        TIMERINFO *pTi = lstDone.GetNext(pos);
        pTi->pHandler->GetRect(&rcSmiley);
        int nWid=rcSmiley.right-rcSmiley.left;
        rgn->CombineRect(&rcSmiley,RGN_OR);
    }
    
    CRect rcClient;
    m_pHost->GetClientRect(&rcClient);
    rgn->CombineRect(&rcClient,RGN_AND);
    
    //刷新表情
    IRenderTarget *pRT = m_pHost->GetRenderTarget(OLEDC_PAINTBKGND,rgn);
    m_pHost->SSendMessage(WM_ERASEBKGND,(WPARAM)pRT);
    
    HDC hdc = pRT->GetDC(0);

    pRT->GetClipBox(&rcClient);
    ALPHAINFO ai;
    CGdiAlpha::AlphaBackup(hdc,&rcClient,ai);

    pos = lstDone.GetHeadPosition();
    while(pos)
    {
        TIMERINFO *pTi = lstDone.GetNext(pos);
        pTi->pHandler->OnTimer(hdc);
        pTi->pHandler->Release();
        delete pTi;
    }
    CGdiAlpha::AlphaRestore(ai);
    
    pRT->ReleaseDC(hdc);
    m_pHost->ReleaseRenderTarget(pRT);

    return S_OK;
}

HRESULT STDMETHODCALLTYPE CSmileyHost::CreateSource( ISmileySource ** ppSource )
{
    *ppSource = CSmileySource::CreateInstance();
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CSmileyHost::InvalidateRect( /* [in] */ LPCRECT pRect )
{
    m_pHost->InvalidateRect(pRect);
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CSmileyHost::GetHostRect( /* [out] */ LPRECT prcHost )
{
    ::GetWindowRect(m_pHost->GetContainer()->GetHostHwnd(),prcHost);
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CSmileyHost::SendMessage( /* [in] */ UINT uMsg, /* [in] */ WPARAM wParam, /* [in] */ LPARAM lParam, /* [out] */ LRESULT *pRet )
{
    LRESULT lRet=m_pHost->SSendMessage(uMsg,wParam,lParam);
    if(pRet) *pRet = lRet;
    return S_OK;
}

ULONG STDMETHODCALLTYPE CSmileyHost::Release( void )
{
    LONG lRet = --m_cRef;
    if(lRet == 0)
    {
        delete this;
    }
    return lRet;
}

ULONG STDMETHODCALLTYPE CSmileyHost::AddRef( void )
{
    return ++m_cRef;
}

HRESULT STDMETHODCALLTYPE CSmileyHost::QueryInterface( /* [in] */ REFIID riid, /* [iid_is][out] */ __RPC__deref_out void __RPC_FAR *__RPC_FAR *ppvObject )
{
    return E_NOINTERFACE;
}

HRESULT STDMETHODCALLTYPE CSmileyHost::SetRichedit(/* [in] */DWORD_PTR dwRichedit)
{
    SASSERT(!m_pHost);
    m_pHost = (SRichEdit *)dwRichedit;
    //订阅richedit的EN_UPDATE消息,用来更新表情坐标
    m_pHost->GetEventSet()->subscribeEvent(EVT_VISIBLECHANGED,Subscriber(&CSmileyHost::OnHostVisibleChanged,this));
    m_pHost->GetEventSet()->subscribeEvent(EventRENotify::EventID,Subscriber(&CSmileyHost::OnHostUpdate,this));
    return S_OK;
}

bool CSmileyHost::OnHostVisibleChanged(SOUI::EventArgs *pEvt)
{
    if(m_pHost->IsVisible(TRUE))
        m_pHost->GetContainer()->RegisterTimelineHandler(this);
    else
        m_pHost->GetContainer()->UnregisterTimelineHandler(this);
    return false;
}

bool CSmileyHost::OnHostUpdate(SOUI::EventArgs *pEvt)
{
    EventRENotify *pReNotify = (EventRENotify*)pEvt;
    if(pReNotify->iNotify == EN_UPDATE)
    {
        ClearTimer();
    }
    return false; 
}



//////////////////////////////////////////////////////////////////////////
// CRichEditOleCallback

CRichEditOleCallback::CRichEditOleCallback():m_dwRef(1),m_iStorage(0)
{
    HRESULT hResult = ::StgCreateDocfile(NULL,
        STGM_TRANSACTED | STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE| STGM_DELETEONRELEASE,
        0, &m_stg );

    if ( m_stg == NULL ||
        hResult != S_OK )
    {
//         AfxThrowOleException( hResult );
    }

    m_pSmileyHost = new CSmileyHost;
}

CRichEditOleCallback::~CRichEditOleCallback()
{
    m_pSmileyHost->ClearTimer();
    m_pSmileyHost->Release();
}

HRESULT STDMETHODCALLTYPE 
CRichEditOleCallback::GetNewStorage(LPSTORAGE* ppStg)
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

HRESULT STDMETHODCALLTYPE 
CRichEditOleCallback::QueryInterface(REFIID iid, void ** ppvObject)
{

    HRESULT hr = S_OK;
    *ppvObject = NULL;

    if ( iid == IID_IUnknown ||
        iid == IID_IRichEditOleCallback )
    {
        *ppvObject = this;
        AddRef();
    }else if( iid == __uuidof(ISmileyHost))
    {
        *ppvObject = m_pSmileyHost;
        m_pSmileyHost->AddRef();
    }
    else
    {
        hr = E_NOINTERFACE;
    }

    return hr;
}



ULONG STDMETHODCALLTYPE 
CRichEditOleCallback::AddRef()
{
    return ++m_dwRef;
}



ULONG STDMETHODCALLTYPE 
CRichEditOleCallback::Release()
{
    if ( --m_dwRef == 0 )
    {
        delete this;
        return 0;
    }

    return m_dwRef;
}


HRESULT STDMETHODCALLTYPE 
CRichEditOleCallback::GetInPlaceContext(LPOLEINPLACEFRAME FAR *lplpFrame,
                                        LPOLEINPLACEUIWINDOW FAR *lplpDoc, LPOLEINPLACEFRAMEINFO lpFrameInfo)
{
    return S_OK;
}


HRESULT STDMETHODCALLTYPE 
CRichEditOleCallback::ShowContainerUI(BOOL fShow)
{
    return S_OK;
}



HRESULT STDMETHODCALLTYPE 
CRichEditOleCallback::QueryInsertObject(LPCLSID lpclsid, LPSTORAGE lpstg, LONG cp)
{
    return S_OK;
}


HRESULT STDMETHODCALLTYPE 
CRichEditOleCallback::DeleteObject(LPOLEOBJECT lpoleobj)
{
    return S_OK;
}



HRESULT STDMETHODCALLTYPE 
CRichEditOleCallback::QueryAcceptData(LPDATAOBJECT lpdataobj, CLIPFORMAT FAR *lpcfFormat,
                                      DWORD reco, BOOL fReally, HGLOBAL hMetaPict)
{
    return S_OK;
}


HRESULT STDMETHODCALLTYPE 
CRichEditOleCallback::ContextSensitiveHelp(BOOL fEnterMode)
{
    return S_OK;
}



HRESULT STDMETHODCALLTYPE 
CRichEditOleCallback::GetClipboardData(CHARRANGE FAR *lpchrg, DWORD reco, LPDATAOBJECT FAR *lplpdataobj)
{
    return E_NOTIMPL;
}


HRESULT STDMETHODCALLTYPE 
CRichEditOleCallback::GetDragDropEffect(BOOL fDrag, DWORD grfKeyState, LPDWORD pdwEffect)
{
    return S_OK;
}

HRESULT STDMETHODCALLTYPE 
CRichEditOleCallback::GetContextMenu(WORD seltyp, LPOLEOBJECT lpoleobj, CHARRANGE FAR *lpchrg,
                                     HMENU FAR *lphmenu)
{
    return S_OK;
}

BOOL CRichEditOleCallback::SetRicheditOleCallback(SRichEdit *pRichedit)
{
    CRichEditOleCallback *pCallback = new CRichEditOleCallback;
    pCallback->m_pSmileyHost->SetRichedit((DWORD_PTR)pRichedit);
    BOOL bRet=pRichedit->SSendMessage(EM_SETOLECALLBACK,0,(LPARAM)pCallback);
    pCallback->Release();
    return bRet;
}

#include "stdafx.h"
#include "souistd.h"
#include "SImRichedit.h"
#include "TOM.h"
#include <atlcomcli.h>
#include "interface\render-i.h"
#include "RichEditOleCallback.h"
#include "atl.mini\SComHelper.h"
#include <time.h>
#include "utils.h"
#include "RichEditOleCtrls.h"
#include "helper\SMenuEx.h"
#include "gdialpha.h"
#include "ExtendEvents.h"
#include "ClipboardConverter.h"

#ifndef LY_PER_INCH
#define LY_PER_INCH 1440
#endif

namespace SOUI
{

    //
    // richedit内部定义的几个timer id
    //
#define RETID_BGND_RECALC	0x01af
#define RETID_AUTOSCROLL	0x01b0
#define RETID_SMOOTHSCROLL	0x01b1
#define RETID_DRAGDROP		0x01b2
#define RETID_MAGELLANTRACK	0x01b3


    //-------------------------------------------------------------------------
    //
    // richedit internal helpers
    //
    //-------------------------------------------------------------------------

    /*
     * 获取指定OLE的cp
     * @param pOle: ole对象
     * @param iOle: richedit里的第几个ole
     * @return: ole的cp
     */
    LONG GetOleCP(IRichEditOle *pOle, int iOle)
    {
        REOBJECT reobj = { 0 };
        reobj.cbStruct = sizeof(REOBJECT);
        pOle->GetObject(iOle, &reobj, REO_GETOBJ_NO_INTERFACES);
        return reobj.cp;
    }

    /*
     * 采用折半查找的方法在[cpMin,cpMax)中找出第一个可见的OLE下标
     *
     * @param pOle: richedit的OLE对象
     * @param iBegin: 查找的起始位置
     * @param iEnd: 查找的结束位置
     * @param cpMin: richedit第一个可见字符的位置
     * @param cpMax: richedit最后一个可见字符的位置
     *
     * @return: 如果找到第一个可见的ole则返回ole的cp，否则返回-1.
     */
    int FindFirstOleInrange(IRichEditOle *pOle, int iBegin, int iEnd, int cpMin, int cpMax)
    {
        if (iBegin == iEnd) return -1;

        int iMid = (iBegin + iEnd) / 2;

        LONG cp = GetOleCP(pOle, iMid);

        if (cp < cpMin)
        {
            return FindFirstOleInrange(pOle, iMid + 1, iEnd, cpMin, cpMax);
        }
        else if (cp >= cpMax)
        {
            return FindFirstOleInrange(pOle, iBegin, iMid, cpMin, cpMax);
        }
        else
        {
            int iRet = iMid;
            while (iRet > iBegin)
            {
                cp = GetOleCP(pOle, iRet - 1);
                if (cp < cpMin) break;
                iRet--;
            }
            return iRet;
        }
    }

    /*
     * 采用折半查找的方法在[cpMin,cpMax)中找出最后一个可见的OLE下标
     *
     * @param pOle: richedit的OLE对象
     * @param iBegin: 查找的起始位置
     * @param iEnd: 查找的结束位置
     * @param cpMin: richedit第一个可见字符的位置
     * @param cpMax: richedit最后一个可见字符的位置
     *
     * @return: 如果找到最后一个可见的ole则返回ole的cp，否则返回-1.
     */
    int FindLastOleInrange(IRichEditOle *pOle, int iBegin, int iEnd, int cpMin, int cpMax)
    {
        if (iBegin == iEnd) return -1;

        int iMid = (iBegin + iEnd) / 2;

        LONG cp = GetOleCP(pOle, iMid);

        if (cp < cpMin)
        {
            return FindLastOleInrange(pOle, iMid + 1, iEnd, cpMin, cpMax);
        }
        else if (cp >= cpMax)
        {
            return FindLastOleInrange(pOle, iBegin, iMid, cpMin, cpMax);
        }
        else
        {
            int iRet = iMid;
            while (iRet < (iEnd - 1))
            {
                cp = GetOleCP(pOle, iRet + 1);
                if (cp >= cpMax) break;
                iRet++;
            }
            return iRet;
        }
    }

    //-------------------------------------------------------------------------
    //
    // impl RichEditDropTarget
    //
    //-------------------------------------------------------------------------

    class RichEditDropTarget : public IDropTarget
    {
    public:
        RichEditDropTarget(IRichEditObjHost *phost)
            :_ref(1)
            , _pHost(phost)
        {
            SASSERT(_pserv);
            _pserv = phost->GetTextServ();
            _pserv->AddRef();
        }

        ~RichEditDropTarget()
        {
            SASSERT(_pserv);
            _pserv->Release();
        }

        //IUnkown
        virtual HRESULT STDMETHODCALLTYPE QueryInterface(
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject)
        {
            HRESULT hr = E_NOINTERFACE;
            if (riid == __uuidof(IUnknown))
                *ppvObject = (IUnknown*) this, hr = S_OK;
            else if (riid == __uuidof(IDropTarget))
                *ppvObject = (IDropTarget*)this, hr = S_OK;
            if (SUCCEEDED(hr)) AddRef();
            return hr;
        }

        virtual ULONG STDMETHODCALLTYPE AddRef(void) { return ++_ref; }

        virtual ULONG STDMETHODCALLTYPE Release(void) {
            ULONG uRet = --_ref;
            if (uRet == 0) delete this;
            return uRet;
        }

        //IDropTarget
        virtual HRESULT STDMETHODCALLTYPE DragEnter(
            /* [unique][in] */ IDataObject *pDataObj,
            /* [in] */ DWORD grfKeyState,
            /* [in] */ POINTL pt,
            /* [out][in] */ DWORD *pdwEffect)
        {
            HRESULT hr = S_FALSE;
            IDropTarget *pDropTarget = NULL;
            hr = _pserv->TxGetDropTarget(&pDropTarget);
            if (SUCCEEDED(hr))
            {
                hr = pDropTarget->DragEnter(pDataObj, grfKeyState, pt, pdwEffect);
                *pdwEffect = DROPEFFECT_COPY;
                pDropTarget->Release();
            }
            return hr;
        }

        virtual HRESULT STDMETHODCALLTYPE DragLeave(void)
        {
            HRESULT hr = S_FALSE;
            IDropTarget *pDropTarget = NULL;
            hr = _pserv->TxGetDropTarget(&pDropTarget);
            if (SUCCEEDED(hr))
            {
                hr = pDropTarget->DragLeave();
                pDropTarget->Release();
            }
            return hr;
        }

        virtual HRESULT STDMETHODCALLTYPE DragOver(
            /* [in] */ DWORD grfKeyState,
            /* [in] */ POINTL pt,
            /* [out][in] */ DWORD *pdwEffect)
        {
            /*
            * 不传递DragOver给richedit，是因为richedit在DragOver时会动态计算
            * 滚动条的位置，导致页面可能会自动往上/往下滚动
            */
            *pdwEffect = DROPEFFECT_COPY;
            return S_OK;
        }

        virtual HRESULT STDMETHODCALLTYPE Drop(
            /* [unique][in] */ IDataObject *pDataObj,
            /* [in] */ DWORD grfKeyState,
            /* [in] */ POINTL pt,
            /* [out][in] */ DWORD *pdwEffect)
        {
            if (*pdwEffect == DROPEFFECT_NONE)
            {
                return S_FALSE;
            }

            if (!_pHost->IsEditable())
            {
                /*
                * 不传递drop给readonly状态的richedit，是因为不希望破坏richedit的已选状态。
                */
                RichFormatConv conv;
                if (conv.InitFromDataObject(pDataObj) != 0)
                {
                    _pHost->AcceptContent(&conv);
                }

                *pdwEffect = DROPEFFECT_COPY;
                return DragLeave();
            }

            HRESULT hr = S_FALSE;
            IDropTarget *pDropTarget = NULL;
            hr = _pserv->TxGetDropTarget(&pDropTarget);
            if (SUCCEEDED(hr))
            {
                hr = pDropTarget->Drop(pDataObj, grfKeyState, pt, pdwEffect);
                pDropTarget->Release();
            }

            return hr;
        }

    protected:

        IRichEditObjHost* _pHost;
        ITextServices*    _pserv;            // pointer to Text Services object
        LONG              _ref;
    };

    //-------------------------------------------------------------------------
    //
    // impl SImRichEdit
    //
    //-------------------------------------------------------------------------

    SImRichEdit::SImRichEdit() : _pTextDoc(NULL)
        , _pLastHoverObj(NULL)
        , _isBackgroundDirty(TRUE)
        , _lastDrawTicks(0)
        , _isDrawEnable(TRUE)
        , _scrollbarAtBottom(TRUE)
        , _scrollbarOutdated(FALSE)
        , _isDragging(FALSE)
        , _readOnlyBeforeDrag(FALSE)
        , _fixVScroll(TRUE)
        , _isCreatIme(false)
    {
        m_evtSet.addEvent(EVENTID(EventQueryAccept));
        m_evtSet.addEvent(EVENTID(EventRichEditObj));
        m_evtSet.addEvent(EVENTID(EventRichEditScroll));
    }

    SImRichEdit::~SImRichEdit()
    {
        GetContainer()->UnregisterTimelineHandler(this);
    }

    //------------------------------------------------------------------------------
    //
    // IRichEditObjHost methods
    //
    //------------------------------------------------------------------------------
    ISwndContainer* SImRichEdit::GetHostContainer()
    {
        return GetContainer();
    }

    CRect SImRichEdit::GetHostRect()
    {
        return GetClientRect();
    }

    CRect SImRichEdit::GetAdjustedRect()
    {
        CRect rc;

        rc = GetClientRect();
        rc.DeflateRect(m_rcInsetPixel);

        return rc;
    }

    int SImRichEdit::GetCharCount()
    {
        SComPtr<ITextRange> range2;
        _pTextDoc->Range((~(ULONG)0) / 2, (~(ULONG)0) / 2, &range2);
        long end;
        range2->GetEnd(&end);
        return end;
    }

    /*
    * 用来记录需要刷新的区域，该函数只是合并脏区域。每60 tick会尝试绘制一次数据，定时刷新在OnNextFame里进行。
    * @param rc: 需要刷新的区域
    */
    void SImRichEdit::DelayDraw(const CRect& rc)
    {
        if (!_pDelayDrawRgn)
        {
            GETRENDERFACTORY->CreateRegion(&_pDelayDrawRgn);
        }

        _pDelayDrawRgn->CombineRect(rc, RGN_OR);
    }

    /*
    * 主动刷新UI数据。
    * @param rcDraw: 需要刷新的区域
    */
    void SImRichEdit::DirectDraw(const CRect& rcDraw)
    {
        if (GetState()&WndState_Invisible) // 不用两个&&,不用能IsVisible做判断
        {
            return;
        }

        if (!_isDrawEnable || !_pBackgroundRt || _isBackgroundDirty)
        {
            return;
        }

        CRect rcClient = GetClientRect();
        rcClient.IntersectRect(rcClient, rcDraw);
        if (rcClient.IsRectNull() || !_pBackgroundRt)
        {
            return;
        }

        CAutoRefPtr<IRegion> rgn;
        GETRENDERFACTORY->CreateRegion(&rgn);
        rgn->CombineRect(rcClient, RGN_OR);

        /*
        * 这里有两种方法去画背景
        * - 1.调用窗口的GetRenderTarget，比较正宗，但是如果richedit被嵌套的层次较深，效率就比较慢
        * - 2.自己维护缓存背景，在onsize的时候去更新背景。好处是速度较快，但是背景有可能是不准确的。
        *
        * 用哪种画背景就看情况了，如果背景是纯色的，那就用第2种咯。
        *
        * =============================================================================
        * 对于触发gif的刷新，也有两种方法
        * - 1.触发richedit的Draw事件，让richedit去触发ole的绘制事件。
        *     这种方法比较方便，因为是richedit去触发的，不需要担心什么位置、
        *     选中状态方面的问题，但是效率较低。
        *
        * - 2.直接传一个dc或者renderTarget给ole对象，让它去画，最后
        *     贴到窗口上。这种方法效率较高，但是需要自己处理是否被选中
        *     等状态，比较麻烦。SoSmiley就是用这种方式。
        *
        * 这里用的是第1种去触发gif刷新,效率勉强还可以接受，如果有需要再改成第2种。
        */

        /*
        * 方法1画背景
        */
        //IRenderTarget *pRT = GetRenderTarget(OLEDC_PAINTBKGND,rgn);
        //SSendMessage(WM_ERASEBKGND,(WPARAM)pRT);
        //RedrawRegion(pRT, rgn);
        //ReleaseRenderTarget(pRT);

        /*
        * 方法2画背景
        */
        int n = GetTickCount();
        CAutoRefPtr<IRenderTarget> pRT = GetContainer()->OnGetRenderTarget(rcClient, 0);
        pRT->BitBlt(rcClient, _pBackgroundRt, rcClient.left, rcClient.top);
        int n1 = GetTickCount() - n;

        n = GetTickCount();
        RedrawRegion(pRT, rgn);
        GetContainer()->OnReleaseRenderTarget(pRT, rcClient, 0);
        int n2 = GetTickCount() - n;

        //STRACE(L"direct draw:%d, n1:%d, n2:%d", GetTickCount(), n1, n2);
    }

    HRESULT SImRichEdit::SendMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *pRet)
    {
        LRESULT lRet = SSendMessage(uMsg, wParam, lParam);
        if (pRet) *pRet = lRet;
        return S_OK;
    }

    ITextDocument* SImRichEdit::GetTextDoc()
    {
        return _pTextDoc;
    }

    ITextServices* SImRichEdit::GetTextServ()
    {
        return m_pTxtHost->GetTextService();
    }

    BOOL SImRichEdit::AcceptContent(RichFormatConv* conv)
    {
        EventQueryAccept evt(this);
        evt.Conv = conv;
        return FireEvent(evt);
    }

    /*
    * RichEdit对象发起的事件通知，比如点击了头像
    * @param obj:      发起事件通知的对象指针
    * @param subEvent: 事件类型，不同的obj有不同的事件类型
    * @param wParam：  参数含义看具体的事件
    * @param lParam：  参数含义看具体的事件
    */
    bool SImRichEdit::NotifyRichObjEvent(RichEditObj* obj, int subEvent, WPARAM wParam, LPARAM lParam)
    {
        EventRichEditObj evt(this);
        evt.RichObj = obj;
        evt.SubEventId = subEvent;
        evt.wParam = wParam;
        evt.lParam = lParam;

        return !!FireEvent(evt);
    }

    void SImRichEdit::EnableDraw(BOOL bEnable)
    {
        _isDrawEnable = bEnable;
    }

    void SImRichEdit::UpdateRichObj(RichEditObj* obj)
    {
        _scrollbarOutdated = TRUE;

        UpdateContext ctx;
        PreUpdate(ctx);
        obj->UpdatePosition();
        CRect rc = obj->GetRect();
        InvalidateRect(rc);
        DoneUpdate(ctx);
    }

    int SImRichEdit::GetRemainingLength()
    {
        return GetLimitText();
    }

    BOOL SImRichEdit::IsEditable()
    {
        return !GetReadOnly() && !_readOnlyBeforeDrag;
    }

    void SImRichEdit::Activate()
    {
        SetFocus();
    }

    /*
    * 插入内容到richedit
    * @param lpszContent: XML格式的内容
    * @param uInsertAt:   插入的位置，除正常的取值范围之外，还可以取以下两个值
    *                     - RECONTENT_LAST   ： 插入到最后
    *                     - RECONTENT_CARET  ： 插入到光标处
    *
    * @return UINT: 消息实际插入的位置
    */
    UINT SImRichEdit::InsertContent(LPCWSTR lpszContent, UINT uInsertAt/*=RECONTENT_LAST*/)
    {
        RichEditContent * p = CreateRichEditConent(lpszContent);
        if (p == NULL)
            return -1;

        CHARRANGE chr = { -1, -1 };

        if (uInsertAt == RECONTENT_CARET)
        {
            _richContents.Add(p);
        }
        else if (uInsertAt >= _richContents.GetCount())
        {
            uInsertAt = _richContents.GetCount();
            if (uInsertAt > 0)
            {
                chr = _richContents.GetAt(uInsertAt - 1)->GetCharRange();
                SetSel(chr.cpMax, chr.cpMax);
            }

            _richContents.Add(p);
        }
        else
        {
            chr = _richContents.GetAt(uInsertAt)->GetCharRange();
            SetSel(chr.cpMin, chr.cpMin);
            _richContents.InsertAt(uInsertAt, p);
        }

        UpdateContext ctx;
        PreUpdate(ctx);

        p->InsertIntoHost(this);
        p->UpdatePosition();
        chr = p->GetCharRange();
        ReLocateContents(uInsertAt, chr.cpMax - chr.cpMin);
        InvalidateRect(p->GetRect());

        if (_isDragging)
        {
            // 如果在拖拽状态下插入了内容，暂时不能设置SetReadOnly(TRUE)， 否则会崩溃，
            // 具体原因见OnMouseMove的注释。
            // 应该等到拖拽结束后(也就是在__super::OnMouseMove结束之后)再设置只读属性
            ctx.bReadOnly = FALSE;
        }
        DoneUpdate(ctx);
        UpdateVisibleCharRanges();

        return uInsertAt;
    }

    SStringW SImRichEdit::GetSelectedContent(CHARRANGE* lpchrg/*=NULL*/)
    {
        SASSERT(lpchrg != NULL);

        CHARRANGE selChr = *lpchrg;
        selChr.cpMin = (selChr.cpMin < 0) ? 0 : selChr.cpMin;
        selChr.cpMax = (selChr.cpMax == -1) ? GetCharCount() : selChr.cpMax;

        SStringW subText;
        SStringW content = L"<RichEditContent>";

        SComPtr<IRichEditOle> ole;
        SSendMessage(EM_GETOLEINTERFACE, 0, (LPARAM)&ole);
        int oleCount = ole->GetObjectCount();

        int oleCp = 0;
        for (int i = 0; i < oleCount && oleCp < selChr.cpMax; ++i)
        {
            REOBJECT reobj = { 0 };
            reobj.cbStruct = sizeof(REOBJECT);

            if (FAILED(ole->GetObject(i, &reobj, REO_GETOBJ_POLEOBJ)))
            {
                break;
            }

            oleCp = reobj.cp;
            if (reobj.cp < selChr.cpMin || reobj.cp >= selChr.cpMax)
            {
                reobj.poleobj->Release();
                continue;
            }

            if (selChr.cpMin < reobj.cp)
            {
                CHARRANGE chr = { selChr.cpMin, reobj.cp };
                GetRangeText(chr, subText);
                content += RichEditText::MakeFormatedText(subText);
            }

            RichEditOleBase* pOle = static_cast<RichEditOleBase*>(reobj.poleobj);
            content += pOle->GetSelFormatedText();

            reobj.poleobj->Release();
            selChr.cpMin = reobj.cp + 1;
        }

        if (selChr.cpMin < selChr.cpMax)
        {
            CHARRANGE chr = { selChr.cpMin, selChr.cpMax };
            GetRangeText(chr, subText);
            content += RichEditText::MakeFormatedText(subText);
        }

        content += L"</RichEditContent>";
        return content;
    }

    void SImRichEdit::DeleteContent(UINT uIndex)
    {
        if (uIndex >= _richContents.GetCount())
        {
            return;
        }

        RichEditObj     * pobj = _pLastHoverObj;
        RichEditContent * prec = _richContents.GetAt(uIndex);
        CHARRANGE         chr = prec->GetCharRange();
        UINT index = 0;

        for (; pobj != NULL; pobj = pobj->GetParent(), ++index)
        {
            if (prec == pobj)
            {
                _pLastHoverObj = NULL;
                break;
            }
        }

        UpdateContext ctx;
        PreUpdate(ctx);

        SetSel(chr.cpMin, chr.cpMax);
        SSendMessage(EM_REPLACESEL, TRUE, (LPARAM)(LPCWSTR)L"");
        ReLocateContents(uIndex, chr.cpMin - chr.cpMax);
        _richContents.RemoveAt(uIndex);

        DoneUpdate(ctx);
        delete prec;
    }

    void SImRichEdit::Clear()
    {
        SelectAll();
        SSendMessage(EM_REPLACESEL, FALSE, (LPARAM)L"", 0);
        EmptyUndoBuffer();

        for (size_t i = 0; i < _richContents.GetCount(); ++i)
        {
            RichEditContent * p = _richContents.GetAt(i);
            p->Release();
        }

        _richContents.RemoveAll();
        _pLastHoverObj = NULL;
    }

    void SImRichEdit::ScrollToBottom()
    {
        SSendMessage(WM_VSCROLL, (WPARAM)SB_BOTTOM, NULL);
    }

    BOOL SImRichEdit::IsScrollAtTop()
    {
        CRect rcTrack = GetSbPartRect(TRUE, SB_THUMBTRACK);
        return rcTrack.top == 0;
    }

    BOOL SImRichEdit::IsScrollAtBottom()
    {
        return _scrollbarAtBottom;
    }

    //-------------------------------------------------------------------------
    //
    // message wrapper
    //
    //-------------------------------------------------------------------------

    void SImRichEdit::GetSel(long* pStartCp, long* pEndCp)
    {
        CHARRANGE cr = { 0, 0 };
        SSendMessage(EM_EXGETSEL, NULL, (LPARAM)&cr);

        *pStartCp = cr.cpMin;
        *pEndCp = cr.cpMax;
    }

    void SImRichEdit::SetSel(int nStart, int nEnd)
    {
        CHARRANGE cr = { nStart, nEnd };
        //SSendMessage(EM_EXSETSEL, NULL, (LPARAM)&cr);
        SSendMessage(EM_SETSEL, nStart, nEnd);
    }

    BOOL SImRichEdit::GetDefCharFormat(CHARFORMAT& cf)
    {
        cf.cbSize = sizeof(CHARFORMAT);
        SSendMessage(EM_GETCHARFORMAT, 0, (LPARAM)&cf);
        return TRUE;
    }

    BOOL SImRichEdit::SetDefCharFormat(CHARFORMAT& cf)
    {
        cf.cbSize = sizeof(CHARFORMAT);
        SSendMessage(EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf);
        return TRUE;
    }

    BOOL SImRichEdit::SetParaFormat(PARAFORMAT2& pf)
    {
        pf.cbSize = sizeof(PARAFORMAT2);
        SSendMessage(EM_SETPARAFORMAT, 0, (LPARAM)&pf);
        return TRUE;
    }

    void SImRichEdit::SetFontSize(int size)
    {
        CHARFORMAT cf;
        GetDefCharFormat(cf);

        cf.dwMask = cf.dwMask | CFM_SIZE;
        cf.yHeight = size * 20;

        SetDefCharFormat(cf);
    }

    void SImRichEdit::SetSelectionColor(COLORREF color)
    {
        CHARFORMAT cf;
        GetDefCharFormat(cf);
        cf.dwMask |= CFM_COLOR;
        cf.crTextColor = color;

        SSendMessage(EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);
    }

    int SImRichEdit::LineFromChar(int ncp)
    {
        return (int)SSendMessage(EM_LINEFROMCHAR, ncp, 0);
    }

    int  SImRichEdit::LineIndex(int nLineNum)
    {
        return SSendMessage(EM_LINEINDEX, nLineNum, 0);
    }

    int SImRichEdit::LineLength(int nLineCP /* = -1 */)
    {
        return (int)SSendMessage(EM_LINELENGTH, nLineCP, 0);
    }

    void  SImRichEdit::PosFromChar(UINT ncp, POINT* pPos)
    {
        POINTL pt = { 0, 0 };
        SSendMessage(EM_POSFROMCHAR, (WPARAM)&pt, ncp);
        pPos->x = pt.x;
        pPos->y = pt.y;
    }

    int  SImRichEdit::GetFirstVisibleLine()
    {
        return (int)SSendMessage(EM_GETFIRSTVISIBLELINE, 0, 0);
    }

    int  SImRichEdit::CharFromPos(POINT pt)
    {
        POINTL  ptl = { pt.x, pt.y };
        return (int)SSendMessage(EM_CHARFROMPOS, 0, (LPARAM)&ptl);
    }

    BOOL SImRichEdit::IsAutoVScroll()
    {
        return (m_dwStyle & ES_AUTOVSCROLL) != 0;
    }

    void SImRichEdit::SetAutoVScroll(BOOL bEnabled)
    {
        if (bEnabled)
        {
            m_dwStyle |= ES_AUTOVSCROLL;
        }
        else
        {
            m_dwStyle &= ~ES_AUTOVSCROLL;
        }
        m_pTxtHost->GetTextService()->OnTxPropertyBitsChange(TXTBIT_SCROLLBARCHANGE, TXTBIT_SCROLLBARCHANGE);
    }

    void SImRichEdit::SelectAll()
    {
        CHARRANGE chr = { 0, -1 };
        SSendMessage(EM_EXSETSEL, 0, (LPARAM)&chr);
    }

    void SImRichEdit::Copy()
    {
        SSendMessage(WM_COPY, 0, 0);
    }

    void SImRichEdit::Cut()
    {
        SSendMessage(WM_CUT, 0, 0);
    }

    void SImRichEdit::Paste()
    {
        REPASTESPECIAL  reps = { 0, (DWORD_PTR)0 };
        SSendMessage(EM_PASTESPECIAL, CF_TEXT, (LPARAM)&reps, NULL);
    }

    void SImRichEdit::EmptyUndoBuffer()
    {
        SSendMessage(EM_EMPTYUNDOBUFFER, 0, 0, NULL);
    }


    //-------------------------------------------------------------------------
    //
    // extent methods
    //
    //-------------------------------------------------------------------------

    void SImRichEdit::DrawVisibleContents(IRenderTarget * pRt)
    {
        for (size_t npos = 0; npos < _richContents.GetCount(); ++npos)
        {
            RichEditContent * pContent = _richContents.GetAt(npos);
            CHARRANGE chr = pContent->GetCharRange();

            if (chr.cpMax < _visibleChr.cpMin || chr.cpMin > _visibleChr.cpMax)
            {
                continue;
            }

            pContent->DrawObject(pRt);
        }
    }

    void SImRichEdit::DrawVisibleGifs(IRenderTarget * pRt, const CRect& validRgnRect)
    {
        CComPtr<IRichEditOle>  ole;
        SSendMessage(EM_GETOLEINTERFACE, 0, (LPARAM)&ole);

        for (int i = _visibleOleChr.cpMin; i <= _visibleOleChr.cpMax; i++)
        {
            REOBJECT reobj = { 0 };
            reobj.cbStruct = sizeof(REOBJECT);

            HRESULT hr = ole->GetObject(i, &reobj, REO_GETOBJ_POLEOBJ);
            if (FAILED(hr))
            {
                break;
            }

            SComPtr<RichEditImageOle> pImageOle = NULL;
            if (reobj.poleobj->QueryInterface(IID_ImageOleCtrl, (VOID**)&pImageOle) == S_OK)
            {
                CRect rcObj = pImageOle->GetRect();
                if (rcObj.IntersectRect(rcObj, validRgnRect))
                {
                    pImageOle->InternalDraw(pRt, _pDelayDrawRgn, reobj.cp);
                }
                reobj.poleobj->Release();
            }
        }
    }

    void SImRichEdit::DirectDrawOles(IRegion* prgn)
    {
        if ((GetState()&WndState_Invisible) ||  // 不用两个&&,不用能IsVisible做判断
            !prgn ||
            prgn->IsEmpty() ||
            _isBackgroundDirty ||
            !_pBackgroundRt)
        {
            return;
        }

        CRect rcCli = GetClientRect();
        prgn->CombineRect(rcCli, RGN_AND);
        ClipChildren(this, prgn);

        CRect rcRegion;
        prgn->GetRgnBox(rcRegion);

        CAutoRefPtr<IRenderTarget> pRt = GetContainer()->OnGetRenderTarget(rcRegion, 0);
        pRt->PushClipRegion(prgn);
        pRt->BitBlt(rcRegion, _pBackgroundRt, rcRegion.left, rcRegion.top);

        HDC hdc = pRt->GetDC(0);
        ALPHAINFO ai;
        CGdiAlpha::AlphaBackup(hdc, &rcRegion, ai);

        DrawVisibleContents(pRt);
        DrawVisibleGifs(pRt, rcRegion);

        CGdiAlpha::AlphaRestore(ai);

        GetContainer()->OnReleaseRenderTarget(pRt, rcRegion, 0);
        pRt->PopClip();
    }

    /*
     * 获取光标的位置，位置是相对于richedit的左上角。
     * 位置并不是实时的，通常可以用来得到光标高度。
     *
     * @param rcCursor: 输出的光标位置
     * @return BOOL: TRUE
     */
    BOOL SImRichEdit::GetCaretRect2(CRect& rcCursor)
    {
        rcCursor = _caretRect;
        return TRUE;
    }

    /*
     * 获取光标的位置，位置是相对于屏幕的左上角。
     * 位置是实时计算的，效率较GetCaretRect2低，通常用来获得精确的光标位置。
     *
     * @param rcCursor: 输出的光标位置
     * @return BOOL: TRUE
     */
    BOOL SImRichEdit::GetCaretRect(CRect& rcCaret)
    {
        LONG cpStart = -1;
        LONG cpEnd = -1;

        GetSel(&cpStart, &cpEnd);
        rcCaret.SetRectEmpty();

        if (cpStart < 0 || cpEnd < 0)
        {
            return FALSE;
        }

        SComPtr<ITextRange> prange;
        _pTextDoc->Range(cpStart, cpEnd, &prange);

        if (!prange)
        {
            return FALSE;
        }

        // http://technet.microsoft.com/zh-cn/hh768766(v=vs.90) 新类型定义
#define _tomClientCoord     256  // 默认获取到的是屏幕坐标， Use client coordinates instead of screen coordinates.
#define _tomAllowOffClient  512  // Allow points outside of the client area.

        POINT ptStart = { 0,0 };
        POINT ptEnd = { 0,0 };

        long lTypeTopLeft = _tomAllowOffClient | _tomClientCoord | tomStart | TA_TOP | TA_LEFT;
        long lTypeRightBottom = _tomAllowOffClient | _tomClientCoord | tomEnd | TA_BOTTOM | TA_RIGHT;

        if (prange->GetPoint(lTypeTopLeft, &ptStart.x, &ptStart.y) != S_OK ||
            prange->GetPoint(lTypeRightBottom, &ptEnd.x, &ptEnd.y) != S_OK)
        {
            return FALSE;
        }

        rcCaret.SetRect(ptStart, ptEnd);
        if (rcCaret.Width() == 0)
        {
            rcCaret.right += 1;
        }

        LPRECT prc = (LPRECT)rcCaret;
        HWND hwnd = GetContainer()->GetHostHwnd();

        ::ClientToScreen(hwnd, (LPPOINT)prc);
        ::ClientToScreen(hwnd, ((LPPOINT)prc) + 1);
        return TRUE;
    }

    /*
     * 创建光标，由richedit调用
     * @param pBmp:    光标位图
     * @param nWid:    光标宽度
     * @param nHeight: 光标高度
     * @return BOOL:   TRUE:创建成功,FASLE:创建失败
     *
     * note: 在有选中内容时，richedit传递过来的宽、高信息和pBmp所携带的宽高有时候并不一致
     */
    BOOL SImRichEdit::CreateCaret(HBITMAP pBmp, int nWid, int nHeight)
    {
        _caretRect.right = _caretRect.left + nWid;
        _caretRect.bottom = _caretRect.top + nHeight;
        return SRichEdit::CreateCaret(pBmp, nWid, nHeight);
    }

    /*
     * 设置光标位置，由richedit调用
     */
    void SImRichEdit::SetCaretPos(int x, int y)
    {
        _caretRect.left = x;
        _caretRect.top = y;
        return SetCaretPos(x, y);
    }

    BOOL SImRichEdit::CanPaste()
    {
        RichFormatConv::ClipboardFmts fmts;
        RichFormatConv conv;

        conv.GetSupportedFormatsFromClipboard(fmts);
        for (UINT i = 0; i < fmts.GetCount(); ++i)
        {
            if (IsClipboardFormatAvailable(fmts[i]))
            {
                return TRUE;
            }
        }

        return FALSE;
    }

    /*
     * 获取指定字符位置的ole
     * @param cp: 字符位置。如果<0，则自动获取当前选择内容的字符位置。如果选择内容>1，返回NULL。
     * return: 对应的ole指针，如果给定的cp不是ole，返回NULL
     */
    RichEditOleBase*  SImRichEdit::GetSelectedOle(int cp/*=-1*/)
    {
        RichEditOleBase* pOleObject = NULL;

        SComPtr<IRichEditOle> ole;
        SSendMessage(EM_GETOLEINTERFACE, 0, (LPARAM)&ole);

        if (cp < 0)
        {
            CHARRANGE chrSel;
            GetSel(&chrSel.cpMin, &chrSel.cpMax);
            if (chrSel.cpMax - chrSel.cpMin != 1)
            {
                return NULL;
            }

            cp = chrSel.cpMin;
        }

        REOBJECT reo = { 0 };
        reo.cp = cp;
        reo.cbStruct = sizeof(REOBJECT);

        if (ole->GetObject(REO_IOB_USE_CP, &reo, REO_GETOBJ_POLEOBJ) == S_OK)
        {
            pOleObject = static_cast<RichEditOleBase*>(reo.poleobj);
            if (!pOleObject)
            {
                //
                // 调用者需要调用Release
                // 这里只是防止pOleObject为空时，外面无法调用Release而做的保护，避免泄漏
                //
                reo.poleobj->Release();
            }
        }

        return pOleObject;
    }

    RichEditContent * SImRichEdit::GetContent(UINT uIndex)
    {
        if (uIndex >= _richContents.GetCount())
            return NULL;

        return _richContents.GetAt(uIndex);
    }

    RichEditContent * SImRichEdit::GetContent(LPCWSTR pszId)
    {
        if (!pszId)
        {
            return NULL;
        }

        for (size_t n = 0; n < _richContents.GetCount(); ++n)
        {
            RichEditContent * p = _richContents[n];
            if (p->GetId() == pszId)
            {
                return p;
            }
        }

        return NULL;
    }

    UINT SImRichEdit::DeleteContent(RichEditContent * prec)
    {
        if (!prec)
        {
            return _richContents.GetCount();
        }

        for (size_t npos = 0; npos < _richContents.GetCount(); ++npos)
        {
            RichEditContent * p = _richContents.GetAt(npos);

            if (p == prec)
            {
                DeleteContent(npos);
                return npos;
            }
        }

        return _richContents.GetCount();
    }

    UINT SImRichEdit::GetContentCount()
    {
        return _richContents.GetCount();
    }

    BOOL SImRichEdit::GetContentIndex(LPCWSTR pszId, UINT& index)
    {
        for (size_t npos = 0; npos < _richContents.GetCount(); ++npos)
        {
            RichEditContent * p = _richContents.GetAt(npos);

            if (p->GetId() == pszId)
            {
                index = npos;
                return TRUE;
            }
        }

        return FALSE;
    }

    /*
     * 从指定的下标开始更新content的位置
     * @param start: 开始更新content的下标
     * @param end:   结束更新的content下标
     *
     * 更新范围是[start, end)
     */
    void SImRichEdit::UpdateContentPosition(UINT start, UINT end/* = RECONTENT_LAST*/)
    {
        _scrollbarOutdated = TRUE;

        UpdateContext ctx;
        PreUpdate(ctx);

        for (size_t n = start; n < end && n < _richContents.GetCount(); ++n)
        {
            _richContents.GetAt(n)->UpdatePosition();
        }

        ctx.bHasScrollBar = HasScrollBar(TRUE); //强制不刷滚动条
        DoneUpdate(ctx);

        UpdateVisibleCharRanges();
    }

    void SImRichEdit::ScrollTo(UINT uIndex)
    {
        if (uIndex >= _richContents.GetCount())
            return;

        RichEditContent * pc = _richContents.GetAt(uIndex);
        CHARRANGE chr = pc->GetCharRange();

        SetSel(chr.cpMin, chr.cpMin);
        SetSel(-1, 0);
    }

    void SImRichEdit::ScrollTo(LPCWSTR pszId)
    {
        for (UINT i = 0; i < _richContents.GetCount(); ++i)
        {
            RichEditContent * p = _richContents.GetAt(i);

            if (p->GetId() == pszId)
            {
                ScrollTo(i);
                return;
            }
        }
    }

    RichEditContent* SImRichEdit::CreateRichEditConent(LPCWSTR lpszContent)
    {
        if (lpszContent == NULL)
            return NULL;

        RichEditContent * pRet = NULL;
        unsigned int flag = pugi::parse_cdata | pugi::parse_escapes | pugi::parse_eol;
        pugi::xml_document  doc;
        if (doc.load_buffer(lpszContent, wcslen(lpszContent) * sizeof(WCHAR), flag))
        {
            pRet = new RichEditContent();
            if (!pRet->InitFromXml(doc.child(RichEditContent::GetClassName())))
            {
                delete pRet;
                return NULL;
            }
        }

        return pRet;
    }

    /*
     * 重新计算content的charrange。通常在conetent列表中间插入、删除了某个content时，
     * 需要重新计算修改位置以下的conent charrange。
     *
     * @param start: 需要计算content的起始位置
     * @param offset: content charrange的偏移。有可能是负数
     */
    void SImRichEdit::ReLocateContents(UINT start, int offset)
    {
        if (start >= _richContents.GetCount())
            return;

        start += 1;
        for (; start < _richContents.GetCount(); ++start)
        {
            RichEditContent * pCurr = _richContents.GetAt(start);
            pCurr->OffsetCharRange(offset);
        }
    }

    RichEditObj * SImRichEdit::GetElementById(LPCWSTR lpszId)
    {
        for (size_t npos = 0; npos < _richContents.GetCount(); ++npos)
        {
            RichEditObj * pObjRet = _richContents.GetAt(npos)->GetById(lpszId);
            if (pObjRet != NULL)
                return pObjRet;
        }

        return NULL;
    }

    RichEditOleBase * SImRichEdit::GetOleById(LPCWSTR lpszId)
    {
        RichEditObj     * pObj = GetElementById(lpszId);
        SStringW cn = pObj->GetClassName();
        //RichEditOleBase * pOle = dynamic_cast<RichEditOleBase*>(pObj);
        RichEditOleBase * pOle = static_cast<RichEditOleBase*>(pObj);
        return pOle;
    }

    BOOL SImRichEdit::IsRenderTargetEmpty(IRenderTarget* pRt)
    {
        if (!pRt)
        {
            return TRUE;
        }

        IBitmap * bmp = (IBitmap*)pRt->GetCurrentObject(OT_BITMAP);
        LPBYTE lpBits = (LPBYTE)bmp->LockPixelBits();
        if (!lpBits)
        {
            return TRUE;
        }

        BYTE r = *lpBits++;
        BYTE g = *lpBits++;
        BYTE b = *lpBits++;
        BYTE a = *lpBits++;

        return a == 0;
    }

    void SImRichEdit::EnableDragDrop(BOOL enable)
    {
        GetContainer()->RevokeDragDrop(m_swnd);
        if (enable)
        {
            RichEditDropTarget* pdt = new RichEditDropTarget(this);
            GetContainer()->RegisterDragDrop(m_swnd, pdt);
            pdt->Release();
        }
    }

    RichEditObj* SImRichEdit::HitTest(RichEditObj * pObject, POINT pt)
    {
        for (; pObject != NULL; pObject = pObject->GetNext())
        {
            if (pObject->GetHitTestable() && pObject->PointInObject(pt))
            {
                return pObject;
            }

            RichEditObj * p = HitTest(pObject->GetFirstChild(), pt);
            if (p != NULL)
            {
                return p;
            }
        }

        return NULL;
    }

    RichEditObj* SImRichEdit::HitTest(POINT pt)
    {
        for (size_t npos = 0; npos < _richContents.GetCount(); ++npos)
        {
            RichEditObj * pObjHitted = HitTest(_richContents.GetAt(npos), pt);
            if (pObjHitted != NULL)
                return pObjHitted;
        }

        return NULL;
    }

    void  SImRichEdit::UpdateVisibleCharRanges()
    {
        //
        // 更新可见字符范围
        //
        CRect clientRect = GetClientRect();
        int nFirstVisibleLine = GetFirstVisibleLine();

        _visibleChr.cpMin = LineIndex(nFirstVisibleLine);
        _visibleChr.cpMax = CharFromPos(clientRect.BottomRight());

        //
        // 更新可见ole范围
        //

        CComPtr<IRichEditOle>  ole;
        SSendMessage(EM_GETOLEINTERFACE, 0, (LPARAM)&ole);

        int oleCount = ole->GetObjectCount();
        _visibleOleChr.cpMin = FindFirstOleInrange(ole, 0, oleCount, _visibleChr.cpMin, _visibleChr.cpMax);
        _visibleOleChr.cpMax = (_visibleOleChr.cpMin == -1)
            ? -1
            : FindLastOleInrange(ole, _visibleOleChr.cpMin, oleCount, _visibleChr.cpMin, _visibleChr.cpMax);

        //STRACE(_T("%s br:(%d,%d), visible chr:[%d,%d], ole:[%d,%d]"), 
        //    m_strName,
        //    clientRect.BottomRight().x,
        //    clientRect.BottomRight().y, 
        //    _visibleChr.cpMin, _visibleChr.cpMax, _visibleOleChr.cpMin, _visibleOleChr.cpMax);
    }

    int SImRichEdit::GetRangeText(const CHARRANGE& chr, SStringW& strText)
    {
        TEXTRANGE txtrg = { chr, NULL };
        strText.Empty();

        if (chr.cpMax <= chr.cpMin)
        {
            return 0;
        }

        txtrg.lpstrText = new WCHAR[chr.cpMax - chr.cpMin + 1];
        memset(txtrg.lpstrText, 0, chr.cpMax - chr.cpMin + 1);

        int nTextLen = SSendMessage(EM_GETTEXTRANGE, 0, (LPARAM)&txtrg);
        if (nTextLen > 0)
        {
            strText = txtrg.lpstrText;
        }

        delete txtrg.lpstrText;
        return nTextLen;
    }

    /*
     * richedit有一个RETID_BGND_RECALC定时器，用作延迟排版。
     * richedit内部如果要排版会先设置一个定时器，在还没触发这个定时器之前，如果又要排版，那么就重新设置定时器，
     * 这样就可以避免太频繁的排版导致效率低下。
     *　
     * 该函数主动向richedit发送重新排版的定时器，强制让richedit进行重排版
     */
    void SImRichEdit::ForceUpdateLayout()
    {
        OnTimer2(RETID_BGND_RECALC);
    }

    void SImRichEdit::MarkBkgndRtDirty()
    {
        _isBackgroundDirty = TRUE;
    }

    void SImRichEdit::SetAutoFixVScroll(BOOL fix)
    {
        _fixVScroll = fix;
    }

    //------------------------------------------------------------------------------
    //
    // internal methods
    //
    //------------------------------------------------------------------------------

    /*
     * 更新背景缓存，通常在OnSize调用该函数。该函数只是修改背景缓存的大小。
     * 在OnSize里调用GetContainer()->OnGetRenderTarget会有问题，所以背景的绘制工作放在OnPaint
     */
    void SImRichEdit::UpdateBkgndRenderTarget()
    {
        // 更新背景RenderTarget
        CRect rcWnd = GetClientRect();
        if (!_pBackgroundRt)
        {
            GETRENDERFACTORY->CreateRenderTarget(&_pBackgroundRt, rcWnd.Width(), rcWnd.Height());
        }
        else
        {
            _pBackgroundRt->Resize(rcWnd.Size());
        }
        _pBackgroundRt->SetViewportOrg(-rcWnd.TopLeft());
        _isBackgroundDirty = TRUE;


        if (_pDelayDrawRgn)
        {
            //STRACE(L"size changed, clear delay draw rgn");
            _pDelayDrawRgn->Clear();
        }
    }

    void SImRichEdit::SetContentsDirty()
    {
        for (size_t npos = 0; npos < _richContents.GetCount(); ++npos)
        {
            RichEditContent * pContent = _richContents.GetAt(npos);
            pContent->SetDirty(TRUE);
        }
    }

    HRESULT SImRichEdit::DefAttributeProc(const SStringW & strAttribName, const SStringW & strValue, BOOL bLoading)
    {
        m_dwStyle |= ES_NOHIDESEL;

        HRESULT hr = SRichEdit::DefAttributeProc(strAttribName, strValue, bLoading);

        if (strAttribName.CompareNoCase(L"readOnly") == 0 ||
            strAttribName.CompareNoCase(L"enableDragdrop") == 0)
        {
            /*
             * richedti的拖拽效果不由是否readonly决定
             * 这样的话需要在iRichEditOleCallback里做手脚，判断如果richedit是readonly，就不能继续粘贴
             */
            if (!bLoading)
            {
                EnableDragDrop(m_fEnableDragDrop);
            }
        }

        return hr;
    }

    /*
     * 在主动更新背景时由于是直接更新到DC上的，而不是从最底层的window一直画到richedit这一层，
     * 所以覆盖在SImRichEdit上的控件都会被擦掉。该函数把SImRichEdit的子控件位置都留出来。
     *
     * @param pWnd: 窗口的指针，通常是richedit的指针
     * @param prgn: 窗口的背景RGN
     *
     * 该函数遍历pWnd的子窗口，把子窗口的位置从RGN里去掉。
     */
    void SImRichEdit::ClipChildren(SWindow * pWnd, IRegion * prgn)
    {
        if (pWnd == NULL)
        {
            return;
        }

        for (SWindow * p = pWnd->GetWindow(GSW_FIRSTCHILD); p != NULL; p = p->GetWindow(GSW_NEXTSIBLING))
        {
            if (p->IsVisible())
            {
                CRect rc = p->GetClientRect();
                prgn->CombineRect(rc, RGN_DIFF);
            }
        }
    }

    void SImRichEdit::OnNextFrame()
    {
        time_t nTicksNow = GetTickCount();
        time_t nInterval = nTicksNow - _lastDrawTicks;

        if (!_pDelayDrawRgn)
        {
            return;
        }

        if (nInterval > 60 && !_pDelayDrawRgn->IsEmpty()) // 60 才刷新一次
        {
            _lastDrawTicks = nTicksNow;
            DirectDrawOles(_pDelayDrawRgn);
            _pDelayDrawRgn->Clear();
        }
    }

    void SImRichEdit::PreUpdate(UpdateContext& context)
    {
        context.bReadOnly = GetReadOnly();
        if (context.bReadOnly)
        {
            SetReadOnly(FALSE);
        }

        context.bHasScrollBar = HasScrollBar(TRUE);
    }

    void SImRichEdit::DoneUpdate(const UpdateContext& context)
    {
        if (context.bHasScrollBar != HasScrollBar(TRUE))
        {
            //滚动条发生了变化,全部内容要重新布局
            for (size_t npos = 0; npos < _richContents.GetCount(); ++npos)
            {
                _richContents.GetAt(npos)->OffsetCharRange(0);
            }
        }

        if (context.bReadOnly)
        {
            SetReadOnly(TRUE);
        }
    }

    void SImRichEdit::DrawScrollbar(BOOL bVertical, UINT uCode, int nPos)
    {
        SCROLLINFO *psi = bVertical ? (&m_siVer) : (&m_siHoz);

        if (uCode != SB_THUMBTRACK && IsVisible(TRUE))
        {
            CRect rcRail = GetScrollBarRect(bVertical);
            if (bVertical)
            {
                rcRail.DeflateRect(0, GetSbArrowSize());
            }
            else
            {
                rcRail.DeflateRect(GetSbArrowSize(), 0);
            }

            CAutoRefPtr<IRenderTarget> pRT = GetRenderTarget(&rcRail, OLEDC_PAINTBKGND, FALSE);
            m_pSkinSb->Draw(pRT, rcRail, MAKESBSTATE(SB_PAGEDOWN, SBST_NORMAL, bVertical));
            psi->nTrackPos = -1;
            CRect rcSlide = GetSbPartRect(bVertical, SB_THUMBTRACK);
            m_pSkinSb->Draw(pRT, rcSlide, MAKESBSTATE(SB_THUMBTRACK, SBST_NORMAL, bVertical));
            ReleaseRenderTarget(pRT);
        }
    }

    /*
     * 按照给定的code和位置重新计算位置
     *
     * @param bVertical:  是否为垂直滚动条
     * @param uCode:      滚动事件
     * @param nPos:       请求设置的滚动条位置
     *
     * return BOOL:       滚动条是否发生了改变
     */
    BOOL SImRichEdit::RecalcScrollbarPos(BOOL bVertical, UINT uCode, int nPos)
    {
        SCROLLINFO *psi = bVertical ? (&m_siVer) : (&m_siHoz);
        int nNewPos = psi->nPos;
        int maxScrollPos = psi->nMax - psi->nPage;

        switch (uCode)
        {
        case SB_LINEUP:
            nNewPos -= GetScrollLineSize(bVertical);
            break;
        case SB_LINEDOWN:
            nNewPos += GetScrollLineSize(bVertical);
            break;
        case SB_PAGEUP:
            nNewPos -= psi->nPage;
            break;
        case SB_PAGEDOWN:
            nNewPos += psi->nPage;
            break;
        case SB_THUMBTRACK:
        case SB_THUMBPOSITION:
            nNewPos = nPos;
            break;
        case SB_TOP:
            nNewPos = psi->nMin;
            break;
        case SB_BOTTOM:
            nNewPos = maxScrollPos;
            break;
        }

        if (nNewPos < psi->nMin)
        {
            nNewPos = psi->nMin;
        }

        if (nNewPos > maxScrollPos)
        {
            nNewPos = maxScrollPos;
        }

        if (psi->nPage == 0)
        {
            nNewPos = 0;
        }

        BOOL scrollposChanged = FALSE;
        if (nNewPos != m_siVer.nPos)
        {
            psi->nPos = nNewPos;
            scrollposChanged = TRUE;
        }

        if (abs(nNewPos - maxScrollPos) <= 3)
        {
            _scrollbarAtBottom = TRUE;
        }
        else
        {
            _scrollbarAtBottom = FALSE;
        }

        return scrollposChanged;
    }

    //------------------------------------------------------------------------------
    //
    // event handlers
    // 因为用户体验方面的原因，开启了richedit的autoVScroll，但是开启这个功能后，richedit在很多
    // 内部的处理都会重新设置滚动条的位置，导致滚动条经常乱动。比如缩放窗口尺寸，拖拽选中内容等。
    // 
    // 为了让richedit的滚动条能受控制，在以下消息的处理函数都额外做了处理：
    //  - OnSize
    //  - OnPaint
    //  - OnScroll
    //  - OnNcCalcSize
    // 基本的思想就是在知道richedit会改变滚动条的地方先记录原来滚动条的位置，等richedit改完
    // 滚动条的位置后再重新设置回去。
    //
    // 以下情况会导致richedit位置不对
    //  - 在滚动条快到底部时，点击滚动条底部的空白，导致向richedit发送一个SB_PAGEDOWN
    //  - 在滚动条快到底部并且autoVScroll开启时，选中一部分内容，往下拉，导致自动往下滚动
    // 
    //------------------------------------------------------------------------------

    /*
     * #define SB_LINEUP           0
     * #define SB_LINEDOWN         1
     * #define SB_PAGEUP           2
     * #define SB_PAGEDOWN         3
     * #define SB_THUMBPOSITION    4 (Up/Down,Drag,EM_EXLINEFROMCHAR,TXTBIT_EXTENTCHANGE,TXTBIT_SCROLLBARCHANGE)
     * #define SB_THUMBTRACK       5 (user drag thumb)
     * #define SB_TOP              6
     * #define SB_BOTTOM           7
     * #define SB_ENDSCROLL        8
     *
     * 注意：
     *  - 不需要处理由于SB_THUMBPOSITION而发起重绘，因为通常这是由richedit自己发起的，随后richedit会通过回调触发重绘。
     */
    BOOL SImRichEdit::OnScroll(BOOL bVertical, UINT uCode, int nPos)
    {
        RecalcScrollbarPos(bVertical, uCode, nPos);

        //if (m_strName != _T("EdtInput"))
        //{
        //    CRect rc = GetClientRect();
        //    STRACE(_T("onscroll code:%d, min:%d, max:%d, pos:%d, height:%d, realPos:%d"),
        //        uCode, m_siVer.nMin, m_siVer.nMax, nPos, rc.Height(), m_siVer.nPos);
        //}

        if (!m_fScrollPending)
        {
            _scrollbarOutdated = FALSE;

            UINT code = (uCode == SB_THUMBTRACK) ? SB_THUMBPOSITION : uCode;

            m_pTxtHost->GetTextService()->TxSendMessage(
                bVertical ? WM_VSCROLL : WM_HSCROLL,
                MAKEWPARAM(code, m_siVer.nPos),
                0,
                NULL);
        }
        else
        {
            /*
             * 用户拖拽内容导致滚动条发生改变时，可能由于richedit内部计算有问题，导致滚动条位置出错。
             *
             * 由于 m_fScrollPending 为TRUE，说明是由richedit主动调用TxSetScroll引起的OnScroll，
             * 为避免死循环，不能在这里调用TxSendMessage设置滚动条位置。
             *
             * 所以这里标记滚动条位置已失效，以自己维护的滚动条位置为准，在OnPaint时重新设置滚动条位置
             */
            LONG pos = 0;
            m_pTxtHost->GetTextService()->TxGetVScroll(NULL, NULL, &pos, NULL, NULL);

            if (pos != m_siVer.nPos)
            {
                _scrollbarOutdated = TRUE;
            }

            UpdateVisibleCharRanges();

            if (_scrollbarAtBottom)
            {
                // notify
                EventRichEditScroll evt(this);
                evt.ScrollAtBottom = TRUE;
                FireEvent(evt);
            }
        }

        if (_pDelayDrawRgn)
        {
            _pDelayDrawRgn->Clear();
        }

        if (uCode == SB_THUMBTRACK || uCode == SB_LINEDOWN || uCode == SB_LINEUP ||
            uCode == SB_PAGEUP || uCode == SB_PAGEDOWN)
        {
            DirectDraw(m_rcClient);
        }

        return 0;
    }

    LRESULT SImRichEdit::OnImeStartComposition(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        _isCreatIme = true;
        SetMsgHandled(FALSE);	//继续让消息往下传
        return S_OK;
    }

    LRESULT SImRichEdit::OnImeComposition(UINT uMsg, WPARAM wParam, LPARAM lParam)
    {
        LRESULT result = S_OK;
        __super::SwndProc(uMsg, wParam, lParam, result);
        _onPostImmComposition(uMsg, wParam, lParam);
        return result;
    }

    BOOL SImRichEdit::_onPostImmComposition(UINT msg, WPARAM wParam, LPARAM lParam)
    {
        HIMC hIMC = NULL;

        if (hIMC = ImmGetContext(GetContainer()->GetHostHwnd()))
        {
            POINT pt;
            GetCaretPos(&pt);
            COMPOSITIONFORM CompForm = { 0 };
            ImmGetCompositionWindow(hIMC, &CompForm);

            LONG x;
            if (_isCreatIme)
                x = pt.x;
            else
                x = CompForm.ptCurrentPos.x;

            _isCreatIme = false;

            LONG yPixPerInch = GetDeviceCaps(GetDC(NULL), LOGPIXELSY);
            CompForm.dwStyle = CFS_FORCE_POSITION;
            CompForm.ptCurrentPos.x = x;
            int offset = m_cfDef.yHeight * yPixPerInch / LY_PER_INCH;
            CRect careRect;
            GetCaretRect2(careRect);
            CompForm.ptCurrentPos.y = pt.y + careRect.Height() - offset;
            ImmSetCompositionWindow(hIMC, &CompForm);

            CANDIDATEFORM candidateForm;
            candidateForm.dwIndex = 0;
            candidateForm.dwStyle = CFS_CANDIDATEPOS;
            candidateForm.ptCurrentPos.x = x;
            candidateForm.ptCurrentPos.y = pt.y + careRect.Height() - offset + 5;

            ImmSetCandidateWindow(hIMC, &candidateForm);

            ImmReleaseContext(GetContainer()->GetHostHwnd(), hIMC);
        }

        return TRUE;
    }

    void SImRichEdit::OnSize(UINT nType, CSize size)
    {
        __super::OnSize(nType, size);

        BOOL scrollAtBottom = _scrollbarAtBottom;
        int scrollPos = GetScrollPos(TRUE);

        UpdateBkgndRenderTarget();
        m_pTxtHost->GetTextService()->OnTxPropertyBitsChange(TXTBIT_EXTENTCHANGE, TXTBIT_EXTENTCHANGE);
        UpdateContentPosition(0);
        ForceUpdateLayout();

        /*
         * 由于设置了autoVScroll，滚动条会经常乱动，所以需要动态设置一把。
         * - 如果滚动条处于窗口底部，不管窗口尺寸怎样改变，都需要保证滚动条始终贴着窗口底部
         * - 如果滚动条不在底部，需要保证滚动条保持在当前位置。
         */
        if (scrollAtBottom)
        {
            OnScroll(TRUE, SB_BOTTOM, 0);
        }
        else if (GetScrollPos(TRUE) != scrollPos)
        {
            OnScroll(TRUE, SB_THUMBPOSITION, scrollPos);
        }
    }

    void SImRichEdit::OnPaint(IRenderTarget * pRt)
    {
        if (_isBackgroundDirty && _pBackgroundRt)
        {
            CRect rcClient = GetClientRect();
            _pBackgroundRt->BitBlt(rcClient, pRt, rcClient.left, rcClient.top);

            if (!IsRenderTargetEmpty(_pBackgroundRt))
            {
                _isBackgroundDirty = FALSE;
            }
        }

        // 有以下几个情况会导致content变脏：
        // * OnSize之后
        // * OnScroll之后
        // * 图片只显示一半的时候，点击图片，图片会完整的出现richedit上
        SetContentsDirty();

        if (!_isDrawEnable)
        {
            return;
        }

        /*
         * 这部分的改动主要是要确保不让richedit自动修改滚动条位置，参考了wince richedit的源码。
         *
         * 由于设置了autoVScroll，在OnPaint时会自动计算滚动条位置，所以需要先手动把ES_AUTOVSCROLL样式去掉
         * TxDraw()-> [richedit内部堆栈] -> TxGetScrollBars()
         * 确保richedit在TxDraw期间不会设置滚动条位置
         *
         * 由于某些原因，我们自己维护的位置和richedit维护的滚动条位置并不一致，这里以我们自己的为准。
         */
        DWORD autoVScroll = m_dwStyle & ES_AUTOVSCROLL;

        m_dwStyle &= ~ES_AUTOVSCROLL;

        if (_scrollbarOutdated && _fixVScroll)
        {
            OnScroll(TRUE, SB_THUMBPOSITION, m_siVer.nPos);
        }

        DrawVisibleContents(pRt);

        SRichEdit::OnPaint(pRt);

        m_dwStyle |= autoVScroll;
    }

    LRESULT SImRichEdit::OnNcCalcSize(BOOL bCalcValidRects, LPARAM lParam)
    {
        // Convert Device Pixels to Himetric
#define DTOHIMETRIC(d, dpi) (LONG)MulDiv(d, HIMETRIC_PER_INCH, dpi)

        if (_originalInset.IsRectNull())
        {
            _originalInset = m_rcInsetPixel;
        }

        m_rcInsetPixel = _originalInset;

        if (HasScrollBar(TRUE))
        {
            m_rcInsetPixel.right -= GetScrollBarRect(TRUE).Width();
        }

        if (HasScrollBar(FALSE))
        {
            m_rcInsetPixel.bottom -= GetScrollBarRect(FALSE).Height();
        }

        //
        // copy from SPanel
        // 由于想把 TXTBIT_EXTENTCHANGE 的通知放到OnSize再发给richedit，所以这里照抄SRichEdit的处理
        // 在发送TXTBIT_EXTENTCHANGE消息给richedit时，可能会导致该函数重入。
        // TXTBIT_EXTENTCHANGE -> SPanel::ShowScrollBar -> OnNcCalcSize
        // 为简化逻辑，这里把TXTBIT_EXTENTCHANGE消息放到OnSize去处理
        //

        SPanel::OnNcCalcSize(bCalcValidRects, lParam);

        CRect rcInsetPixel = m_rcInsetPixel;
        if (!m_fRich && m_fSingleLineVCenter && !(m_dwStyle&ES_MULTILINE))
        {
            rcInsetPixel.top = rcInsetPixel.bottom = (m_rcClient.Height() - m_nFontHeight) / 2;
        }

        m_siHoz.nPage = m_rcClient.Width() - rcInsetPixel.left - rcInsetPixel.right;
        m_siVer.nPage = m_rcClient.Height() - rcInsetPixel.top - rcInsetPixel.bottom;

        HDC hdc = GetDC(GetContainer()->GetHostHwnd());
        LONG xPerInch = ::GetDeviceCaps(hdc, LOGPIXELSX);
        LONG yPerInch = ::GetDeviceCaps(hdc, LOGPIXELSY);
        ReleaseDC(GetContainer()->GetHostHwnd(), hdc);

        m_sizelExtent.cx = DTOHIMETRIC(m_rcClient.Width(), xPerInch);
        m_sizelExtent.cy = DTOHIMETRIC(m_rcClient.Height(), yPerInch);

        m_rcInset.left = DTOHIMETRIC(m_rcInsetPixel.left, xPerInch);
        m_rcInset.right = DTOHIMETRIC(m_rcInsetPixel.right, xPerInch);
        m_rcInset.top = DTOHIMETRIC(m_rcInsetPixel.top, yPerInch);
        m_rcInset.bottom = DTOHIMETRIC(m_rcInsetPixel.bottom, yPerInch);

        return 0;
    }

    LRESULT SImRichEdit::OnCreate(LPVOID)
    {
        if (0 != __super::OnCreate(NULL))
        {
            return 1;
        }

        EnableDragDrop(m_fEnableDragDrop);
        _readOnlyBeforeDrag = GetReadOnly();

        GUID guid = __uuidof(ITextDocument);
        CComPtr<IRichEditOle> pOle = NULL;
        SSendMessage(EM_GETOLEINTERFACE, 0, (LPARAM)&pOle);
        HRESULT hr = pOle->QueryInterface(guid, (void**)&_pTextDoc);
        SASSERT(SUCCEEDED(hr));

        //
        // set IME
        //
        //DWORD dw = SSendMessage(EM_GETEDITSTYLE);
        //dw |= SES_USECTF;
        //SSendMessage(EM_SETEDITSTYLE, dw, dw);

        RichEditOleCallback * pcb = new RichEditOleCallback(this);
        SSendMessage(EM_SETOLECALLBACK, 0, (LPARAM)pcb);
        pcb->Release();
        GetContainer()->RegisterTimelineHandler(this);

        return 0;
    }

    void SImRichEdit::OnDestroy()
    {
        if (_pTextDoc)
        {
            _pTextDoc->Release();
        }
        Clear();

        SRichEdit::OnDestroy();
    }

    void SImRichEdit::OnLButtonDown(UINT nFlags, CPoint point)
    {
        BOOL bHandled = FALSE;
        POINT pt = { point.x, point.y };
        RichEditObj * pRichObj = HitTest(pt);
        if (pRichObj != NULL && pRichObj->NeedToProcessMessage())
        {
            LPARAM lParam = MAKELPARAM(pt.x, pt.y);
            pRichObj->ProcessMessage(WM_LBUTTONDOWN, 0, lParam, bHandled);
            if (bHandled)
            {
                SetFocus();
            }
        }

        SetMsgHandled(bHandled);
    }

    void SImRichEdit::OnLButtonUp(UINT nFlags, CPoint point)
    {
        BOOL bHandled = FALSE;
        POINT pt = { point.x, point.y };
        RichEditObj * pRichObj = HitTest(pt);
        if (pRichObj != NULL && pRichObj->NeedToProcessMessage())
        {
            LPARAM lParam = MAKELPARAM(pt.x, pt.y);
            pRichObj->ProcessMessage(WM_LBUTTONUP, 0, lParam, bHandled);
        }

        SetMsgHandled(bHandled);
    }

    void SImRichEdit::OnRButtonDown(UINT nFlags, CPoint point)
    {
        LRESULT result = 0;
        //SwndProc(GetCurMsg()->uMsg, GetCurMsg()->wParam, GetCurMsg()->lParam, result);
        SetFocus();
    }

    void SImRichEdit::OnRButtonUp(UINT nFlags, CPoint point)
    {
        LRESULT result = 0;
        SwndProc(GetCurMsg()->uMsg, GetCurMsg()->wParam, GetCurMsg()->lParam, result);
        FireCtxMenu(point);
    }

    void SImRichEdit::OnMouseMove(UINT nFlags, CPoint point)
    {
        BOOL bHandled = FALSE;
        RichEditObj * pRichObj = HitTest(point);

        if (pRichObj != NULL && pRichObj->NeedToProcessMessage())
        {
            pRichObj->ProcessMessage(
                WM_MOUSEMOVE, GetCurMsg()->wParam, GetCurMsg()->lParam, bHandled);
        }

        if (_pLastHoverObj != NULL && pRichObj != _pLastHoverObj)
        {
            _pLastHoverObj->ProcessMessage(
                WM_MOUSELEAVE, GetCurMsg()->wParam, GetCurMsg()->lParam, bHandled);
        }
        _pLastHoverObj = pRichObj;

        //
        // richedit坑：richedit的WM_MOUSEMOVE可能会触发拖拽操作。
        // 如果在richedit的拖拽状态下调用了SetReadOnly(TRUE),在拖拽结束之后会崩溃。
        // 原因是richedit在收到SetReadOnly(TRUE)时，会释放正在拖拽的对象，而在
        // 拖拽操作结束之后richedit会继续使用被释放的对象，从而导致崩溃。
        // 
        // 解决办法是先假定richedit已经进入了拖拽状态，在InsertContent时，如果发现是拖拽
        // 拽状态，在插入内容之后暂时不设置SetReadOnly(TRUE),而是等到OnMouseMove结束，
        // 也就是拖拽结束之后再恢复原来的设置。
        //
        _isDragging = TRUE;
        _readOnlyBeforeDrag = GetReadOnly();

        __super::OnMouseMove(nFlags, point);

        _isDragging = FALSE;
        if (_readOnlyBeforeDrag != GetReadOnly())
        {
            SetReadOnly(_readOnlyBeforeDrag);
        }
    }

    void SImRichEdit::OnLButtonDblClick(UINT nFlags, CPoint pt)
    {
        SetMsgHandled(FALSE);

        BOOL bHandled = FALSE;
        RichEditObj * pRichObj = HitTest(pt);

        if (pRichObj != NULL && pRichObj->NeedToProcessMessage())
        {
            pRichObj->ProcessMessage(
                WM_LBUTTONDBLCLK, GetCurMsg()->wParam, GetCurMsg()->lParam, bHandled);

            SetMsgHandled(bHandled);
        }
    }

    BOOL SImRichEdit::OnUpdateToolTip(CPoint pt, SwndToolTipInfo &tipInfo)
    {
        POINT point = { pt.x, pt.y };
        RichEditObj * pContent = HitTest(pt);

        if (pContent != NULL)
        {
            CRect rcOle = pContent->GetRect();
            point.x = point.x - rcOle.left;
            point.y = point.y - rcOle.top;

            if (pContent->OnUpdateToolTip(point, tipInfo))
            {
                tipInfo.rcTarget.OffsetRect(rcOle.TopLeft());
                return TRUE;
            }
        }

        return __super::OnUpdateToolTip(pt, tipInfo);
    }

    BOOL SImRichEdit::OnSetCursor(const CPoint &point)
    {
        LPARAM lParam = 0;
        POINT pt = { point.x, point.y };
        RichEditObj * pRichObj = HitTest(pt);
        if (pRichObj != NULL && pRichObj->NeedToProcessMessage())
        {
            BOOL bHandled = FALSE;
            LPARAM lParam = MAKELPARAM(pt.x, pt.y);
            pRichObj->ProcessMessage(WM_SETCURSOR, 0, lParam, bHandled);

            return TRUE;
        }

        if (!m_style.m_strCursor.IsEmpty())
        {
            return SWindow::OnSetCursor(point);
        }

        return SRichEdit::OnSetCursor(point);
    }

    BOOL SImRichEdit::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
    {
        int nPos = GetScrollPos(TRUE);

        //STRACE(_T("mouse wheel zdeta:%d"), zDelta);

        zDelta /= WHEEL_DELTA;
        int nFinalPos = nPos + zDelta * m_nScrollSpeed * -1;
        if (nFinalPos < 0)
        {
            nFinalPos = 0;
        }

        if (nFinalPos == 0)
        {
            EventRichEditScroll evt(this);
            evt.WheelDelta = zDelta;
            evt.ScrollAtTop = TRUE;

            FireEvent(evt);
        }


        INT nmin, nmax;
        GetScrollRange(TRUE, &nmin, &nmax);
        if (nFinalPos == nPos)
        {
            return FALSE;
        }

        OnScroll(TRUE, SB_THUMBTRACK, nFinalPos);
        DrawScrollbar(TRUE, SB_THUMBPOSITION, nFinalPos);

        return FALSE;
    }

} // namespace SOUI

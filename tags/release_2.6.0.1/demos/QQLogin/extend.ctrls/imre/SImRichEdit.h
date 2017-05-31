
// ------------------------------------------------------------------------------
//
// SImRichEdit.h : interface of the SImRichEdit class
//
// 定义聊天框RichEdit的接口，改控件主要在ms提供的RichEdit上实现了以下功能
//
// 1.气泡
// 2.头像
// 3.各类OLE，@人、图片、文件块等
// 4.拖拽效果
// 5.和剪贴板的交互
// 
// ------------------------------------------------------------------------------
//
// 为了实现高效的GIF动画刷新，针对GIF类图片进行了特殊处理。在刷新GIF时并
// 不请求RichEdit刷新，而是创建临时DC，然后请求OLE对象画在临时DC上，最后
// 到RichEdit上，实测效果，满屏GIF的情况下，CPU不超过4%。
//
// !!!特别注意：由于对GIF的刷新做了特殊处理，如果有额外的控件需要悬浮在
// SimRichEdit的上方（例如toast提示），需将该控件设为SImRichEdit的子控件!!!
//
// ------------------------------------------------------------------------------
//
// QA/注意事项:
// 
// 1. 在RichEdit是readonly的情况下，不能进行缩进设置
//
// 2. 如果要让RichEdit在失去焦点时还要显示选中高亮，需要设置
//    TXTBIT_SAVESELECTION 且不能设置TXTBIT_HIDESELECTION
//
// 3. 被缩进的段落最后一行是空行，需要在该段落最后再加一个空行，而且设置
//    缩进的时候需要把额外加的那个空行包含进去
//
// 4. SetIndents比较费时,能少用尽量少用
//
// 5.
//
// ------------------------------------------------------------------------------

#pragma once

#include <TOM.h>
#include "RichEditObj.h"
#include "RichEditOleBase.h"
#include "IRichEditObjHost.h"
#include "control/SRichEdit.h"
#include "RichEditOleCtrls.h"
#include "atl.mini/SComCli.h"

namespace SOUI
{

    class SImRichEdit : public SRichEdit, public IRichEditObjHost, public ITimelineHandler
    {
        struct UpdateContext
        {
            BOOL bReadOnly;
            BOOL bWordWrap;
            BOOL bHasScrollBar;
        };

        SOUI_CLASS_NAME(SImRichEdit, L"imrichedit");

    public:

        // ------------------------------------------------------------------------------
        //
        // public methods for user
        //
        // ------------------------------------------------------------------------------

        SImRichEdit();
        ~SImRichEdit();

        //
        // IRichEditObjHost methods
        //
        virtual ISwndContainer* GetHostContainer();
        virtual CRect           GetHostRect();
        virtual CRect           GetAdjustedRect();
        virtual int             GetCharCount();
        virtual void            DirectDraw(const CRect& rc);
        virtual void            DelayDraw(const CRect& rc);
        virtual HRESULT         SendMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *pRet);
        virtual ITextDocument*  GetTextDoc();
        virtual ITextServices*  GetTextServ();
        virtual BOOL            AcceptContent(RichFormatConv* conv);
        virtual bool            NotifyRichObjEvent(RichEditObj* obj, int subEvent, WPARAM wParam, LPARAM lParam);
        virtual void            EnableDraw(BOOL bEnable);
        virtual void            UpdateRichObj(RichEditObj* obj);
        virtual int             GetRemainingLength();
        virtual BOOL            IsEditable();
        virtual void            Activate();
        virtual UINT            InsertContent(LPCWSTR lpszContent, UINT uInsertAt = RECONTENT_LAST);
        virtual SStringW        GetSelectedContent(CHARRANGE* lpchrg/*=NULL*/);
        virtual void            DeleteContent(UINT uIndex);
        virtual UINT            GetContentCount();
        virtual void            Clear();
        virtual void            ScrollToBottom();
        virtual BOOL            IsScrollAtTop();
        virtual BOOL            IsScrollAtBottom();

        //
        // extent methods
        //
        RichEditObj*     GetElementById(LPCWSTR lpszId);
        RichEditObj*     HitTest(POINT ptInControl);
        RichEditContent* GetContent(LPCWSTR pszId);
        RichEditContent* GetContent(UINT uIndex);
        RichEditOleBase* GetOleById(LPCWSTR lpszId);
        RichEditOleBase* GetSelectedOle(int cp = -1);
        UINT             DeleteContent(RichEditContent * prec);
        BOOL             GetCaretRect(CRect& rcCaret);
        BOOL             GetCaretRect2(CRect& rcCaret);
        BOOL             CanPaste();
        void             UpdateBkgndRenderTarget();
        void             MarkBkgndRtDirty();
        void             SetAutoFixVScroll(BOOL fix);
        void             ForceUpdateLayout();
        void             ReLocateContents(UINT start, int offset);
        BOOL             GetContentIndex(LPCWSTR pszId, UINT& index);
        void             UpdateContentPosition(UINT start, UINT end = RECONTENT_LAST);
        void             ScrollTo(LPCWSTR pszId);
        void             ScrollTo(UINT uIndex);

        //
        // richedit message wrapper
        //
        void    SetFontSize(int size);
        void    SetSelectionColor(COLORREF cr);
        void    GetSel(long* pStartCp, long* pEndCp);
        void    SetSel(int nStart, int nEnd);
        BOOL    SetParaFormat(PARAFORMAT2& pf);
        int     LineFromChar(int ncp);
        int     LineIndex(int nLineNum);
        int     LineLength(int nLineCP /* = -1 */);
        void    PosFromChar(UINT ncp, POINT* pPos);
        int     CharFromPos(POINT pt);
        int     GetFirstVisibleLine();
        int     GetRangeText(const CHARRANGE& chr, SStringW& s);
        void    SetAutoVScroll(BOOL bEnabled);
        BOOL    IsAutoVScroll();
        void    SelectAll();
        void    Copy();
        void    Cut();
        void    Paste();
        void    EmptyUndoBuffer();
        BOOL    GetDefCharFormat(CHARFORMAT& cf);
        BOOL    SetDefCharFormat(CHARFORMAT& cf);

    protected:

        // ------------------------------------------------------------------------------
        //
        // internal helpers and event handlers
        //
        // ------------------------------------------------------------------------------

        //
        //caret相关方法
        //

        virtual BOOL CreateCaret(HBITMAP pBmp, int nWid, int nHeight);
        virtual void SetCaretPos(int x, int y);

        //
        // internal helpers
        //

        BOOL    IsRenderTargetEmpty(IRenderTarget* pRt);
        void    EnableDragDrop(BOOL enable);
        RichEditObj* HitTest(RichEditObj* p, POINT ptInControl);
        RichEditContent* CreateRichEditConent(LPCWSTR lpszContent);
        void    PreUpdate(UpdateContext&);
        void    DoneUpdate(const UpdateContext&);
        void    ClipChildren(SWindow * pWnd, IRegion * prgn);
        void    DrawScrollbar(BOOL bVertical, UINT uCode, int nPos);
        void    SetContentsDirty();
        void    DrawVisibleGifs(IRenderTarget * pRt, const CRect& validRgnRect);
        void    DirectDrawOles(IRegion* prgn);
        void    DrawVisibleContents(IRenderTarget * pRt);
        void    UpdateVisibleCharRanges();
        BOOL    RecalcScrollbarPos(BOOL bVertical, UINT uCode, int nPos);

        //
        // overrided methods
        //

        HRESULT     DefAttributeProc(const SStringW & strAttribName, const SStringW & strValue, BOOL bLoading);

        //
        // ITimelineHandler, use to delay draw gifs
        //
        void        OnNextFrame();

        //
        // internal message handlers
        //

        LRESULT    OnCreate(LPVOID);
        LRESULT    OnNcCalcSize(BOOL bCalcValidRects, LPARAM lParam);
        void       OnSize(UINT nType, CSize size);
        void       OnDestroy();
        void       OnPaint(IRenderTarget * pRT);
        void       OnLButtonDown(UINT nFlags, CPoint point);
        void       OnLButtonDblClick(UINT nFlags, CPoint point);
        void       OnLButtonUp(UINT nFlags, CPoint point);
        void       OnRButtonDown(UINT nFlags, CPoint point);
        void       OnRButtonUp(UINT nFlags, CPoint point);
        void       OnMouseMove(UINT nFlags, CPoint point);
        BOOL       OnSetCursor(const CPoint &pt);
        BOOL       OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
        BOOL       OnUpdateToolTip(CPoint pt, SwndToolTipInfo &tipInfo);
        BOOL       OnScroll(BOOL bVertical, UINT uCode, int nPos);
        //---------------------------------
        LRESULT	   OnImeStartComposition(UINT uMsg, WPARAM wParam, LPARAM lParam);
        LRESULT	   OnImeComposition(UINT uMsg, WPARAM wParam, LPARAM lParam);
        SOUI_MSG_MAP_BEGIN()
            MSG_WM_CREATE(OnCreate)
            MSG_WM_DESTROY(OnDestroy)
            MSG_WM_PAINT_EX(OnPaint)
            MSG_WM_SIZE(OnSize)
            MSG_WM_NCCALCSIZE(OnNcCalcSize)
            MSG_WM_LBUTTONDOWN(OnLButtonDown)
            MSG_WM_LBUTTONUP(OnLButtonUp)
            MSG_WM_RBUTTONDOWN(OnRButtonDown)
            MSG_WM_RBUTTONUP(OnRButtonUp)
            MSG_WM_LBUTTONDBLCLK(OnLButtonDblClick)
            MSG_WM_MOUSEWHEEL(OnMouseWheel)
            MSG_WM_MOUSEMOVE(OnMouseMove)
            MESSAGE_HANDLER_EX(WM_IME_STARTCOMPOSITION, OnImeStartComposition)
            MESSAGE_HANDLER_EX(WM_IME_COMPOSITION, OnImeComposition)
            SOUI_MSG_MAP_END()

    private:
        BOOL _onPostImmComposition(UINT msg, WPARAM wParam, LPARAM lParam);
    private:

        typedef SArray<RichEditContent*> RichContentArray;

        CRect                       _caretRect;         // 相对于richedit左上角的光标位置。记录调用TxSetCaretPos时的位置，
                                                        // 可用来快速获得光标的高度
        CHARRANGE                   _visibleChr;        // 可见的字符范围
        CHARRANGE                   _visibleOleChr;     // 可见的OLE范围
        CAutoRefPtr<IRenderTarget>  _pBackgroundRt;     // 背景rt
        BOOL                        _isBackgroundDirty; // 标记_pBackgroundRt是否失效
        ITextDocument*              _pTextDoc;
        RichContentArray            _richContents;      // richedit显示的内容
        RichEditObj*                _pLastHoverObj;     // 光标悬浮的最后一个obj
        time_t                      _lastDrawTicks;     // 记录最后一次刷新的时间，用来走定时刷新，60ticks刷一次
        CAutoRefPtr<IRegion>        _pDelayDrawRgn;     // 脏区域
        CRect                       _originalInset;     // 原始设置的内边距
        BOOL                        _isDrawEnable;      // 能否刷新UI
        BOOL                        _scrollbarOutdated; // 标记滚动条是否需要再设置一次，详细含义见OnScroll描述
        BOOL                        _scrollbarAtBottom; // 标记滚动条是否在底部
        BOOL                        _isDragging;        // 标记是否处于拖拽状态
        BOOL                        _readOnlyBeforeDrag;// 拖拽前的只读状态
        BOOL                        _fixVScroll;        // 动态刷新垂直滚动条，确保不会出现一片空白的情况
        bool						_isCreatIme;		//标记是否正要启动输入法，启动后这个标记位置为false
    };

}// namespace SOUI

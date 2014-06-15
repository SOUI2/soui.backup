#pragma once

#include <Richedit.h>
#include <TextServ.h>
#include "DuiPanel.h"
#include "DUISingleton.h"

namespace SOUI
{

class CDuiRichEdit;

class SOUI_EXP CDuiTextServiceHelper: public Singleton<CDuiTextServiceHelper>
{
public:
    CDuiTextServiceHelper()
    {
        m_rich20=LoadLibrary(_T("riched20.dll"));
        if(m_rich20) m_funCreateTextServices= (PCreateTextServices)GetProcAddress(m_rich20,"CreateTextServices");
    }
    ~CDuiTextServiceHelper()
    {
        if(m_rich20) FreeLibrary(m_rich20);
        m_funCreateTextServices=NULL;
    }
    

    HRESULT CreateTextServices( IUnknown *punkOuter, ITextHost *pITextHost, IUnknown **ppUnk )
    {
        if(!m_funCreateTextServices) return E_NOTIMPL;
        return m_funCreateTextServices(punkOuter,pITextHost,ppUnk);
    }

    static BOOL Init(){
        if(ms_Singleton) return FALSE;
        new CDuiTextServiceHelper();
        return TRUE;
    }

    static void Destroy()
    {
        if(ms_Singleton) delete ms_Singleton;
    }
protected:
    HINSTANCE    m_rich20;    //richedit module
    PCreateTextServices    m_funCreateTextServices;
};

class SOUI_EXP CDuiTextHost : public ITextHost
{
    friend class CDuiRichEdit;
public:
    CDuiTextHost(void);
    ~CDuiTextHost(void);

    BOOL Init(CDuiRichEdit* pDuiRichEdit);

    ITextServices * GetTextService()
    {
        return pserv;
    }

    POINT GetCaretPos(){return m_ptCaret;}
protected:

    // -----------------------------
    //    IUnknown interface
    // -----------------------------
    virtual HRESULT _stdcall QueryInterface(REFIID riid, void **ppvObject);
    virtual ULONG _stdcall AddRef(void);
    virtual ULONG _stdcall Release(void);

    // -----------------------------
    //    ITextHost interface
    // -----------------------------

    //@cmember Get the DC for the host
    virtual HDC         TxGetDC();

    //@cmember Release the DC gotten from the host
    virtual INT            TxReleaseDC(HDC hdc);

    //@cmember Show the scroll bar
    virtual BOOL         TxShowScrollBar(INT fnBar, BOOL fShow);

    //@cmember Enable the scroll bar
    virtual BOOL         TxEnableScrollBar (INT fuSBFlags, INT fuArrowflags);

    //@cmember Set the scroll range
    virtual BOOL         TxSetScrollRange(
        INT fnBar,
        LONG nMinPos,
        INT nMaxPos,
        BOOL fRedraw);

    //@cmember Set the scroll position
    virtual BOOL         TxSetScrollPos (INT fnBar, INT nPos, BOOL fRedraw);

    //@cmember InvalidateRect
    virtual void        TxInvalidateRect(LPCRECT prc, BOOL fMode);

    //@cmember Send a WM_PAINT to the window
    virtual void         TxViewChange(BOOL fUpdate);

    //@cmember Create the caret
    virtual BOOL        TxCreateCaret(HBITMAP hbmp, INT xWidth, INT yHeight);

    //@cmember Show the caret
    virtual BOOL        TxShowCaret(BOOL fShow);

    //@cmember Set the caret position
    virtual BOOL        TxSetCaretPos(INT x, INT y);

    //@cmember Create a timer with the specified timeout
    virtual BOOL         TxSetTimer(UINT idTimer, UINT uTimeout);

    //@cmember Destroy a timer
    virtual void         TxKillTimer(UINT idTimer);

    //@cmember Scroll the content of the specified window's client area
    virtual void        TxScrollWindowEx (
        INT dx,
        INT dy,
        LPCRECT lprcScroll,
        LPCRECT lprcClip,
        HRGN hrgnUpdate,
        LPRECT lprcUpdate,
        UINT fuScroll);

    //@cmember Get mouse capture
    virtual void        TxSetCapture(BOOL fCapture);

    //@cmember Set the focus to the text window
    virtual void        TxSetFocus();

    //@cmember Establish a new cursor shape
    virtual void     TxSetCursor(HCURSOR hcur, BOOL fText);

    //@cmember Converts screen coordinates of a specified point to the client coordinates
    virtual BOOL         TxScreenToClient (LPPOINT lppt);

    //@cmember Converts the client coordinates of a specified point to screen coordinates
    virtual BOOL        TxClientToScreen (LPPOINT lppt);

    //@cmember Request host to activate text services
    virtual HRESULT        TxActivate( LONG * plOldState );

    //@cmember Request host to deactivate text services
    virtual HRESULT        TxDeactivate( LONG lNewState );

    //@cmember Retrieves the coordinates of a window's client area
    virtual HRESULT        TxGetClientRect(LPRECT prc);

    //@cmember Get the view rectangle relative to the inset
    virtual HRESULT        TxGetViewInset(LPRECT prc);

    //@cmember Get the default character format for the text
    virtual HRESULT     TxGetCharFormat(const CHARFORMATW **ppCF );

    //@cmember Get the default paragraph format for the text
    virtual HRESULT        TxGetParaFormat(const PARAFORMAT **ppPF);

    //@cmember Get the background color for the window
    virtual COLORREF    TxGetSysColor(int nIndex);

    //@cmember Get the background (either opaque or transparent)
    virtual HRESULT        TxGetBackStyle(TXTBACKSTYLE *pstyle);

    //@cmember Get the maximum length for the text
    virtual HRESULT        TxGetMaxLength(DWORD *plength);

    //@cmember Get the bits representing requested scroll bars for the window
    virtual HRESULT        TxGetScrollBars(DWORD *pdwScrollBar);

    //@cmember Get the character to display for password input
    virtual HRESULT        TxGetPasswordChar(TCHAR *pch);

    //@cmember Get the accelerator character
    virtual HRESULT        TxGetAcceleratorPos(LONG *pcp);

    //@cmember Get the native size
    virtual HRESULT        TxGetExtent(LPSIZEL lpExtent);

    //@cmember Notify host that default character format has changed
    virtual HRESULT     OnTxCharFormatChange (const CHARFORMATW * pcf);

    //@cmember Notify host that default paragraph format has changed
    virtual HRESULT        OnTxParaFormatChange (const PARAFORMAT * ppf);

    //@cmember Bulk access to bit properties
    virtual HRESULT        TxGetPropertyBits(DWORD dwMask, DWORD *pdwBits);

    //@cmember Notify host of events
    virtual HRESULT        TxNotify(DWORD iNotify, void *pv);

    // Far East Methods for getting the Input Context
    //#ifdef WIN95_IME
    virtual HIMC        TxImmGetContext();
    virtual void        TxImmReleaseContext( HIMC himc );
    //#endif

    //@cmember Returns HIMETRIC size of the control bar.
    virtual HRESULT        TxGetSelectionBarWidth (LONG *plSelBarWidth);
protected:
    BOOL m_fUiActive    ; // Whether control is inplace active

    ULONG    cRefs;                    // Reference Count
    ITextServices    *pserv;            // pointer to Text Services object
    CDuiRichEdit    *m_pDuiRichEdit;// duiwindow for text host
    POINT                m_ptCaret;
};

#ifndef LY_PER_INCH
#define LY_PER_INCH 1440
#endif

#ifndef HIMETRIC_PER_INCH
#define HIMETRIC_PER_INCH 2540
#endif

class SOUI_EXP CDuiRichEdit :public CDuiPanel
{
    friend class CDuiTextHost;
public:
    SOUI_CLASS_NAME(CDuiRichEdit, "richedit")

    CDuiRichEdit();
    virtual ~CDuiRichEdit() {}

public://richedit interface
    int GetWindowText(LPWSTR lpString, int nMaxCount );
    int GetWindowTextLength();
    BOOL SetWindowText(LPCWSTR lpszText);
    void SetSel(DWORD dwSelection, BOOL bNoScroll = FALSE);
    void ReplaceSel(LPWSTR pszText,BOOL bCanUndo=TRUE);

    BOOL GetWordWrap(void);
    void SetWordWrap(BOOL fWordWrap);

    BOOL GetReadOnly();
    BOOL SetReadOnly(BOOL bReadOnly);

    LONG GetLimitText();
    BOOL SetLimitText(int nLength);

    WORD GetDefaultAlign();
    void SetDefaultAlign(WORD wNewAlign);

    BOOL GetRichTextFlag();
    void SetRichTextFlag(BOOL fRich);

    LONG GetDefaultLeftIndent();
    void SetDefaultLeftIndent(LONG lNewIndent);

    BOOL SetSaveSelection(BOOL fSaveSelection);

    COLORREF SetDefaultTextColor(COLORREF cr);
protected:

    LRESULT OnCreate(LPVOID);

    void OnDestroy();

    void OnPaint(CDCHandle dc);

    void OnSetDuiFocus();

    void OnKillDuiFocus();

    void OnDuiTimer(char idEvent);

    void OnDuiTimerEx(UINT_PTR idEvent);

    virtual UINT OnGetDuiCode()
    {
        UINT uRet=DUIC_WANTCHARS|DUIC_WANTARROWS;
        if(m_fWantTab) uRet |= DLGC_WANTTAB;
        if(m_dwStyle&ES_WANTRETURN) uRet |= DUIC_WANTRETURN;
        return uRet;
    }

    virtual BOOL OnScroll(BOOL bVertical,UINT uCode,int nPos);

    virtual BOOL OnDuiSetCursor(const CPoint &pt);

    virtual BOOL DuiWndProc(UINT uMsg,WPARAM wParam,LPARAM lParam,LRESULT & lResult);

    HRESULT InitDefaultCharFormat(CHARFORMAT2W* pcf,HFONT hFont=NULL);

    HRESULT InitDefaultParaFormat(PARAFORMAT2* ppf);

    HRESULT OnTxNotify(DWORD iNotify,LPVOID pv);

    virtual HRESULT DefAttributeProc(const CDuiStringA & strAttribName,const CDuiStringA & strValue, BOOL bLoading);

    void OnLButtonDown(UINT nFlags, CPoint point);

    void OnLButtonUp(UINT nFlags, CPoint point);

    void OnRButtonDown(UINT nFlags, CPoint point);

    void OnMouseMove(UINT nFlags, CPoint point);

    void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

    void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);

    void OnSetFont(HFONT font, BOOL bRedraw);

    LRESULT OnSetText(UINT uMsg,WPARAM wparam,LPARAM lparam);

    LRESULT OnSetCharFormat(UINT uMsg, WPARAM wParam, LPARAM lParam);

    LRESULT OnSetParaFormat(UINT uMsg, WPARAM wParam, LPARAM lParam);

    LRESULT OnSetReadOnly(UINT uMsg, WPARAM wParam, LPARAM lParam);

    LRESULT OnSetLimitText(UINT uMsg, WPARAM wParam, LPARAM lParam);

    LRESULT OnNcCalcSize(BOOL bCalcValidRects, LPARAM lParam);

    LRESULT OnSetTextColor(const CDuiStringA &  strValue,BOOL bLoading);

    void OnEnableDragDrop(BOOL bEnable);

protected:
    WND_MSG_MAP_BEGIN()
    MSG_WM_CREATE(OnCreate)
    MSG_WM_DESTROY(OnDestroy)
    MSG_WM_PAINT(OnPaint)
    MSG_WM_NCCALCSIZE(OnNcCalcSize)
    MSG_WM_SETFOCUS_EX(OnSetDuiFocus)
    MSG_WM_KILLFOCUS_EX(OnKillDuiFocus)
    MSG_WM_DUITIMER(OnDuiTimer)
    MSG_UM_TIMEREX(OnDuiTimerEx)
    MSG_WM_LBUTTONDOWN(OnLButtonDown)
    MSG_WM_LBUTTONUP(OnLButtonUp)
    MSG_WM_RBUTTONDOWN(OnRButtonDown)
    MSG_WM_MOUSEMOVE(OnMouseMove)
    MSG_WM_KEYDOWN(OnKeyDown)
    MSG_WM_CHAR(OnChar)
    MSG_WM_SETFONT(OnSetFont)
    MESSAGE_HANDLER_EX(WM_SETTEXT,OnSetText)
    MESSAGE_HANDLER_EX(EM_SETPARAFORMAT,OnSetParaFormat)
    MESSAGE_HANDLER_EX(EM_SETCHARFORMAT,OnSetCharFormat)
    MESSAGE_HANDLER_EX(EM_SETREADONLY,OnSetReadOnly)
    MESSAGE_HANDLER_EX(EM_EXLIMITTEXT,OnSetLimitText)
    WND_MSG_MAP_END()

    SOUI_ATTRS_BEGIN()
    ATTR_INT("style",m_dwStyle,FALSE)
    ATTR_INT("maxbuf",m_cchTextMost,FALSE)
    ATTR_INT("transparent",m_fTransparent,FALSE)
    ATTR_INT("rich",m_fRich,FALSE)
    ATTR_INT("vertical",m_fVertical,FALSE)
    ATTR_INT("wordwrap",m_fWordWrap,FALSE)
    ATTR_INT("allowbeep",m_fAllowBeep,FALSE)
    ATTR_INT("autowordsel",m_fEnableAutoWordSel,FALSE)
    ATTR_INT("vcenter",m_fSingleLineVCenter,FALSE)
    ATTR_RECT("inset",m_rcInsetPixel,FALSE)
    ATTR_CUSTOM("crtext",OnSetTextColor)
    SOUI_ATTRS_END()
    //////////////////////////////////////////////////////////////////////////
    //    RichEdit Properties
    CHARFORMAT2W m_cfDef;                // Default character format
    PARAFORMAT2    m_pfDef;                // Default paragraph format
    DWORD m_cchTextMost;                    // Maximize Characters
    TCHAR m_chPasswordChar;                // Password character
    LONG        m_lAccelPos;            // Accelerator position
    SIZEL        m_sizelExtent;            // Extent array
    CRect        m_rcInset;                // inset margin
    CRect        m_rcInsetPixel;            // inset margin in pixel
    TEXTMETRIC    m_tmFont;                //
    DWORD    m_dwStyle;
    
    UINT    m_fEnableAutoWordSel    :1;    // enable Word style auto word selection?
    UINT    m_fWordWrap            :1;    // Whether control should word wrap
    UINT    m_fRich                :1;    // Whether control is rich text
    UINT    m_fSaveSelection        :1;    // Whether to save the selection when inactive
    UINT    m_fTransparent        :1; // Whether control is transparent
    UINT    m_fVertical            :1;    // Whether control is layout following vertical
    UINT    m_fAllowBeep        :1;    // Whether message beep is allowed in the control
    UINT    m_fWantTab            :1;    // Whether control will deal with tab input
    UINT    m_fSingleLineVCenter:1;    // Whether control that is single line will be vertical centered
    UINT    m_fScrollPending    :1; // Whether scroll is activated by richedit or by panelex.
    UINT    m_fEnableDragDrop    :1;    // 允许在该控件中使用拖放

    BYTE    m_byDbcsLeadByte;

    CDuiTextHost    *m_pTxtHost;
};

class SOUI_EXP CDuiEdit : public CDuiRichEdit
{
    SOUI_CLASS_NAME(CDuiRichEdit, "edit")
public:
    CDuiEdit()
    {
        m_fRich=0;
    }
    virtual ~CDuiEdit() {}
};
}//namespace SOUI

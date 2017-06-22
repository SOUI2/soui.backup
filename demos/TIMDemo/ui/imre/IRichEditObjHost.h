#pragma once
#include <richedit.h>
#include <TOM.h>
#include "wtl.mini\souimisc.h"

namespace SOUI
{
    class RichFormatConv;
    class RichEditObj;


#define RECONTENT_CARET  4294967294 /*内容插入在光标(2^32-1)*/
#define RECONTENT_LAST   4294967295 /*内容插入在结尾(2^32-2)*/

    class IRichEditObjHost
    {
    public:

        virtual ISwndContainer* GetHostContainer() = 0;
        virtual CRect           GetHostRect() = 0;
        virtual CRect           GetAdjustedRect() = 0;
        virtual int             GetCharCount() = 0;
        virtual void            DirectDraw(const CRect& rc) = 0;
        virtual void            DelayDraw(const CRect& rc) = 0;
        virtual HRESULT         SendMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *pRet = NULL) = 0;
        virtual ITextDocument*  GetTextDoc() = 0;
        virtual ITextServices*  GetTextServ() = 0;
        virtual BOOL            AcceptContent(RichFormatConv* conv) = 0;
        virtual bool            NotifyRichObjEvent(RichEditObj* obj, int subEvent, WPARAM wParam, LPARAM lParam) = 0;
        virtual void            EnableDraw(BOOL bEnable) = 0;
        virtual void            UpdateRichObj(RichEditObj* obj) = 0;
        virtual int             GetRemainingLength() = 0;
        virtual BOOL            IsEditable() = 0;
        virtual void            Activate() = 0;
        virtual UINT            InsertContent(LPCWSTR lpszContent, UINT uInsertAt = RECONTENT_LAST) = 0;
        virtual void            DeleteContent(UINT uIndex) = 0;
        virtual UINT            GetContentCount() = 0;
        virtual SStringW        GetSelectedContent(CHARRANGE* lpchrg = NULL) = 0;
        virtual void            Clear() = 0;
        virtual void            ScrollToBottom() = 0;
        virtual BOOL            IsScrollAtTop() = 0;
        virtual BOOL            IsScrollAtBottom() = 0;
    };
};
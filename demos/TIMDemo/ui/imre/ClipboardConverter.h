#pragma once
#include <vector>
#include <GdiPlus.h>
#include <shellapi.h>
#include "pugixml\pugixml.hpp"
#include "HTMLParser.h"

namespace SOUI
{

    extern UINT  g_cfHTMLFormat;
    extern UINT  g_cfRichEditFormat;

    class RichFormatConv : public pugi::xml_tree_walker
    {
    public:
        typedef SArray<UINT> ClipboardFmts;
        typedef std::vector<SStringW> DropFiles;

        RichFormatConv();
        ~RichFormatConv();

        BOOL    GetSupportedFormatsFromClipboard(ClipboardFmts& fmts);
        BOOL    GetSupportedFormatsToClipboard(ClipboardFmts& fmts);
        UINT    GetAvtiveFormat();

        BOOL    InitFromRichContent(const SStringW& s);
        BOOL    InitFromDataObject(LPDATAOBJECT lpdataobj);
        BOOL    InitFromHDrop(HDROP hDrop);

        BOOL    ToRichContent(SStringW& s);
        BOOL    ToDataObject(LPDATAOBJECT * ppobj);

        BOOL    TextOnly() { return m_bTextOnly; }
        BOOL    ImageOnly() { return m_bImageOnly; }

        Gdiplus::Bitmap * GetImageData();
        DropFiles& GetDropFiles();

    private:

        // ------------------------------------------------------------------------------
        //
        // pugixml helpers
        //
        // ------------------------------------------------------------------------------

        bool    begin(pugi::xml_node& node);
        bool    for_each(pugi::xml_node& node);
        bool    end(pugi::xml_node& node);
        void    HandleRichEle(pugi::xml_node& node);

        // ------------------------------------------------------------------------------
        //
        // internal data format -> DATAOBJECT
        // The following methods will be invoked by ToDataObject
        //
        // 可以贴到粘贴板里的格式：
        // 自定义的格式   SO_RichEdit_Format
        // HTML             HTML Format
        // 图片           CF_DIB
        // Unicode          CF_UNICODETEXT
        //
        // ------------------------------------------------------------------------------

        BOOL    GetCustomerFormat(FORMATETC* pfmt, STGMEDIUM * pstg);
        BOOL    GetHTMLFormat(FORMATETC* pfmt, STGMEDIUM * pstg);
        BOOL    GetImageFormat(FORMATETC* pfmt, STGMEDIUM * pstg);
        BOOL    GetUnicodeTextFormat(FORMATETC* pfmt, STGMEDIUM * pstg);

        // ------------------------------------------------------------------------------
        //
        // internal data formt -> rich content
        // The following methods will be invoked by ToRichContent
        //
        // ------------------------------------------------------------------------------

        BOOL    CustomerFormatToRichContent(SStringW& content);
        BOOL    HTMLFormatToRichContent(SStringW& content);
        BOOL    AsciiFormatToRichContent(SStringW& content);
        BOOL    UnicodeFormatToRichContent(SStringW& content);
        BOOL    DropFormatToRichContent(SStringW& content);
        BOOL    ImageFormatToRichContent(SStringW& content, const SStringW& path = L"");

        // ------------------------------------------------------------------------------
        //
        // DATAOBJECT -> internal data format
        // The following methods will be invoked by InitFromDataObject
        //
        // 可以识别粘贴板的格式：
        // 自定义的格式   SO_RichEdit_Format
        // HTML             HTML Format
        // ASCII            CF_TEXT
        // Unicode          CF_UNICODETEXT
        // 图片           CF_DIB
        // 拖拽           CF_HDROP
        // ------------------------------------------------------------------------------

        BOOL    InitFromCustomerFormat(LPDATAOBJECT lpdataobj);
        BOOL    InitFromHTMLFormat(LPDATAOBJECT lpdataobj);
        BOOL    InitFromAsciiTextFormat(LPDATAOBJECT lpdataobj);
        BOOL    InitFromUincodeTextFormat(LPDATAOBJECT lpdataobj);
        BOOL    InitFromImageFormat(LPDATAOBJECT lpdataobj);
        BOOL    InitFromDropFormat(LPDATAOBJECT lpdataobj);

    private:

        SStringW GetImageStoragePath();

    private:

        BOOL    m_bTextOnly;
        BOOL    m_bImageOnly;

        SStringA m_strHTML;
        SStringW m_strImagePath;
        SStringW m_strText;

        Gdiplus::Bitmap * m_pImage;
        DropFiles   m_vecDropFile;
        SStringW    m_strRichContent;
        HTMLParser  m_htmlParser;
        UINT        m_uAcviteFormat;
    };
};
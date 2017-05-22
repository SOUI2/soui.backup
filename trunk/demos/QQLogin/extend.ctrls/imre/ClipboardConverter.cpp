#include "stdafx.h"
#include "souistd.h"
#include "ClipboardConverter.h"
#include "dataobject.h"
#include "utils.h"
#include "HtmlParser.h"
#include "RichEditOleCtrls.h"
#include <shlobj.h>

namespace SOUI
{
    UINT  g_cfHTMLFormat = ::RegisterClipboardFormat(L"HTML Format");
    UINT  g_cfRichEditFormat = ::RegisterClipboardFormat(L"So_RichEdit_Format");

    RichFormatConv::RichFormatConv() : m_bTextOnly(TRUE)
        , m_bImageOnly(TRUE)
        , m_uAcviteFormat(0)
        , m_pImage(NULL)
    {
    }

    RichFormatConv::~RichFormatConv()
    {
        if (m_pImage)
        {
            delete m_pImage;
            m_pImage = NULL;
        }
    }

    BOOL RichFormatConv::GetSupportedFormatsFromClipboard(ClipboardFmts& fmts)
    {
        fmts.RemoveAll();
        fmts.Add(g_cfRichEditFormat);
        fmts.Add(g_cfHTMLFormat);
        fmts.Add(CF_UNICODETEXT);
        fmts.Add(CF_TEXT);
        fmts.Add(CF_DIB);
        fmts.Add(CF_HDROP);

        return TRUE;
    }

    BOOL RichFormatConv::GetSupportedFormatsToClipboard(ClipboardFmts& fmts)
    {
        fmts.RemoveAll();
        fmts.Add(g_cfRichEditFormat);
        fmts.Add(g_cfHTMLFormat);
        fmts.Add(CF_UNICODETEXT);
        fmts.Add(CF_DIB);

        return TRUE;
    }

    UINT RichFormatConv::GetAvtiveFormat()
    {
        return m_uAcviteFormat;
    }

    BOOL RichFormatConv::InitFromRichContent(const SStringW& str)
    {
        pugi::xml_document doc;
        pugi::xml_node root;

        if (!doc.load(str) || !(root = doc.child(L"RichEditContent")))
        {
            return FALSE;
        }

        root.traverse(*this);

        m_strRichContent = str;

        /*
         * 构造HTML
         * Version:0.9..StartHTML:00000097..EndHTML:00000169..StartFragment:00000131..EndFragment:00000133..<html><body>..<!--StartFragment--><DIV>fd</DIV><!--EndFragment-->..</body>..</html>.
         * <------------------------------------------ HeaderSize:97 --------------------------------------><----- StartFragmanetSize:34 ----><contentSize><------- EndFragmentSize:36 -------->
        */

        const int HeaderSize = 97;
        const int StartFragmanetSize = 34;
        const int EndFragmentSize = 36;
        const int ContentSize = m_strHTML.GetLength();

        SStringA formater;
        formater.Format("Version:0.9\r\n"
            "StartHTML:%08d\r\n"
            "EndHTML:%08d\r\n"
            "StartFragment:%08d\r\n"
            "EndFragment:%08d\r\n"
            "<html><body>\r\n"
            "<!--StartFragment-->"
            "%s"
            "<!--EndFragment-->\r\n"
            "</body>\r\n"
            "</html>",
            HeaderSize,
            HeaderSize + StartFragmanetSize + ContentSize + EndFragmentSize,
            HeaderSize + StartFragmanetSize,
            HeaderSize + StartFragmanetSize + ContentSize,
            m_strHTML);

        m_strHTML = formater;
        return TRUE;
    }

    BOOL RichFormatConv::InitFromDataObject(LPDATAOBJECT lpdataobj)
    {
        return (
            InitFromCustomerFormat(lpdataobj) ||
            InitFromHTMLFormat(lpdataobj) ||
            InitFromImageFormat(lpdataobj) ||
            InitFromDropFormat(lpdataobj) ||
            InitFromUincodeTextFormat(lpdataobj) ||
            InitFromAsciiTextFormat(lpdataobj)
            );
    }

    BOOL RichFormatConv::InitFromHDrop(HDROP hDrop)
    {
        UINT uCount = 0;
        TCHAR szPath[MAX_PATH] = { 0 };

        uCount = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);
        if (uCount > 0)
        {
            for (UINT nIndex = 0; nIndex < uCount; ++nIndex)
            {
                DragQueryFile(hDrop, nIndex, szPath, _countof(szPath));
                m_vecDropFile.push_back(szPath);
            }
        }

        m_uAcviteFormat = CF_HDROP;

        return TRUE;
    }

    Gdiplus::Bitmap * RichFormatConv::GetImageData()
    {
        return m_pImage;
    }

    RichFormatConv::DropFiles& RichFormatConv::GetDropFiles()
    {
        return m_vecDropFile;
    }

    BOOL RichFormatConv::ToRichContent(SStringW& s)
    {
        if (m_uAcviteFormat == g_cfRichEditFormat)
        {
            return CustomerFormatToRichContent(s);
        }
        else if (m_uAcviteFormat == g_cfHTMLFormat)
        {
            return HTMLFormatToRichContent(s);
        }
        else if (m_uAcviteFormat == CF_UNICODETEXT)
        {
            return UnicodeFormatToRichContent(s);
        }
        else if (m_uAcviteFormat == CF_TEXT)
        {
            return AsciiFormatToRichContent(s);
        }
        else if (m_uAcviteFormat == CF_DIB)
        {
            return ImageFormatToRichContent(s);
        }
        else if (m_uAcviteFormat == CF_HDROP)
        {
            return DropFormatToRichContent(s);
        }

        return FALSE;
    }

    BOOL RichFormatConv::ToDataObject(LPDATAOBJECT * ppobj)
    {
        FORMATETC fmt[4];
        STGMEDIUM stg[4];
        int count = 0;

        if (!m_bTextOnly)
        {
            /*
             * 如果只有文字，则不需要其它格式
             */
            if (GetCustomerFormat(&fmt[count], &stg[count]))
                count++;

            if (GetImageFormat(&fmt[count], &stg[count]))
                count++;

            if (GetHTMLFormat(&fmt[count], &stg[count]))
                count++;
        }

        /*
         * 这里要留一个UNICODETEXT 的格式，否则拖拽的时候会出现莫名其妙的崩溃
        */
        if (GetUnicodeTextFormat(&fmt[count], &stg[count]))
            count++;

        HRESULT hr = CreateDataObject(fmt, stg, count, ppobj);
        return SUCCEEDED(hr);
    }

    BOOL RichFormatConv::GetCustomerFormat(FORMATETC* pfmt, STGMEDIUM * pstg)
    {
        if (m_strRichContent.IsEmpty())
        {
            return FALSE;
        }

        pfmt->cfFormat = g_cfRichEditFormat;
        pfmt->dwAspect = DVASPECT_CONTENT;
        pfmt->lindex = -1;
        pfmt->ptd = NULL;
        pfmt->tymed = TYMED_HGLOBAL;

        int  nMemSize = (m_strRichContent.GetLength() + 1) * sizeof(wchar_t);
        pstg->pUnkForRelease = NULL;
        pstg->tymed = TYMED_HGLOBAL;
        pstg->hGlobal = GlobalAlloc(GMEM_DDESHARE, nMemSize);
        void* pBuffer = GlobalLock(pstg->hGlobal);

        memset(pBuffer, 0, nMemSize);
        memcpy(pBuffer, (LPCWSTR)m_strRichContent, m_strRichContent.GetLength() * sizeof(wchar_t));

        GlobalUnlock(pstg->hGlobal);

        return TRUE;
    }

    BOOL RichFormatConv::GetHTMLFormat(FORMATETC* pfmt, STGMEDIUM * pstg)
    {
        if (m_strHTML.IsEmpty())
        {
            return FALSE;
        }

        //SStringA htmlUtf8 = S_CW2A(m_strHTML, CP_UTF8);

        pfmt->cfFormat = g_cfHTMLFormat;
        pfmt->dwAspect = DVASPECT_CONTENT;
        pfmt->lindex = -1;
        pfmt->ptd = NULL;
        pfmt->tymed = TYMED_HGLOBAL;

        int  nMemSize = (m_strHTML.GetLength() + 1);
        pstg->pUnkForRelease = NULL;
        pstg->tymed = TYMED_HGLOBAL;
        pstg->hGlobal = GlobalAlloc(GMEM_DDESHARE, nMemSize);
        void* pBuffer = GlobalLock(pstg->hGlobal);

        memset(pBuffer, 0, nMemSize);
        memcpy(pBuffer, m_strHTML.GetBuffer(0), nMemSize);

        GlobalUnlock(pstg->hGlobal);
        return TRUE;
    }

    BOOL RichFormatConv::GetImageFormat(FORMATETC* pfmt, STGMEDIUM * pstg)
    {
        if (!m_bImageOnly || m_strImagePath.IsEmpty())
        {
            return FALSE;
        }

        Gdiplus::Bitmap* bmp = Gdiplus::Bitmap::FromFile(m_strImagePath);
        if (bmp->GetLastStatus() != Gdiplus::Ok)
        {
            delete bmp;
            return FALSE;
        }

        HBITMAP hbmp = NULL;
        if (bmp->GetHBITMAP(Gdiplus::Color(0, 0, 0, 0), &hbmp) != Gdiplus::Ok)
        {
            delete bmp;
            return FALSE;
        }

        DIBSECTION dibsec;
        ::GetObject(hbmp, sizeof(dibsec), &dibsec);

        HGLOBAL hGlobal = GlobalAlloc(0, sizeof(BITMAPINFOHEADER) + dibsec.dsBmih.biSizeImage);
        LPBYTE  pDst = (LPBYTE)::GlobalLock(hGlobal);
        LPBYTE  pSrc = (LPBYTE)dibsec.dsBm.bmBits;

        memcpy(pDst, &dibsec.dsBmih, sizeof(BITMAPINFOHEADER));
        pDst += sizeof(BITMAPINFOHEADER);

        memcpy(pDst, pSrc, dibsec.dsBmih.biSizeImage);

        pfmt->cfFormat = CF_DIB;
        pfmt->dwAspect = DVASPECT_CONTENT;
        pfmt->lindex = -1;
        pfmt->ptd = NULL;
        pfmt->tymed = TYMED_HGLOBAL;

        pstg->pUnkForRelease = NULL;
        pstg->tymed = TYMED_HGLOBAL;
        pstg->hGlobal = hGlobal;

        ::GlobalUnlock(hGlobal);
        delete bmp;

        return TRUE;
    }

    BOOL RichFormatConv::GetUnicodeTextFormat(FORMATETC* pfmt, STGMEDIUM * pstg)
    {
        pfmt->cfFormat = CF_UNICODETEXT;
        pfmt->dwAspect = DVASPECT_CONTENT;
        pfmt->lindex = -1;
        pfmt->ptd = NULL;
        pfmt->tymed = TYMED_HGLOBAL;

        int  nMemSize = (m_strText.GetLength() + 1) * sizeof(wchar_t);
        pstg->pUnkForRelease = NULL;
        pstg->tymed = TYMED_HGLOBAL;
        pstg->hGlobal = GlobalAlloc(GMEM_DDESHARE, nMemSize);
        void* pBuffer = GlobalLock(pstg->hGlobal);

        memset(pBuffer, 0, nMemSize);
        memcpy(pBuffer, (LPCWSTR)m_strText, m_strText.GetLength() * sizeof(wchar_t));

        GlobalUnlock(pstg->hGlobal);

        return TRUE;
    }

    BOOL RichFormatConv::InitFromCustomerFormat(LPDATAOBJECT lpdataobj)
    {
        void * pBuffer = NULL;
        STGMEDIUM stg;
        FORMATETC fmt = { g_cfRichEditFormat, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };

        if (FAILED(lpdataobj->GetData(&fmt, &stg)) || (pBuffer = GlobalLock(stg.hGlobal)) == NULL)
        {
            return FALSE;
        }

        m_strRichContent = (WCHAR*)pBuffer;

        GlobalUnlock(stg.hGlobal);
        m_uAcviteFormat = g_cfRichEditFormat;

        return TRUE;
    }

    BOOL RichFormatConv::InitFromHTMLFormat(LPDATAOBJECT lpdataobj)
    {
        void * pBuffer = NULL;
        STGMEDIUM stg;
        FORMATETC fmt = { g_cfHTMLFormat, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };

        if (FAILED(lpdataobj->GetData(&fmt, &stg)) || (pBuffer = GlobalLock(stg.hGlobal)) == NULL)
        {
            return FALSE;
        }

        SStringW strHtml = S_CA2W((char*)pBuffer, CP_UTF8);

        /*
         * 替换掉UTF8中的空格字符。UTF8中的空格编码为c2a0，转换为unicode
         * 之后变成了a000， 所以需要手动将a000替换为2000，也就是L" ",否则会显示为乱码
        */

        WCHAR space[] = { 0xa0, 0x00 };
        strHtml.Replace(space, L" ");

        SStringW tmpstr = strHtml;
        if (tmpstr.MakeLower().Find(_T("<img")) < 0)
        {
            return FALSE;  // 只粘贴带图片的HTML格式,否则当作普通的UNICODE TEXT处理
        }

        SStringW    start = _T("<!--StartFragment");
        SStringW    end = _T("<!--EndFragment-->");
        int         nstart = strHtml.Find(start);
        int         nend = strHtml.Find(end);

        if (nstart < 0 || nend <= nstart)
        {
            return FALSE;
        }

        SStringW fragment = strHtml.Mid(nstart, nend - nstart + end.GetLength());
        fragment.Replace(_T("\r"), _T(""));
        fragment.Replace(_T("\n"), _T(""));

        if (!m_htmlParser.Parse(fragment))
        {
            return FALSE;
        }

        m_uAcviteFormat = g_cfHTMLFormat;

        return TRUE;
    }

    BOOL RichFormatConv::InitFromUincodeTextFormat(LPDATAOBJECT lpdataobj)
    {
        void * pBuffer = NULL;
        STGMEDIUM stg;
        FORMATETC fmt = { CF_UNICODETEXT, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };

        if (FAILED(lpdataobj->GetData(&fmt, &stg)) || (pBuffer = GlobalLock(stg.hGlobal)) == NULL)
        {
            return FALSE;
        }

        m_strRichContent = (wchar_t*)pBuffer;

        m_uAcviteFormat = CF_UNICODETEXT;

        return TRUE;
    }

    BOOL RichFormatConv::InitFromAsciiTextFormat(LPDATAOBJECT lpdataobj)
    {
        void * pBuffer = NULL;
        STGMEDIUM stg;
        FORMATETC fmt = { CF_TEXT, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };

        if (FAILED(lpdataobj->GetData(&fmt, &stg)) || (pBuffer = GlobalLock(stg.hGlobal)) == NULL)
        {
            return FALSE;
        }

        SStringA txtAscii = (char*)pBuffer;
        m_strRichContent = S_CA2W(txtAscii);

        m_uAcviteFormat = CF_TEXT;

        return TRUE;
    }

    BOOL RichFormatConv::InitFromImageFormat(LPDATAOBJECT lpdataobj)
    {
        void * pBuffer = NULL;
        STGMEDIUM stg;
        FORMATETC fmt = { CF_DIB, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };

        //CImage img;
        if (FAILED(lpdataobj->GetData(&fmt, &stg)) || (pBuffer = GlobalLock(stg.hGlobal)) == NULL)
        {
            return FALSE;
        }

        LPBITMAPINFOHEADER  lpBI = (LPBITMAPINFOHEADER)pBuffer;
        LPBITMAPINFO        pBMInfo = (LPBITMAPINFO)lpBI;
        void              * pDIBBits = (void*)(lpBI + 1);

        /*
         * 光栅偏移
        */
        int nColors = lpBI->biClrUsed ? lpBI->biClrUsed : 1 << lpBI->biBitCount;
        if (lpBI->biBitCount > 8)
        {
            LPDWORD lpBase = (LPDWORD)(pBMInfo->bmiColors + pBMInfo->bmiHeader.biClrUsed);
            int    nOffset = lpBI->biCompression == BI_BITFIELDS ? 3 : 0;
            pDIBBits = (LPVOID*)(lpBase + nOffset);
        }
        else
        {
            pDIBBits = (LPVOID*)(pBMInfo->bmiColors + nColors);
        }

        if (m_pImage)
        {
            delete m_pImage;
        }

        m_pImage = Gdiplus::Bitmap::FromBITMAPINFO(pBMInfo, pDIBBits);
        if (m_pImage && m_pImage->GetLastStatus() != Gdiplus::Ok)
        {
            delete m_pImage;
            m_pImage = NULL;

            return FALSE;
        }

        m_uAcviteFormat = CF_DIB;

        return TRUE;

        //HDC hDC            = ::GetDC(NULL);
        //HDC hDCMem         = ::CreateCompatibleDC(hDC);
        //HBITMAP hBitmap    = ::CreateCompatibleBitmap(hDC, lpBI->biWidth, abs(lpBI->biHeight));
        //HBITMAP hOldBmp    = (HBITMAP)::SelectObject(hDCMem, hBitmap);

        //::StretchDIBits(hDCMem,
        //    0,  0,  lpBI->biWidth, abs(lpBI->biHeight),
        //    0,  0,  lpBI->biWidth,  abs(lpBI->biHeight),
        //    pDIBBits, 
        //    (BITMAPINFO*)lpBI, 
        //    DIB_RGB_COLORS, SRCCOPY);

        ///*
        // * 保存成文件
        //*/
        //SStringW tmpPath;
        //tmpPath.Format(L"%s%s.png", GetTempPath(), GenGuid()); 

        //Gdiplus::ImageCodecInfo info;
        //BOOL    bRet = FALSE;
        //DWORD   dwTotalSize = 0;
        //LPBYTE  pBytes = NULL;

        //if (GetCodecInfo(L"image/png", &info) >= 0&&
        //    GetBitmapData(pBytes, dwTotalSize, hBitmap))
        //{
        //    IBitmap * pImg=NULL;
        //    GETRENDERFACTORY->CreateBitmap(&pImg);
        //    pImg->LoadFromMemory(pBytes, dwTotalSize);
        //    pImg->Save(tmpPath, &info.FormatID);

        //    m_strRichContent.Format(L"<RichEditContent><img src=\"%s\" /></RichEditContent>", tmpPath);

        //    pImg->Release();
        //    delete []pBytes;
        //    
        //    bRet = TRUE;
        //}

        //// do release
        //::SelectObject(hDCMem, hOldBmp);
        //::DeleteObject(hBitmap);
        //::DeleteDC(hDCMem);
        //::ReleaseDC(NULL, hDC);

        //return bRet;
    }

    BOOL RichFormatConv::InitFromDropFormat(LPDATAOBJECT lpdataobj)
    {
        STGMEDIUM stg;
        FORMATETC fmt = { CF_HDROP, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };

        if (FAILED(lpdataobj->GetData(&fmt, &stg)))
        {
            return FALSE;
        }

        return InitFromHDrop((HDROP)stg.hGlobal);
    }

    BOOL RichFormatConv::CustomerFormatToRichContent(SStringW& content)
    {
        if (m_uAcviteFormat != g_cfRichEditFormat)
        {
            return FALSE;
        }

        content = m_strRichContent;
        content.Replace(L"<SoRichEditFormat>", L"<RichEditContent>");
        content.Replace(L"</SoRichEditFormat>", L"</RichEditContent>");

        return TRUE;
    }

    BOOL RichFormatConv::HTMLFormatToRichContent(SStringW& content)
    {
        if (m_uAcviteFormat != g_cfHTMLFormat)
        {
            return FALSE;
        }

        content = L"<RichEditContent>";
        BOOL bEndWithDiv = FALSE;

        HTMLElements& elements = m_htmlParser.GetElements();
        HTMLElements::iterator it = elements.begin();

        for (; it != elements.end(); ++it)
        {
            SStringW value;
            SStringW tag = it->GetName();

            if (tag.CompareNoCase(L"text") == 0)
            {
                value = it->GetAttrubite(_T("value"));
                content += RichEditText::MakeFormatedText(value);
            }
            else if (tag.CompareNoCase(L"br") == 0 ||
                tag.CompareNoCase(L"li") == 0)
            {
                content += L"<text>\r\n</text>";
            }
            else if (tag.CompareNoCase(L"img") == 0)
            {
                value = it->GetAttrubite(_T("src"));
                //HTMLHelper::HttpPath2LocalPath(value);

                content += SStringW().Format(L"<img path=\"%s\" />", XMLEscape(value));;
            }
            else if (tag.CompareNoCase(L"div") == 0)
            {
                if (bEndWithDiv)
                {
                    content += L"<text>\r\n</text>";
                }
                bEndWithDiv = false;
            }
            else if (tag.CompareNoCase(L"/div") == 0)
            {
                bEndWithDiv = true;
            }
        }

        content += L"</RichEditContent>";

        return TRUE;
    }

    BOOL RichFormatConv::AsciiFormatToRichContent(SStringW& content)
    {
        if (m_uAcviteFormat != CF_TEXT)
        {
            return FALSE;
        }

        content.Format(
            L"<RichEditContent>"
            L"<text>%s</text>"
            L"</RichEditContent>", XMLEscape(m_strRichContent));

        return TRUE;
    }

    BOOL RichFormatConv::UnicodeFormatToRichContent(SStringW& content)
    {
        if (m_uAcviteFormat != CF_UNICODETEXT)
        {
            return FALSE;
        }

        content.Format(
            L"<RichEditContent>"
            L"<text>%s</text>"
            L"</RichEditContent>", XMLEscape(m_strRichContent));

        return TRUE;
    }

    BOOL RichFormatConv::DropFormatToRichContent(SStringW& content)
    {
        return FALSE;
    }

    BOOL RichFormatConv::ImageFormatToRichContent(SStringW& content, const SStringW& path)
    {
        if (m_uAcviteFormat != CF_DIB || m_pImage == NULL)
        {
            return FALSE;
        }

        int wid = m_pImage->GetWidth();
        int hei = m_pImage->GetHeight();

        SStringW strImagePath = path;

        if (strImagePath.IsEmpty())
        {
            Gdiplus::ImageCodecInfo info;
            strImagePath.Format(L"%s%s.png", GetImageStoragePath(), GenGuid());

            if (GetCodecInfo(L"image/png", &info) < 0 ||
                m_pImage->Save(strImagePath, &info.Clsid, 0) != Gdiplus::Ok)
            {
                return FALSE;
            }
        }

        content.Format(L"<RichEditContent><img path=\"%s\" /></RichEditContent>", strImagePath);

        return TRUE;
    }

    void RichFormatConv::HandleRichEle(pugi::xml_node& node)
    {
        if (wcscmp(node.name(), RichEditImageOle::GetClassName()) == 0)
        {
            SStringW img;
            m_strImagePath = node.attribute(RichEditImageOle::TagPath).as_string();
            img.Format(L"<img src=\"file:///%s\" />", m_strImagePath);

            SStringA utf8 = S_CW2A(img, CP_UTF8);
            m_strHTML += utf8;

            m_bTextOnly = FALSE;
        }
        else if (wcscmp(node.name(), RichEditText::GetClassName()) == 0)
        {
            SStringW txt = node.text().get();

            m_strText += txt;

            //
            // 由于pugixml解析字符串时，默认会打开 parse_eol 选项，所有的\r、\n会统一转化为\n
            // 所以换行以\n为准
            //
            txt.Replace(L"\n", L"<br/>");
            SStringA utf8 = S_CW2A(txt, CP_UTF8);
            m_strHTML += utf8;

            m_bImageOnly = FALSE;
        }
        else if (
            wcslen(node.name()) > 0 &&
            wcscmp(node.name(), RichEditContent::GetClassName()) != 0)
        {
            m_bTextOnly = FALSE;
            m_bImageOnly = FALSE;
        }
    }

    bool RichFormatConv::begin(pugi::xml_node& node)
    {
        HandleRichEle(node);
        return true;
    }

    bool RichFormatConv::for_each(pugi::xml_node& node)
    {
        HandleRichEle(node);
        return true;
    }

    bool RichFormatConv::end(pugi::xml_node& node)
    {
        HandleRichEle(node);
        return true;
    }

    SStringW RichFormatConv::GetImageStoragePath()
    {
        WCHAR szTempPath[MAX_PATH + 1];
        ::GetTempPathW(MAX_PATH + 1, szTempPath);

        SStringW path;
        path.Format(L"%sImClient\\", szTempPath);

        SHCreateDirectoryEx(NULL, path, NULL);

        return path;
    }

};
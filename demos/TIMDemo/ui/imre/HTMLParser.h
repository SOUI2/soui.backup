#pragma once
#include <vector>

class HTMLHelper
{
public:
    static void ReplaceHtmlSpecChars(SOUI::SStringW& sHtml);
    static void ConvertToHtmlSpecChars(SOUI::SStringW& sText);
    static void HttpPath2LocalPath(SOUI::SStringW& sHttpPath);
    static void LocalPath2HttpPath(SOUI::SStringW& sLocalPath);
    static void EncodeHtmlClipboardFormat(
        SOUI::SStringW& sHtml,
        SOUI::SStringW& sClipboardFormat);
};

class HTMLTaget
{
public:
    HTMLTaget(const SOUI::SStringW& sTarget);
    BOOL            Attach(const SOUI::SStringW& sTarget);
    BOOL            IsSpaceChar(TCHAR ch);
    SOUI::SStringW  GetName();
    SOUI::SStringW  GetAttrubite(LPCWSTR pszAttrName);

private:
    SOUI::SStringW m_sTarget;
};

typedef std::vector<HTMLTaget> HTMLElements;
class HTMLParser
{
public:
    BOOL Parse(const SOUI::SStringW& sHtml);

    HTMLElements& GetElements()
    {
        return m_htmlElements;
    }

private:
    HTMLElements m_htmlElements;
};

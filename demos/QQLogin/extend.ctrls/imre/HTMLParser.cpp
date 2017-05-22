#include "stdafx.h"
#include "souistd.h"

using namespace SOUI;
#include "HtmlParser.h"
//////////////////////////////////////////////////////////////////////////
// HtmlHelper

/*
 * 把特殊字符的HTML编码转化为UNICODE字符。
 * 只保证常规的ASCII编码能正常转换，额外的特殊字符不转换，例如�
 */
void HTMLHelper::ReplaceHtmlSpecChars(SStringW& sHtml)
{
    sHtml.Replace(_T("&quot;"), _T("\""));
    sHtml.Replace(_T("&#34;"), _T("\""));
    sHtml.Replace(_T("&#8221;"), _T("\""));
    sHtml.Replace(_T("&amp;"), _T("&"));
    sHtml.Replace(_T("&lt;"), _T("<"));
    sHtml.Replace(_T("&#60;"), _T("<"));
    sHtml.Replace(_T("&gt;"), _T(">"));
    sHtml.Replace(_T("&#62;"), _T(">"));
    sHtml.Replace(_T("&nbsp;"), _T(" "));
    sHtml.Replace(_T("&#160;"), _T(" "));
    sHtml.Replace(_T("&apos;"), _T("'"));
    sHtml.Replace(_T("&#8217;"), _T("'"));
    sHtml.Replace(_T("&#39;"), _T("'"));
}

/*
 * 把特殊字符的UNICODE字符转化为HTML编码
 */
void HTMLHelper::ConvertToHtmlSpecChars(SStringW& sText)
{
    //HTML特殊字符替换，如<，>，#，&，'，", 空格
    sText.Replace(_T("&"), _T("&amp;"));
    sText.Replace(_T("<"), _T("&lt;"));
    sText.Replace(_T(">"), _T("&gt;"));
    sText.Replace(_T(" "), _T("&nbsp;"));
    sText.Replace(_T("\""), _T("&quot;"));
    sText.Replace(_T("'"), _T("&apos;"));
    sText.Replace(_T("\r"), _T("<br/>"));
}

void HTMLHelper::HttpPath2LocalPath(SStringW& sHttpPath)
{
    sHttpPath.Replace(_T("file:///"), _T(""));
    sHttpPath.Replace(_T("%20"), _T(" "));
    sHttpPath.Replace(_T("%23"), _T("#"));
    sHttpPath.Replace(_T("%25"), _T("%"));
    sHttpPath.Replace(_T("%7B"), _T("{"));
    sHttpPath.Replace(_T("%7D"), _T("}"));
    sHttpPath.Replace(_T("%60"), _T("`"));
    sHttpPath.Replace(_T("%5E"), _T("^"));
    sHttpPath.Replace(_T("/"), _T("\\"));
}

void HTMLHelper::LocalPath2HttpPath(SStringW& sLocalPath)
{
    sLocalPath.Replace(_T("\\"), _T("/"));
    sLocalPath.Replace(_T("%"), _T("%25")); // 这个位置不能动
    sLocalPath.Replace(_T(" "), _T("%20"));
    sLocalPath.Replace(_T("#"), _T("%23"));
    sLocalPath.Replace(_T("{"), _T("%7B"));
    sLocalPath.Replace(_T("}"), _T("%7D"));
    sLocalPath.Replace(_T("`"), _T("%60"));
    sLocalPath.Replace(_T("^"), _T("%5E"));
    sLocalPath.Insert(0, _T("file:///"));
}

void HTMLHelper::EncodeHtmlClipboardFormat(
    SOUI::SStringW& sHtml,
    SOUI::SStringW& sClipboardFormat)
{
    // 构造HTML
    //Version:0.9..StartHTML:00000097..EndHTML:00000169..StartFragment:00000131..EndFragment:00000133..<html><body>..<!--StartFragment--><DIV>fd</DIV><!--EndFragment-->..</body>..</html>.
    //<------------------------------------------ HeaderSize:97 --------------------------------------><----- StartFragmanetSize:34 ----><contentSize><------- EndFragmentSize:36 -------->
    const int HeaderSize = 97;
    const int StartFragmanetSize = 34;
    const int EndFragmentSize = 36;
    const int ContentSize = sHtml.GetLength();

    sClipboardFormat.Format(_T("Version:0.9\r\n")
        _T("StartHTML:%08d\r\n")
        _T("EndHTML:%08d\r\n")
        _T("StartFragment:%08d\r\n")
        _T("EndFragment:%08d\r\n")
        _T("<html><body>\r\n")
        _T("<!--StartFragment-->")
        _T("%s")
        _T("<!--EndFragment-->\r\n")
        _T("</body>\r\n")
        _T("</html>"),
        HeaderSize,
        HeaderSize + StartFragmanetSize + ContentSize + EndFragmentSize,
        HeaderSize + StartFragmanetSize,
        HeaderSize + StartFragmanetSize + ContentSize,
        sHtml);
}

//////////////////////////////////////////////////////////////////////////
// HtmlTarget
HTMLTaget::HTMLTaget(const SStringW& sTarget)
{
    Attach(sTarget);
}

BOOL HTMLTaget::Attach(const SStringW& tag)
{
    int nLength = tag.GetLength();
    if (nLength < 2 ||
        tag.GetAt(0) != TCHAR('<') ||
        tag.GetAt(nLength - 1) != TCHAR('>'))
    {
        return FALSE;
    }

    m_sTarget = tag;
    return TRUE;
}

SStringW HTMLTaget::GetName()
{
    SStringW sTargetNme;
    for (int i = 1; i < m_sTarget.GetLength(); ++i)
    {
        TCHAR ch = m_sTarget.GetAt(i);
        if (ch == TCHAR(' ') || ch == TCHAR('>'))
        {
            break;
        }
        sTargetNme.AppendFormat(_T("%c"), ch);
        //sTargetNme.AppendChar(ch);
    }

    return sTargetNme;
}

BOOL HTMLTaget::IsSpaceChar(TCHAR ch)
{
    return ch == TCHAR(' ') || ch == TCHAR('\t');
}

SStringW HTMLTaget::GetAttrubite(LPCWSTR pszAttrName)
{
    int nIndex = m_sTarget.Find(pszAttrName);
    if (nIndex < 0) return _T("");

    nIndex += _tcslen(pszAttrName);
    while (IsSpaceChar(m_sTarget.GetAt(nIndex)))nIndex++;

    if (m_sTarget.GetAt(nIndex) != TCHAR('='))
    {
        // 非法的名值对格式
        return _T("");
    }

    //去掉等号后面的空白字符
    nIndex += 1;
    while (IsSpaceChar(m_sTarget.GetAt(nIndex)))nIndex++;

    TCHAR chQuotation = m_sTarget.GetAt(nIndex);
    if (chQuotation != TCHAR('"') && chQuotation != TCHAR('\''))
    {
        return _T("");
    }

    nIndex += 1;
    int nAttrEnd = m_sTarget.Find(chQuotation, nIndex);
    if (nAttrEnd < 0)
    {
        return _T("");
    }

    SStringW sAttrValue = m_sTarget.Mid(nIndex, nAttrEnd - nIndex);
    return sAttrValue;
}

//////////////////////////////////////////////////////////////////////////
// HtmlParser

BOOL HTMLParser::Parse(const SStringW& sHtml)
{
    m_htmlElements.clear();

    int nIndex = 0;
    while (true)
    {
        int nlt = sHtml.Find(TCHAR('<'), nIndex);

        if (nlt < 0)
        {
            SStringW text = sHtml.Right(sHtml.GetLength() - nIndex);

            if (text.GetLength() > 0)
            {
                SStringW sTarget;
                HTMLHelper::ReplaceHtmlSpecChars(text);

                sTarget.Format(_T("<text value=\"%s\" />"), text);

                HTMLTaget target(sTarget);
                m_htmlElements.push_back(target);
            }

            return TRUE;
        }

        // 文本信息
        if (nIndex != nlt)
        {
            SStringW text = sHtml.Mid(nIndex, nlt - nIndex);
            if (text.GetLength() > 0)
            {
                SStringW sTarget;
                HTMLHelper::ReplaceHtmlSpecChars(text);

                sTarget.Format(_T("<text value=\"%s\" />"), text);

                HTMLTaget target(sTarget);
                m_htmlElements.push_back(target);
            }
        }

        nIndex = nlt + 1;
        int ngt = sHtml.Find(TCHAR('>'), nIndex);
        if (ngt < 0)
        {
            SStringW text = sHtml.Right(sHtml.GetLength() - nIndex);
            if (text.GetLength() > 0)
            {
                SStringW sTarget;
                HTMLHelper::ReplaceHtmlSpecChars(text);

                sTarget.Format(_T("<text value=\"%s\" />"), text);

                HTMLTaget target(sTarget);
                m_htmlElements.push_back(target);
            }

            return TRUE;
        }

        nIndex = ngt + 1;

        SStringW tag = sHtml.Mid(nlt, ngt - nlt + 1);
        HTMLTaget target(tag);
        m_htmlElements.push_back(target);
    }

    return TRUE;
}

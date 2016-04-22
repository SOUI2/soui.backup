#include "stdafx.h"
#include "utils.h"


//////////////////////////////////////////////////////////////////////////
// xml helpers
SStringW XMLEscape(SStringW& strXML)
{
    strXML.Replace(L"&", L"&amp;");
    strXML.Replace(L"<", L"&lt;");
    strXML.Replace(L">", L"&gt;");
    strXML.Replace(L"'", L"&apos;");
    strXML.Replace(L"\"", L"&quot;");
    return strXML;
}

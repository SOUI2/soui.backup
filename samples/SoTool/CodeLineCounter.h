#pragma once

namespace SOUI
{
    struct CCodeConfig
    {
        SStringW strType;
        SStringW strExt;
        SStringW strSingleLineRemark;
        SStringW strMultiLinesRemarkBegin;
        SStringW strMultiLinesRemarkEnd;
    };

    BOOL CountCodeLines(LPCTSTR pszFileName, const CCodeConfig & config,int & nCodeLines,int & nRemLines,int & nBlankLines);

}

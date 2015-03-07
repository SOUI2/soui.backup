#include "stdafx.h"
#include "CodeLineCounter.h"

namespace SOUI
{
    // These flags determine the encoding of input data for XML document
    enum encoding
    {
        encoding_utf8,		// UTF8 encoding
        encoding_utf16_le,	// Little-endian UTF16
        encoding_utf16_be,	// Big-endian UTF16
        encoding_utf16,		// UTF16 with native endianness
        encoding_utf32_le,	// Little-endian UTF32
        encoding_utf32_be,	// Big-endian UTF32
        encoding_utf32,		// UTF32 with native endianness
        encoding_wchar,		// The same encoding wchar_t has (either UTF16 or UTF32)
        encoding_latin1
    };

    encoding guess_buffer_encoding(char d0, char d1, char d2, char d3)
    {
        // look for BOM in first few bytes
        if (d0 == 0 && d1 == 0 && d2 == 0xfe && d3 == 0xff) return encoding_utf32_be;
        if (d0 == 0xff && d1 == 0xfe && d2 == 0 && d3 == 0) return encoding_utf32_le;
        if (d0 == 0xfe && d1 == 0xff) return encoding_utf16_be;
        if (d0 == 0xff && d1 == 0xfe) return encoding_utf16_le;
        if (d0 == 0xef && d1 == 0xbb && d2 == 0xbf) return encoding_utf8;

        // no known BOM detected, assume ans
        return encoding_latin1;
    }
    
    enum remstate
    {
        none,
        blank,
        normal,
        singlerem,
        multirem1,
        multirem2,
        multirem21,
    };

    remstate HandlerLine(const SStringW & str,const CCodeConfig & config,remstate curState)
    {
        remstate st = none;
        if(curState == multirem1)
        {//look for multirem2
            int nPos =str.Find(config.strMultiLinesRemarkEnd);
            if(nPos != -1)
            {
                remstate st2 = HandlerLine(str.Right(str.GetLength()-nPos-config.strMultiLinesRemarkEnd.GetLength()),config,normal);
                if(st2 == multirem1)
                    st = multirem21;
                else
                    st = multirem2;
            }else
            {
                SStringW str2 = str;
                str2.TrimBlank();
                if(str2.IsEmpty())
                    st = blank;
            }
        }else
        {//look for singlerem or multirem1
            SStringW str2 = str;
            str2.TrimBlank();
            if(!config.strSingleLineRemark.IsEmpty() && str2.Left(config.strSingleLineRemark.GetLength()) == config.strSingleLineRemark)
            {//single remark
                st = singlerem;
            }else if(!config.strMultiLinesRemarkBegin.IsEmpty() && str2.Left(config.strMultiLinesRemarkBegin.GetLength()) == config.strMultiLinesRemarkBegin)
            {//multi rem begin
                remstate st2 = HandlerLine(str.Right(str.GetLength()-config.strMultiLinesRemarkBegin.GetLength()),config,multirem1);
                if(st2 != multirem2)
                    st = multirem1;
                else// find multi rem end in the same line, treat it as normal
                    st = normal;
            }
        }
        return st;
    }

    BOOL CountCodeLines( LPCTSTR pszFileName, const CCodeConfig & config,int & nCodeLines,int & nRemarkLines,int & nBlankLines )
    {
        FILE *f = _tfopen(pszFileName,_T("rb"));
        if(!f) return FALSE;
        char bom[4]={0};
        fread(bom,1,4,f);
        encoding enc = guess_buffer_encoding(bom[0],bom[1],bom[2],bom[3]);

        BOOL canHanle = FALSE;;
        if(enc == encoding_utf16_le)
        {
            canHanle = TRUE;
            fseek(f,-2,SEEK_CUR);
        }else if(enc == encoding_utf8)
        {
            canHanle = TRUE;
            fseek(f,-1,SEEK_CUR);
        }else if(enc == encoding_latin1)
        {
            canHanle = TRUE;
        }
        
        if(canHanle)
        {
            nCodeLines = 0;
            nBlankLines = 0;
            nRemarkLines = 0;
            
            remstate stCur = none;

            for(;;)
            {
                wchar_t szLine[1024];
                if(enc == encoding_utf16_le)
                {
                    if(!fgetws(szLine,1024,f)) 
                        break;
                }else
                {
                    char szLine2[1024];
                    if(!fgets(szLine2,1024,f)) 
                        break;
                    MultiByteToWideChar(enc == encoding_utf8? CP_UTF8:CP_ACP,0,szLine2,-1,szLine,1024);
                }
                SStringW strLine(szLine);
                strLine.TrimRight('\n');//去掉行尾的换行符
                
                remstate st = HandlerLine(strLine,config,stCur);
                if(stCur == multirem1)
                {
                    if(st != blank)
                        nRemarkLines ++;
                    else
                        nBlankLines ++;

                    if(st == multirem2)
                        stCur = none;
                    else if(st == multirem21)
                        stCur = multirem1;
                }else if(st == singlerem)
                {
                    nRemarkLines ++;
                }else if(st == blank)
                {
                    nBlankLines ++;
                }else if(st == multirem1)
                {
                    nRemarkLines ++;
                    stCur = st;
                }else
                {
                    nCodeLines ++;
                }
            }
        }
        fclose(f);
        return canHanle;
    }
}
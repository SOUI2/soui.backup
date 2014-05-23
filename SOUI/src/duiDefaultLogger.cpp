/***********************************************************************
    filename:   duiDefaultLogger.cpp
    created:    25/1/2006
    author:     Andrew Zabolotny

    purpose:    Implementation of the DefaultLogger class
*************************************************************************/
/***************************************************************************
 *   Copyright (C) 2004 - 2006 Paul D Turner & The SOUI Development Team
 *
 *   Permission is hereby granted, free of charge, to any person obtaining
 *   a copy of this software and associated documentation files (the
 *   "Software"), to deal in the Software without restriction, including
 *   without limitation the rights to use, copy, modify, merge, publish,
 *   distribute, sublicense, and/or sell copies of the Software, and to
 *   permit persons to whom the Software is furnished to do so, subject to
 *   the following conditions:
 *
 *   The above copyright notice and this permission notice shall be
 *   included in all copies or substantial portions of the Software.
 *
 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 *   IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 *   OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 *   ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 *   OTHER DEALINGS IN THE SOFTWARE.
 ***************************************************************************/
#include "duistd.h"
#include "DuiDefaultLogger.h"
#include <ctime>
#include <iomanip>
#include <stdio.h>
#include <tchar.h>


// Start of SOUI namespace section
namespace SOUI
{

/*************************************************************************
    Constructor
*************************************************************************/
DefaultLogger::DefaultLogger(void) :
    d_caching(true),d_pFile(NULL)
{
    // create log header
    logEvent(_T("+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+"));
    logEvent(_T("+                     SOUI - Event log                                   +"));
    logEvent(_T("+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+\n"));
    logEvent2(_T("SOUI::Logger singleton created: (%p)"),static_cast<void*>(this));
}

/*************************************************************************
    Destructor
*************************************************************************/
DefaultLogger::~DefaultLogger(void)
{
    if (d_pFile)
    {
        FlushCaches();
        logEvent2(_T("SOUI::Logger singleton destroyed: (%p)"),static_cast<void*>(this));
        fclose(d_pFile);
    }
}

/*************************************************************************
    Logs an event
*************************************************************************/
void DefaultLogger::logEvent(LPCTSTR message, LoggingLevel level /* = Standard */)
{
    time_t  et;
    time(&et);
    tm* etm = localtime(&et);

    if (etm)
    {
        // clear sting stream
        CDuiStringT strbuf;

        strbuf.Format(_T("%04d/%02d/%02d %02d:%02d:%02d "),1900+etm->tm_year,etm->tm_mon+1,etm->tm_mday,etm->tm_hour,etm->tm_min,etm->tm_sec);
        // write event type code
        switch(level)
        {
        case Errors:
            strbuf+= _T("(Error)\t");
            break;

        case Warnings:
            strbuf+= _T("(Warn)\t");
            break;

        case Standard:
            strbuf+= _T("(Std) \t");
            break;

        case Informative:
            strbuf+= _T("(Info) \t");
            break;

        case Insane:
            strbuf+= _T("(Insan)\t");
            break;

        default:
            strbuf+= _T("(Unkwn)\t");
            break;
        }

        strbuf+= message;
        strbuf+=_T("\n");

        if (d_caching)
        {
            d_cache.Add(LOGRECORD(strbuf, level));
        }
        else if (d_level >= level)
        {
            if(d_pFile)
            {
                // write message
                fprintf(d_pFile,(LPCSTR)DUI_CT2A(strbuf));
                fflush(d_pFile);
            }
        }
    }
}


void DefaultLogger::setLogFilename(LPCTSTR filename, bool append)
{
    if(d_pFile)
    {
        fclose(d_pFile);
        d_pFile=NULL;
    }
    d_pFile=_tfopen(filename,append?_T("a+"):_T("w"));
    FlushCaches();
}

void DefaultLogger::FlushCaches()
{
    if(!d_pFile) return;
    // write out cached log strings.
    if (d_caching)
    {
        d_caching = false;

        for(UINT i=0; i<d_cache.GetCount(); i++)
        {
            if (d_level >= d_cache[i].level)
            {
                fprintf(d_pFile,(LPCSTR)DUI_CT2A(d_cache[i].msg));
                fflush(d_pFile);
            }
        }

        d_cache.RemoveAll();
    }
}

} // End of  SOUI namespace section

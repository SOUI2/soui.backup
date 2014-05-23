/***********************************************************************
    filename:     DUIDefaultLogger.h
    created:    25/1/2006
    author:        Andrew Zabolotny

    purpose:    Defines interface for the default Logger implementation
*************************************************************************/
/***************************************************************************
 *   Copyright (C) 2004 - 2006 Paul D Turner & The DUI Development Team
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
#ifndef _DUIDefaultLogger_h_
#define _DUIDefaultLogger_h_

#include "DuiLogger.h"


// Start of DUI namespace section
namespace SOUI
{

/*!
\brief
    Default implementation for the Logger class.
    If you want to redirect DUI logs to some place other than a text file,
    implement your own Logger implementation and create a object of the
    Logger type before creating the DUI::System singleton.
*/
class SOUI_EXP DefaultLogger : public DuiLogger
{
public:
    /*!
    \brief
        Constructor for DefaultLogger object.
    */
    DefaultLogger(void);

    /*!
    \brief Destructor for DefaultLogger object.
    */
    virtual ~DefaultLogger(void);


    /*!
    \brief
        Add an event to the log.

    \param message
        String object containing the message to be added to the event log.

    \param level
        LoggingLevel for this message.  If \a level is greater than the current set logging level, the message is not logged.

    \return
        Nothing
    */
    virtual void logEvent(LPCTSTR message, LoggingLevel level = Standard);


    /*!
    \brief
        Set the name of the log file where all subsequent log entries should be written.

    \note
        When this is called, and the log file is created, any cached log entries are
        flushed to the log file.

    \param filename
        Name of the file to put log messages.

    \param append
        - true if events should be added to the end of the current file.
        - false if the current contents of the file should be discarded.
     */
    virtual void setLogFilename(LPCTSTR filename, bool append = false);

protected:

    void FlushCaches();

    /*************************************************************************
        Implementation Data
    *************************************************************************/
    FILE * d_pFile;
    struct LOGRECORD
    {
        LOGRECORD() {}
        LOGRECORD(const CDuiStringT & _msg,LoggingLevel _level)
        {
            msg=_msg;
            level=_level;
        }
        CDuiStringT msg;
        LoggingLevel level;
    };
    CDuiArray<LOGRECORD> d_cache; //!< Used to cache log entries before log file is created.
    bool d_caching;                 //!< true while log entries are beign cached (prior to logfile creation)
};


} // End of  SOUI namespace section


#endif    // end of guard _DUIDefaultLogger_h_

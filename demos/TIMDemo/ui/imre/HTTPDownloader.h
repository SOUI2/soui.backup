#pragma once

#ifndef _WININET_
#include <wininet.h>
#endif
#pragma comment(lib, "wininet.lib")

class HTTPDownloader
{
public:
    HTTPDownloader();
    ~HTTPDownloader();

    int OpenURL(LPCTSTR pstrURL,
        DWORD_PTR dwContext = 1,
        DWORD dwFlags = INTERNET_FLAG_TRANSFER_ASCII,
        LPCTSTR pstrHeaders = NULL,
        DWORD dwHeadersLength = 0);

    int  DownLoadFile(LPCWSTR pDownLoadTo);

    BOOL QueryStatusCode(DWORD& dwStatusCode);

protected:

    BOOL OpenSession(LPCTSTR pstrAgent = NULL,
        DWORD_PTR dwContext = 1,
        DWORD dwAccessType = LOCAL_INTERNET_ACCESS,
        LPCTSTR pstrProxyName = NULL,
        LPCTSTR pstrProxyBypass = NULL,
        DWORD dwFlags = 0);

    BOOL ParseURL(LPCTSTR pstrURL,
        DWORD& dwServiceType,
        SOUI::SStringW& strServer,
        SOUI::SStringW& strObject,
        INTERNET_PORT& nPort);

    static BOOL ParseURLWorker(LPCTSTR pstrURL,
        LPURL_COMPONENTS lpComponents,
        DWORD& dwServiceType,
        INTERNET_PORT& nPort,
        DWORD dwFlags);

    static DWORD GetInternetHandleType(HINTERNET hQuery);

private:
    HINTERNET m_hOpener;
    HINTERNET m_hSession;
};

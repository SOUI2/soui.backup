#include "stdafx.h"
#include "souistd.h"
#include "HTTPDownloader.h"

using namespace SOUI;
HTTPDownloader::HTTPDownloader()
{
    m_hSession = NULL;
    m_hOpener = NULL;
    OpenSession();
}

HTTPDownloader::~HTTPDownloader()
{
    if (m_hOpener != NULL)
    {
        InternetCloseHandle(m_hOpener);
        m_hOpener = NULL;
    }

    if (m_hSession != NULL)
    {
        InternetCloseHandle(m_hSession);
        m_hSession = NULL;
    }
}

BOOL HTTPDownloader::OpenSession(LPCTSTR pstrAgent,
    DWORD_PTR dwContext,
    DWORD dwAccessType,
    LPCTSTR pstrProxyName,
    LPCTSTR pstrProxyBypass,
    DWORD dwFlags)
{
    if (m_hSession == NULL)
    {
        m_hSession = InternetOpen(pstrAgent,
            dwAccessType,
            pstrProxyName,
            pstrProxyBypass,
            dwFlags);
    }

    return (m_hSession != NULL);
}

BOOL HTTPDownloader::ParseURLWorker(LPCTSTR pstrURL,
    LPURL_COMPONENTS lpComponents,
    DWORD& dwServiceType,
    INTERNET_PORT& nPort,
    DWORD dwFlags)
{
    // this function will return bogus stuff if lpComponents
    // isn't set up to copy the components

    assert(lpComponents != NULL && pstrURL != NULL);
    if (lpComponents == NULL || pstrURL == NULL)
        return FALSE;
    assert(lpComponents->dwHostNameLength == 0 ||
        lpComponents->lpszHostName != NULL);
    assert(lpComponents->dwUrlPathLength == 0 ||
        lpComponents->lpszUrlPath != NULL);
    assert(lpComponents->dwUserNameLength == 0 ||
        lpComponents->lpszUserName != NULL);
    assert(lpComponents->dwPasswordLength == 0 ||
        lpComponents->lpszPassword != NULL);

    LPTSTR pstrCanonicalizedURL;
    TCHAR szCanonicalizedURL[INTERNET_MAX_URL_LENGTH];
    DWORD dwNeededLength = INTERNET_MAX_URL_LENGTH;
    BOOL bRetVal;
    BOOL bMustFree = FALSE;

    // Decoding is done in InternetCrackUrl/UrlUnescape 
    // so we don't need the ICU_DECODE flag here.

    DWORD dwCanonicalizeFlags = dwFlags &
        (ICU_NO_ENCODE | ICU_NO_META |
            ICU_ENCODE_SPACES_ONLY | ICU_BROWSER_MODE);

    DWORD dwCrackFlags = 0;

    BOOL bUnescape = FALSE;

    if ((dwFlags & (ICU_ESCAPE | ICU_DECODE)) && (lpComponents->dwUrlPathLength != 0))
    {
        // We use only the ICU_ESCAPE flag for decoding even if
        // ICU_DECODE is passed.

        // Also, if ICU_BROWSER_MODE is passed we do the unescaping
        // manually because InternetCrackUrl doesn't do
        // Browser mode unescaping

        if (dwFlags & ICU_BROWSER_MODE)
            bUnescape = TRUE;
        else
            dwCrackFlags |= ICU_ESCAPE;
    }

    bRetVal = InternetCanonicalizeUrl(pstrURL, szCanonicalizedURL,
        &dwNeededLength, dwCanonicalizeFlags);

    if (!bRetVal)
    {
        if (::GetLastError() != ERROR_INSUFFICIENT_BUFFER)
            return FALSE;

        pstrCanonicalizedURL = new TCHAR[dwNeededLength];
        if (pstrCanonicalizedURL == NULL)
            return FALSE;

        bMustFree = TRUE;
        bRetVal = InternetCanonicalizeUrl(pstrURL, pstrCanonicalizedURL,
            &dwNeededLength, dwCanonicalizeFlags);
        if (!bRetVal)
        {
            delete[] pstrCanonicalizedURL;
            return FALSE;
        }
    }
    else
        pstrCanonicalizedURL = szCanonicalizedURL;

    // now that it's safely canonicalized, crack it

    bRetVal = InternetCrackUrl(pstrCanonicalizedURL, 0,
        dwCrackFlags, lpComponents);

    if (bUnescape)
    {
        if (FAILED(UrlUnescape(lpComponents->lpszUrlPath,
            NULL,
            NULL,
            URL_UNESCAPE_INPLACE | URL_DONT_UNESCAPE_EXTRA_INFO)))
        {
            if (bMustFree)
                delete[] pstrCanonicalizedURL;

            return FALSE;
        }

        lpComponents->dwUrlPathLength = lstrlen(lpComponents->lpszUrlPath);
    }

    if (bMustFree)
        delete[] pstrCanonicalizedURL;

    if (!bRetVal)
        dwServiceType = INTERNET_SCHEME_UNKNOWN;
    else
    {
        nPort = lpComponents->nPort;
        dwServiceType = lpComponents->nScheme;
    }

    return bRetVal;
}

BOOL HTTPDownloader::ParseURL(LPCTSTR pstrURL,
    DWORD& dwServiceType,
    SOUI::SStringW& strServer,
    SOUI::SStringW& strObject,
    INTERNET_PORT& nPort)
{
    dwServiceType = INTERNET_SCHEME_UNKNOWN;

    assert(pstrURL != NULL);
    if (pstrURL == NULL)
        return FALSE;

    URL_COMPONENTS urlComponents;
    memset(&urlComponents, 0, sizeof(URL_COMPONENTS));
    urlComponents.dwStructSize = sizeof(URL_COMPONENTS);
    urlComponents.dwHostNameLength = INTERNET_MAX_URL_LENGTH;
    urlComponents.lpszHostName = strServer.GetBuffer(INTERNET_MAX_URL_LENGTH + 1);
    urlComponents.dwUrlPathLength = INTERNET_MAX_URL_LENGTH;
    urlComponents.lpszUrlPath = strObject.GetBuffer(INTERNET_MAX_URL_LENGTH + 1);

    BOOL bRetVal = ParseURLWorker(pstrURL,
        &urlComponents,
        dwServiceType,
        nPort,
        ICU_BROWSER_MODE);

    strServer.ReleaseBuffer();
    strObject.ReleaseBuffer();
    return bRetVal;
}

DWORD HTTPDownloader::GetInternetHandleType(HINTERNET hQuery)
{
    DWORD dwServiceType;
    DWORD dwTypeLen = sizeof(dwServiceType);

    if (hQuery == NULL ||
        !InternetQueryOption(hQuery,
            INTERNET_OPTION_HANDLE_TYPE,
            &dwServiceType, &dwTypeLen))
    {
        return INTERNET_SCHEME_UNKNOWN;
    }

    return dwServiceType;
}

int HTTPDownloader::OpenURL(LPCTSTR   pstrURL,
    DWORD_PTR dwContext       /* = 0 */,
    DWORD     dwFlags         /* = INTERNET_FLAG_TRANSFER_BINARY */,
    LPCTSTR   pstrHeaders     /* = NULL */,
    DWORD     dwHeadersLength /* = 0 */)
{
    DWORD dwServiceType;
    SStringW strServer;
    SStringW strObject;
    INTERNET_PORT nPort;

    BOOL bParsed = ParseURL(pstrURL, dwServiceType, strServer, strObject, nPort);

    //bool bRet = false;
    int nRet = -1;
    // if it turns out to be a local file...
    if (bParsed && dwServiceType == INTERNET_SCHEME_FILE)
    {
        // treated local file as error
        return -1;

        //ENSURE( INTERNET_MAX_URL_LENGTH >= strObject.GetLength() );

        //DWORD    dwUnescapedUrlLen = INTERNET_MAX_URL_LENGTH+1;
        //SStringW strUnescapedUrl;
        //LPTSTR   pstrUnescapedUrl = strUnescapedUrl.GetBuffer(INTERNET_MAX_URL_LENGTH+1);

        //HRESULT hr = UrlUnescape((LPTSTR)((LPCTSTR)strObject),
        //    pstrUnescapedUrl,
        //    &dwUnescapedUrlLen,
        //    URL_DONT_UNESCAPE_EXTRA_INFO);

        //strUnescapedUrl.ReleaseBuffer();

        //if (FAILED(hr))
        //    return false;


        //int nMode = CFile::modeRead | CFile::shareCompat;
        //if (dwFlags & INTERNET_FLAG_TRANSFER_BINARY)
        //    nMode |= CFile::typeBinary;
        //else
        //    nMode |= CFile::typeText;

        //pResult = new CStdioFile(strUnescapedUrl, nMode);
        //do 
    }
    else
    {
        m_hOpener = InternetOpenUrl(m_hSession, pstrURL, pstrHeaders,
            dwHeadersLength, dwFlags, dwContext);

        if (m_hOpener == NULL)
            return -2;

        if (!bParsed)
            dwServiceType = GetInternetHandleType(m_hOpener);

        switch (dwServiceType)
        {
        case INTERNET_HANDLE_TYPE_HTTP_REQUEST:
        case INTERNET_SCHEME_HTTP:
        case INTERNET_SCHEME_HTTPS:
            nRet = 0;
            //pResult = new CHttpFile(hOpener, m_hSession, strObject, strServer,
            //    CHttpConnection::szHtmlVerbs[CHttpConnection::HTTP_VERB_GET],
            //    dwContext);
            //_afxSessionMap.SetAt(hOpener, this);
            break;

        default:
            //TRACE(traceInternet, 0, "Error: Unidentified service type: %8.8X\n", dwServiceType);
            //pResult = NULL;
            break;
        }
    }

    return nRet;
}

BOOL HTTPDownloader::QueryStatusCode(DWORD& dwStatusCode)
{
    assert(m_hOpener != NULL);

    TCHAR szBuffer[80];
    DWORD dwLen = _countof(szBuffer);
    BOOL bRet;

    bRet = HttpQueryInfo(m_hOpener,
        HTTP_QUERY_STATUS_CODE,
        szBuffer,
        &dwLen,
        NULL);

    if (bRet)
        dwStatusCode = (DWORD)_ttol(szBuffer);
    return bRet;
}

int  HTTPDownloader::DownLoadFile(LPCWSTR pDownLoadTo)
{
    HANDLE hFile = ::CreateFile(pDownLoadTo,
        GENERIC_WRITE,
        FILE_SHARE_READ,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
        return -1;
    }

    int    nRet = 0;
    char   buff[8192];
    DWORD  dwBytesReaded = 0;
    while (true)
    {
        dwBytesReaded = 0;
        if (!InternetReadFile(m_hOpener, (LPVOID)buff, sizeof(buff), &dwBytesReaded))
        {
            nRet = -2;
            break;
        }

        if (dwBytesReaded == 0)
        {
            // end of file, no need to set nRet
            break;
        }

        DWORD dwBytesWrited = 0;
        if (!::WriteFile(hFile, buff, dwBytesReaded, &dwBytesWrited, NULL))
        {
            nRet = -3;
            break;
        }
    }

    CloseHandle(hFile);
    return nRet;
}

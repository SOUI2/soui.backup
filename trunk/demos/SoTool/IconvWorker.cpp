#include "IconvWorker.h"

#include "FileMap.h"

#include <cassert>
#include <Shlwapi.h>

BYTE g_byUtf8BOM[] = {0xEF, 0xBB, 0xBF};
BYTE g_byUnicodeBOM[] = {0xFF, 0xFE};


BOOL WriteFileHelper(HANDLE hFile, LPCVOID pBuffer, int nSize)
{
    DWORD dwWritten = 0;
    BOOL bResult = ::WriteFile(hFile, pBuffer, nSize, &dwWritten, NULL);
    if(!bResult)
        bResult = bResult;
    bResult = bResult && (dwWritten == (unsigned int)nSize);
    if(!bResult)
        bResult = bResult;
    return bResult;
}

CIconvWorker::CIconvWorker(void)
{
    m_arrFiles = NULL;
    m_bStop = FALSE;
    m_strTargetPath = _T("");
    m_bOverwrite = FALSE;
    m_bWriteBom = TRUE;
    m_nSrcCodepage = CodeAuto;
    m_nDstCodepage = CodeAuto;
    m_pBuffer = NULL;
    m_nBufferSize = 0;
}

CIconvWorker::~CIconvWorker(void)
{
    ReleaseBuffer();
}

void CIconvWorker::SetCodepage(CodePageValue nSrcCodepage, CodePageValue nDstCodepage)
{
    m_nSrcCodepage = nSrcCodepage;
    m_nDstCodepage = nDstCodepage;
}

void CIconvWorker::SetFiles(std::vector<std::wstring>* arrFiles)
{
    m_arrFiles = arrFiles;
}

void CIconvWorker::SetOverwrite(BOOL bOverwrite)
{
    m_bOverwrite = bOverwrite;
}

void CIconvWorker::SetWriteBom(BOOL bWriteBom)
{
    m_bWriteBom = bWriteBom;
}


static void replace_all(std::wstring &str, const std::wstring &old_value, const std::wstring &new_value)
{
	while (true)
	{
		std::wstring::size_type   pos(0);
		if ((pos = str.find(old_value)) != std::wstring::npos)
			str.replace(pos, old_value.length(), new_value);
		else   break;
	}
}


void CIconvWorker::SetTargetPath(LPCTSTR szTargetPath)
{
    m_strTargetPath = szTargetPath;
	replace_all(m_strTargetPath,L"/", L"\\");

    if(m_strTargetPath.length() > 0
		&& m_strTargetPath[m_strTargetPath.length() - 1] != _T('\\'))
    {
        m_strTargetPath += _T('\\');
    }
}

int CIconvWorker::ConvertFile(const std::wstring &strSrcTemp,const std::wstring &sSrcFile)
{  
	std::wstring strSrc = sSrcFile;

	// Get File Data
	CFileMap m;
	if (!m.MapFile(strSrc.c_str()))
	{
		return 0;
	}

	const BYTE * pData = static_cast<const BYTE *>(m.GetData());
	int nSize = m.GetSize();

	CodePageValue nSrcCodepage = m_nSrcCodepage;
	if (nSrcCodepage == CodeAuto)
		nSrcCodepage = GetFileCodepage(pData, nSize);
	else
		GetFileCodepage(pData, nSize);

	std::wstring strDstPath;
	CodePageValue nDstCodepage = m_nDstCodepage;
	if (nSrcCodepage == nDstCodepage)
	{
		m.Close();
		return 0;
	}
		  
	int bResult = -1;
	 
	if ((nSrcCodepage == CodeUnicode && nDstCodepage != CodeUnicode)
		|| (nSrcCodepage != CodeUnicode && nDstCodepage == CodeUnicode))
	{
		if (!ConvFile(pData, nSize, strSrcTemp.c_str(), nSrcCodepage, nDstCodepage))
			return bResult;

		// Copy to dst
		strDstPath = GetDstPath(strSrc.c_str());

		m.Close();
		if (!CopyFile(strSrcTemp.c_str(), strDstPath.c_str(), FALSE))
			return bResult;

		bResult = 1;
	}
	else
	{
		// nSrcCodepage != CodeUnicode && nDstCodepage != CodeUnicode
		// Convert to unicode first
		if (!ConvFile(pData, nSize, strSrcTemp.c_str(), nSrcCodepage, CodeUnicode))
			return bResult;

		m.Close();

		// Convert to dst codepage
		std::wstring strDstTemp = GetTempFilePath();
		CFileMap mTemp;
		if (!mTemp.MapFile(strSrcTemp.c_str()))
			return bResult;

		pData = static_cast<const BYTE *>(mTemp.GetData());
		nSize = mTemp.GetSize();
		GetFileCodepage(pData, nSize);
		if (!ConvFile(pData, nSize, strDstTemp.c_str(), CodeUnicode, nDstCodepage))
			return bResult;

		strDstPath = GetDstPath(strSrc.c_str());
		mTemp.Close();
		if (!CopyFile(strDstTemp.c_str(), strDstPath.c_str(), FALSE))
			return bResult;

		bResult = 1;
	} 

	return bResult; 

}

BOOL CIconvWorker::Convert(std::vector<std::wstring>* failedFiles, std::vector<std::wstring>* outFiles)
{
    m_bStop = FALSE;

    if(failedFiles)
        failedFiles->clear();
    if(outFiles)
		outFiles->clear();

	std::wstring strSrcTemp = GetTempFilePath();
    int nCount = m_arrFiles->size();
    for(int i=0; !m_bStop && i<nCount; ++ i)
    {
        std::wstring strSrc = (*m_arrFiles)[i];

        // Get File Data
        CFileMap m;
        if(!m.MapFile(strSrc.c_str()))
        {
            if(failedFiles)
                failedFiles->push_back(strSrc);
            continue;
        }

        const BYTE * pData = static_cast<const BYTE *>(m.GetData());
        int nSize = m.GetSize();

        CodePageValue nSrcCodepage = m_nSrcCodepage;
        if(nSrcCodepage == CodeAuto)
            nSrcCodepage = GetFileCodepage(pData, nSize);
        else
            GetFileCodepage(pData, nSize);

        std::wstring strDstPath;
        CodePageValue nDstCodepage = m_nDstCodepage;
        if(nSrcCodepage == nDstCodepage)
        {
            m.Close();
            if(!m_bOverwrite)
            {
                // Copy to dst
                strDstPath = GetDstPath(strSrc.c_str());
				if (CopyFile(strSrc.c_str(), strDstPath.c_str(), FALSE))
                {
                    if(outFiles)
                        outFiles->push_back(strSrc);
                }
                else
                {
                    if(failedFiles)
						failedFiles->push_back(strSrc);
                }
            }
            continue;
        }

        for(;;)
        {
            BOOL bResult = FALSE;
            if((nSrcCodepage == CodeUnicode && nDstCodepage != CodeUnicode)
                || (nSrcCodepage != CodeUnicode && nDstCodepage == CodeUnicode))
            {
                if(!ConvFile(pData, nSize, strSrcTemp.c_str(), nSrcCodepage, nDstCodepage))
                    break;

                // Copy to dst
				strDstPath = GetDstPath(strSrc.c_str());

                m.Close();
				if (!CopyFile(strSrcTemp.c_str(), strDstPath.c_str(), FALSE))
                    break;
                if(outFiles)
                    outFiles->push_back(strDstPath);
                bResult = TRUE;
            }
            else
            {
                // nSrcCodepage != CodeUnicode && nDstCodepage != CodeUnicode
                // Convert to unicode first
				if (!ConvFile(pData, nSize, strSrcTemp.c_str(), nSrcCodepage, CodeUnicode))
                    break;

                m.Close();

                // Convert to dst codepage
				std::wstring strDstTemp = GetTempFilePath();
                CFileMap mTemp;
				if (!mTemp.MapFile(strSrcTemp.c_str()))
                    break;

                pData = static_cast<const BYTE *>(mTemp.GetData());
                nSize = mTemp.GetSize();
                GetFileCodepage(pData, nSize);
				if (!ConvFile(pData, nSize, strDstTemp.c_str(), CodeUnicode, nDstCodepage))
                    break;

				strDstPath = GetDstPath(strSrc.c_str());
                mTemp.Close();
				if (!CopyFile(strDstTemp.c_str(), strDstPath.c_str(), FALSE))
                    break;
                if(outFiles)
                    outFiles->push_back(strDstPath);
                bResult = TRUE;
            }

            if(!bResult)
                failedFiles->push_back(strSrc);

            break;
        }
    }

    return (failedFiles->size() == 0);
}

void CIconvWorker::Stop()
{
    m_bStop = TRUE;
}

std::wstring CIconvWorker::GetDstPath(LPCTSTR szSrcPath)
{
    if(m_bOverwrite)
        return szSrcPath;

    LPCTSTR szFileName = ::PathFindFileName(szSrcPath);
    std::wstring strResult = m_strTargetPath + szFileName;
    return strResult;
}

CodePageValue CIconvWorker::GetFileCodepage(const BYTE *& pData, int& nSize)
{
    if(nSize >= 3 && memcmp(g_byUtf8BOM, pData, 3) == 0)
    {
        pData += 3;
        nSize -= 3;
        return CodeUtf8;
    }
    else if(nSize >= 2 && memcmp(g_byUnicodeBOM, pData, 2) == 0)
    {
        pData += 2;
        nSize -= 2;
        return CodeUnicode;
    }
    else
    {
        return CodeAnsi;
    }
}

std::wstring CIconvWorker::GetTempFilePath()
{
    TCHAR szTmpPath[MAX_PATH];
    TCHAR szTmpFile[MAX_PATH];
    ::GetTempPath(MAX_PATH, szTmpPath);
    ::GetTempFileName(szTmpPath, _T("iconv"), rand(), szTmpFile);
    return szTmpFile;
}

BOOL CIconvWorker::ConvFile(const BYTE * pData, int nSize, LPCTSTR szDstPath, CodePageValue nSrcCodepage, CodePageValue nDstCodepage)
{
    BOOL bMultiByteToWideChar = FALSE;
    DWORD dwCodepage = GetRealCodepage(nSrcCodepage, nDstCodepage, bMultiByteToWideChar);
    if(dwCodepage == -1)
        return FALSE;

    LPBYTE pBuffer = NULL;
    int nBufferSize = 0;
    if(bMultiByteToWideChar)
    {
        nBufferSize = ::MultiByteToWideChar(dwCodepage,
            0,
            (LPCSTR)pData,
            nSize,
            NULL,
            0);
        if(nBufferSize <= 0)
            return (nSize == 0);

        pBuffer = GetBuffer(nBufferSize * 2 + 2);
        if(pBuffer == NULL)
            return FALSE;

        nBufferSize = ::MultiByteToWideChar(dwCodepage,
            0,
            (LPCSTR)pData,
            nSize,
            (LPWSTR)pBuffer,
            nBufferSize);
        if(nBufferSize == 0)
        {
            return FALSE;
        }
        nBufferSize = nBufferSize * 2;
    }
    else
    {
        nBufferSize = ::WideCharToMultiByte(dwCodepage,
            0,
            (LPCTSTR)pData,
            nSize / 2,
            0,
            0,
            0,
            0);
        if(nBufferSize == 0)
            return (nSize == 0);

        pBuffer = GetBuffer(nBufferSize + 1);
        if(::WideCharToMultiByte(dwCodepage,
            0,
            (LPCTSTR)pData,
            nSize / 2,
            (LPSTR)pBuffer,
            nBufferSize,
            0,
            0) == 0)
        {
            return FALSE;
        }
    }

    HANDLE hFile = ::CreateFile(szDstPath, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
    if(hFile == INVALID_HANDLE_VALUE)
        return FALSE;

    BOOL bResult = TRUE;

    // Write BOM

    if(m_bWriteBom)
    {
        if(nDstCodepage == CodeUnicode)
        {
            bResult = WriteFileHelper(hFile, g_byUnicodeBOM, sizeof(g_byUnicodeBOM));
        }
        else if(nDstCodepage == CodeUtf8)
        {
            bResult = WriteFileHelper(hFile, g_byUtf8BOM, sizeof(g_byUtf8BOM));
        }
    }

    if(bResult)
    {
        // Write Content
        SetLastError(0);
        bResult = WriteFileHelper(hFile, pBuffer, nBufferSize);
        if(!bResult)
        {
            bResult = bResult;
        }
    }
    else
    {
        bResult = bResult;
    }

    ::CloseHandle(hFile);

    return bResult;
}

DWORD CIconvWorker::GetRealCodepage(CodePageValue nSrcCodepage, CodePageValue nDstCodepage, BOOL& bMultiByteToWideChar)
{
	DWORD dwCodepage = (DWORD)-1;
    assert(nSrcCodepage == CodeUnicode || nDstCodepage == CodeUnicode);
    bMultiByteToWideChar = (nDstCodepage == CodeUnicode);

    struct
    {
        CodePageValue codepage;
        DWORD dwCodepage;
    } data[] =
    {
        {CodeAnsi,      CP_ACP},
        {CodeUnicode,   CP_ACP},
        {CodeUtf8,      CP_UTF8},
        {CodeChinese,   936},
    };

    CodePageValue codepage = (nSrcCodepage == CodeUnicode) ? nDstCodepage : nSrcCodepage;
    for(int i=0; i<_countof(data); ++ i)
    {
        if(codepage == data[i].codepage)
            dwCodepage = data[i].dwCodepage;
    }
    return dwCodepage;
}

LPBYTE CIconvWorker::GetBuffer(int nSize)
{
    if(nSize <= m_nBufferSize)
        return m_pBuffer;
    ReleaseBuffer();

    m_pBuffer = static_cast<LPBYTE>(malloc(nSize));
    if(m_pBuffer)
        m_nBufferSize = nSize;
    return m_pBuffer;
}

void CIconvWorker::ReleaseBuffer()
{
    if(m_pBuffer)
    {
        free(m_pBuffer);
        m_pBuffer = NULL;
        m_nBufferSize = 0;
    }
}

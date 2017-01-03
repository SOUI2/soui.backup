#include "stdafx.h"
#include "FileMap.h"

CFileMap::CFileMap(void)
{
    m_hFile = INVALID_HANDLE_VALUE;
    m_hMap = NULL;
    m_pData = 0;
    m_dwSize = 0;
}

CFileMap::~CFileMap(void)
{
    Close();
}

BOOL CFileMap::MapFile(LPCTSTR szFilePath)
{
    Close();

    m_hFile = ::CreateFile(szFilePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
    if(m_hFile == INVALID_HANDLE_VALUE)
        return FALSE;
    m_dwSize = ::GetFileSize(m_hFile, NULL);

    m_hMap = ::CreateFileMapping(m_hFile, NULL, PAGE_READONLY, 0, m_dwSize, NULL);
    if(m_hMap == NULL)
        return FALSE;
    m_pData = ::MapViewOfFile(m_hMap, FILE_MAP_READ, 0, 0, m_dwSize);
    return (m_pData != NULL);
}

void CFileMap::Close()
{
    if(m_pData)
    {
        ::UnmapViewOfFile(m_pData);
        m_pData = 0;
    }
    if(m_hMap != NULL)
    {
        ::CloseHandle(m_hMap);
        m_hMap = NULL;
    }
    if(m_hFile != INVALID_HANDLE_VALUE)
    {
        CloseHandle(m_hFile);
        m_hFile = INVALID_HANDLE_VALUE;
        m_dwSize = 0;
    }
}

DWORD CFileMap::GetSize() const
{
    return m_dwSize;
}

LPCVOID CFileMap::GetData() const
{
    return m_pData;
}

#pragma once

class CFileMap
{
public:
    CFileMap(void);
    ~CFileMap(void);

    BOOL MapFile(LPCTSTR szFilePath);
    void Close();

    DWORD   GetSize() const;
    LPCVOID GetData() const;

private:
    HANDLE  m_hFile;
    HANDLE  m_hMap;
    LPCVOID m_pData;
    DWORD   m_dwSize;
};

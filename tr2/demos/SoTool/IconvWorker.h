#pragma once
#include <vector>
#include <string>

enum CodePageValue
{
    CodeAuto,
    CodeAnsi,
    CodeUnicode,
    CodeUtf8,
    CodeChinese,
};

class CIconvWorker 
{
public:
    CIconvWorker(void);
    ~CIconvWorker(void); 

    void SetCodepage(CodePageValue nSrcCodepage, CodePageValue nDstCodepage);
	void SetFiles(std::vector<std::wstring>* arrFiles);
    void SetOverwrite(BOOL bOverwrite);
    void SetWriteBom(BOOL bWriteBom);
    void SetTargetPath(LPCTSTR szTargetPath);
	BOOL Convert(std::vector<std::wstring>* failedFiles, std::vector<std::wstring>* outFiles);
	int ConvertFile(const std::wstring &strSrcTemp, const std::wstring &sSrcFile);

    void Stop();

	static std::wstring GetTempFilePath();

private:
	std::wstring GetDstPath(LPCTSTR szSrcPath);
    CodePageValue GetFileCodepage(const BYTE *& pData, int& nSize);
    BOOL    ConvFile(const BYTE *pData, int nSize, LPCTSTR szDstPath, CodePageValue nSrcCodepage, CodePageValue nDstCodepage);
    DWORD   GetRealCodepage(CodePageValue nSrcCodepage, CodePageValue nDstCodepage, BOOL& bMultiByteToWideChar);

    LPBYTE  GetBuffer(int nSize);
    void    ReleaseBuffer();

private:
	std::vector<std::wstring>*  m_arrFiles;

    BOOL         m_bStop;
	std::wstring m_strTargetPath;
    BOOL        m_bOverwrite;
    BOOL        m_bWriteBom;

    CodePageValue m_nSrcCodepage;
    CodePageValue m_nDstCodepage;

    LPBYTE      m_pBuffer;
    int         m_nBufferSize;
};

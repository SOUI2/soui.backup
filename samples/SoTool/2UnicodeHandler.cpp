#include "StdAfx.h"
#include "2UnicodeHandler.h"
#include <helper/SplitString.h>

C2UnicodeHandler::C2UnicodeHandler(void)
{
}

C2UnicodeHandler::~C2UnicodeHandler(void)
{
}

void C2UnicodeHandler::OnInit( SWindow *pPageRoot )
{
    CFolderHander::OnInit(pPageRoot->FindChildByName(L"page_2unicode"));

    pugi::xml_document xmlCodeSyntax;

    SStringT strSyntax = SApplication::getSingleton().GetAppDir() + _T("\\syntax.xml");
    if(!xmlCodeSyntax.load_file(strSyntax))
    {
        LOADXML(xmlCodeSyntax,L"syntax",L"xml");
    }
    InitLang(xmlCodeSyntax.child(L"config").child(L"languages"));
}


void C2UnicodeHandler::EnumFileInfo(const FILETYPEMAP &cfg, const SStringW & strPath, HSTREEITEM hItem,SList<SStringW> & lstFileInfo , int &nDirs)
{
    //枚举当前目录下的文件
    WIN32_FIND_DATAW fd;
    SStringW strFind = strPath + L"\\*.*";

    HANDLE hFind = FindFirstFileW(strFind,&fd);
    if(hFind != INVALID_HANDLE_VALUE)
    {
        while(FindNextFileW(hFind,&fd))
        {
            if(!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                LPCWSTR pszExt = wcsrchr(fd.cFileName,L'.');
                if(!pszExt) continue;
                pszExt ++;

                const FILETYPEMAP::CPair * pPair = cfg.Lookup(pszExt);
                if(pPair)
                {
                    lstFileInfo.AddTail(strPath + L"\\" + fd.cFileName);
                }
            }
        }
        FindClose(hFind);
    }

    //查找选中的子目录
    HSTREEITEM hChild = m_pDirTree->GetChildItem(hItem);
    while(hChild)
    {
        if(m_pDirTree->GetCheckState(hChild))
        {
            SStringT strDirName;
            m_pDirTree->GetItemText(hChild,strDirName);
            SStringW strPath2 = strPath + L"\\" + S_CT2W(strDirName);
            EnumFileInfo(cfg,strPath2,hChild,lstFileInfo,nDirs);
            nDirs ++;
        }
        hChild = m_pDirTree->GetNextSiblingItem(hChild);
    }
}


/***********************************************************************
 *             map_fileW
 *
 * Helper function to map a file to memory:
 *  name			-	file name
 *  [RETURN] ptr		-	pointer to mapped file
 *  [RETURN] filesize           -       pointer size of file to be stored if not NULL
 */
static const void *map_fileW( LPCWSTR name, LPDWORD filesize )
{
    HANDLE hFile, hMapping;
    LPVOID ptr = NULL;

    hFile = CreateFileW( name, GENERIC_READ, FILE_SHARE_READ, NULL,
                         OPEN_EXISTING, FILE_FLAG_RANDOM_ACCESS, 0 );
    if (hFile != INVALID_HANDLE_VALUE)
    {
        hMapping = CreateFileMappingW( hFile, NULL, PAGE_READONLY, 0, 0, NULL );
        if (hMapping)
        {
            ptr = MapViewOfFile( hMapping, FILE_MAP_READ, 0, 0, 0 );
            CloseHandle( hMapping );
            if (filesize)
                *filesize = GetFileSize( hFile, NULL );
        }
        CloseHandle( hFile );
    }
    return ptr;
}

int ChangeFile2Unicode(LPCWSTR pszFileName)
{
    DWORD dwSize=0;
    const LPBYTE pBytes = (const LPBYTE)map_fileW(pszFileName,&dwSize);
    if(!pBytes) return FALSE;
    BOOL bUnicode = FALSE;
    if(dwSize>=2 && pBytes[0] == 0xff && pBytes[1]==0xfe)
        bUnicode = TRUE;//utf-16

    if(dwSize>=3 && pBytes[0] == 0xef && pBytes[1]==0xbb && pBytes[2]==0xbf)
        bUnicode = TRUE;//utf-8
    
    if(bUnicode)
    {
        UnmapViewOfFile(pBytes);
        return 0;
    }

    int dwLen = MultiByteToWideChar(CP_ACP,0,(LPCSTR)pBytes,dwSize,NULL,0);
    wchar_t *pBuf = new wchar_t[dwLen];
    if(!pBuf)
    {
        UnmapViewOfFile(pBytes);
        return -1;//out of memory
    }
    MultiByteToWideChar(CP_ACP,0,(LPCSTR)pBytes,dwSize,pBuf,dwLen);
    UnmapViewOfFile(pBytes);

    FILE *f = _wfopen(pszFileName,L"wb");
    if(f)
    {
        BYTE bom[2]={0xff,0xfe};
        fwrite(bom,1,2,f);
        fwrite(pBuf,sizeof(wchar_t),dwLen,f);
        fclose(f);
    }
    delete []pBuf;
    return f?1:-1;
}

void C2UnicodeHandler::OnGo()
{
    if(m_strDir.IsEmpty())
    {
        SMessageBox(m_pFolderRoot->GetContainer()->GetHostHwnd(),_T("没有指定扫描目录"),_T("错误"),MB_OK|MB_ICONSTOP);
        return;
    }
    SStringT strTypes = m_pFolderRoot->FindChildByName(L"edit_filetypes")->GetWindowText();
    SStringWList lstTypes;
    SplitString(S_CT2W(strTypes),L';',lstTypes);
    FILETYPEMAP cfg;
    for(int i=0;i<lstTypes.GetCount();i++)
    {
        cfg[lstTypes[i]] = 1;
    }
    if(cfg.IsEmpty())
    {
        SMessageBox(m_pFolderRoot->GetContainer()->GetHostHwnd(),_T("没有指定文件类型"),_T("错误"),MB_OK|MB_ICONSTOP);
        return;
    }

    //获得文件列表，计每个目标文件的文件大小
    HSTREEITEM hRoot = m_pDirTree->GetRootItem();
    SList<SStringW> lstFileInfo;
    int nDirs = 1;
    EnumFileInfo(cfg,m_strDir,hRoot,lstFileInfo,nDirs);
    if(lstFileInfo.GetCount() == 0)
    {
        SMessageBox(m_pFolderRoot->GetContainer()->GetHostHwnd(),_T("指定的目录下没有找到满足条件的文件类型"),_T("提示"),MB_OK|MB_ICONINFORMATION);
        return;
    }

    if(SMessageBox(m_pFolderRoot->GetContainer()->GetHostHwnd(),_T("转换成Unicode有可能会损坏源文件，请先做好备分！\\n确定转换吗？"),_T("提示"),MB_OKCANCEL|MB_ICONWARNING)==IDCANCEL)
    {
        return;
    }

    SWindow *pPageResult = m_pFolderRoot->FindChildByName(L"page_dir");   
    SSplitWnd_Row *pSplitTree = m_pFolderRoot->FindChildByName2<SSplitWnd_Row>(L"split_tree");
    pSplitTree->ShowPane(1);

    SProgress *pProgBar = pSplitTree->FindChildByName2<SProgress>(L"prog_run");
    pProgBar->SetRange(0,lstFileInfo.GetCount());
    pProgBar->SetValue(0);

    int nSuccess = 0, nKeep =0, nFailed =0;
    //将代码转换为unicode
    DWORD dwProg = 0;
    SPOSITION pos = lstFileInfo.GetHeadPosition();
    while(pos)
    {
        SStringW strFileName = lstFileInfo.GetNext(pos);
        int nState = ChangeFile2Unicode(strFileName)?1:0;
        switch(nState)
        {
        case 0:nKeep ++;break;
        case -1:nFailed++;break;
        case 1:nSuccess++;break;
        }
        pProgBar->SetValue(++dwProg);
        pProgBar->UpdateWindow();
        Sleep(10);
    }

    pSplitTree->HidePane(1);

    SStringT strMsg = SStringT().Format(_T("转换成功：%d\\n转换失败:%d\\n保持原编码:%d"),nSuccess,nFailed,nKeep);
    SMessageBox(m_pFolderRoot->GetContainer()->GetHostHwnd(),strMsg,_T("转换完成"),MB_OK);

}
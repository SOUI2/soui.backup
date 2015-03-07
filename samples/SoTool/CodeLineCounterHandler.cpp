#include "StdAfx.h"
#include "CodeLineCounterHandler.h"
#include <helper/SplitString.h>

CCodeLineCounterHandler::CCodeLineCounterHandler(void):m_pPageRoot(NULL)
{
}

CCodeLineCounterHandler::~CCodeLineCounterHandler(void)
{
}

void CCodeLineCounterHandler::OnInit( SWindow *pRoot )
{
    m_pPageRoot = pRoot->FindChildByName(L"page_codeline");
    SASSERT(m_pPageRoot);
    m_pDirTree = m_pPageRoot->FindChildByName2<STreeCtrl>(L"tree_dir");
    pugi::xml_document xmlCodeLinesCounterCfg;
    LOADXML(xmlCodeLinesCounterCfg,L"codelinecounterconfig",L"xml");
    
    pugi::xml_node xmlNode = xmlCodeLinesCounterCfg.child(L"config").child(L"filetypes");
    if(xmlNode)
    {
        pugi::xml_node xmlType = xmlNode.child(L"filetype");
        while(xmlType)
        {
            CCodeConfig cfg;
            cfg.strType = xmlType.attribute(L"type").value();
            cfg.strExt = xmlType.attribute(L"ext").value();
            cfg.strSingleLineRemark = xmlType.attribute(L"singleLineRemark").value();
            cfg.strMultiLinesRemarkBegin = xmlType.attribute(L"multiLinesRemarkBegin").value();
            cfg.strMultiLinesRemarkEnd = xmlType.attribute(L"multiLinesRemarkEnd").value();
            m_mapCodeCfg[cfg.strExt] = cfg;
            xmlType = xmlType.next_sibling(L"filetype");
        }
    }
    xmlNode = xmlCodeLinesCounterCfg.child(L"config").child(L"languages");
    if(xmlNode)
    {
        SComboBox * pCbx = m_pPageRoot->FindChildByName2<SComboBox>(L"cbx_filetypes");
        SASSERT(pCbx);
        pugi::xml_node xmlLang = xmlNode.child(L"language");
        while(xmlLang)
        {
            SStringW strName = xmlLang.attribute(L"name").value();
            SStringW strExts = xmlLang.attribute(L"exts").value();
            SStringW str = strName + L":" + strExts;
            pCbx->InsertItem(-1, S_CW2T(str),0,0);

            xmlLang = xmlLang.next_sibling(L"language");
        }
    }
}

void CCodeLineCounterHandler::OnBtnPickerDir()
{

}

void CCodeLineCounterHandler::OnKillFocus_Dir( EventArgs *pEvt )
{
    SEdit *pEdit = sobj_cast<SEdit>(pEvt->sender);
    SStringT strDir = pEdit->GetWindowText();
    DWORD attr = GetFileAttributes(strDir);
    if(attr==INVALID_FILE_ATTRIBUTES) return ;
    if((attr & FILE_ATTRIBUTE_DIRECTORY) == 0) return;
    if(strDir == m_strDir) return;

    m_strDir = strDir;
    m_pDirTree->RemoveAllItems();
    HSTREEITEM hRoot = m_pDirTree->InsertItem(_T("root"),0,1);
    InitDirTree(hRoot,m_strDir);
}

void CCodeLineCounterHandler::InitDirTree(HSTREEITEM hTreeItem,const SStringT & strPath)
{
    WIN32_FIND_DATA fd;
    SStringT strFind = strPath + _T("\\*.*");
    HANDLE hFind=FindFirstFile(strFind,&fd);
    if(hFind != INVALID_HANDLE_VALUE)
    {
        while(FindNextFile(hFind,&fd))
        {
            if(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY
                && !(fd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN))
            {
                SStringT strName= fd.cFileName;
                if(strName != _T(".") && strName != _T(".."))
                {
                    HSTREEITEM hItem = m_pDirTree->InsertItem(strName,0,1,hTreeItem);
                    InitDirTree(hItem,strPath + _T("\\") + strName);
                }
            }
        }
        FindClose(hFind);
    }
}

void CCodeLineCounterHandler::OnBtnGo()
{
    if(m_strDir.IsEmpty())
    {
        SMessageBox(m_pPageRoot->GetContainer()->GetHostHwnd(),_T("没有指定扫描目录"),_T("错误"),MB_OK|MB_ICONSTOP);
        return;
    }
    SStringT strTypes = m_pPageRoot->FindChildByName(L"cbx_filetypes")->GetWindowText();
    SStringTList strlst;
    SplitString(strTypes,_T(':'),strlst);
    if(strlst.GetCount()!=2)
    {
        SMessageBox(m_pPageRoot->GetContainer()->GetHostHwnd(),_T("没有指定文件类型"),_T("错误"),MB_OK|MB_ICONSTOP);
        return;
    }
    SStringTList lstTypes;
    SplitString(strlst[1],_T(';'),lstTypes);
    

    //获得文件列表，计每个目标文件的文件大小
    HSTREEITEM hRoot = m_pDirTree->GetRootItem();
    SList<FILEINFO> lstFileInfo;
    int nDirs = 1;
    DWORD szAll = EnumFileInfo(m_strDir,hRoot,lstFileInfo,nDirs);
    if(lstFileInfo.GetCount() == 0)
    {
        SMessageBox(m_pPageRoot->GetContainer()->GetHostHwnd(),_T("指定的目录下没有找到满足条件的文件类型"),_T("提示"),MB_OK|MB_ICONINFORMATION);
        return;
    }

    SWindow *pPageResult = m_pPageRoot->FindChildByName(L"page_dir");   
    SSplitWnd_Row *pSplitTree = m_pPageRoot->FindChildByName2<SSplitWnd_Row>(L"split_tree");
    pSplitTree->ShowPane(1);

    SProgress *pProgBar = pSplitTree->FindChildByName2<SProgress>(L"prog_run");
    pProgBar->SetRange(0,szAll);
    pProgBar->SetValue(0);

    int nCodeLinesAll=0,nBlankLinesAll=0,nRemLinesAll=0;

    //统计代码行
    DWORD dwProg = 0;
    SPOSITION pos = lstFileInfo.GetHeadPosition();
    while(pos)
    {
        FILEINFO fi = lstFileInfo.GetNext(pos);
        int nCodeLines=0,nBlankLines=0,nRemLines=0;
        CountCodeLines(S_CW2T(fi.strFileName),fi.cfg,nCodeLines,nRemLines,nBlankLines);
        nCodeLinesAll += nCodeLines;
        nBlankLinesAll += nBlankLines;
        nRemLinesAll += nRemLines;

        dwProg += fi.dwSize;
        pProgBar->SetValue(dwProg);
        pProgBar->UpdateWindow();
    }

    pSplitTree->HidePane(1);
    STabCtrl *pTab = m_pPageRoot->FindChildByName2<STabCtrl>(L"tab_codeline");
    pTab->SetCurSel(1);
    SWindow *pWndRes = pTab->GetPage(_T("page_result"),FALSE);
    pWndRes->FindChildByName(L"txt_folders")->SetWindowText(SStringT().Format(_T("%d"),nDirs));
    pWndRes->FindChildByName(L"txt_files")->SetWindowText(SStringT().Format(_T("%d"),lstFileInfo.GetCount()));
    pWndRes->FindChildByName(L"txt_codelines")->SetWindowText(SStringT().Format(_T("%d"),nCodeLinesAll));
    pWndRes->FindChildByName(L"txt_blanklines")->SetWindowText(SStringT().Format(_T("%d"),nBlankLinesAll));
    pWndRes->FindChildByName(L"txt_remarklines")->SetWindowText(SStringT().Format(_T("%d"),nRemLinesAll));

}


DWORD CCodeLineCounterHandler::EnumFileInfo(const SStringW & strPath, HSTREEITEM hItem,SList<FILEINFO> & lstFileInfo , int &nDirs)
{
    DWORD fileSizeDir = 0;

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
                
                CODECFGMAP::CPair * pPair = m_mapCodeCfg.Lookup(pszExt);
                if(!pPair) continue;
                FILEINFO fi;
                fi.strFileName = strPath + L"\\" + fd.cFileName;
                fi.dwSize = fd.nFileSizeLow;
                fileSizeDir += fi.dwSize;
                fi.cfg = pPair->m_value;
                lstFileInfo.AddTail(fi);
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
            fileSizeDir += EnumFileInfo(strPath2,hChild,lstFileInfo,nDirs);
            nDirs ++;
        }
        hChild = m_pDirTree->GetNextItem(hChild);
    }
    return fileSizeDir;
}
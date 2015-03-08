#include "StdAfx.h"
#include "CodeLineCounterHandler.h"
#include <helper/SplitString.h>
#include "droptarget.h"
//////////////////////////////////////////////////////////////////////////
class CDropTarget_Dir : public CDropTarget
{
protected:
    CCodeLineCounterHandler *m_pLineCounterHandler;
public:
    CDropTarget_Dir(CCodeLineCounterHandler *pLineCounterHandler):m_pLineCounterHandler(pLineCounterHandler)
    {
    }
    ~CDropTarget_Dir()
    {
    }
public:
    virtual HRESULT STDMETHODCALLTYPE Drop( 
        /* [unique][in] */ __RPC__in_opt IDataObject *pDataObj,
        /* [in] */ DWORD grfKeyState,
        /* [in] */ POINTL pt,
        /* [out][in] */ __RPC__inout DWORD *pdwEffect)
    {
        FORMATETC format =
        {
            CF_HDROP, 0, DVASPECT_CONTENT, -1, TYMED_HGLOBAL
        };
        STGMEDIUM medium;
        if(FAILED(pDataObj->GetData(&format, &medium)))
        {
            return S_FALSE;
        }

        HDROP hdrop = static_cast<HDROP>(GlobalLock(medium.hGlobal));

        if(!hdrop)
        {
            return S_FALSE;
        }

        bool success = false;
        TCHAR filename[MAX_PATH];
        success=!!DragQueryFile(hdrop, 0, filename, MAX_PATH);
        if(success) 
        {
            if(GetFileAttributes(filename) & FILE_ATTRIBUTE_DIRECTORY)
            {
                m_pLineCounterHandler->InitDir(filename);
            }
        }
        DragFinish(hdrop);
        GlobalUnlock(medium.hGlobal);


        *pdwEffect=DROPEFFECT_LINK;
        return S_OK;
    }
};

//////////////////////////////////////////////////////////////////////////
// CLineInfo
class CLineInfo
{
public:
    CLineInfo(int _code=0,int _blank=0, int _remark=0)
        :nCodeLines(_code)
        ,nBlankLines(_blank)
        ,nRemarkLines(_remark)
    {
    }

    void operator = (const CLineInfo & src)
    {
        nCodeLines = src.nCodeLines;
        nBlankLines = src.nBlankLines;
        nRemarkLines = src.nRemarkLines;
    }

    CLineInfo & operator += (const CLineInfo & src)
    {
        nCodeLines += src.nCodeLines;
        nBlankLines += src.nBlankLines;
        nRemarkLines += src.nRemarkLines;
        return *this;
    }
    int nCodeLines;
    int nBlankLines;
    int nRemarkLines;
};

//////////////////////////////////////////////////////////////////////////
// CCodeLineCounterHandler
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

    SWindow *pEditDir = m_pPageRoot->FindChildByName(L"edit_dir");
    SASSERT(pEditDir);
    IDropTarget *pDT = new CDropTarget_Dir(this);
    pRoot->GetContainer()->RegisterDragDrop(pEditDir->GetSwnd(),pDT);

    m_pDirTree = m_pPageRoot->FindChildByName2<STreeCtrl>(L"tree_dir");

    pugi::xml_document xmlCodeSyntax;

    SStringT strSyntax = SApplication::getSingleton().GetAppDir() + _T("\\syntax.xml");
    if(!xmlCodeSyntax.load_file(strSyntax))
    {
        LOADXML(xmlCodeSyntax,L"syntax",L"xml");
    }
    
    pugi::xml_node xmlNode = xmlCodeSyntax.child(L"config").child(L"filetypes");
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
    xmlNode = xmlCodeSyntax.child(L"config").child(L"languages");
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


void CCodeLineCounterHandler::OnKillFocus_Dir( EventArgs *pEvt )
{
    SEdit *pEdit = sobj_cast<SEdit>(pEvt->sender);
    SStringT strDir = pEdit->GetWindowText();
    InitDir(strDir,TRUE);
}


void CCodeLineCounterHandler::InitDir( const SStringT & strDir,BOOL bInput/*=FALSE */)
{
    DWORD attr = GetFileAttributes(strDir);
    if(attr==INVALID_FILE_ATTRIBUTES) return ;
    if((attr & FILE_ATTRIBUTE_DIRECTORY) == 0) return;
    if(strDir == m_strDir) return;
    
    if(!bInput)
    {
        m_pPageRoot->FindChildByName(L"edit_dir")->SetWindowText(strDir);
    }
    m_strDir = strDir;
    m_pDirTree->RemoveAllItems();
    HSTREEITEM hRoot = m_pDirTree->InsertItem(_T("root"),0,1);
    InitDirTree(hRoot,m_strDir);
    m_pDirTree->SetCheckState(hRoot,TRUE);
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

DWORD CCodeLineCounterHandler::EnumFileInfo(const CODECFGMAP &cfg, const SStringW & strPath, HSTREEITEM hItem,SList<FILEINFO> & lstFileInfo , int &nDirs)
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

                const CODECFGMAP::CPair * pPair = cfg.Lookup(pszExt);
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
            fileSizeDir += EnumFileInfo(cfg,strPath2,hChild,lstFileInfo,nDirs);
            nDirs ++;
        }
        hChild = m_pDirTree->GetNextSiblingItem(hChild);
    }
    return fileSizeDir;
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
    SStringWList lstTypes;
    SplitString(S_CT2W(strlst[1]),L';',lstTypes);
    CODECFGMAP cfg;
    for(int i=0;i<lstTypes.GetCount();i++)
    {
        cfg[lstTypes[i]] = m_mapCodeCfg[lstTypes[i]];
    }

    //获得文件列表，计每个目标文件的文件大小
    HSTREEITEM hRoot = m_pDirTree->GetRootItem();
    SList<FILEINFO> lstFileInfo;
    int nDirs = 1;
    DWORD szAll = EnumFileInfo(cfg,m_strDir,hRoot,lstFileInfo,nDirs);
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

    CLineInfo lineInfoAll;

    SMap<SStringT,CLineInfo> mapLineInfo;
    for(int i=0;i<lstTypes.GetCount();i++)
    {
        mapLineInfo[lstTypes[i]] = CLineInfo();
    }

    //统计代码行
    DWORD dwProg = 0;
    SPOSITION pos = lstFileInfo.GetHeadPosition();
    while(pos)
    {
        FILEINFO fi = lstFileInfo.GetNext(pos);

        CLineInfo lineInfo;
        CountCodeLines(S_CW2T(fi.strFileName),fi.cfg,lineInfo.nCodeLines,lineInfo.nRemarkLines,lineInfo.nBlankLines);
        lineInfoAll += lineInfo;
        mapLineInfo[fi.cfg.strExt] += lineInfo;

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
    pWndRes->FindChildByName(L"txt_codelines")->SetWindowText(SStringT().Format(_T("%d"),lineInfoAll.nCodeLines));
    pWndRes->FindChildByName(L"txt_blanklines")->SetWindowText(SStringT().Format(_T("%d"),lineInfoAll.nBlankLines));
    pWndRes->FindChildByName(L"txt_remarklines")->SetWindowText(SStringT().Format(_T("%d"),lineInfoAll.nRemarkLines));

    SListCtrl *plstReport = pWndRes->FindChildByName2<SListCtrl>(L"lst_linecounter_report");
    plstReport->DeleteAllItems();
    for(int i=0;i<lstTypes.GetCount();i++)
    {
        CCodeConfig codeCfg = cfg[lstTypes[i]];
        CLineInfo lineInfo = mapLineInfo[lstTypes[i]];

        plstReport->InsertItem(i,codeCfg.strType);
        plstReport->SetSubItemText(i,1,codeCfg.strExt);
        plstReport->SetSubItemText(i,2,SStringT().Format(_T("%d"),lineInfo.nCodeLines));
        plstReport->SetSubItemText(i,3,SStringT().Format(_T("%d"),lineInfo.nBlankLines));
        plstReport->SetSubItemText(i,4,SStringT().Format(_T("%d"),lineInfo.nRemarkLines));
    }
}


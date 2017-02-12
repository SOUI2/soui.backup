#include "stdafx.h"
#include "CalcMd5Handler.h"


const void *map_fileW( LPCWSTR name, LPDWORD filesize );

CCalcMd5Handler::CCalcMd5Handler(void)
{
}

CCalcMd5Handler::~CCalcMd5Handler(void)
{
}

void CCalcMd5Handler::OnFileDropdown( HDROP hDrop )
{
    bool success = false;
    TCHAR filename[MAX_PATH];
    success=!!DragQueryFile(hDrop, 0, filename, MAX_PATH);
    if(success) 
    {
        if(!(GetFileAttributes(filename) & FILE_ATTRIBUTE_DIRECTORY))
        {
            SWindow *pEditInput = m_pPageRoot->FindChildByName(L"edit_input");
            SASSERT(pEditInput);
            pEditInput->SetWindowText(filename);
            CalcFileMd5(filename);
        }
    }

}

void CCalcMd5Handler::OnDirEnterFinish( EventArgs *pEvt )
{
    SWindow *pEdit = sobj_cast<SWindow>(pEvt->sender);
    SStringT strFile = pEdit->GetWindowText();
    CalcFileMd5(strFile);
}

void CCalcMd5Handler::CalcFileMd5( const SStringT &strFileName )
{
    SStringT strMd5;
    DWORD fileSize =0;
    const LPBYTE pData =(const LPBYTE)map_fileW(S_CT2W(strFileName),&fileSize);
    if(pData)
    {
        SSplitWnd *pSplit = m_pPageRoot->FindChildByName2<SSplitWnd>(L"split_frame");
        SProgress *pProg = m_pPageRoot->FindChildByName2<SProgress>(L"prog_run");
        pProg->SetRange(0,fileSize);
        pProg->SetValue(0);
        pSplit->ShowPane(1);
        m_dwPrevProg = 0;

        unsigned char buff[16];
        MD5_CTX context;
        MD5Init (&context);
        MD5Update(&context, pData, fileSize,this);
        MD5Final(&context,buff);
        UnmapViewOfFile(pData);

        TCHAR szMD5[16*2+1];
        for(int i=0;i<16;i++)
        {
            _stprintf(szMD5+i*2,_T("%x"),(buff[i] & 0xF0)>>4);
            _stprintf(szMD5+i*2+1,_T("%x"),buff[i] & 0x0F);
        }
        szMD5[16*2]=0;
        strMd5 = szMD5;

        pSplit->HidePane(1);

    }else
    {
        strMd5 = _T("文件打开失败!");
    }

    strMd5 += _T("\t");
    int pos = strFileName.ReverseFind(_T('\\'));
    if(pos!=-1)
    {
        strMd5 += strFileName.Right(strFileName.GetLength()-pos-1);
    }else
    {
        strMd5 += strFileName;
    }
    SRichEdit *pEditOutput = m_pPageRoot->FindChildByName2<SRichEdit>(L"edit_output");
    SASSERT(pEditOutput);
    if(pEditOutput->GetWindowTextLength()!=0)
    {
        pEditOutput->SetSel(-1);
        pEditOutput->ReplaceSel(L"\r\n");
    }
    pEditOutput->ReplaceSel(S_CT2W(strMd5));
}

void CCalcMd5Handler::OnInit( SWindow *pRoot )
{
    m_pPageRoot = pRoot->FindChildByName(L"page_md5");
    SASSERT(m_pPageRoot);
    SWindow *pEditInput = m_pPageRoot->FindChildByName(L"edit_input");
    SASSERT(pEditInput);

    IDropTarget *pDT = new CDropTarget(this);
    m_pPageRoot->GetContainer()->RegisterDragDrop(pEditInput->GetSwnd(),pDT);
    pDT->Release();

}

void CCalcMd5Handler::OnCalcMd5Prog( DWORD dwTotal,DWORD dwProg )
{
    if((dwProg-m_dwPrevProg)*100/dwTotal > 2)
    {//max t0 50 steps
        m_dwPrevProg = dwProg;
        SProgress *pProg = m_pPageRoot->FindChildByName2<SProgress>(L"prog_run");
        SASSERT(pProg);
        pProg->SetValue(dwProg);
        STRACEW(L"OnCalcMd5Prog,prog:%d",dwProg);
        pProg->UpdateWindow();
    }
}

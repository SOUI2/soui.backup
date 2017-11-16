#pragma once
#include <helper/SAdapterBase.h>

struct SOFTINFO
{
    const wchar_t *pszBigSkinName;
    const wchar_t *pszSmallSkinName;
    const wchar_t *pszFileName;
    const DWORD    dwSize;
    const wchar_t *modify_time;
};
const int type_count = 16;
SOFTINFO info[type_count] =
{
    { L"skin_file_icon_b_apktype", L"skin_file_icon_s_apktype", L"apk", 150 * (1 << 20), L"2017-11-14 11:37:18" },
    { L"skin_file_icon_b_doctype", L"skin_file_icon_s_doctype", L"doc", 150 * (1 << 20), L"2017-11-14 11:37:18" },
    { L"skin_file_icon_b_exetype", L"skin_file_icon_s_exetype", L"exe", 150 * (1 << 20), L"2017-11-14 11:37:18" },
    { L"skin_file_icon_b_foldertype", L"skin_file_icon_s_foldertype", L"folder", 150 * (1 << 20), L"2017-11-14 11:37:18" },
    { L"skin_file_icon_b_imgtype", L"skin_file_icon_s_imgtype", L"img", 150 * (1 << 20), L"2017-11-14 11:37:18" },
    { L"skin_file_icon_b_ipatype", L"skin_file_icon_s_ipatype", L"ipa", 150 * (1 << 20), L"2017-11-14 11:37:18" },
    { L"skin_file_icon_b_musictype", L"skin_file_icon_s_musictype", L"music", 150 * (1 << 20), L"2017-11-14 11:37:18" },
    { L"skin_file_icon_b_othertype", L"skin_file_icon_s_othertype", L"other", 150 * (1 << 20), L"2017-11-14 11:37:18" },
    { L"skin_file_icon_b_pdftype", L"skin_file_icon_s_pdftype", L"pdf", 150 * (1 << 20), L"2017-11-14 11:37:18" },
    { L"skin_file_icon_b_ppttype", L"skin_file_icon_s_ppttype", L"ppt", 150 * (1 << 20), L"2017-11-14 11:37:18" },
    { L"skin_file_icon_b_rartype", L"skin_file_icon_s_rartype", L"rar", 150 * (1 << 20), L"2017-11-14 11:37:18" },
    { L"skin_file_icon_b_torrenttype", L"skin_file_icon_s_torrenttype", L"torrent", 150 * (1 << 20), L"2017-11-14 11:37:18" },
    { L"skin_file_icon_b_txttype", L"skin_file_icon_s_txttype", L"txt", 150 * (1 << 20), L"2017-11-14 11:37:18" },
    { L"skin_file_icon_b_videotype", L"skin_file_icon_s_videotype", L"video", 150 * (1 << 20), L"2017-11-14 11:37:18" },
    { L"skin_file_icon_b_vsdtype", L"skin_file_icon_s_vsdtype", L"vsd", 150 * (1 << 20), L"2017-11-14 11:37:18" },
    { L"skin_file_icon_b_xlstype", L"skin_file_icon_s_xlstype", L"xls", 150 * (1 << 20), L"2017-11-14 11:37:18" },
};

class CTestTileAdapter : public SAdapterBase
{
public:
    CTestTileAdapter()
    {
    
    }
    virtual int getCount()
    {
        return 50000;
    }
    
    virtual void getView(int position, SWindow *pItem, pugi::xml_node xmlTemplate)
    {
        if(pItem->GetChildrenCount() == 0)
        {
            pItem->InitFromXml(xmlTemplate);
        }
        SImageWnd *pImg = pItem->FindChildByName2<SImageWnd>(L"tile_item_img");
        pImg->SetSkin(GETSKIN(info[position % type_count].pszBigSkinName, pImg->GetScale()));
        SStatic *pTxt = pItem->FindChildByName2<SStatic>(L"tile_item_txt");
        pTxt->SetWindowText(SStringT().Format(_T("我的文件 %d"), position));
        pTxt->GetRoot()->SetUserData(position);
        pTxt->GetEventSet()->subscribeEvent(EVT_CMD, Subscriber(&CTestTileAdapter::OnButtonClick, this));
    }
    
    bool OnButtonClick(EventArgs *pEvt)
    {
        SButton *pBtn = sobj_cast<SButton>(pEvt->sender);
        int iItem = pBtn->GetRoot()->GetUserData();
        SMessageBox(NULL, SStringT().Format(_T("button of %d item was clicked"), iItem), _T("haha"), MB_OK);
        return true;
    }
    
};

class CTestMcAdapterFix : public SMcAdapterBase
{
public:
    CTestMcAdapterFix()
    {
    
    }
    
    virtual int getCount()
    {
        return 50000;
    }
    
    SStringT getSizeText(DWORD dwSize)
    {
        int num1 = dwSize / (1 << 20);
        dwSize -= num1 * (1 << 20);
        int num2 = dwSize * 100 / (1 << 20);
        return SStringT().Format(_T("%d.%02dM"), num1, num2);
    }
    
    virtual void getView(int position, SWindow *pItem, pugi::xml_node xmlTemplate)
    {
        if(pItem->GetChildrenCount() == 0)
        {
            pItem->InitFromXml(xmlTemplate);
        }
        
        SOFTINFO psi = info[position % type_count];
        pItem->FindChildByName(L"list_item_img")->SetAttribute(L"skin", psi.pszSmallSkinName);
        pItem->FindChildByName(L"list_item_btn")->SetWindowText(S_CW2T(psi.pszFileName));
        pItem->FindChildByName(L"list_item_size")->SetWindowText(getSizeText(psi.dwSize));
        pItem->FindChildByName(L"list_item_modify_time")->SetWindowText(S_CW2T(psi.modify_time));
        
        SButton *pBtnUninstall = pItem->FindChildByName2<SButton>(L"list_item_btn");
        pBtnUninstall->SetUserData(position);
        pBtnUninstall->GetEventSet()->subscribeEvent(EVT_CMD, Subscriber(&CTestMcAdapterFix::OnButtonClick, this));
    }
    
    bool OnButtonClick(EventArgs *pEvt)
    {
        SButton *pBtn = sobj_cast<SButton>(pEvt->sender);
        int iItem = pBtn->GetUserData();
        
        if(SMessageBox(NULL, SStringT().Format(_T("Are you sure to uninstall the selected [%d] software?"), iItem), _T("uninstall"), MB_OKCANCEL | MB_ICONQUESTION) == IDOK)
        {
        
        }
        return true;
    }
    
    SStringW GetColumnName(int iCol) const
    {
        return SStringW().Format(L"col_%d", iCol + 1);
    }
    
    struct SORTCTX
    {
        int iCol;
        SHDSORTFLAG stFlag;
    };
    
    bool OnSort(int iCol, SHDSORTFLAG *stFlags, int nCols)
    {
        if(iCol == 5)  //最后一列“操作”不支持排序
        {
            return false;
        }
        
        SHDSORTFLAG stFlag = stFlags[iCol];
        switch(stFlag)
        {
            case ST_NULL:
                stFlag = ST_UP;
                break;
            case ST_DOWN:
                stFlag = ST_UP;
                break;
            case ST_UP:
                stFlag = ST_DOWN;
                break;
        }
        for(int i = 0; i < nCols; i++)
        {
            stFlags[i] = ST_NULL;
        }
        stFlags[iCol] = stFlag;
        
        SORTCTX ctx = { iCol, stFlag };
        qsort_s(info, type_count, sizeof(SOFTINFO), SortCmp, &ctx);
        return true;
    }
    
    static int __cdecl SortCmp(void *context, const void *p1, const void *p2)
    {
        SORTCTX *pctx = (SORTCTX *)context;
        const SOFTINFO *pSI1 = (const SOFTINFO *)p1;
        const SOFTINFO *pSI2 = (const SOFTINFO *)p2;
        int nRet = 0;
        switch(pctx->iCol)
        {
            case 0://name
                nRet = wcscmp(pSI1->pszFileName, pSI2->pszFileName);
                break;
            case 1://size
                nRet = (int)(pSI1->dwSize - pSI2->dwSize);
                break;
            case 2://modify time
                nRet = wcscmp(pSI1->modify_time, pSI2->modify_time);
                break;
        }
        if(pctx->stFlag == ST_UP)
        {
            nRet = -nRet;
        }
        return nRet;
    }
};

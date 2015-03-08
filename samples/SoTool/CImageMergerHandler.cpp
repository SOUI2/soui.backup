#include "StdAfx.h"
#include "CImageMergerHandler.h"
#include "droptarget.h"

class CDropTarget_Canvas : public CDropTarget
{
protected:
    CImageMergerHandler *m_pImgMerge;
public:
    CDropTarget_Canvas(CImageMergerHandler *pImgMerge):m_pImgMerge(pImgMerge)
    {
    }
    ~CDropTarget_Canvas()
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
        UINT files = DragQueryFile(hdrop,-1,NULL,0);
        for(UINT i=0;i<files;i++)
        {
            success=!!DragQueryFile(hdrop, i, filename, MAX_PATH);
            if(success)
            {
                m_pImgMerge->AddFile(filename);
            }
        }
        DragFinish(hdrop);
        GlobalUnlock(medium.hGlobal);


        *pdwEffect=DROPEFFECT_LINK;
        return S_OK;
    }
};

CImageMergerHandler::CImageMergerHandler(void):m_pPageRoot(NULL),m_pImgCanvas(NULL)
{
}

CImageMergerHandler::~CImageMergerHandler(void)
{
}

void CImageMergerHandler::OnInit(SWindow *pRoot)
{
    m_pPageRoot = pRoot->FindChildByName(L"page_imagemerge");
    m_pImgCanvas = m_pPageRoot->FindChildByName2<SImgCanvas>(L"wnd_canvas");//CDropTarget_Canvas
    IDropTarget *pDT = new CDropTarget_Canvas(this);
    pRoot->GetContainer()->RegisterDragDrop(m_pImgCanvas->GetSwnd(),pDT);
    pDT->Release();
}

void CImageMergerHandler::AddFile(LPCWSTR pszFileName)
{
    m_pImgCanvas->AddFile(pszFileName);
}

void CImageMergerHandler::OnSave()
{
    CFileDialogEx dlgSave(FALSE,_T("png"),0,6,_T("png files(*.png)\0*.png\0All files (*.*)\0*.*\0\0"));
    if(dlgSave.DoModal()==IDOK)
    {
        m_pImgCanvas->Save2File(dlgSave.m_szFileName);
    }
}

void CImageMergerHandler::OnClear()
{
    m_pImgCanvas->Clear();
}

void CImageMergerHandler::OnModeHorz()
{
    if(!m_pImgCanvas) return;
    m_pImgCanvas->SetVertical(FALSE);
}

void CImageMergerHandler::OnModeVert()
{
    if(!m_pImgCanvas) return;
    m_pImgCanvas->SetVertical(TRUE);
}


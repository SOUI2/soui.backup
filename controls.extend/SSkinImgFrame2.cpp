#include "stdafx.h"
#include "SSkinImgFrame2.h"
#include <souicoll.h>

namespace SOUI
{
    typedef SMap<SStringW,IBitmap *> IMGPOOL;
    static IMGPOOL s_imgPool;
    
    SSkinImgFrame2::SSkinImgFrame2(void)
    {
    }

    SSkinImgFrame2::~SSkinImgFrame2(void)
    {
        if(GetImage())
        {
            if(GetImage()->Release() == 0)
            {
                s_imgPool.RemoveKey(m_strImgKey);
            }
        }
    }

    LRESULT SSkinImgFrame2::OnAttrSrc(const SStringW & strValue,BOOL bLoading)
    {
        int iPos = strValue.Find(L'{');
        if(iPos==-1) return E_FAIL;
        m_strImgKey = strValue.Left(iPos);
        SStringW strRgn = strValue.Right(strValue.GetLength()-iPos);
        if(wscanf(strRgn,L"{%d,%d,%d,%d}",&m_rcImg.left,&m_rcImg.top,&m_rcImg.right,&m_rcImg.bottom)!=4)
            return E_FAIL;
        
        IMGPOOL::CPair * p = s_imgPool.Lookup(m_strImgKey);
        if(p)
        {
            SSkinImgFrame::SetImage(p->m_value);
            p->m_value->AddRef();
        }else
        {
            IBitmap *pImg=LOADIMAGE2(m_strImgKey);
            if(!pImg) return E_FAIL;
            s_imgPool[m_strImgKey]=pImg;
            SSkinImgFrame::SetImage(pImg);
        }
        return S_OK;
    }

}

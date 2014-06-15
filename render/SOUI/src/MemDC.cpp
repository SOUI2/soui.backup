#include "duistd.h"
#include "MemDC.h"
namespace SOUI
{

CMemDC::CMemDC()
    :m_bBmpOwner(FALSE)
    ,m_bHasBitmap(FALSE)
    ,m_hOldBmp(NULL)
{
}

CMemDC::CMemDC( HDC hdc, const CRect & rc )
    :m_bBmpOwner(FALSE)
    ,m_bHasBitmap(FALSE)
    ,m_hOldBmp(NULL)
{
    InitDC(hdc,rc);
}

CMemDC::CMemDC( HDC hdc,HBITMAP hBmp)
    :m_bBmpOwner(FALSE)
    ,m_bHasBitmap(TRUE)
{
    CreateCompatibleDC(hdc);
    DUIASSERT(m_hDC != NULL);
    if(hBmp)
        m_hOldBmp=__super::SelectBitmap(hBmp);
    else
        m_hOldBmp=NULL;
    SetViewportOrg(0,0);
}

CMemDC::~CMemDC(void)
{
    DeleteDC();
}

HBITMAP CMemDC::SelectBitmap( HBITMAP hBmp )
{
    DUIASSERT(m_hDC);
    if(hBmp)
    {
        m_hOldBmp=__super::SelectBitmap(hBmp);
        m_bBmpOwner=FALSE;
        m_bHasBitmap=TRUE;
        SetViewportOrg(0,0);
        return m_hOldBmp;
    }
    else if(m_bHasBitmap)
    {
        HBITMAP hBmp=__super::SelectBitmap(m_hOldBmp);
        m_hOldBmp=NULL;
        m_bBmpOwner=FALSE;
        m_bHasBitmap=FALSE;
        return hBmp;
    }
    else
    {
        return NULL;
    }
}

void CMemDC::DeleteDC()
{
    if(m_hDC && m_hOldBmp)
    {
        HBITMAP hBmp=__super::SelectBitmap(m_hOldBmp);
        if(m_bBmpOwner) DeleteObject(hBmp);
    }
    __super::DeleteDC();
    m_bHasBitmap=FALSE;
    m_bBmpOwner=FALSE;
}

BOOL CMemDC::InitDC( HDC hdc,const CRect &rc )
{
    if(m_hDC) return FALSE;
    CreateCompatibleDC(hdc);
    if(!m_hDC) return FALSE;
    HBITMAP hBmp=CreateCompatibleBitmap(hdc,rc.Width(),rc.Height());
    if(!hBmp)
    {
        __super::DeleteDC();
        return FALSE;
    }
    m_hOldBmp=__super::SelectBitmap(hBmp);
    SetViewportOrg(-rc.left, -rc.top);
    m_bHasBitmap=TRUE;
    m_bBmpOwner=TRUE;
    return TRUE;
}

}//namespace SOUI

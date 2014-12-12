#include "souistd.h"
#include "core/SwndLayout.h"
#include "core/SWnd.h"

namespace SOUI
{

    //////////////////////////////////////////////////////////////////////////
    SWindowRepos::SWindowRepos(SWindow *pWnd):m_pWnd(pWnd)
    {
        SASSERT(m_pWnd);
        m_rcWnd = m_pWnd->m_rcWindow;
        m_pWnd->m_rcWindow.SetRect(POS_INIT,POS_INIT,POS_INIT,POS_INIT);
    }

    SWindowRepos::~SWindowRepos()
    {
        if(m_pWnd->m_rcWindow != m_rcWnd)
        {
            m_pWnd->OnRelayout(m_rcWnd,m_pWnd->m_rcWindow);
        }
    }
    
    //////////////////////////////////////////////////////////////////////////
    //
    SwndLayout::SwndLayout( SWindow *pOwner )
    :m_pOwner(pOwner)
    ,nCount(0)
    ,uPositionType(0)
    ,fOffsetX(0.0f)
    ,fOffsetY(0.0f)
    ,uSpecifyWidth(0)
    ,uSpecifyHeight(0)
    ,nSepSpace(2)
    {
        InitLayoutState();
    }
    
    LPCWSTR SwndLayout::ParsePosition(LPCWSTR pszPos,BOOL bFirst2Pos,POSITION_ITEM &pos)
    {
        if(!pszPos) return NULL;

        if(pszPos[0]==POSFLAG_DEFSIZE && bFirst2Pos)
        {//如果在前面两个坐标中定义size，自动忽略
            SASSERT(FALSE);
            pszPos++;
        }
        if(pszPos[0]==POSFLAG_REFCENTER) pos.pit=PIT_CENTER,pszPos++;
        else if(pszPos[0]==POSFLAG_PERCENT) pos.pit=PIT_PERCENT,pszPos++;
        else if(pszPos[0]==POSFLAG_REFPREV_NEAR) pos.pit=PIT_PREV_NEAR,pszPos++;
        else if(pszPos[0]==POSFLAG_REFNEXT_NEAR) pos.pit=PIT_NEXT_NEAR,pszPos++;
        else if(pszPos[0]==POSFLAG_REFPREV_FAR) pos.pit=PIT_PREV_FAR,pszPos++;
        else if(pszPos[0]==POSFLAG_REFNEXT_FAR) pos.pit=PIT_NEXT_FAR,pszPos++;
        else if(pszPos[0]==POSFLAG_DEFSIZE) pos.pit=PIT_OFFSET,pszPos++;
        else pos.pit=PIT_NORMAL;

        pos.bMinus=FALSE;
        if(pszPos[0]==L'-')
        {
            pszPos++;
            pos.bMinus=TRUE;
        }

        pos.nPos=(float)_wtof(pszPos);

        const wchar_t *pNext=wcschr(pszPos,L',');
        if(pNext) pNext++;
        return pNext;
    }

    CRect SwndLayout::GetWindowLayoutRect(SWindow *pWindow)
    {
        CRect rc = pWindow->m_rcWindow;
        
        if(!pWindow->m_bDisplay && !pWindow->m_bVisible)
        {
            rc.right=rc.left;
            rc.bottom=rc.top;
        }
        return rc;
    }
    
    int SwndLayout::PositionItem2Value(const POSITION_ITEM &pos ,int nMin, int nMax,BOOL bX)
    {
        int nRet=0;
        int nWid=nMax-nMin;

        switch(pos.pit)
        {
        case PIT_CENTER: 
            nRet=(int)pos.nPos * (pos.bMinus?-1:1) + nWid/2 + nMin;
            break;
        case PIT_NORMAL: 
            if(pos.bMinus)
                nRet=nMax-(int)pos.nPos;
            else
                nRet=nMin+(int)pos.nPos;
            break;
        case PIT_PERCENT: 
            if(pos.bMinus)
                nRet=nMin+(int)((100.0f-pos.nPos)*nWid/100);
            else
                nRet=nMin+(int)(pos.nPos*nWid/100);
            if(nRet>nMax) nRet=nMax;
            break;
        case PIT_PREV_NEAR:
        case PIT_PREV_FAR:
            {
                SWindow *pRefWnd=m_pOwner->GetWindow(GSW_PREVSIBLING);
                if(!pRefWnd) pRefWnd=m_pOwner->GetWindow(GSW_PARENT);
                if(pRefWnd)
                {//需要确定参考窗口是否完成布局
                    CRect rcRef = GetWindowLayoutRect(pRefWnd);
                    if(bX)
                    {
                        LONG refPos = (pos.pit == PIT_PREV_NEAR)?rcRef.right:rcRef.left;
                        if(refPos == POS_INIT || refPos==POS_WAIT)
                            nRet=POS_WAIT;
                        else
                            nRet=refPos+(int)pos.nPos*(pos.bMinus?-1:1);
                    }else
                    {
                        LONG refPos = (pos.pit == PIT_PREV_NEAR)?rcRef.bottom:rcRef.top;
                        if(refPos == POS_INIT || refPos==POS_WAIT)
                            nRet=POS_WAIT;
                        else
                            nRet=refPos+(int)pos.nPos*(pos.bMinus?-1:1);
                    }
                }
            }
            break;
        case PIT_NEXT_NEAR:
        case PIT_NEXT_FAR:
            {
                SWindow *pRefWnd=m_pOwner->GetWindow(GSW_NEXTSIBLING);
                if(!pRefWnd) pRefWnd=m_pOwner->GetWindow(GSW_PARENT);
                if(pRefWnd)
                {//需要确定参考窗口是否完成布局
                    CRect rcRef = GetWindowLayoutRect(pRefWnd);
                    if(bX)
                    {
                        LONG refPos = (pos.pit == PIT_NEXT_NEAR)?rcRef.left:rcRef.right;
                        if(refPos == POS_INIT || refPos==POS_WAIT)
                            nRet=POS_WAIT;
                        else
                            nRet=refPos+(int)pos.nPos*(pos.bMinus?-1:1);
                    }else
                    {
                        LONG refPos = (pos.pit == PIT_NEXT_NEAR)?rcRef.top:rcRef.bottom;
                        if(refPos == POS_INIT || refPos==POS_WAIT)
                            nRet=POS_WAIT;
                        else
                            nRet=refPos+(int)pos.nPos*(pos.bMinus?-1:1);
                    }
                }
            }
            break;
        }

        return nRet;

    }
    
    int SwndLayout::CalcPosition(const CRect & rcContainer,CRect &rcWindow )
    {
        int nRet=0;

        if(nCount==4)
        {//指定了4个坐标
            if(rcWindow.left == POS_INIT || rcWindow.left == POS_WAIT)
                rcWindow.left=PositionItem2Value(Left,rcContainer.left,rcContainer.right,TRUE);
            if(rcWindow.left==POS_WAIT) nRet++;

            if(rcWindow.top == POS_INIT || rcWindow.top == POS_WAIT)
                rcWindow.top=PositionItem2Value(Top,rcContainer.top,rcContainer.bottom,FALSE);
            if(rcWindow.top==POS_WAIT) nRet++;

            if(rcWindow.right == POS_INIT || rcWindow.right == POS_WAIT)
            {
                if(Right.pit!=PIT_OFFSET)
                    rcWindow.right=PositionItem2Value(Right,rcContainer.left,rcContainer.right,TRUE);
                else if(rcWindow.left!=POS_WAIT)
                    rcWindow.right=rcWindow.left+(LONG)Right.nPos;
                else
                    rcWindow.right=POS_WAIT;
            }
            if(rcWindow.right==POS_WAIT) nRet++;

            if(rcWindow.bottom == POS_INIT || rcWindow.bottom == POS_WAIT)
            {
                if(Bottom.pit!=PIT_OFFSET)
                    rcWindow.bottom=PositionItem2Value(Bottom,rcContainer.top,rcContainer.bottom,FALSE);
                else if(rcWindow.top!=POS_WAIT)
                    rcWindow.bottom=rcWindow.top+(LONG)Bottom.nPos;
                else
                    rcWindow.bottom=POS_WAIT;
            }
            if(rcWindow.bottom==POS_WAIT) nRet++;
        }else 
        {
            CPoint pt=rcWindow.TopLeft();
            if((uPositionType & SizeX_FitParent) &&  (uPositionType &SizeY_FitParent))
            {//充满父窗口
                pt.x=rcContainer.left;
                pt.y=rcContainer.top;
            }else if(nCount==2)
            {//只指定了两个坐标
                if(pt.x==POS_INIT || pt.x==POS_WAIT) pt.x=PositionItem2Value(Left,rcContainer.left,rcContainer.right,TRUE);
                if(pt.x==POS_WAIT) nRet++;
                if(pt.y==POS_INIT || pt.y==POS_WAIT) pt.y=PositionItem2Value(Top,rcContainer.top,rcContainer.bottom,FALSE);
                if(pt.y==POS_WAIT) nRet++;
            }else //if(nCount==0)
            {//自动排版
                SWindow *pSibling=m_pOwner->GetWindow(GSW_PREVSIBLING);
                if(!pSibling)
                {
                    pt.x=rcContainer.left;
                    pt.y=rcContainer.top;
                }else
                {
                    CRect rcSib = pSibling->m_rcWindow;
                    if(rcSib.right==POS_INIT || rcSib.right == POS_WAIT)
                        pt.x=POS_WAIT,nRet++;
                    else
                        pt.x=rcSib.right+nSepSpace;

                    if(rcSib.top==POS_INIT || rcSib.top==POS_WAIT)
                        pt.y=POS_WAIT,nRet++;
                    else
                        pt.y=rcSib.top;
                }
            }
            if(nRet==0)
                rcWindow=CRect(pt,CalcSize(rcContainer));
            else 
                rcWindow.left=pt.x,rcWindow.top=pt.y;
        }

        if(nRet==0)
        {//没有坐标等待计算了
            rcWindow.NormalizeRect();
            //处理窗口的偏移(offset)属性
            CSize sz = rcWindow.Size();
            CPoint ptOffset;
            ptOffset.x = (LONG)(sz.cx * fOffsetX);
            ptOffset.y = (LONG)(sz.cy * fOffsetY);
            rcWindow.OffsetRect(ptOffset);
        }
        return nRet;
    }

    BOOL SwndLayout::CalcChildrenPosition(SList<SWindowRepos*> *pListChildren,const CRect & rcContainer)
    {
        SPOSITION pos=pListChildren->GetHeadPosition();
        int nChildrenCount=pListChildren->GetCount();
        while(pos)
        {
            SPOSITION posOld=pos;
            SWindow *pChild=pListChildren->GetNext(pos)->GetWindow();
            if(0 == pChild->m_layout.CalcPosition(rcContainer,pChild->m_rcWindow))
            {
                delete pListChildren->GetAt(posOld);
                pListChildren->RemoveAt(posOld);
            }
        }
        if(0==pListChildren->GetCount())
            return TRUE;
        if(nChildrenCount == pListChildren->GetCount())
        {//窗口布局依赖死锁
            SASSERT(FALSE);
            return FALSE;
        }else
        {
            return CalcChildrenPosition(pListChildren,rcContainer);
        }
    }

    void SwndLayout::ParseStrPostion( LPCWSTR pszValue)
    {
        uPositionType &= ~Pos_Float;

        nCount=0;
        while(nCount<4 && pszValue)
        {
            pszValue=ParsePosition(pszValue,nCount<2,Item[nCount++]);
        }


        if (2 == nCount || 4 == nCount)
        {
            if(2 == nCount)
            {
                uPositionType = (uPositionType & ~SizeX_Mask) | SizeX_FitContent;
                uPositionType = (uPositionType & ~SizeY_Mask) | SizeY_FitContent;
            }
        }
        else
            nCount = 0;

    }

    CSize SwndLayout::CalcSize(const CRect & rcContainer)
    {
        CSize sz;
        if(uPositionType & SizeX_Specify)
            sz.cx=uSpecifyWidth;
        else if(uPositionType & SizeX_FitParent)
            sz.cx=rcContainer.right-rcContainer.left;
        if(uPositionType & SizeY_Specify)
            sz.cy=uSpecifyHeight;
        else if(uPositionType & SizeY_FitParent)
            sz.cy=rcContainer.bottom-rcContainer.top;
        if((uPositionType & SizeX_FitContent) || (uPositionType & SizeY_FitContent) && nCount!=4)
        {
            CSize szDesire=m_pOwner->GetDesiredSize(rcContainer);    
            if(uPositionType & SizeX_FitContent)
                sz.cx=szDesire.cx;
            if(uPositionType & SizeY_FitContent)
                sz.cy=szDesire.cy;
        }
        return sz;
    }

    BOOL SwndLayout::IsFitContent()
    {
        return nCount!=4 && (uPositionType & (SizeX_FitContent|SizeY_FitContent));
    }

    BOOL SwndLayout::IsFloat()
    {
        return uPositionType & Pos_Float;
    }

    void SwndLayout::InitLayoutState()
    {
        m_pOwner->m_rcWindow.SetRect(POS_INIT,POS_INIT,POS_INIT,POS_INIT);
    }

}

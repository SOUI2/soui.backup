#include "souistd.h"
#include "core/SwndLayoutBuilder.h"

namespace SOUI
{

    //////////////////////////////////////////////////////////////////////////
    SWindowRepos::SWindowRepos(SWindow *pWnd):m_pWnd(pWnd)
    {
        SASSERT(m_pWnd);
        m_rcWnd = m_pWnd->m_rcWindow;
        SwndLayoutBuilder::InitLayoutState(m_pWnd->m_rcWindow);
    }

    SWindowRepos::~SWindowRepos()
    {
        if(m_pWnd->m_rcWindow != m_rcWnd)
        {
            m_pWnd->OnRelayout(m_rcWnd,m_pWnd->m_rcWindow);
        }
    }
    
    //////////////////////////////////////////////////////////////////////////
    // SwndLayout

    void SwndLayoutBuilder::InitLayoutState(CRect &rcWindow)
    {
        rcWindow.SetRect(POS_INIT,POS_INIT,POS_INIT,POS_INIT);
    }

    BOOL SwndLayoutBuilder::IsWaitingPos( int nPos )
    {
        return nPos == POS_INIT || nPos == POS_WAIT;
    }

    CRect SwndLayoutBuilder::GetWindowLayoutRect(SWindow *pWindow)
    {
        CRect rc = pWindow->m_rcWindow;
        
        if(!pWindow->m_bDisplay && !pWindow->m_bVisible)
        {
            rc.right=rc.left;
            rc.bottom=rc.top;
        }
        return rc;
    }
    
    int SwndLayoutBuilder::PositionItem2Value(SWindow *pWindow,const POSITION_ITEM &pos ,int nMin, int nMax,BOOL bX)
    {
        int nRet=0;
        int nSize=nMax-nMin;

        switch(pos.pit)
        {
        case PIT_CENTER: 
            nRet=(int)pos.nPos * pos.cMinus + nSize/2 + nMin;
            break;
        case PIT_NORMAL: 
            if(pos.cMinus == -1)
                nRet=nMax-(int)pos.nPos;
            else
                nRet=nMin+(int)pos.nPos;
            break;
        case PIT_PERCENT: 
            if(pos.cMinus == -1)
                nRet=nMin+(int)((100.0f-pos.nPos)*nSize/100);
            else
                nRet=nMin+(int)(pos.nPos*nSize/100);
            if(nRet>nMax) nRet=nMax;
            break;
        case PIT_PREV_NEAR:
        case PIT_PREV_FAR:
            {
                CRect rcRef;
                SWindow *pRefWnd=pWindow->GetWindow(GSW_PREVSIBLING);
                if(pRefWnd)
                {
                    rcRef = GetWindowLayoutRect(pRefWnd);
                }else
                {
                    pRefWnd=pWindow->GetWindow(GSW_PARENT);
                    SASSERT(pRefWnd);
                    rcRef = GetWindowLayoutRect(pRefWnd);
                    rcRef.right = rcRef.left;
                    rcRef.bottom = rcRef.top;
                }
                if(bX)
                {
                    LONG refPos = (pos.pit == PIT_PREV_NEAR)?rcRef.right:rcRef.left;
                    if(IsWaitingPos(refPos))
                        nRet=POS_WAIT;
                    else
                        nRet=refPos+(int)pos.nPos*pos.cMinus;
                }else
                {
                    LONG refPos = (pos.pit == PIT_PREV_NEAR)?rcRef.bottom:rcRef.top;
                    if(IsWaitingPos(refPos))
                        nRet=POS_WAIT;
                    else
                        nRet=refPos+(int)pos.nPos*pos.cMinus;
                }
            }
            break;
        case PIT_NEXT_NEAR:
        case PIT_NEXT_FAR:
            {
                CRect rcRef;
                SWindow *pRefWnd=pWindow->GetWindow(GSW_NEXTSIBLING);
                if(pRefWnd)
                {
                    rcRef = GetWindowLayoutRect(pRefWnd);
                }else
                {
                    pRefWnd=pWindow->GetWindow(GSW_PARENT);
                    SASSERT(pRefWnd);
                    rcRef = GetWindowLayoutRect(pRefWnd);
                    rcRef.left = rcRef.right;
                    rcRef.top = rcRef.bottom;
                }

                if(bX)
                {
                    LONG refPos = (pos.pit == PIT_NEXT_NEAR)?rcRef.left:rcRef.right;
                    if(IsWaitingPos(refPos))
                        nRet=POS_WAIT;
                    else
                        nRet=refPos+(int)pos.nPos*pos.cMinus;
                }else
                {
                    LONG refPos = (pos.pit == PIT_NEXT_NEAR)?rcRef.top:rcRef.bottom;
                    if(IsWaitingPos(refPos))
                        nRet=POS_WAIT;
                    else
                        nRet=refPos+(int)pos.nPos*pos.cMinus;
                }
            }
            break;
        }

        return nRet;

    }
    
    int SwndLayoutBuilder::CalcPosition(SWindow *pWnd,const CRect & rcContainer,CRect & rcWindow, const SwndLayout * pSwndLayout/*=NULL*/)
    {
        int nRet=0;
        
        if(pSwndLayout == NULL) pSwndLayout = pWnd->GetLayout();

        if(pSwndLayout->nCount==4)
        {//指定了4个坐标
            if(IsWaitingPos(rcWindow.left))
                rcWindow.left=PositionItem2Value(pWnd,pSwndLayout->pos[PI_LEFT],rcContainer.left,rcContainer.right,TRUE);
            if(rcWindow.left==POS_WAIT) nRet++;

            if(IsWaitingPos(rcWindow.top))
                rcWindow.top=PositionItem2Value(pWnd,pSwndLayout->pos[PI_TOP],rcContainer.top,rcContainer.bottom,FALSE);
            if(rcWindow.top==POS_WAIT) nRet++;

            if(IsWaitingPos(rcWindow.right))
            {
                if(pSwndLayout->pos[PI_RIGHT].pit == PIT_SIZE)
                {
                    if(!IsWaitingPos(rcWindow.left))
                    {
                        if(pSwndLayout->pos[PI_RIGHT].cMinus == -1)
                        {
                            CSize szWnd=pWnd->GetDesiredSize(rcContainer);
                            rcWindow.right = rcWindow.left + szWnd.cx;
                        }else
                        {
                            rcWindow.right = rcWindow.left + (int)pSwndLayout->pos[PI_RIGHT].nPos;
                        }
                    }
                }else
                {
                    rcWindow.right=PositionItem2Value(pWnd,pSwndLayout->pos[PI_RIGHT],rcContainer.left,rcContainer.right,TRUE);
                }
            }
            if(rcWindow.right==POS_WAIT) nRet++;

            if(IsWaitingPos(rcWindow.bottom ))
            {
                if(pSwndLayout->pos[PI_BOTTOM].pit == PIT_SIZE)
                {
                    if(!IsWaitingPos(rcWindow.top))
                    {
                        if(pSwndLayout->pos[PI_BOTTOM].cMinus  == -1)
                        {
                            CSize szWnd=pWnd->GetDesiredSize(rcContainer);
                            rcWindow.bottom = rcWindow.top + szWnd.cy;
                        }else
                        {
                            rcWindow.bottom = rcWindow.top + (int)pSwndLayout->pos[PI_BOTTOM].nPos;
                        }
                    }
                }else
                {
                    rcWindow.bottom=PositionItem2Value(pWnd,pSwndLayout->pos[PI_BOTTOM],rcContainer.top,rcContainer.bottom,FALSE);
                }
            }
            if(rcWindow.bottom==POS_WAIT) nRet++;
        }else 
        {
            CPoint pt = pWnd->m_rcWindow.TopLeft();
            if(pSwndLayout->nCount==2)
            {//指定了两个坐标
                if(IsWaitingPos(pt.x)) pt.x=PositionItem2Value(pWnd,pSwndLayout->pos[PI_LEFT],rcContainer.left,rcContainer.right,TRUE);
                if(pt.x==POS_WAIT) nRet++;
                if(IsWaitingPos(pt.y)) pt.y=PositionItem2Value(pWnd,pSwndLayout->pos[PI_TOP],rcContainer.top,rcContainer.bottom,FALSE);
                if(pt.y==POS_WAIT) nRet++;
            }else //if(nCount==0)
            {
                if(IsWaitingPos(pt.x) && pSwndLayout->IsFitParent(PD_X))
                {
                    pt.x=rcContainer.left;
                }
                if(IsWaitingPos(pt.y) && pSwndLayout->IsFitParent(PD_Y))
                {
                    pt.y=rcContainer.top;
                }
                //自动排版
                SWindow *pSibling=pWnd->GetWindow(GSW_PREVSIBLING);
                if(!pSibling)
                {//没有兄弟窗口，从父窗口左上角开始
                    if(IsWaitingPos(pt.x)) pt.x=rcContainer.left;
                    if(IsWaitingPos(pt.y)) pt.y=rcContainer.top;
                }else
                {
                    CRect rcSib = pSibling->m_rcWindow;
                    if(IsWaitingPos(pt.x))
                    {
                        if(!IsWaitingPos(rcSib.right))
                        {
                            SWindow *pParent = pWnd->GetParent();
                            SASSERT(pParent);
                            pt.x=rcSib.right+pParent->m_style.m_bySepSpace;
                        }else
                        {
                            pt.x=POS_WAIT,nRet++;
                        }
                    }
                    if(IsWaitingPos(pt.y))
                    {
                        if(!IsWaitingPos(rcSib.top))
                        {
                            pt.y=rcSib.top;
                        }else
                        {
                            pt.y=POS_WAIT,nRet++;
                        }
                    }
                }
            }
            if(nRet==0)
                rcWindow=CRect(pt,CalcSize(pWnd,rcContainer,pSwndLayout));
            else 
                rcWindow.left=pt.x,rcWindow.top=pt.y;
        }

        if(nRet==0)
        {//没有坐标等待计算了
            rcWindow.NormalizeRect();
            //处理窗口的偏移(offset)属性
            CSize sz = rcWindow.Size();
            CPoint ptOffset;
            ptOffset.x = (LONG)(sz.cx * pSwndLayout->fOffsetX);
            ptOffset.y = (LONG)(sz.cy * pSwndLayout->fOffsetY);
            rcWindow.OffsetRect(ptOffset);
        }
        return nRet;
    }

    BOOL SwndLayoutBuilder::CalcChildrenPosition(SList<SWindowRepos*> *pListChildren,const CRect & rcContainer)
    {
        SPOSITION pos=pListChildren->GetHeadPosition();
        int nChildrenCount=pListChildren->GetCount();
        while(pos)
        {
            SPOSITION posOld=pos;
            SWindow *pChild=pListChildren->GetNext(pos)->GetWindow();
            if(0 == SwndLayoutBuilder::CalcPosition(pChild,rcContainer,pChild->m_rcWindow))
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

    CSize SwndLayoutBuilder::CalcSize(SWindow *pWnd,const CRect & rcContainer,const SwndLayout * pSwndLayout)
    {
        CSize sz;
        if(!pSwndLayout) pSwndLayout = pWnd->GetLayout();
        if(pSwndLayout->IsSpecifySize(PD_X))
            sz.cx=pSwndLayout->uSpecifyWidth;
        else if(pSwndLayout->IsFitParent(PD_X))
            sz.cx=rcContainer.right-rcContainer.left;
        if(pSwndLayout->IsSpecifySize(PD_Y))
            sz.cy=pSwndLayout->uSpecifyHeight;
        else if(pSwndLayout->IsFitParent(PD_Y))
            sz.cy=rcContainer.bottom-rcContainer.top;

        if((pSwndLayout->IsFitContent(PD_ALL) ) && pSwndLayout->nCount!=4)
        {
            CSize szDesire=pWnd->GetDesiredSize(rcContainer);    
            if(pSwndLayout->IsFitContent(PD_X))
                sz.cx=szDesire.cx;
            if(pSwndLayout->IsFitContent(PD_Y))
                sz.cy=szDesire.cy;
        }
        return sz;
    }

}

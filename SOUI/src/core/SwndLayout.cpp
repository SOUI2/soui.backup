#include "souistd.h"
#include "core/SwndLayout.h"
#include "core/SWnd.h"

namespace SOUI
{

    SwndLayout::SwndLayout( SWindow *pOwner )
    :m_pOwner(pOwner)
    ,nCount(0)
    ,uPositionType(0)
    ,pos2Type(POS2_LEFTTOP)
    ,uSpecifyWidth(0)
    ,uSpecifyHeight(0)
    ,nSepSpace(2)
    {

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
            if(pos.pit != PIT_PERCENT)//百分比值时，不允许使用负值，直接忽略
                pos.bMinus=TRUE;
        }

        pos.nPos=(float)_wtof(pszPos);

        const wchar_t *pNext=wcschr(pszPos,L',');
        if(pNext) pNext++;
        return pNext;
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
                    CRect rcRef;
                    pRefWnd->GetWindowRect(&rcRef);
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
                    CRect rcRef;
                    pRefWnd->GetWindowRect(&rcRef);
                    if(bX)
                    {
                        LONG refPos = (pos.pit == PIT_NEXT_NEAR)?rcRef.left:rcRef.right;
                        if(refPos == POS_INIT || refPos==POS_WAIT)
                            nRet=POS_WAIT;
                        else
                            nRet=refPos+(int)pos.nPos*(pos.bMinus?-1:1);
                    }else
                    {
                        LONG refPos = (pos.pit == PIT_PREV_NEAR)?rcRef.top:rcRef.bottom;
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
    
    int SwndLayout::CalcPosition(LPRECT lpRcContainer,CRect &rcWindow )
    {
        int nRet=0;

        CRect rcContainer;
        if(!lpRcContainer)
        {
            SASSERT(m_pOwner->GetParent());
            rcContainer=m_pOwner->GetParent()->GetChildrenLayoutRect();
            lpRcContainer=&rcContainer;
        }

        if(nCount==4)
        {//指定了4个坐标
            if(rcWindow.left == POS_INIT || rcWindow.left == POS_WAIT)
                rcWindow.left=PositionItem2Value(Left,lpRcContainer->left,lpRcContainer->right,TRUE);
            if(rcWindow.left==POS_WAIT) nRet++;

            if(rcWindow.top == POS_INIT || rcWindow.top == POS_WAIT)
                rcWindow.top=PositionItem2Value(Top,lpRcContainer->top,lpRcContainer->bottom,FALSE);
            if(rcWindow.top==POS_WAIT) nRet++;

            if(rcWindow.right == POS_INIT || rcWindow.right == POS_WAIT)
            {
                if(Right.pit!=PIT_OFFSET)
                    rcWindow.right=PositionItem2Value(Right,lpRcContainer->left,lpRcContainer->right,TRUE);
                else if(rcWindow.left!=POS_WAIT)
                    rcWindow.right=rcWindow.left+(LONG)Right.nPos;
                else
                    rcWindow.right=POS_WAIT;
            }
            if(rcWindow.right==POS_WAIT) nRet++;

            if(rcWindow.bottom == POS_INIT || rcWindow.bottom == POS_WAIT)
            {
                if(Bottom.pit!=PIT_OFFSET)
                    rcWindow.bottom=PositionItem2Value(Bottom,lpRcContainer->top,lpRcContainer->bottom,FALSE);
                else if(rcWindow.top!=POS_WAIT)
                    rcWindow.bottom=rcWindow.top+(LONG)Bottom.nPos;
                else
                    rcWindow.bottom=POS_WAIT;
            }
            if(rcWindow.bottom==POS_WAIT) nRet++;
        }else 
        {
            CPoint pt=rcWindow.TopLeft();
            CSize sz=CalcSize(lpRcContainer);
            if((uPositionType & SizeX_FitParent) &&  (uPositionType &SizeY_FitParent))
            {//充满父窗口
                pt.x=lpRcContainer->left;
                pt.y=lpRcContainer->top;
            }else if(nCount==2)
            {//只指定了两个坐标
                if(pt.x==POS_INIT || pt.x==POS_WAIT) pt.x=PositionItem2Value(Left,lpRcContainer->left,lpRcContainer->right,TRUE);
                if(pt.x==POS_WAIT) nRet++;
                if(pt.y==POS_INIT || pt.y==POS_WAIT) pt.y=PositionItem2Value(Top,lpRcContainer->top,lpRcContainer->bottom,FALSE);
                if(pt.y==POS_WAIT) nRet++;

                if(nRet==0)
                {
                    switch(pos2Type)
                    {
                    case POS2_CENTER:
                        pt.Offset(-sz.cx/2,-sz.cy/2);
                        break;
                    case POS2_RIGHTTOP:
                        pt.Offset(-sz.cx,0);
                        break;
                    case POS2_LEFTBOTTOM:
                        pt.Offset(0,-sz.cy);
                        break;
                    case POS2_RIGHTBOTTOM:
                        pt.Offset(-sz.cx,-sz.cy);
                        break;
                    case POS2_LEFTTOP:
                    default:
                        break;
                    }
                }
            }else //if(nCount==0)
            {//自动排版
                SWindow *pSibling=m_pOwner->GetWindow(GSW_PREVSIBLING);
                if(!pSibling)
                {
                    pt.x=lpRcContainer->left;
                    pt.y=lpRcContainer->top;
                }else
                {
                    CRect rcSib;
                    pSibling->GetWindowRect(&rcSib);
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
            if(nRet==0)    rcWindow=CRect(pt,sz);
            else rcWindow.left=pt.x,rcWindow.top=pt.y;
        }

        if(nRet==0) rcWindow.NormalizeRect();
        return nRet;
    }

    BOOL SwndLayout::CalcChildrenPosition(SList<SWindow*> *pListChildren)
    {
        CRect rcContainer=m_pOwner->GetChildrenLayoutRect();
        POSITION pos=pListChildren->GetHeadPosition();
        int nChildrenCount=pListChildren->GetCount();
        while(pos)
        {
            POSITION posOld=pos;
            SWindow *pChild=pListChildren->GetNext(pos);
            if(0==pChild->SSendMessage(WM_WINDOWPOSCHANGED,0,(LPARAM)&rcContainer))
                pListChildren->RemoveAt(posOld);
        }
        if(0==pListChildren->GetCount())
            return TRUE;
        if(nChildrenCount == pListChildren->GetCount())
        {//窗口布局依赖死锁
            SASSERT(FALSE);
            return FALSE;
        }else
        {
            return CalcChildrenPosition(pListChildren);
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

    CSize SwndLayout::CalcSize( LPRECT pRcContainer )
    {
        CSize sz;
        if(uPositionType & SizeX_Specify)
            sz.cx=uSpecifyWidth;
        else if(uPositionType & SizeX_FitParent)
            sz.cx=pRcContainer->right-pRcContainer->left;
        if(uPositionType & SizeY_Specify)
            sz.cy=uSpecifyHeight;
        else if(uPositionType & SizeY_FitParent)
            sz.cy=pRcContainer->bottom-pRcContainer->top;
        if((uPositionType & SizeX_FitContent) || (uPositionType & SizeY_FitContent) && nCount!=4)
        {
            CSize szDesire=m_pOwner->GetDesiredSize(pRcContainer);    
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


}

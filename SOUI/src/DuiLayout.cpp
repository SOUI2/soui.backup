#include "duistd.h"
#include "DuiLayout.h"
#include "duiwnd.h"

namespace SOUI
{
    LPCSTR CDuiLayout::ParsePosition(const char * pszPos,BOOL bFirst2Pos,DUIDLG_POSITION_ITEM &pos)
    {
        if(!pszPos) return NULL;

        if(pszPos[0]==POSFLAG_DEFSIZE && bFirst2Pos)
        {//如果在前面两个坐标中定义size，自动忽略
            DUIASSERT(FALSE);
            pszPos++;
        }
        if(pszPos[0]==POSFLAG_REFCENTER) pos.pit=PIT_CENTER,pszPos++;
        else if(pszPos[0]==POSFLAG_PERCENT) pos.pit=PIT_PERCENT,pszPos++;
        else if(pszPos[0]==POSFLAG_REFPREV) pos.pit=PIT_PREVSIBLING,pszPos++;
        else if(pszPos[0]==POSFLAG_REFNEXT) pos.pit=PIT_NEXTSIBLING,pszPos++;
        else if(pszPos[0]==POSFLAG_DEFSIZE) pos.pit=PIT_OFFSET,pszPos++;
        else pos.pit=PIT_NORMAL;

        pos.bMinus=FALSE;
        if(pszPos[0]=='-')
        {
            pszPos++;
            if(pos.pit != PIT_PERCENT)//百分比值时，不允许使用负值，直接忽略
                pos.bMinus=TRUE;
        }

        pos.nPos=(float)atof(pszPos);

        const char *pNext=strchr(pszPos,',');
        if(pNext) pNext++;
        return pNext;
    }



    int CDuiLayout::PositionItem2Value(SWindow *pWnd, const DUIDLG_POSITION_ITEM &pos ,int nMin, int nMax,BOOL bX)
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
        case PIT_PREVSIBLING:
            {
                SWindow *pRefWnd=pWnd->GetDuiWindow(GDUI_PREVSIBLING);
                if(!pRefWnd) pRefWnd=pWnd->GetDuiWindow(GDUI_PARENT);
                if(pRefWnd)
                {//需要确定参考窗口是否完成布局
                    CRect rcRef;
                    pRefWnd->GetRect(&rcRef);
                    if(bX)
                    {
                        if(rcRef.right == POS_INIT || rcRef.right==POS_WAIT)
                            nRet=POS_WAIT;
                        else
                            nRet=rcRef.right+(int)pos.nPos*(pos.bMinus?-1:1);
                    }else
                    {
                        if(rcRef.bottom == POS_INIT || rcRef.bottom==POS_WAIT)
                            nRet=POS_WAIT;
                        else
                            nRet=rcRef.bottom+(int)pos.nPos*(pos.bMinus?-1:1);
                    }
                }
            }
            break;
        case PIT_NEXTSIBLING:
            {
                SWindow *pRefWnd=pWnd->GetDuiWindow(GDUI_NEXTSIBLING);
                if(!pRefWnd) pRefWnd=pWnd->GetDuiWindow(GDUI_PARENT);
                if(pRefWnd)
                {//需要确定参考窗口是否完成布局
                    CRect rcRef;
                    pRefWnd->GetRect(&rcRef);
                    if(bX)
                    {
                        if(rcRef.left == POS_INIT || rcRef.left==POS_WAIT)
                            nRet=POS_WAIT;
                        else
                            nRet=rcRef.left+(int)pos.nPos*(pos.bMinus?-1:1);
                    }else
                    {
                        if(rcRef.top == POS_INIT || rcRef.top==POS_WAIT)
                            nRet=POS_WAIT;
                        else
                            nRet=rcRef.top+(int)pos.nPos*(pos.bMinus?-1:1);
                    }
                }
            }
            break;
        }

        return nRet;

    }
    int CDuiLayout::CalcPosition(SWindow *pWnd, LPRECT lpRcContainer,const DUIWND_POSITION & dlgpos,CRect &rcWindow )
    {
        int nRet=0;

        CRect rcContainer;
        if(!lpRcContainer)
        {
            DUIASSERT(pWnd->GetParent());
            rcContainer=pWnd->GetParent()->GetChildrenLayoutRect();
            lpRcContainer=&rcContainer;
        }

        UINT uPositionType=dlgpos.uPositionType;

        if(dlgpos.nCount==4)
        {//指定了4个坐标
            if(rcWindow.left == POS_INIT || rcWindow.left == POS_WAIT)
                rcWindow.left=PositionItem2Value(pWnd,dlgpos.Left,lpRcContainer->left,lpRcContainer->right,TRUE);
            if(rcWindow.left==POS_WAIT) nRet++;

            if(rcWindow.top == POS_INIT || rcWindow.top == POS_WAIT)
                rcWindow.top=PositionItem2Value(pWnd,dlgpos.Top,lpRcContainer->top,lpRcContainer->bottom,FALSE);
            if(rcWindow.top==POS_WAIT) nRet++;

            if(rcWindow.right == POS_INIT || rcWindow.right == POS_WAIT)
            {
                if(dlgpos.Right.pit!=PIT_OFFSET)
                    rcWindow.right=PositionItem2Value(pWnd,dlgpos.Right,lpRcContainer->left,lpRcContainer->right,TRUE);
                else if(rcWindow.left!=POS_WAIT)
                    rcWindow.right=rcWindow.left+(LONG)dlgpos.Right.nPos;
                else
                    rcWindow.right=POS_WAIT;
            }
            if(rcWindow.right==POS_WAIT) nRet++;

            if(rcWindow.bottom == POS_INIT || rcWindow.bottom == POS_WAIT)
            {
                if(dlgpos.Bottom.pit!=PIT_OFFSET)
                    rcWindow.bottom=PositionItem2Value(pWnd,dlgpos.Bottom,lpRcContainer->top,lpRcContainer->bottom,FALSE);
                else if(rcWindow.top!=POS_WAIT)
                    rcWindow.bottom=rcWindow.top+(LONG)dlgpos.Bottom.nPos;
                else
                    rcWindow.bottom=POS_WAIT;
            }
            if(rcWindow.bottom==POS_WAIT) nRet++;
        }else 
        {
            CPoint pt=rcWindow.TopLeft();
            CSize sz=pWnd->CalcSize(lpRcContainer);
            if((uPositionType & SizeX_FitParent) &&  (uPositionType &SizeY_FitParent))
            {//充满父窗口
                pt.x=lpRcContainer->left;
                pt.y=lpRcContainer->top;
            }else if(dlgpos.nCount==2)
            {//只指定了两个坐标
                if(pt.x==POS_INIT || pt.x==POS_WAIT) pt.x=PositionItem2Value(pWnd,dlgpos.Left,lpRcContainer->left,lpRcContainer->right,TRUE);
                if(pt.x==POS_WAIT) nRet++;
                if(pt.y==POS_INIT || pt.y==POS_WAIT) pt.y=PositionItem2Value(pWnd,dlgpos.Top,lpRcContainer->top,lpRcContainer->bottom,FALSE);
                if(pt.y==POS_WAIT) nRet++;

                if(nRet==0)
                {
                    switch(dlgpos.pos2Type)
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
            }else //if(dlgpos.nCount==0)
            {//自动排版
                SWindow *pSibling=pWnd->GetDuiWindow(GDUI_PREVSIBLING);
                if(!pSibling)
                {
                    pt.x=lpRcContainer->left;
                    pt.y=lpRcContainer->top;
                }else
                {
                    CRect rcSib;
                    pSibling->GetRect(&rcSib);
                    if(rcSib.right==POS_INIT || rcSib.right == POS_WAIT)
                        pt.x=POS_WAIT,nRet++;
                    else
                        pt.x=rcSib.right+pWnd->m_nSepSpace;

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

    BOOL CDuiLayout::CalcChildrenPosition(SWindow *pWnd,CDuiList<SWindow*> *pListChildren)
    {
        CRect rcContainer=pWnd->GetChildrenLayoutRect();
        POSITION pos=pListChildren->GetHeadPosition();
        int nChildrenCount=pListChildren->GetCount();
        while(pos)
        {
            POSITION posOld=pos;
            SWindow *pChild=pListChildren->GetNext(pos);
            if(0==pChild->DuiSendMessage(WM_WINDOWPOSCHANGED,0,(LPARAM)&rcContainer))
                pListChildren->RemoveAt(posOld);
        }
        if(0==pListChildren->GetCount())
            return TRUE;
        if(nChildrenCount == pListChildren->GetCount())
        {//窗口布局依赖死锁
            DUIASSERT(FALSE);
            return FALSE;
        }else
        {
            return CalcChildrenPosition(pWnd,pListChildren);
        }
    }

    void CDuiLayout::StrPos2DuiWndPos( LPCSTR pszValue,DUIWND_POSITION &dlgpos )
    {
        dlgpos.uPositionType &= ~Pos_Float;

        dlgpos.nCount=0;
        while(dlgpos.nCount<4 && pszValue)
        {
            pszValue=ParsePosition(pszValue,dlgpos.nCount<2,dlgpos.Item[dlgpos.nCount++]);
        }


        if (2 == dlgpos.nCount || 4 == dlgpos.nCount)
        {
            if(2 == dlgpos.nCount)
            {
                dlgpos.uPositionType = (dlgpos.uPositionType & ~SizeX_Mask) | SizeX_FitContent;
                dlgpos.uPositionType = (dlgpos.uPositionType & ~SizeY_Mask) | SizeY_FitContent;
            }
        }
        else
            dlgpos.nCount = 0;

    }
}

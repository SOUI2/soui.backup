#include "souistd.h"
#include "layout\SouiLayout.h"

namespace SOUI{
    enum
    {
        POS_INIT=0x11000000,    //坐标的初始化值
        POS_WAIT=0x12000000,    //坐标的计算依赖于其它窗口的布局
    };

	SouiLayout::SouiLayout(void)
	{
	}

	SouiLayout::~SouiLayout(void)
	{
	}

    bool SouiLayout::IsParamAcceptable(ILayoutParam *pLayoutParam) const
    {
        return pLayoutParam->IsClass(SouiLayoutParam::GetClassName());
    }

    ILayoutParam * SouiLayout::CreateLayoutParam() const
    {
        return new SouiLayoutParam;
    }

    void SouiLayout::CalcPostionOfChildren(SWindow * pParent)
    {
        //throw std::logic_error("The method or operation is not implemented.");
    }

    BOOL SouiLayout::IsWaitingPos( int nPos )
    {
        return nPos == POS_INIT || nPos == POS_WAIT;
    }

    SWindow * SouiLayout::GetRefSibling(SWindow *pCurWnd,int uCode)
    {
        SASSERT(uCode == GSW_NEXTSIBLING || uCode == GSW_PREVSIBLING);
        SWindow *pRet = pCurWnd->GetWindow(uCode);
        while(pRet)
        {
            if(pRet->IsVisible(FALSE) || pRet->IsDisplay()) break;
            pRet = pRet->GetWindow(uCode);
        }
        return pRet;
    }

    CRect SouiLayout::GetWindowLayoutRect(SWindow *pWindow)
    {
        return pWindow->GetWindowRect();
    }

    int SouiLayout::PositionItem2Value(SWindow *pWindow,const POSITION_ITEM &pos ,int nMin, int nMax,BOOL bX)
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
                SWindow *pRefWnd=GetRefSibling(pWindow,GSW_PREVSIBLING);
                if(pRefWnd)
                {
                    rcRef = GetWindowLayoutRect(pRefWnd);
                }else
                {
                    pRefWnd=pWindow->GetWindow(GSW_PARENT);
                    SASSERT(pRefWnd);
                    rcRef = pRefWnd->GetClientRect();
                    //将parent看成一个虚拟的prev sibling
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
                SWindow *pRefWnd=GetRefSibling(pWindow,GSW_NEXTSIBLING);
                if(pRefWnd)
                {
                    rcRef = GetWindowLayoutRect(pRefWnd);
                }else
                {
                    pRefWnd=pWindow->GetWindow(GSW_PARENT);
                    SASSERT(pRefWnd);
                    rcRef = pRefWnd->GetClientRect();
                    //将parent看成一个虚拟的next sibling
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
        case PIT_SIB_LEFT:// PIT_SIB_LEFT == PIT_SIB_TOP
        case PIT_SIB_RIGHT://PIT_SIB_RIGHT == PIT_SIB_BOTTOM
            {
                SASSERT(pos.nRefID>0);
                SWindow *pParent=pWindow->GetParent();
                SASSERT(pParent);
                SWindow *pRefWnd = pParent->FindChildByID(pos.nRefID);
                SASSERT(pRefWnd);
                CRect rcRef = GetWindowLayoutRect(pRefWnd);

                if(bX)
                {
                    LONG refPos = (pos.pit == PIT_SIB_LEFT)?rcRef.left:rcRef.right;
                    if(IsWaitingPos(refPos))
                        nRet=POS_WAIT;
                    else
                        nRet=refPos+(int)pos.nPos*pos.cMinus;
                }else
                {
                    LONG refPos = (pos.pit == PIT_SIB_TOP)?rcRef.top:rcRef.bottom;//PIT_SIB_TOP == PIT_SIB_LEFT
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

    int SouiLayout::CalcPosition(SWindow *pWnd,const CRect & rcContainer,CRect & rcWindow, const SwndLayout * pSwndLayout/*=NULL*/)
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
                            CSize szWnd=pWnd->GetDesiredSize(&rcContainer);
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
                            CSize szWnd=pWnd->GetDesiredSize(&rcContainer);
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
                SWindow *pSibling=GetRefSibling(pWnd,GSW_PREVSIBLING);
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

    BOOL SouiLayout::CalcChildrenPosition(SList<SWindowRepos*> *pListChildren,const CRect & rcContainer)
    {
        SPOSITION pos=pListChildren->GetHeadPosition();
        int nChildrenCount=pListChildren->GetCount();
        while(pos)
        {
            SPOSITION posOld=pos;
            SWindow *pChild=pListChildren->GetNext(pos)->GetWindow();
            if(0 == CalcPosition(pChild,rcContainer,pChild->m_rcWindow))
            {
                delete pListChildren->GetAt(posOld);
                pListChildren->RemoveAt(posOld);
            }
        }
        if(0==pListChildren->GetCount())
            return TRUE;
        if(nChildrenCount == pListChildren->GetCount())
        {//窗口布局依赖死锁
            SASSERT_FMTW(FALSE,L"窗口布局依赖死锁");
            return FALSE;
        }else
        {
            return CalcChildrenPosition(pListChildren,rcContainer);
        }
    }

    CSize SouiLayout::MeasureChildren(SWindow * pParent,int nWidth,int nHeight) const
    {
        struct SWNDPOS{
            SWindow *pWnd;
            CRect    rcWnd;
        };
        SList<SWindowRepos*> lstWnd;
        SList<SWNDPOS>       lstWndPos;

        SWindow *pChild=GetWindow(GSW_FIRSTCHILD);
        while(pChild)
        {
            if(!pChild->IsFloat() && (pChild->IsVisible(FALSE) || pChild->IsDisplay()))
            {//不显示且不占位的窗口不参与计算
                SWNDPOS wndPos;
                wndPos.pWnd = pChild;
                wndPos.rcWnd = pChild->m_rcWindow;
                lstWndPos.AddTail(wndPos);
                lstWnd.AddTail(new SWindowRepos(pChild,TRUE));
            }
            pChild=pChild->GetWindow(GSW_NEXTSIBLING);
        }
        CalcChildrenPosition(&lstWnd,nWidth,nHeight);

        pChild = GetWindow(GSW_FIRSTCHILD);

        CRect rcContainer;

        while(pChild)
        {
            CRect rcWnd = pChild->GetWindowRect();
            int nReqWid = rcWnd.right;
            if(nReqWid<0) nReqWid = -rcWnd.right + rcWnd.Width();
            int nReqHei = rcWnd.bottom;
            if(nReqHei<0) nReqHei = -rcWnd.bottom + rcWnd.Height();

            rcContainer.right = max(rcContainer.right,max(nReqWid,rcWnd.right));
            rcContainer.bottom = max(rcContainer.bottom,max(nReqHei,rcWnd.bottom));

            pChild = pChild->GetWindow(GSW_NEXTSIBLING);
        }

        SPOSITION pos = lstWndPos.GetHeadPosition();
        while(pos)
        {
            SWNDPOS wndPos = lstWndPos.GetNext(pos);
            wndPos.pWnd->m_rcWindow = wndPos.rcWnd;
        }

        return rcContainer;
    }


	bool SouiLayoutParam::IsMatchParent(ORIENTATION orientation) const
	{
		return orientation == Vert ?(m_height == SIZE_MATCH_PARENT):(m_width == SIZE_MATCH_PARENT);
	}

	bool SouiLayoutParam::IsSpecifiedSize(ORIENTATION orientation) const
	{
		return orientation == Vert ?(m_height > SIZE_SPEC):(m_width > SIZE_SPEC);
	}

	int SouiLayoutParam::GetSpecifiedSize(ORIENTATION orientation) const
	{
		return orientation == Vert ?(m_height):(m_width);
	}

	HRESULT SouiLayoutParam::OnAttrOffset(const SStringW & strValue,BOOL bLoading)
	{
		float fx,fy;
		if(2!=swscanf(strValue,L"%f,%f",&fx,&fy))
		{
			return E_FAIL;
		}
		fOffsetX = fx;
		fOffsetY = fy;
		return S_OK;
	}

	BOOL SouiLayoutParam::ParsePosition12( const SStringW & strPos1, const SStringW &strPos2 )
	{
		if(strPos1.IsEmpty() || strPos2.IsEmpty()) 
			return FALSE;
		POSITION_ITEM pos1,pos2;
		if(!StrPos2ItemPos(strPos1,pos1) || !StrPos2ItemPos(strPos2,pos2) )
			return FALSE;
		if(pos1.pit == PIT_SIZE || pos2.pit == PIT_SIZE)//前面2个属性不能是size类型
			return FALSE;
		pos [PI_LEFT] = pos1;
		pos [PI_TOP] = pos2;
		nCount = 2;
		return TRUE;
	}

	BOOL SouiLayoutParam::ParsePosition34( const SStringW & strPos3, const SStringW &strPos4 )
	{
		if(strPos3.IsEmpty() || strPos4.IsEmpty()) return FALSE;
		POSITION_ITEM pos3,pos4;
		if(!StrPos2ItemPos(strPos3,pos3) || !StrPos2ItemPos(strPos4,pos4) ) return FALSE;

		pos [PI_RIGHT] = pos3;
		pos [PI_BOTTOM] = pos4;
		nCount = 4;
		return TRUE;
	}

	BOOL SouiLayoutParam::StrPos2ItemPos( const SStringW &strPos,POSITION_ITEM & pos )
	{
		if(strPos.IsEmpty()) return FALSE;

		if(strPos.Left(4)==L"sib.")
		{
			int nOffset = 0;
			if(strPos.Mid(4,5) == L"left@")
			{
				pos.pit = PIT_SIB_LEFT;
				nOffset = 5;

			}else if(strPos.Mid(4,6) == L"right@")
			{
				pos.pit = PIT_SIB_RIGHT;
				nOffset = 6;
			}else if(strPos.Mid(4,4) == L"top@")
			{
				pos.pit = PIT_SIB_TOP;
				nOffset = 4;
			}else if(strPos.Mid(4,7) == L"bottom@")
			{
				pos.pit = PIT_SIB_BOTTOM;
				nOffset = 7;
			}else
			{
				return FALSE;
			}
			int nSibID = 0;
			int nValue = 0;
			SStringW strValue = strPos.Mid(4+nOffset);
			if(2 != swscanf(strValue,L"%d:%d",&nSibID,&nValue))
				return FALSE;
			if(nSibID == 0) 
				return FALSE;

			pos.nRefID = nSibID;
			if(nValue < 0)
			{
				pos.nPos = (float)(-nValue);
				pos.cMinus = -1;
			}else
			{
				pos.nPos = (float)nValue;
				pos.cMinus = 1;
			}
		}else
		{
			LPCWSTR pszPos = strPos;
			switch(pszPos[0])
			{
			case POSFLAG_REFCENTER: pos.pit=PIT_CENTER,pszPos++;break;
			case POSFLAG_PERCENT: pos.pit=PIT_PERCENT,pszPos++;break;
			case POSFLAG_REFPREV_NEAR: pos.pit=PIT_PREV_NEAR,pszPos++;break;
			case POSFLAG_REFNEXT_NEAR: pos.pit=PIT_NEXT_NEAR,pszPos++;break;
			case POSFLAG_REFPREV_FAR: pos.pit=PIT_PREV_FAR,pszPos++;break;
			case POSFLAG_REFNEXT_FAR: pos.pit=PIT_NEXT_FAR,pszPos++;break;
			case POSFLAG_SIZE:pos.pit=PIT_SIZE,pszPos++;break;
			default: pos.pit=PIT_NORMAL;break;
			}

			pos.nRefID = -1;//not ref sibling using id
			if(pszPos [0] == L'-')
			{
				pos.cMinus = -1;
				pszPos ++;
			}else
			{
				pos.cMinus = 1;
			}
			pos.nPos=(float)_wtof(pszPos);
		}

		return TRUE;
	}

	HRESULT SouiLayoutParam::OnAttrPos(const SStringW & strValue,BOOL bLoading)
	{
		SStringWList strLst;
		SplitString(strValue,L',',strLst);
		if(strLst.GetCount() != 2 && strLst.GetCount() != 4) 
		{
			SASSERT_FMTW(L"Parse pos attribute failed, strPos=%s",strValue);
			return E_FAIL;
		}
		//增加pos属性中的空格兼容。
		for(size_t i=0;i<strLst.GetCount();i++)
		{
			strLst.GetAt(i).TrimBlank();
		}
		BOOL bRet = TRUE;

		bRet = ParsePosition12(strLst[0],strLst[1]);
		if(strLst.GetCount() == 4)
		{
			bRet = ParsePosition34(strLst[2],strLst[3]);
		}
		if(bRet && nCount == 4)
		{//检测X,Y方向上是否为充满父窗口
			if((pos[0].pit == PIT_NORMAL && pos[0].nPos == 0 && pos[0].cMinus==1)
				&&(pos[2].pit == PIT_NORMAL && pos[2].nPos == 0 && pos[2].cMinus==-1))
			{
				m_width = SIZE_MATCH_PARENT;
			}else if(pos[2].pit == PIT_SIZE)
			{   
				if(pos[2].cMinus == -1)
					m_width = SIZE_WRAP_CONTENT;
				else
					m_width = (int)pos[2].nPos;
			}

			if((pos[1].pit == PIT_NORMAL && pos[1].nPos == 0 && pos[1].cMinus==1)
				&&(pos[3].pit == PIT_NORMAL && pos[3].nPos == 0 && pos[3].cMinus==-1))
			{
				m_height = SIZE_MATCH_PARENT;
			}
			else if(pos[3].pit == PIT_SIZE)
			{
				if(pos[3].cMinus == -1)
					m_height = SIZE_WRAP_CONTENT;
				else
					m_height = (int)pos[3].nPos;
			}
		}

		return S_OK;
	}

	HRESULT SouiLayoutParam::OnAttrSize(const SStringW & strValue,BOOL bLoading)
	{
		SStringWList values;
		if(2!=SplitString(strValue,L',',values))
			return E_FAIL;
		OnAttrWidth(values[0],bLoading);
		OnAttrHeight(values[1],bLoading);
		return S_OK;
	}

	HRESULT SouiLayoutParam::OnAttrHeight(const SStringW & strValue,BOOL bLoading)
	{
		if(strValue.CompareNoCase(L"matchParent") == 0 || strValue.CompareNoCase(L"full") == 0)
			m_height = SIZE_MATCH_PARENT;
		else if(strValue.CompareNoCase(L"wrapContent") == 0)
			m_height = SIZE_WRAP_CONTENT;
		else
			m_height = _wtoi(strValue);
		return S_OK;
	}

	HRESULT SouiLayoutParam::OnAttrWidth(const SStringW & strValue,BOOL bLoading)
	{
		if(strValue.CompareNoCase(L"matchParent") == 0 || strValue.CompareNoCase(L"full") == 0)
			m_width = SIZE_MATCH_PARENT;
		else if(strValue.CompareNoCase(L"wrapContent") == 0)
			m_width = SIZE_WRAP_CONTENT;
		else
			m_width = _wtoi(strValue);
		return S_OK;
	}



    void SouiLayout::_MeasureChildren(SList<WndPos> *pListChildren,int nWidth,int nHeight,int & nNewWidth,int & nNewHeight)
    {
        if(pListChildren->IsEmpty()) return;
        int nResolved = 0;
        for(SPOSITION pos = pListChildren->GetHeadPosition();pos;)
        {
            WndPos wndPos = pListChildren->GetNext(pos);
            SouiLayoutParam *pLayoutParam = wndPos.pWnd->GetLayoutParam<SouiLayoutParam>();
            if(wndPos.rc.left == POS_INIT || wndPos.rc.left == POS_WAIT) 
            {
                wndPos.rc.left = CalcChildLeft(wndPos.pWnd,pLayoutParam);
                if(wndPos.rc.left != POS_WAIT) nResolved ++;
            }
            if(wndPos.rc.right == POS_INIT || wndPos.rc.right == POS_WAIT) 
            {
                wndPos.rc.right = CalcChildRight(wndPos.pWnd,pLayoutParam);
                if(wndPos.rc.right != POS_WAIT) nResolved ++;
            }
            if(wndPos.rc.top == POS_INIT || wndPos.rc.top == POS_WAIT) 
            {
                wndPos.rc.top = CalcChildTop(wndPos.pWnd,pLayoutParam);
                if(wndPos.rc.top != POS_WAIT) nResolved ++;
            }
            if(wndPos.rc.bottom == POS_INIT || wndPos.rc.bottom == POS_WAIT) 
            {
                wndPos.rc.bottom = CalcChildBottom(wndPos.pWnd,pLayoutParam);
                if(wndPos.rc.bottom != POS_WAIT) nResolved ++;
            }
        }
        if(nResolved == 0)
        {
            return;//error
        }

    }


}

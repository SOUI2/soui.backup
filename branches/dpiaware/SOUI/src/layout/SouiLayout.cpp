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


    int SouiLayout::PositionItem2Value(SList<WndPos> *pLstChilds,SPOSITION position,const POSITION_ITEM &pos , int nMax,BOOL bX)
    {
        int nRet=POS_WAIT;

        switch(pos.pit)
        {
        case PIT_CENTER: //参考中心
            if(nMax != SIZE_WRAP_CONTENT) nRet=(int)pos.nPos * pos.cMinus + nSize/2;
            break;
        case PIT_NORMAL: 
            if(pos.cMinus == -1)
			{//参考右边或者下边
				if(nMax != SIZE_WRAP_CONTENT) nRet=nMax-(int)pos.nPos;
			}else
			{
				nRet=(int)pos.nPos;
			}
            break;
        case PIT_PERCENT: 
			if(nMax != SIZE_WRAP_CONTENT)
			{
				float fPercent = pos.nPos;
				if(fPercent<0.0f) fPercent = 0.0f;
				if(fPercent>1.0f) fPercent = 1.0f;
				if(pos.cMinus == -1)
					nRet=(int)((100.0f-pos.nPos)*nMax/100);
				else
					nRet=(int)(pos.nPos*nMax/100);
			}
            break;
        case PIT_PREV_NEAR:
        case PIT_PREV_FAR:
            {
				SPOSITION positionPrev = pLstChilds->Prev(position);
				int nRef = POS_WAIT;
				if(positionPrev)
				{
					WndPos wndPos = pLstChilds->GetAt(positionPrev);
					if(bX)
					{
						if(!wndPos.bWaitOffsetX) nRef = (pos.pit == PIT_PREV_NEAR)?wndPos.rc.right:wndPos.rc.left;
					}
					else
					{
						if(!wndPos.bWaitOffsetY) nRef = (pos.pit == PIT_PREV_NEAR)?wndPos.rc.bottom:wndPos.rc.top;
					}
				}else
				{
					nRef = 0;
				}
				if(!IsWaitingPos(nRef))
					nRet=nRef+(int)pos.nPos*pos.cMinus;
            }
            break;
        case PIT_NEXT_NEAR:
        case PIT_NEXT_FAR:
			{
				SPOSITION positionNext = pLstChilds->Next(position);
				int nRef = nMax;
				if(positionNext)
				{
					nRef = POS_WAIT;
					WndPos wndPos = pLstChilds->GetAt(positionNext);
					if(bX)
					{
						if(!wndPos.bWaitOffsetX) nRef = (pos.pit == PIT_PREV_NEAR)?wndPos.rc.left:wndPos.rc.right;
					}
					else
					{
						if(!wndPos.bWaitOffsetY) nRef = (pos.pit == PIT_PREV_NEAR)?wndPos.rc.top:wndPos.rc.bottom;
					}
				}
				if(!IsWaitingPos(nRef))
					nRet=nRef+(int)pos.nPos*pos.cMinus;
			}
            break;
        case PIT_SIB_LEFT:// PIT_SIB_LEFT == PIT_SIB_TOP
        case PIT_SIB_RIGHT://PIT_SIB_RIGHT == PIT_SIB_BOTTOM
            {
				WndPos wndPos = pLstChilds->GetAt(position);
				SASSERT(pos.nRefID>0);

				WndPos wndPosRef = {0};
				SPOSITION posTmp = pLstChilds->GetHeadPosition();
				while(posTmp)
				{
					WndPos wp = pLstChilds->GetNext(posTmp);
					if(wp.pWnd->GetID() == pos.nRefID)
					{
						wndPosRef = wp;
						break;
					}
				}
				if(!wndPosRef.pWnd)
				{//没有找到时,使用父窗口信息
					wndPosRef.rc = CRect(0,0,nMax,nMax);
					wndPosRef.bWaitOffsetX = wndPosRef.bWaitOffsetY = false;
				}
				CRect rcRef = wndPosRef.rc;

                if(bX)
                {
					if(!wndPosRef.bWaitOffsetX)
					{
						LONG refPos = (pos.pit == PIT_SIB_LEFT)?rcRef.left:rcRef.right;
						if(IsWaitingPos(refPos))
							nRet=POS_WAIT;
						else
							nRet=refPos+(int)pos.nPos*pos.cMinus;
					}
                }else
                {
					if(!wndPosRef.bWaitOffsetY)
					{
						LONG refPos = (pos.pit == PIT_SIB_TOP)?rcRef.top:rcRef.bottom;//PIT_SIB_TOP == PIT_SIB_LEFT
						if(IsWaitingPos(refPos))
							nRet=POS_WAIT;
						else
							nRet=refPos+(int)pos.nPos*pos.cMinus;
					}
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

	bool fequal(float a,float b)
	{
		return fabs(a-b) < 0.0000001;
	}

    CSize SouiLayout::MeasureChildren(SWindow * pParent,int nWidth,int nHeight) const
    {
        SList<WndPos>       lstWndPos;

        SWindow *pChild=GetWindow(GSW_FIRSTCHILD);
        while(pChild)
        {
            if(!pChild->IsFloat() && (pChild->IsVisible(FALSE) || pChild->IsDisplay()))
            {//不显示且不占位的窗口不参与计算
                WndPos wndPos;
                wndPos.pWnd = pChild;
                wndPos.rc = CRect(POS_INIT,POS_INIT,POS_INIT,POS_INIT);
				SouiLayoutParam *pParam = pChild->GetLayoutParam<SouiLayoutParam>();
				wndPos.bWaitOffsetX = pParam->IsOffsetRequired(Horz);
				wndPos.bWaitOffsetY = pParam->IsOffsetRequired(Vert);
                lstWndPos.AddTail(wndPos);
            }
            pChild=pChild->GetWindow(GSW_NEXTSIBLING);
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

	bool SouiLayoutParam::IsWrapContent(ORIENTATION orientation) const
	{
		return orientation == Vert ?(m_height == SIZE_WRAP_CONTENT):(m_width == SIZE_WRAP_CONTENT);
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

	bool SouiLayoutParam::IsOffsetRequired(ORIENTATION orientation) const
	{
		return fabs(orientation==Vert?fOffsetY:fOffsetX) < 0.0000001f;
	}


    void SouiLayout::_MeasureChildren1(SList<WndPos> *pListChildren,int nWidth,int nHeight)
    {
		//step 1:计算出所有不需要计算窗口大小就可以确定的坐标
		int nResolved = 0;
		do{
			nResolved = 0;
			for(SPOSITION pos = pListChildren->GetHeadPosition();pos;pListChildren->GetNext(pos))
			{
				WndPos &wndPos = pListChildren->GetAt(pos);
				SouiLayoutParam *pLayoutParam = wndPos.pWnd->GetLayoutParam<SouiLayoutParam>();
				if(IsWaitingPos(wndPos.rc.left)) 
				{
					wndPos.rc.left = PositionItem2Value(pListChildren,pos,pLayoutParam->pos[0],nWidth,TRUE);
					if(wndPos.rc.left != POS_WAIT) nResolved ++;
				}
				if(IsWaitingPos(wndPos.rc.top)) 
				{
					wndPos.rc.top = PositionItem2Value(pListChildren,pos,pLayoutParam->pos[1],nHeight,FALSE);
					if(wndPos.rc.top != POS_WAIT) nResolved ++;
				}
				if(IsWaitingPos(wndPos.rc.right)) 
				{
					if(pLayoutParam->IsSpecifiedSize(Horz))
					{
						if(!IsWaitingPos(wndPos.rc.left))
						{
							wndPos.rc.right = wndPos.rc.left + pLayoutParam->GetSpecifiedSize(Horz);
							nResolved ++;
						}
					}else if(!pLayoutParam->IsWrapContent(Horz))
					{
						wndPos.rc.right = PositionItem2Value(pListChildren,pos,pLayoutParam->pos[2],nWidth,TRUE);
						if(wndPos.rc.right != POS_WAIT) nResolved ++;
					}
				}
				if(IsWaitingPos(wndPos.rc.bottom)) 
				{
					if(pLayoutParam->IsSpecifiedSize(Vert))
					{
						if(!IsWaitingPos(wndPos.rc.top))
						{
							wndPos.rc.bottom = wndPos.rc.top + pLayoutParam->GetSpecifiedSize(Vert);
							nResolved ++;
						}
					}else if(!pLayoutParam->IsWrapContent(Vert))
					{
						wndPos.rc.bottom = PositionItem2Value(pListChildren,pos,pLayoutParam->pos[3],nHeight,FALSE);
						if(wndPos.rc.bottom != POS_WAIT) nResolved ++;
					}
				}

			}
		}while(nResolved);

		//step 2:计算出自适应大小窗口的Size,对于可以确定的窗口完成offset操作
		do{
			nResolved = 0;
			for(SPOSITION pos = pListChildren->GetHeadPosition();pos;pListChildren->GetNext(pos))
			{
				WndPos &wndPos = pListChildren->GetAt(pos);
				SouiLayoutParam *pLayoutParam = wndPos.pWnd->GetLayoutParam<SouiLayoutParam>();
				if(IsWaitingPos(wndPos.rc.left) || IsWaitingPos(wndPos.rc.top)) continue;//至少确定了一个点后才开始计算

				if(pLayoutParam->IsWrapContent(Horz) || pLayoutParam->IsWrapContent(Vert))
				{//
					int nWid = IsWaitingPos(wndPos.rc.right)? SIZE_WRAP_CONTENT : (wndPos.rc.right - wndPos.rc.left);
					int nHei = IsWaitingPos(wndPos.rc.bottom)? SIZE_WRAP_CONTENT : (wndPos.rc.bottom - wndPos.rc.top);
					CSize szWnd = wndPos.pWnd->GetDesiredSize(nWid,nHei);
					if(pLayoutParam->IsWrapContent(Horz)) 
					{
						wndPos.rc.right = wndPos.rc.left + szWnd.cx;
						nResolved ++;
					}
					if(pLayoutParam->IsWrapContent(Vert)) 
					{
						wndPos.rc.bottom = wndPos.rc.top + szWnd.cy;
						nResolved ++;
					}
				}
				if(!IsWaitingPos(wndPos.rc.right) && wndPos.bWaitOffsetX)
				{
					wndPos.rc.OffsetRect(wndPos.rc.Width()*pLayoutParam->fOffsetX,0);
					wndPos.bWaitOffsetX=false;
				}
				if(!IsWaitingPos(wndPos.rc.bottom) && wndPos.bWaitOffsetY)
				{
					wndPos.rc.OffsetRect(wndPos.rc.Height()*pLayoutParam->fOffsetY,0);
					wndPos.bWaitOffsetY=false;
				}
			}
		}while(nResolved);

		//step 3:重复step 1
    }



}

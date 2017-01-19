#include "souistd.h"
#include "layout\SouiLayout.h"
#include <math.h>

namespace SOUI{
    enum
    {
        POS_INIT=0x11000000,    //坐标的初始化值
        POS_WAIT=0x12000000,    //坐标的计算依赖于其它窗口的布局
    };


	SouiLayoutParam::SouiLayoutParam()
	{
		Clear();
	}

    bool SouiLayoutParam::IsMatchParent(ORIENTATION orientation) const
    {
        switch(orientation)
        {
        case Horz:
            return m_width == SIZE_MATCH_PARENT;
        case Vert:
            return m_height == SIZE_MATCH_PARENT;
        case Any:
            return IsMatchParent(Horz) || IsMatchParent(Vert);
        case Both:
        default:
            return IsMatchParent(Horz) && IsMatchParent(Vert);
        }
    }

    bool SouiLayoutParam::IsSpecifiedSize(ORIENTATION orientation) const
    {
        switch(orientation)
        {
        case Horz:
            return m_width >= SIZE_SPEC;
        case Vert:
            return m_height >= SIZE_SPEC;
        case Any:
            return IsSpecifiedSize(Horz) || IsSpecifiedSize(Vert);
        case Both:
        default:
            return IsSpecifiedSize(Horz) && IsSpecifiedSize(Vert);
        }
    }

    bool SouiLayoutParam::IsWrapContent(ORIENTATION orientation) const
    {
        switch(orientation)
        {
        case Horz:
            return m_width == SIZE_WRAP_CONTENT || nCount == 0;
        case Vert:
            return m_height == SIZE_WRAP_CONTENT|| nCount == 0;
        case Any:
            return IsWrapContent(Horz) || IsWrapContent(Vert);
        case Both:
        default:
            return IsWrapContent(Horz) && IsWrapContent(Vert);
        }
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
		}else
		{
			SetWrapContent(Horz);
			SetWrapContent(Vert);
		}


        return S_OK;
    }

    HRESULT SouiLayoutParam::OnAttrSize(const SStringW & strValue,BOOL bLoading)
    {
        SIZE sz;
        if(2!=swscanf(strValue,L"%d,%d",&sz.cx,&sz.cy)) return E_FAIL;
        m_width = sz.cx;
        m_height = sz.cy;
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
        return fabs(orientation==Vert?fOffsetY:fOffsetX) > 0.00000001f;
    }

    int GetPosExtra(const POSITION_ITEM &pos)
    {
        return pos.cMinus?(int)pos.nPos:0;
    }

    int SouiLayoutParam::GetExtraSize(ORIENTATION orientation) const
    {
        if(orientation == Vert)
            return GetPosExtra(pos[2]);
        else
            return GetPosExtra(pos[3]);
    }

	void SouiLayoutParam::Clear()
	{
		nCount = 0;
		fOffsetX = fOffsetY = 0.0f;

		m_width = m_height = SIZE_UNDEF;
	}

	void SouiLayoutParam::SetMatchParent(ORIENTATION orientation)
	{
        switch(orientation)
        {
        case Horz:
            m_width = SIZE_MATCH_PARENT;
            break;
        case Vert:
            m_height = SIZE_MATCH_PARENT;
            break;
        case Both:
            m_width = m_height = SIZE_MATCH_PARENT;
            break;
        }
	}

	void SouiLayoutParam::SetWrapContent(ORIENTATION orientation)
	{
		switch(orientation)
        {
        case Horz:
			m_width = SIZE_WRAP_CONTENT;
            break;
        case Vert:
			m_height = SIZE_WRAP_CONTENT;
            break;
        case Both:
            m_width = m_height = SIZE_WRAP_CONTENT;
            break;
        }
	}

	void SouiLayoutParam::SetSpecifiedSize(ORIENTATION orientation, int nSize)
	{
        switch(orientation)
        {
        case Horz:
            m_width = nSize;
            break;
        case Vert:
            m_height = nSize;
            break;
        case Both:
            m_width = m_height = nSize;
            break;
        }
	}

    //////////////////////////////////////////////////////////////////////////

	SouiLayout::SouiLayout(void)
	{
	}

	SouiLayout::~SouiLayout(void)
	{
	}

    bool SouiLayout::IsParamAcceptable(ILayoutParam *pLayoutParam) const
    {
        return !!pLayoutParam->IsClass(SouiLayoutParam::GetClassName());
    }

    ILayoutParam * SouiLayout::CreateLayoutParam() const
    {
		ILayoutParam * pRet = NULL;
		CreateLayoutParam((IObjRef**)&pRet);
        return pRet;
    }

	HRESULT SouiLayout::CreateLayoutParam(IObjRef ** ppObj)
	{
		* ppObj = new SouiLayoutParam();
		return S_OK;
	}

	HRESULT SouiLayout::CreateLayout(IObjRef ** ppObj)
	{
		* ppObj = new SouiLayout();
		return S_OK;
	}

    BOOL SouiLayout::IsWaitingPos( int nPos ) const
    {
        return nPos == POS_INIT || nPos == POS_WAIT || nPos == SIZE_WRAP_CONTENT;
    }


    int SouiLayout::PositionItem2Value(SList<WndPos> *pLstChilds,SPOSITION position,const POSITION_ITEM &pos , int nMax,BOOL bX) const
    {
        int nRet=POS_WAIT;

        switch(pos.pit)
        {
        case PIT_CENTER: //参考中心
            if(nMax != SIZE_WRAP_CONTENT) nRet=(int)pos.nPos * pos.cMinus + nMax/2;
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
						if(!wndPos.bWaitOffsetX) nRef = (pos.pit == PIT_NEXT_NEAR)?wndPos.rc.left:wndPos.rc.right;
					}
					else
					{
						if(!wndPos.bWaitOffsetY) nRef = (pos.pit == PIT_NEXT_NEAR)?wndPos.rc.top:wndPos.rc.bottom;
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

    CSize SouiLayout::MeasureChildren(SWindow * pParent,int nWidth,int nHeight) const
    {
        SList<WndPos>       lstWndPos;

        SWindow *pChild= pParent->GetWindow(GSW_FIRSTCHILD);
        while(pChild)
        {
            if(!pChild->IsFloat() && (pChild->IsVisible(FALSE) || pChild->IsDisplay()))
            {//不显示且不占位的窗口不参与计算
                WndPos wndPos;
                wndPos.pWnd = pChild;
                wndPos.rc = CRect(POS_INIT,POS_INIT,POS_INIT,POS_INIT);
				SouiLayoutParam *pParam = pChild->GetLayoutParamT<SouiLayoutParam>();
				wndPos.bWaitOffsetX = pParam->IsOffsetRequired(Horz);
				wndPos.bWaitOffsetY = pParam->IsOffsetRequired(Vert);
                lstWndPos.AddTail(wndPos);
            }
            pChild=pChild->GetWindow(GSW_NEXTSIBLING);
        }
        
        //计算子窗口位置
        CalcPositionEx(&lstWndPos,nWidth,nHeight);

        //计算子窗口范围
        int nMaxX = 0,nMaxY = 0;
        SPOSITION pos = lstWndPos.GetHeadPosition();
        while(pos)
        {
            WndPos wndPos = lstWndPos.GetNext(pos);
            SouiLayoutParam *pParam = wndPos.pWnd->GetLayoutParamT<SouiLayoutParam>();
            if(!IsWaitingPos(wndPos.rc.right))
            {
                nMaxX = max(nMaxX,wndPos.rc.right + pParam->GetExtraSize(Horz));
            }
            if(!IsWaitingPos(wndPos.rc.bottom))
            {
                nMaxY = max(nMaxX,wndPos.rc.bottom + pParam->GetExtraSize(Vert));
            }
        }

        if(!IsWaitingPos(nWidth)) nWidth = nMaxX;
        if(!IsWaitingPos(nHeight)) nHeight = nMaxY;
        return CSize(nWidth,nHeight);
    }



    /*
    计算子窗口容器大小逻辑：
    1:引用父窗口左上角的窗口称之为I类确定性窗口。
    2:引用I类窗口的窗口称为II类确定性窗口。
    3:左边引用父窗口左上角或者I,II类确定性窗口，右边引用父窗口右下角的窗口为I不确定性窗口，这类窗口自动转换成自适应大小窗口。
    4:左右都引用父窗口右下角的窗口为II类不确定窗口，这类窗口不影响父窗口大小。
    5:引用I,II类不确定大小窗口的窗口同样不影响父窗口大小。

    只要一个控件左边位置能确定，控件的右边也可以保证可以确定。
    如果左边位置不能确定，则控件大小不影响父窗口大小。
    */
    void SouiLayout::CalcPositionEx(SList<WndPos> *pListChildren,int nWidth,int nHeight) const
    {
        CalcPostion(pListChildren,nWidth,nHeight);

        //将参考父窗口右边或者底边的子窗口设置为wrap_content并计算出大小

        int nResolved = 0;
        for(SPOSITION pos = pListChildren->GetHeadPosition();pos;pListChildren->GetNext(pos))
        {
            WndPos &wndPos = pListChildren->GetAt(pos);
            SouiLayoutParam *pLayoutParam = wndPos.pWnd->GetLayoutParamT<SouiLayoutParam>();
            if(!IsWaitingPos(wndPos.rc.left) &&
                !IsWaitingPos(wndPos.rc.top) &&
                (IsWaitingPos(wndPos.rc.right) && IsWaitingPos(nWidth) || 
                IsWaitingPos(wndPos.rc.bottom) && IsWaitingPos(nHeight)))
            {
                int nWid = IsWaitingPos(wndPos.rc.right)? nWidth : (wndPos.rc.right - wndPos.rc.left);
                int nHei = IsWaitingPos(wndPos.rc.bottom)? nHeight : (wndPos.rc.bottom - wndPos.rc.top);
                CSize szWnd = wndPos.pWnd->GetDesiredSize(nWid,nHei);
                if(pLayoutParam->IsWrapContent(Horz)) 
                {
                    wndPos.rc.right = wndPos.rc.left + szWnd.cx;
                    if(wndPos.bWaitOffsetX)
                    {
                        wndPos.rc.OffsetRect((int)(wndPos.rc.Width()*pLayoutParam->fOffsetX),0);
                        wndPos.bWaitOffsetX=false;
                    }
                    nResolved ++;
                }
                if(pLayoutParam->IsWrapContent(Vert)) 
                {
                    wndPos.rc.bottom = wndPos.rc.top + szWnd.cy;
                    if(wndPos.bWaitOffsetY)
                    {
                        wndPos.rc.OffsetRect(0,(int)(wndPos.rc.Height()*pLayoutParam->fOffsetY));
                        wndPos.bWaitOffsetY=false;
                    }
                    nResolved ++;
                }
            }
        }

    }


	static const POSITION_ITEM posRefLeft={PIT_PREV_NEAR,-1,1,0};
	static const POSITION_ITEM posRefTop={PIT_PREV_FAR,-1,1,0};

    int SouiLayout::CalcPostion(SList<WndPos> *pListChildren,int nWidth,int nHeight) const
    {
        int nResolvedAll=0;

        int nResolvedStep1 = 0;
        int nResolvedStep2 = 0;
        do{
            nResolvedStep1 = 0;
            nResolvedStep2 = 0;

            //step 1:计算出所有不需要计算窗口大小就可以确定的坐标
            int nResolved = 0;
            do{
                nResolved = 0;
                for(SPOSITION pos = pListChildren->GetHeadPosition();pos;pListChildren->GetNext(pos))
                {
                    WndPos &wndPos = pListChildren->GetAt(pos);
                    SouiLayoutParam *pLayoutParam = wndPos.pWnd->GetLayoutParamT<SouiLayoutParam>();
                    if(IsWaitingPos(wndPos.rc.left)) 
                    {
						const POSITION_ITEM &posRef = pLayoutParam->nCount>=2 ? pLayoutParam->pos[0]:posRefLeft;
                        wndPos.rc.left = PositionItem2Value(pListChildren,pos,posRef,nWidth,TRUE);
                        if(wndPos.rc.left != POS_WAIT) nResolved ++;
                    }
                    if(IsWaitingPos(wndPos.rc.top)) 
                    {
						const POSITION_ITEM &posRef = pLayoutParam->nCount>=2 ? pLayoutParam->pos[1]:posRefTop;
                        wndPos.rc.top = PositionItem2Value(pListChildren,pos,posRef,nHeight,FALSE);
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
                        }else if(!pLayoutParam->IsWrapContent(Horz) && pLayoutParam->nCount==4)
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
                        }else if(!pLayoutParam->IsWrapContent(Vert) && pLayoutParam->nCount==4)
                        {
                            wndPos.rc.bottom = PositionItem2Value(pListChildren,pos,pLayoutParam->pos[3],nHeight,FALSE);
                            if(wndPos.rc.bottom != POS_WAIT) nResolved ++;
                        }
                    }

                }

                nResolvedStep1 += nResolved;
            }while(nResolved);

            if(nResolvedStep1>0)
            {
                int nResolved = 0;
                //step 2:计算出自适应大小窗口的Size,对于可以确定的窗口完成offset操作
                do{
                    nResolved = 0;
                    for(SPOSITION pos = pListChildren->GetHeadPosition();pos;pListChildren->GetNext(pos))
                    {
                        WndPos &wndPos = pListChildren->GetAt(pos);
                        SouiLayoutParam *pLayoutParam = wndPos.pWnd->GetLayoutParamT<SouiLayoutParam>();
                        if(IsWaitingPos(wndPos.rc.left) || IsWaitingPos(wndPos.rc.top)) continue;//至少确定了一个点后才开始计算

                        if((IsWaitingPos(wndPos.rc.right) && pLayoutParam->IsWrapContent(Horz)) 
							|| (IsWaitingPos(wndPos.rc.bottom) && pLayoutParam->IsWrapContent(Vert)))
                        {//
                            int nWid = IsWaitingPos(wndPos.rc.right)? nWidth : (wndPos.rc.right - wndPos.rc.left);
                            int nHei = IsWaitingPos(wndPos.rc.bottom)? nHeight : (wndPos.rc.bottom - wndPos.rc.top);
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
                            wndPos.rc.OffsetRect((int)(wndPos.rc.Width()*pLayoutParam->fOffsetX),0);
                            wndPos.bWaitOffsetX=false;
                        }
                        if(!IsWaitingPos(wndPos.rc.bottom) && wndPos.bWaitOffsetY)
                        {
                            wndPos.rc.OffsetRect(0,(int)(wndPos.rc.Height()*pLayoutParam->fOffsetY));
                            wndPos.bWaitOffsetY=false;
                        }
                    }
                    nResolvedStep2 += nResolved;
                }while(nResolved);
            }//end if(nResolvedStep1>0)

            nResolvedAll += nResolvedStep1 + nResolvedStep2;
        }while(nResolvedStep2 || nResolvedStep1);
        
        return nResolvedAll;
    }

	void SouiLayout::LayoutChildren(SWindow * pParent)
	{
		SList<WndPos>       lstWndPos;

		SWindow *pChild=pParent->GetWindow(GSW_FIRSTCHILD);
		while(pChild)
		{
			if(!pChild->IsFloat() && (pChild->IsVisible(FALSE) || pChild->IsDisplay()))
			{//不显示且不占位的窗口不参与计算
				WndPos wndPos;
				wndPos.pWnd = pChild;
				wndPos.rc = CRect(POS_INIT,POS_INIT,POS_INIT,POS_INIT);
				SouiLayoutParam *pParam = pChild->GetLayoutParamT<SouiLayoutParam>();
				wndPos.bWaitOffsetX = pParam->IsOffsetRequired(Horz);
				wndPos.bWaitOffsetY = pParam->IsOffsetRequired(Vert);
				lstWndPos.AddTail(wndPos);
			}
			pChild=pChild->GetWindow(GSW_NEXTSIBLING);
		}

		if(lstWndPos.IsEmpty())
			return;

		CRect rcParent = pParent->GetClientRect();
		//计算子窗口位置
		CalcPostion(&lstWndPos,rcParent.Width(),rcParent.Height());

		//偏移窗口坐标
		SPOSITION pos = lstWndPos.GetHeadPosition();
		while(pos)
		{
			WndPos wp = lstWndPos.GetNext(pos);
			wp.rc.OffsetRect(rcParent.TopLeft());
			wp.pWnd->OnRelayout(wp.rc);
		}
	}


}

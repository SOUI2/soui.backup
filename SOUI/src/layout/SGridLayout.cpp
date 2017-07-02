#include "souistd.h"
#include "layout\SGridLayout.h"

namespace SOUI
{
	HRESULT SGridLayoutParam::OnAttrSize(const SStringW & strValue,BOOL bLoading)
	{
		SStringWList szStr ;
		if(2!=SplitString(strValue,L',',szStr)) return E_FAIL;

		OnAttrWidth(szStr[0],bLoading);
		OnAttrHeight(szStr[1],bLoading);
		return S_OK;
	}

	HRESULT SGridLayoutParam::OnAttrWidth(const SStringW & strValue,BOOL bLoading)
	{
		if(strValue.CompareNoCase(L"matchParent") == 0)
			width.setMatchParent();
		else if(strValue.CompareNoCase(L"wrapContent") == 0)
			width.setWrapContent();
		else
			width.parseString(strValue);
		return S_OK;
	}

	HRESULT SGridLayoutParam::OnAttrHeight(const SStringW & strValue,BOOL bLoading)
	{
		if(strValue.CompareNoCase(L"matchParent") == 0)
			height.setMatchParent();
		else if(strValue.CompareNoCase(L"wrapContent") == 0)
			height.setWrapContent();
		else
			height.parseString(strValue);
		return S_OK;
	}

	SGridLayoutParam::SGridLayoutParam()
	{
		Clear();
	}


	void SGridLayoutParam::Clear()
	{
		width.setWrapContent();
		height.setWrapContent();
		iRow=iCol=-1;
		nColSpan=nRowSpan=1;
		xGravity = gCenter;
		yGravity = gMiddle;
	}

	void SGridLayoutParam::SetMatchParent(ORIENTATION orientation)
	{
		switch(orientation)
		{
		case Horz:
			width.setMatchParent();
			break;
		case Vert:
			height.setMatchParent();
			break;
		case Both:
			width.setMatchParent();
			height.setMatchParent();
			break;
		}
	}

	void SGridLayoutParam::SetWrapContent(ORIENTATION orientation)
	{
		switch(orientation)
		{
		case Horz:
			width.setWrapContent();
			break;
		case Vert:
			height.setWrapContent();
			break;
		case Both:
			width.setWrapContent();
			height.setWrapContent();
			break;
		}
	}

	void SGridLayoutParam::SetSpecifiedSize(ORIENTATION orientation, const SLayoutSize& layoutSize)
	{
		switch(orientation)
		{
		case Horz:
			width = layoutSize;
			break;
		case Vert:
			height = layoutSize;
			break;
		case Both:
			width = height = layoutSize;
			break;
		}

	}

	bool SGridLayoutParam::IsMatchParent(ORIENTATION orientation) const
	{
		switch(orientation)
		{
		case Horz:
			return width.isMatchParent();
		case Vert:
			return height.isMatchParent();
		case Any:
			return IsMatchParent(Horz)|| IsMatchParent(Vert);
		case Both:
		default:
			return IsMatchParent(Horz) && IsMatchParent(Vert);
		}
	}

	bool SGridLayoutParam::IsWrapContent(ORIENTATION orientation) const
	{
		switch(orientation)
		{
		case Horz:
			return width.isWrapContent();
		case Vert:
			return height.isWrapContent();
		case Any:
			return IsWrapContent(Horz)|| IsWrapContent(Vert);
		case Both:
		default:
			return IsWrapContent(Horz) && IsWrapContent(Vert);
		}
	}

	bool SGridLayoutParam::IsSpecifiedSize(ORIENTATION orientation) const
	{
		switch(orientation)
		{
		case Horz:
			return width.isSpecifiedSize();
		case Vert:
			return height.isSpecifiedSize();
		case Any:
			return IsSpecifiedSize(Horz)|| IsSpecifiedSize(Vert);
		case Both:
		default:
			return IsSpecifiedSize(Horz) && IsSpecifiedSize(Vert);
		}

	}

	SLayoutSize SGridLayoutParam::GetSpecifiedSize(ORIENTATION orientation) const
	{
		switch(orientation)
		{
		case Horz:
			return width;
		case Vert:
			return height;
		case Any: 
		case Both:
		default:
			SASSERT_FMTA(FALSE,"GetSpecifiedSize can only be applied for Horz or Vert");
			return SLayoutSize();
		}
	}

	void * SGridLayoutParam::GetRawData()
	{
		return (SGridLayoutParam*)this;
	}


	//////////////////////////////////////////////////////////////////////////
	SGridLayout::SGridLayout(void)
	{
	}

	SGridLayout::~SGridLayout(void)
	{
	}

	bool SGridLayout::IsParamAcceptable(ILayoutParam *pLayoutParam) const
	{
		return !!pLayoutParam->IsClass(SGridLayoutParam::GetClassName());
	}


	ILayoutParam * SGridLayout::CreateLayoutParam() const
	{
		return new SGridLayoutParam();
	}


	/*
	* MeasureChildren 计算gridlayout的子窗口大小
	*/
	CSize SGridLayout::MeasureChildren(SWindow * pParent,int nWidth,int nHeight) const
	{
		SUNUSED(nWidth);
		SUNUSED(nHeight);
		int cells = m_nCols*m_nRows;
		CSize * pCellsSize = new CSize[cells];
		bool  * pCellsOccupy=new bool[cells];

		for(int i=0;i<cells;i++)
		{
			pCellsOccupy[i]=false;
		}

		int iRow=0,iCol=0;
		SWindow *pCell = pParent->GetNextLayoutChild(NULL);
		while(pCell)
		{
			SGridLayoutParam * pLayoutParam = pCell->GetLayoutParamT<SGridLayoutParam>();
			SASSERT(pLayoutParam);
			//将当前网络所占用的空间位置清0
			int colSpan = pLayoutParam->nColSpan;
			int rowSpan = pLayoutParam->nRowSpan;

			colSpan = smin(colSpan,m_nCols-iCol);
			rowSpan = smin(rowSpan,m_nRows-iRow);
			SASSERT(colSpan>=1);
			SASSERT(rowSpan>=1);
			SASSERT(pCellsOccupy[iRow*m_nCols+iCol]==false);//保证至少有一个空间可用
			//计算可占用空间
			for(int y=0;y<rowSpan;y++) for(int x=0;x<colSpan;x++)
			{
				int iCell = (iRow+y)*m_nCols+iCol+x;
				if(pCellsOccupy[iCell])
				{//colSpan优先
					rowSpan = y+1;
					if(y==0)
						colSpan = x+1;
					break;
				}
			}

			//计算出网络大小
			CSize szCell = pCell->GetDesiredSize(-1,-1);
			//填充网格,把大小平均分散到网格中。
			szCell.cx/=colSpan;
			szCell.cy/=rowSpan;
			for(int y=0;y<rowSpan;y++) for(int x=0;x<colSpan;x++)
			{
				int iCell = (iRow+y)*m_nCols+iCol+x;
				pCellsOccupy[iCell]=true;
				pCellsSize[iCell]=szCell;
			}
			
			//计算下一个网络的排列位置(先在当前行查找，再到下面行从0开始查找)
			bool bFind = false;
			for(int x=iCol+1;x<m_nCols;x++)
			{
				if(!pCellsOccupy[iRow*m_nCols+x])
				{
					bFind = true;
					iCol = x;
					break;
				}
			}
			for(int y=iRow+1;y<m_nRows && !bFind;y++) for(int x=0;x<m_nCols;x++)
			{
				if(!pCellsOccupy[y*m_nCols+x])
				{
					iRow = y;
					iCol = x;
					bFind = true;
					break;
				}
			}
			if(!bFind) break;
			pCell = pParent->GetNextLayoutChild(pCell);
		}

		CSize szRet;
		//计算列宽
		for(int x=0;x<m_nCols;x++)
		{
			int maxWid = 0;
			for(int y=0;y<m_nRows;y++)
			{
				int iCell=y*m_nCols+x;
				maxWid = smax(pCellsSize[iCell].cx,maxWid);
			}
			szRet.cx += maxWid;
		}
		//计算列高
		for(int y=0;y<m_nRows;y++)
		{
			int maxHei = 0;
			for(int x=0;x<m_nCols;x++)
			{
				int iCell=y*m_nCols+x;
				maxHei = smax(pCellsSize[iCell].cy,maxHei);
			}
			szRet.cy += maxHei;
		}

		delete []pCellsSize;
		delete []pCellsOccupy;
		return szRet;
	}

	void SGridLayout::LayoutChildren(SWindow * pParent)
	{
		CRect rcParent = pParent->GetChildrenLayoutRect();
		//先计算出每个格子的大小,算法和MeasureChildren一样，后面再考虑如何优化
		int cells = m_nCols*m_nRows;
		CSize * pCellsSize = new CSize[cells];
		bool  * pCellsOccupy=new bool[cells];
		float * pCellsColWeight = new float[cells];
		float * pCellsRowWeight = new float[cells];
		SWindow **pCellsChild = new SWindow*[cells];
		CPoint * pCellsSpan = new CPoint[cells];

		for(int i=0;i<cells;i++)
		{
			pCellsOccupy[i]=false;
			pCellsChild[i]=NULL;
		}

		int iRow=0,iCol=0;
		SWindow *pCell = pParent->GetNextLayoutChild(NULL);
		while(pCell)
		{
			pCellsChild[iRow*m_nCols+iCol] = pCell;
			SGridLayoutParam * pLayoutParam = pCell->GetLayoutParamT<SGridLayoutParam>();
			SASSERT(pLayoutParam);
			//将当前网络所占用的空间位置清0
			int colSpan = pLayoutParam->nColSpan;
			int rowSpan = pLayoutParam->nRowSpan;

			colSpan = smin(colSpan,m_nCols-iCol);
			rowSpan = smin(rowSpan,m_nRows-iRow);
			SASSERT(colSpan>=1);
			SASSERT(rowSpan>=1);
			SASSERT(pCellsOccupy[iRow*m_nCols+iCol]==false);//保证至少有一个空间可用
			//计算可占用空间
			for(int y=0;y<rowSpan;y++) for(int x=0;x<colSpan;x++)
			{
				int iCell = (iRow+y)*m_nCols+iCol+x;
				if(pCellsOccupy[iCell])
				{//colSpan优先
					rowSpan = y+1;
					if(y==0)
						colSpan = x+1;
					break;
				}
			}

			//计算出网络大小
			CSize szCell = pCell->GetDesiredSize(-1,-1);
			//填充网格,把大小平均分散到网格中。
			szCell.cx/=colSpan;
			szCell.cy/=rowSpan;

			float colWeight = pLayoutParam->fColWeight/colSpan;
			float rowWeight = pLayoutParam->fRowWeight/rowSpan;
			for(int y=0;y<rowSpan;y++) for(int x=0;x<colSpan;x++)
			{
				int iCell = (iRow+y)*m_nCols+iCol+x;
				pCellsOccupy[iCell]=true;
				pCellsSize[iCell]=szCell;
				pCellsColWeight[iCell] = colWeight;
				pCellsRowWeight[iCell] = rowWeight;
				pCellsSpan[iCell].x = 0;
				pCellsSpan[iCell].y = 0;
			}
			pCellsSpan[iRow*m_nCols+iCol]=CPoint(colSpan,rowSpan);

			//计算下一个网络的排列位置(先在当前行查找，再到下面行从0开始查找)
			bool bFind = false;
			for(int x=iCol+1;x<m_nCols;x++)
			{
				if(!pCellsOccupy[iRow*m_nCols+x])
				{
					bFind = true;
					iCol = x;
					break;
				}
			}
			for(int y=iRow+1;y<m_nRows && !bFind;y++) for(int x=0;x<m_nCols;x++)
			{
				if(!pCellsOccupy[y*m_nCols+x])
				{
					iRow = y;
					iCol = x;
					bFind = true;
					break;
				}
			}
			if(!bFind) break;
			pCell = pParent->GetNextLayoutChild(pCell);
		}


		int *pCellsWidth = new int[m_nCols];
		int nTotalWidth=0;
		float *pColsWeight = new float[m_nCols];
		float totalColWeight =0.0f;
		//计算列宽及相应的weight
		for(int x=0;x<m_nCols;x++)
		{
			int maxWid = 0;
			float maxWeight = 0.0f;
			for(int y=0;y<m_nRows;y++)
			{
				int iCell=y*m_nCols+x;
				maxWid = smax(pCellsSize[iCell].cx,maxWid);
				maxWeight = smax(pCellsColWeight[iCell],maxWeight);
			}
			pCellsWidth[x] = maxWid;
			nTotalWidth += maxWid;
			pColsWeight[x] = maxWeight;
			totalColWeight += maxWeight;
		}
		//计算列高
		int *pCellsHeight = new int[m_nRows];
		int nTotalHeight =0;
		float *pRowsWeight = new float[m_nRows];
		float fTotalRowsWeight = 0.0f;
		for(int y=0;y<m_nRows;y++)
		{
			int maxHei = 0;
			float maxWeight = 0.0f;
			for(int x=0;x<m_nCols;x++)
			{
				int iCell=y*m_nCols+x;
				maxHei = smax(pCellsSize[iCell].cy,maxHei);
				maxWeight = smax(pCellsRowWeight[iCell],maxWeight);
			}
			pCellsHeight[y] = maxHei;
			nTotalHeight += maxHei;
			pRowsWeight[y]=maxWeight;
			fTotalRowsWeight += maxWeight;
		}

		delete []pCellsOccupy;

		delete []pCellsColWeight;
		delete []pCellsRowWeight;

		//分配weight
		if(nTotalWidth<rcParent.Width())
		{
			int nRemain = rcParent.Width() - nTotalWidth;
			for(int i=0;i<m_nCols;i++)
			{
				pCellsWidth[i]+=(int)(nRemain*pColsWeight[i]/totalColWeight);
			}
		}
		if(nTotalHeight < rcParent.Height())
		{
			int nRemain = rcParent.Height()-nTotalHeight;
			for(int i=0;i<m_nRows;i++)
			{
				pCellsHeight[i]+=(int)(nRemain*pRowsWeight[i]/totalColWeight);
			}
		}
		delete []pColsWeight;
		delete []pRowsWeight;

		//计算子窗口位置
		CPoint pt = rcParent.TopLeft();
		for(int y=0;y<m_nRows;y++)
		{
			for(int x=0;x<m_nCols;x++)
			{
				int iCell = y*m_nCols+x;
				if(pCellsSpan[iCell].x==0 || pCellsSpan[iCell].y==0) continue;

				CSize szCell;
				for(int xx=0;xx<pCellsSpan[iCell].x;xx++)
					szCell.cx += pCellsWidth[x+xx];
				for(int yy=0;yy<pCellsSpan[iCell].y;yy++)
					szCell.cy += pCellsHeight[y+yy];

				SWindow *pCell = pCellsChild[iCell];
				SGridLayoutParam * pLayoutParam = pCell->GetLayoutParamT<SGridLayoutParam>();

				CSize szDesired = pCellsSize[iCell];
				if(pLayoutParam->IsMatchParent(Horz))
					szDesired.cx = szCell.cx;
				if(pLayoutParam->IsMatchParent(Vert))
					szDesired.cy = szCell.cy;
				CPoint pt2=pt;
				switch(pLayoutParam->xGravity)
				{
				case gLeft:break;
				case gCenter:pt2.x+=(szCell.cx-szDesired.cx)/2;break;
				case gRight:pt2.x+=(szCell.cx-szDesired.cx);break;
				}
				switch(pLayoutParam->yGravity)
				{
				case gTop:break;
				case gMiddle:pt2.y+=(szCell.cy-szDesired.cy)/2;break;
				case gBottom:pt2.y+=(szCell.cy-szDesired.cy);break;
				}
				CRect rcCell(pt,szDesired);
				pCell->OnRelayout(rcCell);			
			}
			pt.x=rcParent.left;
			pt.y += pCellsHeight[y];
		}

		delete []pCellsSize;
		delete []pCellsWidth;
		delete []pCellsHeight;

		delete []pCellsChild;
		delete []pCellsSpan;
	}

}

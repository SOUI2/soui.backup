#include "souistd.h"
#include "layout\SouiLayout.h"

namespace SOUI{
	SouiLayout::SouiLayout(void)
	{
	}

	SouiLayout::~SouiLayout(void)
	{
	}


	bool SoutLayoutParam::IsMatchParent(ORIENTATION orientation) const
	{
		return orientation == Vert ?(m_height == SIZE_MATCH_PARENT):(m_width == SIZE_MATCH_PARENT);
	}

	bool SoutLayoutParam::IsSpecifiedSize(ORIENTATION orientation) const
	{
		return orientation == Vert ?(m_height > SIZE_SPEC):(m_width > SIZE_SPEC);
	}

	int SoutLayoutParam::GetSpecifiedSize(ORIENTATION orientation) const
	{
		return orientation == Vert ?(m_height):(m_width);
	}

	HRESULT SoutLayoutParam::OnAttrOffset(const SStringW & strValue,BOOL bLoading)
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

	BOOL SoutLayoutParam::ParsePosition12( const SStringW & strPos1, const SStringW &strPos2 )
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

	BOOL SoutLayoutParam::ParsePosition34( const SStringW & strPos3, const SStringW &strPos4 )
	{
		if(strPos3.IsEmpty() || strPos4.IsEmpty()) return FALSE;
		POSITION_ITEM pos3,pos4;
		if(!StrPos2ItemPos(strPos3,pos3) || !StrPos2ItemPos(strPos4,pos4) ) return FALSE;

		pos [PI_RIGHT] = pos3;
		pos [PI_BOTTOM] = pos4;
		nCount = 4;
		return TRUE;
	}

	BOOL SoutLayoutParam::StrPos2ItemPos( const SStringW &strPos,POSITION_ITEM & pos )
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

	HRESULT SoutLayoutParam::OnAttrPos(const SStringW & strValue,BOOL bLoading)
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

	HRESULT SoutLayoutParam::OnAttrSize(const SStringW & strValue,BOOL bLoading)
	{
		SStringWList values;
		if(2!=SplitString(strValue,L',',values))
			return E_FAIL;
		OnAttrWidth(values[0],bLoading);
		OnAttrHeight(values[1],bLoading);
		return S_OK;
	}

	HRESULT SoutLayoutParam::OnAttrHeight(const SStringW & strValue,BOOL bLoading)
	{
		if(strValue.CompareNoCase(L"matchParent") == 0 || strValue.CompareNoCase(L"full") == 0)
			m_height = SIZE_MATCH_PARENT;
		else if(strValue.CompareNoCase(L"wrapContent") == 0)
			m_height = SIZE_WRAP_CONTENT;
		else
			m_height = _wtoi(strValue);
		return S_OK;
	}

	HRESULT SoutLayoutParam::OnAttrWidth(const SStringW & strValue,BOOL bLoading)
	{
		if(strValue.CompareNoCase(L"matchParent") == 0 || strValue.CompareNoCase(L"full") == 0)
			m_width = SIZE_MATCH_PARENT;
		else if(strValue.CompareNoCase(L"wrapContent") == 0)
			m_width = SIZE_WRAP_CONTENT;
		else
			m_width = _wtoi(strValue);
		return S_OK;
	}

}

#include "souistd.h"
#include "layout\SLinearLayout.h"

namespace SOUI
{
    bool SLinearLayoutParam::IsMatchParent(ORIENTATION orientation) const
    {
        return orientation==Vert?(m_width == SIZE_MATCH_PARENT) : (m_height == SIZE_MATCH_PARENT);
    }

    bool SLinearLayoutParam::IsSpecifiedSize(ORIENTATION orientation) const
    {
        return orientation==Vert?(m_width > SIZE_SPEC) : (m_height > SIZE_SPEC);
    }

    int SLinearLayoutParam::GetSpecifiedSize(ORIENTATION orientation) const
    {
        return orientation==Vert?m_width : m_height;
    }

    HRESULT SLinearLayoutParam::OnAttrWidth(const SStringW & strValue,BOOL bLoading)
    {
        if(strValue.CompareNoCase(L"matchParent") == 0)
            m_width = SIZE_MATCH_PARENT;
        else if(strValue.CompareNoCase(L"matchContent") == 0)
            m_width = SIZE_MATCH_CONTENT;
        else
            m_width = _wtoi(strValue);
        return S_OK;
    }

    HRESULT SLinearLayoutParam::OnAttrHeight(const SStringW & strValue,BOOL bLoading)
    {
        if(strValue.CompareNoCase(L"matchParent") == 0)
            m_height = SIZE_MATCH_PARENT;
        else if(strValue.CompareNoCase(L"matchContent") == 0)
            m_height = SIZE_MATCH_CONTENT;
        else
            m_height = _wtoi(strValue);
        return S_OK;
    }


    SLinearLayout::SLinearLayout(void)
    {
    }

    SLinearLayout::~SLinearLayout(void)
    {
    }

    void SLinearLayout::CalcPostionOfChildren(SWindow * pParent)
    {
        CRect rcParent = pParent->GetClientRect();

        
        CSize *pSize = new CSize [pParent->GetChildrenCount()];

        int offset = 0;
        float fWeight= 0.0f;

        {//assign width or height

            SWindow *pChild = pParent->GetWindow(GSW_FIRSTCHILD);
            int iChild = 0;

            while(pChild)
            {
                ILayoutParam * pParam;// = pChild->GetLayoutParam();
                SLinearLayoutParam *pLinearLayoutParam = sobj_cast<SLinearLayoutParam>(pParam);

                CSize szChild(-1,-1);
                if(pLinearLayoutParam->IsMatchParent(Horz))
                    szChild.cx = rcParent.Width();
                else if(pLinearLayoutParam->IsSpecifiedSize(Horz))
                    szChild.cx = pLinearLayoutParam->GetSpecifiedSize(Horz);

                if(pLinearLayoutParam->IsMatchParent(Vert))
                    szChild.cy = rcParent.Width();
                else if(pLinearLayoutParam->IsSpecifiedSize(Vert))
                    szChild.cy = pLinearLayoutParam->GetSpecifiedSize(Vert);
                
                if(pLinearLayoutParam->m_weight > 0.0f)
                {
                    if(m_orientation == Horz)
                        szChild.cx = 0;
                    else
                        szChild.cy = 0;
                    fWeight += pLinearLayoutParam->m_weight;
                }

                if(szChild.cx == -1 || szChild.cy == -1)
                {
                    CSize szCalc = pChild->GetDesiredSize(NULL);
                    if(szChild.cx == -1) szChild.cx = szCalc.cx;
                    if(szChild.cy == -1) szChild.cy = szCalc.cy;
                }

                pSize [iChild++] = szChild;
                offset += m_orientation == Vert ? szChild.cy:szChild.cx;
                pChild = pChild->GetWindow(GSW_NEXTSIBLING);
            }
        }

        int size = m_orientation == Vert? rcParent.Height():rcParent.Width();
        if(fWeight > 0.0f && size > offset)
        {//assign size by weight
            int nRemain = size - offset;
            SWindow *pChild = pParent->GetWindow(GSW_FIRSTCHILD);
            int iChild = 0;
            while(pChild)
            {
                ILayoutParam * pParam;// = pChild->GetLayoutParam();
                SLinearLayoutParam *pLinearLayoutParam = sobj_cast<SLinearLayoutParam>(pParam);

                if(pLinearLayoutParam->m_weight > 0.0f)
                {
                    int & szChild = m_orientation == Vert? pSize[iChild].cy:pSize[iChild].cx;
                    szChild = (int)(nRemain*pLinearLayoutParam->m_weight/fWeight);
                }
                iChild ++;
                pChild = pChild->GetWindow(GSW_NEXTSIBLING);
            }
        }
        {//assign position
            int nRemain = size - offset;
            SWindow *pChild = pParent->GetWindow(GSW_FIRSTCHILD);
            int iChild = 0;
            
            offset = 0;
            while(pChild)
            {
                ILayoutParam * pParam;// = pChild->GetLayoutParam();
                SLinearLayoutParam *pLinearLayoutParam = sobj_cast<SLinearLayoutParam>(pParam);

                
                if(m_orientation == Vert)
                {
                    CRect rcChild(CPoint(0,offset),pSize[iChild]);
                    rcChild.OffsetRect(rcParent.TopLeft());
                    if(pLinearLayoutParam->m_gravity == G_Center)
                        rcChild.OffsetRect((rcParent.Width()-rcChild.Width())/2,0);
                    else if(pLinearLayoutParam->m_gravity == G_Right)
                        rcChild.OffsetRect(rcParent.Width()-rcChild.Width(),0);
                    
                    pChild->OnRelayout(pChild->GetWindowRect(),rcChild);

                    offset += rcChild.Height();
                }else
                {
                    CRect rcChild(CPoint(offset,0),pSize[iChild]);
                    rcChild.OffsetRect(rcParent.TopLeft());
                    if(pLinearLayoutParam->m_gravity == G_Center)
                        rcChild.OffsetRect(0,(rcParent.Height()-rcChild.Height())/2);
                    else if(pLinearLayoutParam->m_gravity == G_Right)
                        rcChild.OffsetRect(0,rcParent.Height()-rcChild.Height());

                    pChild->OnRelayout(pChild->GetWindowRect(),rcChild);

                    offset += rcChild.Width();
                }


                iChild ++;
                pChild = pChild->GetWindow(GSW_NEXTSIBLING);
            }

        }

    }





}

/*
 * 各RichEditObj对象的实现文件
 *
 * 使用richedit的问题注意:
 * 1.SetIndents比较费时,能少用尽量少用
 * 2.如果行末有回车,在设置段落缩进时需要在行末手动加一个回车,然后把回车也选进段落进行缩进
 * 
*/

#include "stdafx.h"
#include "RichEditObj.h"
#include "SImRichedit.h"
#include <atlcomcli.h>
#include "helper\SplitString.h"
#include "RichEditOleCtrls.h"
#include "RichEditObjFactory.h"

//////////////////////////////////////////////////////////////////////////
// helpers

float px2pt(int px)
{
    static int dpi=0;
    if (!dpi)
    {
        HDC hDC = GetDC(NULL);
        dpi=GetDeviceCaps(hDC,LOGPIXELSX);
        ReleaseDC(NULL,hDC);
    }

    return px*72 / (float)dpi;
}

//////////////////////////////////////////////////////////////////////////
// IRichEditObj
RichEditObj::RichEditObj()
    :m_pParent(NULL)
    ,m_pFirstChild(NULL)
    ,m_pLastChild(NULL)
    ,m_pNextSibling(NULL)
    ,m_pPrevSibling(NULL)
    ,m_nChildrenCount(0)
    ,m_pObjectHost(NULL)
    ,m_alignType(ALIGN_LEFT)
    ,m_ulRef(1)
{
    m_chrContent.cpMin = -1;
    m_chrContent.cpMax = -1;
}

RichEditObj::~RichEditObj()
{
    OnDestroy();
}

ULONG RichEditObj::AddRef(void)
{
    return ++m_ulRef;
}

ULONG RichEditObj::Release(void)
{
    if (0 == --m_ulRef)
    {
        delete this;
        return 0;
    }
    return m_ulRef;
}

void RichEditObj::DestroyObject()
{
    if(!GetParent()) 
    {
        OnDestroy();
    }
    else 
    {
        GetParent()->DestroyChild(this);
    }
}

BOOL RichEditObj::DestroyChild(RichEditObj * pChild)
{
    if(this != pChild->GetParent()) 
        return FALSE;

    RemoveChild(pChild);
    pChild->Release();

    return TRUE;
}

void RichEditObj::OnDestroy()
{
    //destroy children objects
    RichEditObj *pChild = m_pFirstChild;
    while ( pChild )
    {
        RichEditObj *pNextChild = pChild->GetNext();
        pChild->Release();

        pChild=pNextChild;
    }

    m_pFirstChild=m_pLastChild=NULL;
    m_nChildrenCount=0;
}

UINT RichEditObj::GetChildrenCount()
{
    return m_nChildrenCount;
}

void RichEditObj::InsertChild(RichEditObj *pNewChild, RichEditObj *pInsertAfter/*=REOBJ_LAST*/)
{
    if(pNewChild->m_pParent == this) 
        return;

    pNewChild->m_pParent=this;
    pNewChild->m_pPrevSibling=pNewChild->m_pNextSibling=NULL;

    if(pInsertAfter==m_pLastChild) pInsertAfter=REOBJ_LAST;

    if(pInsertAfter==REOBJ_LAST)
    {
        //insert obj at head
        pNewChild->m_pPrevSibling=m_pLastChild;
        if(m_pLastChild) m_pLastChild->m_pNextSibling=pNewChild;
        else m_pFirstChild=pNewChild;
        m_pLastChild=pNewChild;
    }
    else if(pInsertAfter==REOBJ_FIRST)
    {
        //insert obj at tail
        pNewChild->m_pNextSibling=m_pFirstChild;
        if(m_pFirstChild) m_pFirstChild->m_pPrevSibling=pNewChild;
        else m_pLastChild=pNewChild;
        m_pFirstChild=pNewChild;
    }
    else
    {
        //insert obj at middle
        SASSERT(pInsertAfter->m_pParent == this);
        SASSERT(m_pFirstChild && m_pLastChild);
        RichEditObj *pNext=pInsertAfter->m_pNextSibling;
        SASSERT(pNext);
        pInsertAfter->m_pNextSibling=pNewChild;
        pNewChild->m_pPrevSibling=pInsertAfter;
        pNewChild->m_pNextSibling=pNext;
        pNext->m_pPrevSibling=pNewChild;
    }
    m_nChildrenCount++;
}

BOOL RichEditObj::RemoveChild(RichEditObj *pChild)
{
    if(this != pChild->GetParent()) 
        return FALSE;

    RichEditObj *pPrevSib=pChild->m_pPrevSibling;
    RichEditObj *pNextSib=pChild->m_pNextSibling;

    if(pPrevSib) 
        pPrevSib->m_pNextSibling=pNextSib;
    else 
        m_pFirstChild=pNextSib;

    if(pNextSib) 
        pNextSib->m_pPrevSibling=pPrevSib;
    else 
        m_pLastChild=pPrevSib;

    pChild->m_pParent=NULL;
    pChild->m_pNextSibling = NULL;
    pChild->m_pPrevSibling = NULL;
    m_nChildrenCount--;

    return TRUE;
}

RichEditObj * RichEditObj::GetById(LPCWSTR lpszId)
{
    if (lpszId == m_strId)
        return this;

    for (RichEditObj * p = m_pFirstChild; p != NULL; p = p->GetNext())
    {
        RichEditObj * pObjMatched = p->GetById(lpszId);
        if (pObjMatched)
        {
            return pObjMatched;
        }
    }

    return NULL;
}

RichEditObj* RichEditObj::FindChildByName( LPCWSTR pszName , int nDeep)
{
    if(!pszName || nDeep ==0) return NULL;

    RichEditObj *pChild = GetFirstChild();
    while(pChild)
    {
        if (pChild->m_strName == pszName)
            return pChild;
        pChild = pChild->GetNext();
    }

    if(nDeep>0) nDeep--;
    if(nDeep==0) return NULL;

    pChild = GetFirstChild();
    while(pChild)
    {
        RichEditObj *pChildFind=pChild->FindChildByName(pszName,nDeep);
        if(pChildFind) return pChildFind;
        pChild = pChild->GetNext();
    }

    return NULL;
}

void RichEditObj::UpdatePosition()
{
    for (RichEditObj * p = m_pFirstChild; p != NULL; p = p->GetNext())
    {
        p->UpdatePosition();
    }
}

void RichEditObj::OffsetCharRange(int nOffset)
{
    m_chrContent.cpMin += nOffset;
    m_chrContent.cpMax += nOffset;

    for (RichEditObj * p = m_pFirstChild; p != NULL; p = p->GetNext())
    {
        p->OffsetCharRange(nOffset);
    }
}

void RichEditObj::SetDirty(BOOL bDirty)
{
    m_bDirty = bDirty;

    for (RichEditObj * p = m_pFirstChild; p != NULL; p = p->GetNext())
    {
        p->SetDirty(bDirty);
    }
}

void RichEditObj::AdjustMessageParam(UINT msg, WPARAM& wParam, LPARAM& lParam)
{
    switch(msg)
    {
    case WM_MOUSEMOVE:
    case WM_MOUSELEAVE:
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_SETCURSOR:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
        CRect rcObj = GetRect();
        POINT ptOrgin = {GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
        int x = ptOrgin.x - rcObj.left;
        int y = ptOrgin.y - rcObj.top;
        lParam = MAKELPARAM(x, y);
        break;
    }
}

LRESULT RichEditObj::ProcessMessage(UINT msg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{            
    for (RichEditObj * p = m_pFirstChild; p != NULL && !bHandled; p = p->GetNext())
    {
        p->ProcessMessage(msg, wParam, lParam, bHandled);
        if (bHandled)
        {
            return 0;
        }
    }

    return 0;
}

BOOL RichEditObj::OnUpdateToolTip(CPoint pt, SwndToolTipInfo &tipInfo)
{
    for (RichEditObj * p = m_pFirstChild; p != NULL; p = p->GetNext())
    {
        p->OnUpdateToolTip(pt, tipInfo);
    }

    return 0;
}

void RichEditObj::DrawObject(IRenderTarget * pRT)
{
    for (RichEditObj * p = m_pFirstChild; p != NULL; p = p->GetNext())
    {
        p->DrawObject(pRT);
    }
}

BOOL RichEditObj::InsertIntoHost(IRichEditObjHost * pHost)
{
    m_pObjectHost = pHost;

    CHARRANGE chr;
    m_pObjectHost->SendMessage(EM_EXGETSEL, NULL, (LPARAM)&chr);
    m_chrContent.cpMin = chr.cpMax;
    int nContentLen = pHost->GetContentLength();
    
    RichEditObj * p = m_pFirstChild;
    while( p )
    {
        p->InsertIntoHost(pHost);
        p = p->GetNext();
    }

    m_chrContent.cpMax = m_chrContent.cpMin + pHost->GetContentLength() - nContentLen;
    return TRUE;
}

BOOL RichEditObj::InitFromXml(pugi::xml_node xmlNode)
{
    if (xmlNode)
    {
        SObject::InitFromXml(xmlNode);
        CreateChildren(xmlNode);
    }

    return TRUE;
}

BOOL RichEditObj::CreateChildren(pugi::xml_node xmlNode)
{
    for (pugi::xml_node xmlChild=xmlNode.first_child(); xmlChild; xmlChild=xmlChild.next_sibling())
    {
        if(xmlChild.type() != pugi::node_element) continue;

        RichEditObj * pChild = RichEditObjFactory::GetInstance().CreateObjectByName(xmlChild.name());
        if(pChild)
        {
            InsertChild(pChild);
            pChild->InitFromXml(xmlChild);
        }
    }

    return TRUE;
}

//////////////////////////////////////////////////////////////////////////
// RichEditText
RichEditText::RichEditText() : m_crText(RGB(0,0,0))
    ,m_nLineCount(1)
    ,m_bUnderline(FALSE)
    ,m_bBold(FALSE)
    ,m_bItalic(FALSE)
    ,m_nFontSize(10)
{
}

BOOL RichEditText::InitFromXml(pugi::xml_node xmlNode)
{
    m_strText = xmlNode.text().get();

    // 计算行数
    for (int nlf = m_strText.Find(0x0a); nlf >= 0; ++m_nLineCount)
    {
        nlf = m_strText.Find(0x0a, nlf+1);
    }

    return __super::InitFromXml(xmlNode);
}

BOOL RichEditText::InsertIntoHost(IRichEditObjHost * pHost)
{
    m_pObjectHost = pHost;
    m_pObjectHost->SendMessage(EM_EXGETSEL, NULL, (LPARAM)&m_chrContent);

    int nLength = m_pObjectHost->GetContentLength();
    m_pObjectHost->SendMessage(EM_REPLACESEL, TRUE, (LPARAM)(LPCWSTR)m_strText);
    m_chrContent.cpMin = m_chrContent.cpMax;
    m_chrContent.cpMax += m_pObjectHost->GetContentLength() - nLength;
    m_pObjectHost->SendMessage(EM_EXSETSEL, NULL, (LPARAM)&m_chrContent);

    // 设置字体样式
    CHARFORMATW cf = {0};
    cf.cbSize = sizeof(CHARFORMATW);

    cf.dwMask = CFM_COLOR;
    cf.crTextColor = m_crText&0x00ffffff;

    if (m_bUnderline)
    {
        cf.dwMask |=CFM_UNDERLINE;
        cf.dwEffects |= CFE_UNDERLINE;
    }

    if (m_bBold)
    {
        cf.dwMask |=CFM_BOLD;
        cf.dwEffects |= CFE_BOLD;
    }

    if (m_bItalic)
    {
        cf.dwMask |=CFM_ITALIC;
        cf.dwEffects |= CFE_ITALIC;
    }

    if (!m_strFont.IsEmpty())
    {
        wcscpy_s(cf.szFaceName, LF_FACESIZE-1, m_strFont);
        cf.dwMask |= CFM_FACE;
    }

    cf.dwMask |= CFM_SIZE;
    cf.yHeight = m_nFontSize * 20;

    m_pObjectHost->SendMessage(EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cf);

    // 把光标移到最后
    CHARRANGE chr = {m_chrContent.cpMax, m_chrContent.cpMax};
    m_pObjectHost->SendMessage(EM_EXSETSEL, NULL, (LPARAM)&chr);

    return TRUE;
}

//////////////////////////////////////////////////////////////////////////
// RichEditBkImg
RichEditBkImg::RichEditBkImg():m_nPosCount(0)
{

}

void RichEditBkImg::DrawObject(IRenderTarget * pRT)
{
    if (m_bDirty)
    {
        // 等所依赖的前一个/后一个兄弟节点先更新位置信息完毕
        CalcPosition(m_itemPos, m_nPosCount);
    }

    CRect rcHost = m_pObjectHost->GetHostRect();
    CRect rcTemp = rcHost;
    if (rcHost.IntersectRect(m_rcObj,rcTemp))
    {    
        SSkinImgFrame * pSkin = ImageProvider::GetImage(m_strSource);
        if (pSkin)
        { 
            pSkin->Draw(pRT, m_rcObj, 0);
        }
    }
}

BOOL RichEditBkImg::StrPos2ItemPos(const SStringW &strPos, POSITION_ITEM & pos)
{
    if(strPos.IsEmpty()) return FALSE;

    LPCWSTR pszPos = strPos;
    switch(pszPos[0])
    {
    //case POSFLAG_REFCENTER: pos.pit=PIT_CENTER,pszPos++;break;
    //case POSFLAG_PERCENT: pos.pit=PIT_PERCENT,pszPos++;break;
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

    return TRUE;
}

BOOL RichEditBkImg::ParsePosition34(POSITION_ITEM* pPosItem, const SStringW & strPos3, const SStringW &strPos4 )
{
    if(strPos3.IsEmpty() || strPos4.IsEmpty()) return FALSE;
    POSITION_ITEM pos3,pos4;
    if(!StrPos2ItemPos(strPos3,pos3) || !StrPos2ItemPos(strPos4,pos4) ) return FALSE;

    pPosItem [PI_RIGHT] = pos3;
    pPosItem [PI_BOTTOM] = pos4;
    return TRUE;
}

BOOL RichEditBkImg::ParsePosition12(POSITION_ITEM* pPosItem, const SStringW & strPos1, const SStringW &strPos2 )
{
    if(strPos1.IsEmpty() || strPos2.IsEmpty()) 
        return FALSE;
    POSITION_ITEM pos1,pos2;
    if(!StrPos2ItemPos(strPos1,pos1) || !StrPos2ItemPos(strPos2,pos2) )
        return FALSE;
    if(pos1.pit == PIT_SIZE || pos2.pit == PIT_SIZE)//前面2个属性不能是size类型
        return FALSE;
    pPosItem [PI_LEFT] = pos1;
    pPosItem [PI_TOP] = pos2;
    return TRUE;
}

HRESULT RichEditBkImg::OnAttrPos(const SStringW& strValue, BOOL bLoading)
{
    return OnInternalAttrPos(m_itemPos, m_nPosCount, strValue, bLoading);
}

HRESULT RichEditBkImg::OnInternalAttrPos(POSITION_ITEM* pPosItem, int& nPosCount, const SStringW& strValue, BOOL bLoading)
{
    SStringWList strLst;
    SplitString(strValue,L',',strLst);
    if(strLst.GetCount() != 2 && strLst.GetCount() != 4) 
    {
        SASSERT_FMTW(L"Parse pos attribute failed, strPos=%s",strValue);
        return FALSE;
    }

    //增加pos属性中的空格兼容。
    for(size_t i=0;i<strLst.GetCount();i++)
    {
        strLst.GetAt(i).TrimBlank();
    }
    BOOL bRet = TRUE;
    BOOL bRet2 = TRUE;

    bRet = ParsePosition12(pPosItem, strLst[0],strLst[1]);
    if(strLst.GetCount() == 4)
    {
        bRet2 = ParsePosition34(pPosItem, strLst[2],strLst[3]);
    }

    if (bRet)
    {
        nPosCount = 2;
    }
    if (bRet2)
    {
        nPosCount = 4;
    }
    return bRet && bRet2;
}

int RichEditBkImg::PositionItem2Value(const POSITION_ITEM &pos ,int nMin, int nMax,BOOL bX)
{
    int nRet=0;

    switch(pos.pit)
    {
    case PIT_NORMAL: 
        if(pos.cMinus == -1)
            nRet=nMax-(int)pos.nPos;
        else
            nRet=nMin+(int)pos.nPos;
        break;

    case PIT_PREV_NEAR: //“[”相对于前一兄弟窗口。用于X时，参考前一兄弟窗口的right，用于Y时参考前一兄弟窗口的bottom
    case PIT_PREV_FAR:  //“{”相对于前一兄弟窗口。用于X时，参考前一兄弟窗口的left，用于Y时参考前一兄弟窗口的top
        {
            CRect rcRef;
            RichEditObj *pRefObj=GetPrev();
            if(pRefObj)
            {
                rcRef = pRefObj->GetRect();
            }else
            {
                rcRef=m_pObjectHost->GetAdjustedRect();
                rcRef.right = rcRef.left;
                rcRef.bottom = rcRef.top;
            }
            if(bX)
            {
                LONG refPos = (pos.pit == PIT_PREV_NEAR)?rcRef.right:rcRef.left;
                nRet=refPos+(int)pos.nPos*pos.cMinus;
            }else
            {
                LONG refPos = (pos.pit == PIT_PREV_NEAR)?rcRef.bottom:rcRef.top;
                nRet=refPos+(int)pos.nPos*pos.cMinus;
            }
        }
        break;

    case PIT_NEXT_NEAR: //“]”相对于后一兄弟窗口。用于X时，参考后一兄弟的left,用于Y时参考后一兄弟的top
    case PIT_NEXT_FAR:  //“}”相对于后一兄弟窗口。用于X时，参考后一兄弟的right,用于Y时参考后一兄弟的bottom
        {
            CRect rcRef;
            RichEditObj *pRefObj = GetNext();
            if(pRefObj)
            {
                rcRef = pRefObj->GetRect();
            }else
            {
                rcRef = m_pObjectHost->GetAdjustedRect();
                rcRef.left = rcRef.right;
                rcRef.top = rcRef.bottom;
            }

            if(bX)
            {
                LONG refPos = (pos.pit == PIT_NEXT_NEAR)?rcRef.left:rcRef.right;
                nRet=refPos+(int)pos.nPos*pos.cMinus;
            }else
            {
                LONG refPos = (pos.pit == PIT_NEXT_NEAR)?rcRef.top:rcRef.bottom;
                nRet=refPos+(int)pos.nPos*pos.cMinus;
            }
        }
        break;
    }

    return nRet;
}

CRect RichEditBkImg::GetRect()
{
    if (m_bDirty)
    {
        CalcPosition(m_itemPos, m_nPosCount);
        m_bDirty= FALSE;
    }
    return m_rcObj;
}

void RichEditBkImg::CalcPosition(POSITION_ITEM * pItemsPos, int nPosCount)
{
    if (nPosCount == 0 || !m_pObjectHost)
    {
        return;
    }

    CRect rcHost=m_pObjectHost->GetAdjustedRect();

    // left
    m_rcObj.left = PositionItem2Value(pItemsPos[PI_LEFT], rcHost.left, rcHost.right, TRUE);

    // top
    m_rcObj.top = PositionItem2Value(pItemsPos[PI_TOP], rcHost.top, rcHost.bottom, FALSE);

    // right
    if (pItemsPos[PI_RIGHT].pit == PIT_SIZE)
    {
        m_rcObj.right = m_rcObj.left + (LONG)pItemsPos[PI_RIGHT].nPos;
    }
    else
    {
        m_rcObj.right = PositionItem2Value(pItemsPos[PI_RIGHT], rcHost.left, rcHost.right, TRUE);
    }

    // bottom
    if (pItemsPos[PI_BOTTOM].pit == PIT_SIZE)
    {
        m_rcObj.bottom = m_rcObj.top + (LONG)pItemsPos[PI_BOTTOM].nPos;
    }
    else
    {
        m_rcObj.bottom = PositionItem2Value(pItemsPos[PI_BOTTOM], rcHost.top, rcHost.bottom, FALSE);
    }
    m_bDirty = FALSE;
}

//////////////////////////////////////////////////////////////////////////
// RichEditBubble
RichEditBubble::RichEditBubble():m_nLeftPosCount(0)
    ,m_nRightPosCount(0)    
{
}

CRect RichEditBubble::GetRect()
{
    if (m_alignType == ALIGN_LEFT)
    {
        memcpy(m_itemPos, m_itemLeftPos, sizeof(m_itemLeftPos));
        m_nPosCount = m_nLeftPosCount;
    }
    else
    {
        memcpy(m_itemPos, m_itemRightPos, sizeof(m_itemLeftPos));
        m_nPosCount = m_nRightPosCount;
    }

    return RichEditBkImg::GetRect();
}

void RichEditBubble::DrawObject(IRenderTarget * pRT)
{
    if (m_alignType == ALIGN_LEFT)
    {
        memcpy(m_itemPos, m_itemLeftPos, sizeof(m_itemLeftPos));
        m_nPosCount = m_nLeftPosCount;
        m_strSource = m_strLeftBubble;
    }
    else
    {
        memcpy(m_itemPos, m_itemRightPos, sizeof(m_itemLeftPos));
        m_nPosCount = m_nRightPosCount;
        m_strSource = m_strRightBubble;
    }

    RichEditBkImg::DrawObject(pRT);
}


HRESULT RichEditBubble::OnAttrPosLeft(const SStringW& strValue, BOOL bLoading)
{
    return OnInternalAttrPos(m_itemLeftPos, m_nLeftPosCount, strValue, bLoading);
}

HRESULT RichEditBubble::OnAttrPosRight(const SStringW& strValue, BOOL bLoading)
{
    return OnInternalAttrPos(m_itemRightPos, m_nRightPosCount, strValue, bLoading);
}


//////////////////////////////////////////////////////////////////////////
// RichEditAvatar
RichEditAvatar::RichEditAvatar():m_nLeftPosCount(0)
,m_nRightPosCount(0)    
{
}

CRect RichEditAvatar::GetRect()
{
    if (m_alignType == ALIGN_LEFT)
    {
        memcpy(m_itemPos, m_itemLeftPos, sizeof(m_itemLeftPos));
        m_nPosCount = m_nLeftPosCount;
    }
    else
    {
        memcpy(m_itemPos, m_itemRightPos, sizeof(m_itemLeftPos));
        m_nPosCount = m_nRightPosCount;
    }

    return RichEditBkImg::GetRect();
}

void RichEditAvatar::DrawObject(IRenderTarget * pRT)
{
    if (m_alignType == ALIGN_LEFT)
    {
        memcpy(m_itemPos, m_itemLeftPos, sizeof(m_itemLeftPos));
        m_nPosCount = m_nLeftPosCount;
    }
    else
    {
        memcpy(m_itemPos, m_itemRightPos, sizeof(m_itemLeftPos));
        m_nPosCount = m_nRightPosCount;
    }

    RichEditBkImg::DrawObject(pRT);
}

HRESULT RichEditAvatar::OnAttrPosLeft(const SStringW& strValue, BOOL bLoading)
{
    return OnInternalAttrPos(m_itemLeftPos, m_nLeftPosCount, strValue, bLoading);
}

HRESULT RichEditAvatar::OnAttrPosRight(const SStringW& strValue, BOOL bLoading)
{
    return OnInternalAttrPos(m_itemRightPos, m_nRightPosCount, strValue, bLoading);
}

//////////////////////////////////////////////////////////////////////////
// RichEditPara
RichEditPara::RichEditPara():
    m_bNeedUpdateLayout(TRUE)
    ,m_bBreakAtTheEnd(TRUE)
    ,m_bSimulateAlign(FALSE)
    ,m_bWrapped(FALSE)
    ,m_nLineCount(0)
    ,m_bInited(FALSE)
    ,m_bDisableLayout(FALSE)
{
    m_bDirty = TRUE;
}

CRect RichEditPara::GetRect()
{
    if (m_bDirty)
    {
        m_bDirty = FALSE;
        CalcParagraphRect();

        if (m_bWrapped)
        {
            // 在自动换行的情况下,认为左右缩进都帖到头
            CRect rcClient = m_pObjectHost->GetHostRect();

            if (m_alignType == RichEditObj::ALIGN_RIGHT)
            {
                m_rcObj.right = rcClient.right - m_rcMargin.left;
                m_rcObj.left  = rcClient.left + m_rcMargin.right;
            }
            else
            {
                m_rcObj.right = rcClient.right - m_rcMargin.right;
                m_rcObj.left  = rcClient.left + m_rcMargin.left;
            }
        }
    }

    return m_rcObj;
}

BOOL RichEditPara::GetLineRect(int nLineNo, CRect& rcLine)
{
    rcLine.SetRectEmpty();

    int ncpStart;
    int nLength;
    m_pObjectHost->SendMessage(EM_LINEINDEX, nLineNo, 0, (LRESULT*)&ncpStart);
    m_pObjectHost->SendMessage(EM_LINELENGTH, ncpStart, 0, (LRESULT*)&nLength);

    SComPtr<ITextRange>  spRangeLine;
    ITextDocument * pdoc = m_pObjectHost->GetTextDoc();
    pdoc->Range(ncpStart, ncpStart + nLength, &spRangeLine);
    if (!spRangeLine)
    {
        return FALSE;
    }

    CHARRANGE chr = {ncpStart, ncpStart + nLength};
    // http://technet.microsoft.com/zh-cn/hh768766(v=vs.90) 新类型定义
#define _tomClientCoord     256  // 默认获取到的是屏幕坐标， Use client coordinates instead of screen coordinates.
#define _tomAllowOffClient  512  // Allow points outside of the client area.
    long lTypeTopLeft     = _tomAllowOffClient|_tomClientCoord|tomStart|TA_TOP|TA_LEFT;
    long lTypeRightBottom = _tomAllowOffClient|_tomClientCoord|tomEnd|TA_BOTTOM|TA_RIGHT;

    POINT   ptEnd, ptStart;
    spRangeLine->GetPoint(lTypeTopLeft,     &ptStart.x, &ptStart.y);
    spRangeLine->GetPoint(lTypeRightBottom, &ptEnd.x,   &ptEnd.y);

    rcLine.SetRect(ptStart, ptEnd);
    if (rcLine.Width() == 0) 
    {
        rcLine.right += 1;
    }

    return TRUE;
}

BOOL RichEditPara::CalcParagraphRect()
{
    int nLineStart = 0;
    int nLineEnd = 0;
    m_pObjectHost->SendMessage(EM_EXLINEFROMCHAR, 0, m_chrContent.cpMin, (LRESULT*)&nLineStart);
    m_pObjectHost->SendMessage(EM_EXLINEFROMCHAR, 0, m_chrContent.cpMax, (LRESULT*)&nLineEnd);

    /*
     * 计算cpMin和cpMax之间的每一行的rect，然后做并集
     * 如果行数较多,效率是比较低的,不知道还有没有更好的办法计算
    */
    m_rcObj.SetRectEmpty();
    for (int n = nLineStart; n <= nLineEnd; ++n)
    {
        CRect rcLine;
        GetLineRect(n, rcLine);
        m_rcObj.UnionRect(m_rcObj, rcLine);
    }

    return TRUE;
}

BOOL RichEditPara::InsertIntoHost(IRichEditObjHost * pHost)
{
    RichEditObj::InsertIntoHost(pHost);

    if (m_bBreakAtTheEnd)
    {
        /* 
         * 这个换行不算进char range里，但是在SetIndents时要加上。
         * 主要是为了保证内容里最后一行是空行时,空行也能设置缩进
        */

        pHost->SendMessage(EM_REPLACESEL, TRUE, (LPARAM)L"\r\n");
    }

    // 计算总行数
    RichEditObj * pChild = GetFirstChild();
    for (; pChild; pChild = pChild->GetNext())
    {
        if (pChild->IsClass(RichEditText::GetClassName()))
        {
            m_nLineCount += ((RichEditText*)pChild)->GetLineCount();
        }
    }

    if (m_nLineCount == 0)
        m_nLineCount = 1;

    return TRUE;
}

BOOL RichEditPara::IsWrapped()
{
    int  nLineStart = 0;
    int  nLineEnd   = 0;
    m_pObjectHost->SendMessage(EM_EXLINEFROMCHAR, 0, m_chrContent.cpMin, (LRESULT*)&nLineStart);
    m_pObjectHost->SendMessage(EM_EXLINEFROMCHAR, 0, m_chrContent.cpMax, (LRESULT*)&nLineEnd);

    // 如果显示的行数比原始插入时的行数要多,认为自动换行了
    return (nLineEnd - nLineStart + 1) > m_nLineCount;
}

void RichEditPara::OffsetCharRange(int nOffset)
{
    RichEditObj::OffsetCharRange(nOffset);

    m_bNeedUpdateLayout = TRUE;
    UpdatePosition();
}

void RichEditPara::SetAlign(AlignType align)
{
    if (align != m_alignType)
    {
        m_alignType = align;
        m_bNeedUpdateLayout = TRUE;
    }
}

void RichEditPara::UpdatePosition()
{
    RichEditObj::UpdatePosition();

    if (m_bDisableLayout)
    {
        // 如果不需要布局,直接返回
        return;
    }

    CComPtr<ITextPara>   ppara;
    CComPtr<ITextRange>  prange;
    ITextDocument* pdoc = m_pObjectHost->GetTextDoc();

    CHARRANGE chr = m_chrContent;
    if (m_bBreakAtTheEnd)
    {
        chr.cpMax += 1;
    }

    pdoc->Range(chr.cpMin, chr.cpMax, &prange);
    prange->GetPara(&ppara);

    if (!m_bInited)
    {
        m_bInited = TRUE;
        ppara->SetSpaceBefore(px2pt(m_rcMargin.top));
        ppara->SetSpaceAfter(px2pt(m_rcMargin.bottom));
    }

    CRect rcAdjust    = m_pObjectHost->GetAdjustedRect();
    CRect rcClient    = m_pObjectHost->GetHostRect();
    int nLeftIndents    = m_alignType==ALIGN_RIGHT? m_rcMargin.right : m_rcMargin.left;
    int nRightIndents   = m_alignType==ALIGN_RIGHT? m_rcMargin.left: m_rcMargin.right;
    nRightIndents   += rcClient.Width() - rcAdjust.Width();// 计算真实的右边距

    // 尽量不要使用模拟计算,能提高速度
    if (!m_bSimulateAlign || m_alignType == ALIGN_LEFT)
    {
        m_bWrapped = IsWrapped();
        if (m_bNeedUpdateLayout)
        {
            int nAlign = tomAlignLeft;
            if (m_alignType == ALIGN_RIGHT)
                nAlign = tomAlignRight;
            else if (m_alignType == ALIGN_CENTER)
                nAlign = tomAlignCenter;

            ppara->SetAlignment(nAlign);
            ppara->SetIndents(0, px2pt(nLeftIndents), px2pt(nRightIndents));
            m_bNeedUpdateLayout = FALSE;
        }

        return;
    }

    /* 模拟计算右对齐
     * 注意：SetIndents和CalcParagraphRect很耗时,所以尽量不使用模拟计算靠右
     *
     * 1.先按最靠边的左右缩进设置
     * 2.按照cp计算段落的rect
     * 3.用richedit.width() - para.widht()计算左缩进,模拟右对齐
     * 4.根据计算出的左/缩进重新设置
    */ 

    // step 1
    ppara->SetIndents(0, px2pt(nLeftIndents), px2pt(nRightIndents));
    if ((m_bWrapped = IsWrapped()))
        return;

    // step 2
    CalcParagraphRect();

    // step 3
    if (m_alignType == ALIGN_RIGHT)
    {
        nLeftIndents = rcClient.Width() - m_rcObj.Width() - nRightIndents - 1; // 这里不减1,段落就换行了
    }
    else
    {
        int mid = (rcClient.Width() - m_rcObj.Width() - nRightIndents -nLeftIndents) / 2 - 1;
        nLeftIndents  = m_rcMargin.left + mid;
        nRightIndents = m_rcMargin.right + mid;
    }

    // step 4
    ppara->SetIndents(0, px2pt(nLeftIndents), px2pt(nRightIndents));
    m_bNeedUpdateLayout = FALSE;
    m_bDirty = TRUE;
}

//////////////////////////////////////////////////////////////////////////
// RichEditContent
void RichEditContent::UpdatePosition()
{
    m_bDirty = TRUE;
    m_rcObj.SetRect(0,0,0,0);

    for (RichEditObj * p = m_pFirstChild; p != NULL; p = p->GetNext())
    {
        if (m_bAutoLayout)
        {
            CRect rcHost = m_pObjectHost->GetHostRect();
            AlignType type = rcHost.Width() > THRESHOLD_FOR_AUTOLAYOUT ? ALIGN_LEFT : ALIGN_RIGHT;
            p->SetAlign(type);
        }

        p->UpdatePosition();
    }
}

CRect RichEditContent::GetRect()
{
    if (m_bDirty)
    {
        m_bDirty = FALSE;
        m_rcObj.SetRect(0,0,0,0);

        for (RichEditObj * p = m_pFirstChild; p != NULL; p = p->GetNext())
        {
            CRect rcChild = p->GetRect();
            CRect rcTemp = m_rcObj;
            m_rcObj.UnionRect(rcChild, rcTemp);
        }
    }
    return m_rcObj;
}

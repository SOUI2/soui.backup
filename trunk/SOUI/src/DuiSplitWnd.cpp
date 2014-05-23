#include "duistd.h"
#include "DuiSplitWnd.h"

namespace SOUI
{

#define DEF_SEPSIZE    5


CDuiSplitPane::CDuiSplitPane():m_nPriority(0),m_nSizeIdeal(20),m_nSizeMin(0)
{

}


//////////////////////////////////////////////////////////////////////////
CDuiSplitWnd::CDuiSplitWnd(void)
    :m_bColMode(TRUE)
    ,m_bAdjustable(TRUE)
    ,m_nSepSize(DEF_SEPSIZE)
    ,m_pSkinSep(NULL)
    ,m_iDragBeam(-1)
{
}

CDuiSplitWnd::~CDuiSplitWnd(void)
{
}


BOOL CDuiSplitWnd::SetPaneInfo( UINT iPane,int nIdealSize,int nMinSize,int nPriority )
{
    if(iPane>m_arrPane.GetCount()-1) return FALSE;
    if(nIdealSize!=-1) m_arrPane[iPane]->m_nSizeIdeal=nIdealSize;
    if(nMinSize!=-1) m_arrPane[iPane]->m_nSizeMin=nMinSize;
    if(nPriority!=-1) m_arrPane[iPane]->m_nPriority=nPriority;
    Relayout(m_bColMode?layout_vert:layout_horz);
    return TRUE;
}

BOOL CDuiSplitWnd::GetPaneInfo( UINT iPane,int *pnIdealSize,int *pnMinSize,int *pnPriority )
{
    if(iPane>m_arrPane.GetCount()-1) return FALSE;
    if(pnIdealSize) *pnIdealSize=m_arrPane[iPane]->m_nSizeIdeal;
    if(pnMinSize) *pnMinSize=m_arrPane[iPane]->m_nSizeMin;
    if(pnPriority) *pnPriority=m_arrPane[iPane]->m_nPriority;
    return TRUE;
}

BOOL CDuiSplitWnd::ShowPanel(UINT iPane)
{
    if (iPane < 0 || iPane >= m_arrPane.GetCount()) return FALSE;

    m_arrPane[iPane]->SetVisible(TRUE);
    Relayout(m_bColMode?layout_vert:layout_horz);
    NotifyInvalidate();
    return TRUE;
}

BOOL CDuiSplitWnd::HidePanel(UINT iPane)
{
    if (iPane < 0 || iPane >= m_arrPane.GetCount()) return FALSE;

    m_arrPane[iPane]->SetVisible(FALSE);
    Relayout(m_bColMode?layout_vert:layout_horz);
    NotifyInvalidate();
    return TRUE;
}

int CDuiSplitWnd::GetVisiblePanelCount()
{
    int nCount = 0;
    for(UINT i=0; i<m_arrPane.GetCount(); i++)
    {
        if (m_arrPane[i]->IsVisible())
            nCount++;
    }
    return nCount;
}

int CDuiSplitWnd::GetNextVisiblePanel(UINT iPanel)
{
    if (iPanel + 1 >=  m_arrPane.GetCount())
        return -1;

    for(UINT i = iPanel + 1; i < m_arrPane.GetCount(); i++)
    {
        if (m_arrPane[i]->IsVisible())
            return (int)i;
    }
    return -1;
}

BOOL CDuiSplitWnd::LoadChildren( pugi::xml_node xmlNode )
{
    if(!xmlNode) return FALSE;
    pugi::xml_node xmlParent=xmlNode.parent();
    DUIASSERT(xmlParent);
    pugi::xml_node xmlPane=xmlParent.child("pane");
    while(xmlPane)
    {
        CDuiSplitPane *pPane=new CDuiSplitPane();
        InsertChild(pPane);
        if(pPane->Load(xmlPane))
        {
            pPane->AddRef();
            m_arrPane.Add(pPane);
        }
        xmlPane=xmlPane.next_sibling("pane");
    }
    return TRUE;
}


BOOL CDuiSplitWnd::OnDuiSetCursor(const CPoint &pt)
{
    if (!m_bAdjustable) return FALSE;

    SetCursor(LoadCursor(NULL,MAKEINTRESOURCE(m_bColMode?IDC_SIZEWE:IDC_SIZENS)));
    return TRUE;
}

void CDuiSplitWnd::OnDestroy()
{
    __super::OnDestroy();
    for(UINT i=0; i<m_arrPane.GetCount(); i++)
    {
        m_arrPane[i]->Release();
    }
    m_arrPane.RemoveAll();
}

void CDuiSplitWnd::OnPaint( CDCHandle dc )
{
    CRect rcClient;
    GetClient(&rcClient);

    if(m_pSkinSep)
    {
        CRect rcSep=rcClient;
        if(m_bColMode)
            rcSep.right=rcClient.left;
        else
            rcSep.bottom=rcSep.top;
        long &RB= m_bColMode?rcSep.right:rcSep.bottom;
        long &LT= m_bColMode?rcSep.left:rcSep.top;

        for(UINT i=0; i<m_arrPane.GetCount()-1; i++)
        {
            CRect rcPane;
            if (!m_arrPane[i]->IsVisible()) continue;
            m_arrPane[i]->GetRect(&rcPane);
            RB+=m_bColMode?rcPane.Width():rcPane.Height();
            LT=RB;
            RB+=m_nSepSize;
            m_pSkinSep->Draw(dc,rcSep,0);
        }
    }
}

int CDuiSplitWnd::FunComp( const void * p1,const void * p2 )
{
    const PANEORDER *pPane1=(PANEORDER*)p1;
    const PANEORDER *pPane2=(PANEORDER*)p2;
    return pPane1->pPane->m_nPriority-pPane2->pPane->m_nPriority;
}

LRESULT CDuiSplitWnd::OnWindowPosChanged( LPRECT lpWndPos )
{
    CRect rcWnd=m_rcWindow;
    LRESULT lRet=__super::OnWindowPosChanged(lpWndPos);
    if(lRet==0)
    {
        UINT uMode=0;
        if(rcWnd.Width()!=m_rcWindow.Width()) uMode |= layout_horz;
        if(rcWnd.Height()!=m_rcWindow.Height()) uMode |= layout_vert;
        if(rcWnd.TopLeft()!=m_rcWindow.TopLeft() || rcWnd.BottomRight()!=m_rcWindow.BottomRight()) uMode |= layout_pos;
        Relayout(uMode);
    }
    return lRet;
}

void CDuiSplitWnd::OnLButtonDown( UINT nFlags,CPoint pt )
{
    __super::OnLButtonDown(nFlags,pt);

    if (!m_bAdjustable) return;

    CRect rcClient;
    GetClient(&rcClient);

    CRect rcBeam=rcClient;
    if(m_bColMode) rcBeam.right=rcBeam.left;
    else rcBeam.bottom=rcBeam.top;

    long & nLT= m_bColMode?rcBeam.left:rcBeam.top;
    long & nRB= m_bColMode?rcBeam.right:rcBeam.bottom;

    //find the clicked beam
    for(UINT i=0; i<m_arrPane.GetCount(); i++)
    {
        CRect rcPane;
        if (!m_arrPane[i]->IsVisible()) continue;
        m_arrPane[i]->GetRect(&rcPane);
        nLT=m_bColMode?rcPane.right:rcPane.bottom;
        nRB=nLT+m_nSepSize;
        if(rcBeam.PtInRect(pt))
        {
            m_iDragBeam=i;
            break;
        }
    }
    m_ptClick=pt;
}

void CDuiSplitWnd::OnLButtonUp( UINT nFlags,CPoint pt )
{
    m_iDragBeam=-1;
    __super::OnLButtonUp(nFlags,pt);
}

void CDuiSplitWnd::OnMouseMove( UINT nFlags,CPoint pt )
{
    if(-1==m_iDragBeam) return;

    LockUpdate();
    CRect rcPane1,rcPane2;
    int nNextPanel = GetNextVisiblePanel(m_iDragBeam);
    if (nNextPanel == -1) return;

    m_arrPane[m_iDragBeam]->GetRect(&rcPane1);
    m_arrPane[nNextPanel]->GetRect(&rcPane2);

    CPoint ptMove=pt-m_ptClick;

    if(m_bColMode)
    {
        rcPane1.right+=ptMove.x;
        rcPane2.left+=ptMove.x;
        if(ptMove.x<0)
        {
            //decrease left pane
            int iTest=rcPane1.Width()-m_arrPane[m_iDragBeam]->m_nSizeMin;
            if(iTest<0)
            {
                rcPane1.right-=iTest;
                rcPane2.left-=iTest;
            }
        }
        else
        {
            //decrease right pane
            int iTest=rcPane2.Width()-m_arrPane[m_iDragBeam+1]->m_nSizeMin;
            if(iTest<0)
            {
                rcPane1.right+=iTest;
                rcPane2.left+=iTest;
            }
        }
    }
    else
    {
        rcPane1.bottom+=ptMove.y;
        rcPane2.top+=ptMove.y;
        if(ptMove.y<0)
        {
            //decrease top pane
            int iTest=rcPane1.Height()-m_arrPane[m_iDragBeam]->m_nSizeMin;
            if(iTest<0)
            {
                rcPane1.bottom-=iTest;
                rcPane2.top-=iTest;
            }
        }
        else
        {
            //decrease bottom pane
            int iTest=rcPane2.Height()-m_arrPane[m_iDragBeam+1]->m_nSizeMin;
            if(iTest<0)
            {
                rcPane1.bottom+=iTest;
                rcPane2.top+=iTest;
            }
        }
    }
    m_arrPane[m_iDragBeam]->Move(rcPane1);
    m_arrPane[m_iDragBeam+1]->Move(rcPane2);

    if(m_arrPane[m_iDragBeam]->m_nPriority<=m_arrPane[m_iDragBeam+1]->m_nPriority)
    {
        m_arrPane[m_iDragBeam]->m_nSizeIdeal=m_bColMode?rcPane1.Width():rcPane1.Height();
    }
    else
    {
        m_arrPane[m_iDragBeam+1]->m_nSizeIdeal=m_bColMode?rcPane2.Width():rcPane2.Height();
    }
    UnlockUpdate();
    NotifyInvalidate();
    m_ptClick=pt;
}

void CDuiSplitWnd::Relayout(UINT uMode)
{
    CRect rcClient;
    GetClient(&rcClient);

    BOOL bRowSplit=!m_bColMode;
    BOOL bColSplit=m_bColMode;

    if((uMode & layout_vert) && bColSplit)
    {//列模式，垂直方向发生变化
        for(UINT i=0;i<m_arrPane.GetCount(); i++)
        {
            CRect rcPane;
            m_arrPane[i]->GetRect(&rcPane);
            rcPane.top=rcClient.top;
            rcPane.bottom=rcClient.bottom;
            m_arrPane[i]->Move(rcPane);
        }
    }else if(uMode & layout_horz && bRowSplit)
    {//行模式，水平方向发生变化
        for(UINT i=0;i<m_arrPane.GetCount(); i++)
        {
            CRect rcPane;
            m_arrPane[i]->GetRect(&rcPane);
            rcPane.left=rcClient.left;
            rcPane.right=rcClient.right;
            m_arrPane[i]->Move(rcPane);
        }
    }else if(uMode & layout_pos)
    {//只是窗口位置发生变化
        //获得第一个窗格的原坐标
        CRect rcPane1;
        if(m_arrPane.GetCount())
        {
            m_arrPane[0]->GetRect(&rcPane1);
        }
        //与客户区坐标比较计算出偏移量
        CPoint ptOffset=rcClient.TopLeft()-rcPane1.TopLeft();

        for(UINT i=0;i<m_arrPane.GetCount(); i++)
        {
            CRect rcPane;
            m_arrPane[i]->GetRect(&rcPane);
            rcPane.OffsetRect(ptOffset);
            m_arrPane[i]->Move(rcPane);
        }
    }
    
    //检查列模式下水平方向有没有变化 或者 行模式下垂直方向上有没有变化
    if((bRowSplit && !(uMode & layout_vert)) || ( bColSplit && !(uMode & layout_horz)))
        return;

    //计算出理想的及最小的宽度或者高度
    int nTotalIdeal=0,nTotalMin=0;
    for(UINT i=0; i<m_arrPane.GetCount(); i++)
    {
        if (!m_arrPane[i]->IsVisible()) continue;
        nTotalIdeal+=m_arrPane[i]->m_nSizeIdeal;
        nTotalMin+=m_arrPane[i]->m_nSizeMin;
    }

    int nInter=(GetVisiblePanelCount()-1)*m_nSepSize;
    int nSize=m_bColMode?rcClient.Width():rcClient.Height();

    CRect rcPane=rcClient;
    if(m_bColMode) rcPane.right=rcPane.left;
    else rcPane.bottom=rcPane.top;

    long & nLT= m_bColMode?rcPane.left:rcPane.top;
    long & nRB= m_bColMode?rcPane.right:rcPane.bottom;

    if(nTotalMin+nInter > nSize)
    {
        //set all pane to minimize size
        for(UINT i=0; i<m_arrPane.GetCount(); i++)
        {
            if (!m_arrPane[i]->IsVisible()) continue;
            nRB+=m_arrPane[i]->m_nSizeMin;
            m_arrPane[i]->Move(&rcPane);
            nLT=nRB+m_nSepSize;
            nRB=nLT;
        }
    }
    else if(nTotalIdeal+nInter<nSize)
    {
        //set all pane to nIdealSize except the lowest priority one, which will be extent to fill all space
        int iLowest=0,nPriority=-1;
        for(UINT i=0; i<m_arrPane.GetCount(); i++)
        {
            if (!m_arrPane[i]->IsVisible()) continue;
            if(m_arrPane[i]->m_nPriority>nPriority)
            {
                nPriority=m_arrPane[i]->m_nPriority;
                iLowest=i;
            }
        }
        for(UINT i=0; i<m_arrPane.GetCount(); i++)
        {
            if (!m_arrPane[i]->IsVisible()) continue;
            if(i!=iLowest)
                nRB+=m_arrPane[i]->m_nSizeIdeal;
            else
                nRB+=m_arrPane[i]->m_nSizeIdeal+nSize-(nTotalIdeal+nInter);
            m_arrPane[i]->Move(&rcPane);
            nLT=nRB+m_nSepSize;
            nRB=nLT;
        }
    }
    else
    {
        //set high priority pane size to ideal size and set low pane to remain size
        PANEORDER *pnPriority=new PANEORDER[m_arrPane.GetCount()];
        int *pPaneSize=new int [m_arrPane.GetCount()];

        for(UINT i=0; i<m_arrPane.GetCount(); i++)
        {
            pnPriority[i].idx=i;
            pnPriority[i].pPane=m_arrPane[i];
        }
        qsort(pnPriority,m_arrPane.GetCount(),sizeof(PANEORDER),FunComp);

        //为每一个格子分配空间
        int nRemainSize=nSize-nInter;
        BOOL bMinimize=FALSE;
        for(UINT i=0; i<m_arrPane.GetCount(); i++)
        {
            if (!m_arrPane[i]->IsVisible()) continue;
            if(!bMinimize)
            {
                int nRequiredMin=0;
                for(UINT j=i+1; j<m_arrPane.GetCount(); j++)
                {
                    if (!m_arrPane[j]->IsVisible()) continue;
                    nRequiredMin+=pnPriority[j].pPane->m_nSizeMin;
                }
                if(nRequiredMin<nRemainSize-pnPriority[i].pPane->m_nSizeIdeal)
                    pPaneSize[pnPriority[i].idx]=pnPriority[i].pPane->m_nSizeIdeal;
                else
                {
                    pPaneSize[pnPriority[i].idx]=nRemainSize-nRequiredMin;
                    bMinimize=TRUE;
                }
            }
            else
            {
                pPaneSize[pnPriority[i].idx]=pnPriority[i].pPane->m_nSizeMin;
            }
            nRemainSize-=pPaneSize[pnPriority[i].idx];
        }

        //设置格子位置
        for(UINT i=0; i<m_arrPane.GetCount(); i++)
        {
            if (!m_arrPane[i]->IsVisible()) continue;
            nRB+=pPaneSize[i];
            m_arrPane[i]->Move(&rcPane);
            nLT=nRB+m_nSepSize;
            nRB=nLT;
        }

        delete [] pPaneSize;
        delete [] pnPriority;
    }
}

}//namespace SOUI
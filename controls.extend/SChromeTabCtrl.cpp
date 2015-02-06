#include "stdafx.h"
#include "SChromeTabCtrl.h"
#include <control/SCmnCtrl.h>


namespace SOUI
{
    const wchar_t KXmlTabStyle[] = L"tabStyle";
    const wchar_t KXmlCloseBtnStyle[]=L"closeBtnStyle";
    const wchar_t KXmlNewBtnStyle[]=L"newBtnStyle";

    //////////////////////////////////////////////////////////////////////////
    // SChromeTab
    class SChromeTab : public SWindow , public SAnimator
    {
        SOUI_CLASS_NAME(SChromeTab,L"chromeTab")
        friend class SChromeTabCtrl;
    public:
        SChromeTab(SChromeTabCtrl* pHost);

        void MoveTo(const CRect & rcEnd);
        
        SOUI_ATTRS_BEGIN()
            ATTR_INT(L"allowClose",m_bAllowClose,FALSE)
        SOUI_ATTRS_END()

		SOUI_MSG_MAP_BEGIN()
		    MSG_WM_MOUSEMOVE(OnMouseMove)
			MSG_WM_LBUTTONDOWN(OnLButtonDown)
		    MSG_WM_LBUTTONUP(OnLButtonUp)
		SOUI_MSG_MAP_END()
        
    protected:
        virtual void OnAnimatorState(int percent);
        virtual void OnFinalRelease(){delete this;}
		void OnMouseMove(UINT nFlags,CPoint pt);
		void OnLButtonUp(UINT nFlags,CPoint pt);
		void OnLButtonDown(UINT nFlags,CPoint pt);
	
        CRect m_rcBegin, m_rcEnd;
        BOOL    m_bAllowClose;
        CPoint  m_ptDrag;
		int     m_nOrder;
		bool    m_bDrag;
		SChromeTabCtrl* m_pHost;
    };

    SChromeTab::SChromeTab(SChromeTabCtrl* pHost):m_bAllowClose(TRUE),m_pHost(pHost),m_nOrder(-1)
    {
        m_bClipClient = TRUE;
    }

    void SChromeTab::MoveTo( const CRect & rcEnd )
    {
        m_rcBegin = m_rcWindow;
        m_rcEnd = rcEnd;
        Stop();
        Start(200);
    }

    void SChromeTab::OnAnimatorState( int percent )
    {
        CRect rcTemp;
        rcTemp.left = m_rcBegin.left + (m_rcEnd.left-m_rcBegin.left)*percent/100;
        rcTemp.top  = m_rcBegin.top  + (m_rcEnd.top -m_rcBegin.top )*percent/100;
        rcTemp.right= m_rcBegin.right + (m_rcEnd.right-m_rcBegin.right)*percent/100;
        rcTemp.bottom= m_rcBegin.bottom + (m_rcEnd.bottom-m_rcBegin.bottom)*percent/100;
        Move(rcTemp);
    }

	void SChromeTab::OnMouseMove(UINT nFlags,CPoint pt)
	{
        if(nFlags & MK_LBUTTON)
		{
			CRect rcWnd = GetWindowRect();
			if(m_pHost->m_tabAlign == SChromeTabCtrl::TDIR_HORZ)
			    rcWnd.OffsetRect(pt.x-m_ptDrag.x,0);
			else
			    rcWnd.OffsetRect(0,pt.y-m_ptDrag.y);
            Move(rcWnd);
            m_ptDrag = pt;
			m_pHost->ChangeTabPos(this,pt);
			m_bDrag = true;
		}
	}
	void SChromeTab::OnLButtonUp(UINT nFlags,CPoint pt)
	{
        ReleaseCapture();
        ModifyState(0, WndState_PushDown,TRUE);
		if(!m_bDrag)
		    FireCommand();
		else
            m_pHost->UpdateChildrenPosition();		    
	}
	
	void SChromeTab::OnLButtonDown(UINT nFlags,CPoint pt)
	{
        BringWindowToTop();
        SetCapture();
        ModifyState(WndState_PushDown, 0,TRUE);
	    m_ptDrag = pt;
	    m_bDrag  = false;
	}


    //////////////////////////////////////////////////////////////////////////
    //  SChromeBtnNew
    class SChromeBtnNew : public SChromeTab
    {
    public:
        virtual void OnFinalRelease()
        {
            delete this;
        }
    };



    //////////////////////////////////////////////////////////////////////////
    // SChromeTabCtrl
    SChromeTabCtrl::SChromeTabCtrl(void):m_iCurSel(-1),m_tabAlign(TDIR_HORZ),m_nDesiredSize(200)
    {
        m_evtSet.addEvent(EVT_CHROMETAB_CLOSE);
        m_evtSet.addEvent(EVT_CHROMETAB_NEW);
        m_evtSet.addEvent(EVT_CHROMETAB_SELCHANGED);
    }

    SChromeTabCtrl::~SChromeTabCtrl(void)
    {
    }
	int SChromeTabCtrl::ChangeTabPos(SChromeTab* pCurMove,CPoint ptCur)
	{
		CRect rcWnd;
        for(int i =0;i<(int)m_lstTab.GetCount();i++)
		{
			  if(m_lstTab[i] == pCurMove)
			  {
				  continue ;
			  }
              m_lstTab[i]->GetWindowRect(rcWnd);
			  rcWnd.left = rcWnd.right-rcWnd.Width()/2;
			  if(rcWnd.left <= ptCur.x && rcWnd.right >= ptCur.x)
			  {
				  rcWnd.left -= rcWnd.Width();
				  if(pCurMove->m_nOrder > m_lstTab[i]->m_nOrder)
				  {
                      rcWnd.OffsetRect(rcWnd.Width(),0); 
				  }
				  else
				  {
                      rcWnd.OffsetRect(-rcWnd.Width(),0); 
				  }
				  int order = pCurMove->m_nOrder ;
				  pCurMove->m_nOrder = m_lstTab[i]->m_nOrder;
				  m_lstTab[i]->m_nOrder = order;
                  m_lstTab[i]->Move(rcWnd);
				  SChromeTab* pTemp = m_lstTab[i];
				  m_lstTab[pCurMove->m_nOrder] = pCurMove;
				  m_lstTab[pTemp->m_nOrder] = pTemp;
			  }
		}
        return 1;
	}

    BOOL SChromeTabCtrl::CreateChildren( pugi::xml_node xmlNode )
    {
        pugi::xml_node xmlTabs = xmlNode.child(L"tabs");//所有tab都必须在tabs标签内
        int i =0;
        for (pugi::xml_node xmlChild=xmlTabs.first_child(); xmlChild; xmlChild=xmlChild.next_sibling())
        {
            if(wcscmp(xmlChild.name() , SChromeTab::GetClassName())!=0) 
                continue;
            SChromeTab * pTab = new SChromeTab(this);
			pTab->m_nOrder = i++;
            SASSERT(pTab);
            m_lstTab.Add(pTab);
            InsertChild(pTab);
            pTab->InitFromXml(xmlChild);
            pTab->GetEventSet()->subscribeEvent(EventCmd::EventID,Subscriber(&SChromeTabCtrl::OnTabClick,this));
        }
        
        pugi::xml_node xmlTabStyle = xmlNode.child(KXmlTabStyle);
        if(xmlTabStyle)
        {
            m_xmlStyle.append_copy(xmlTabStyle);
        }

        pugi::xml_node xmlNewBtn = xmlNode.child(KXmlNewBtnStyle);
        if(xmlNewBtn)
        {
            m_pBtnNew = new SChromeTab(this);
            InsertChild(m_pBtnNew);
            m_pBtnNew->InitFromXml(xmlNewBtn);
            m_pBtnNew->GetEventSet()->subscribeEvent(EventCmd::EventID,Subscriber(&SChromeTabCtrl::OnBtnNewClick,this));
        }

        pugi::xml_node xmlCloseBtn =xmlNode.child(KXmlCloseBtnStyle);
        if(xmlCloseBtn)
        {
            m_xmlStyle.append_copy(xmlCloseBtn);

            for(UINT i = 0;i<m_lstTab.GetCount();i++)
            {//自动插入一个closeBtn
                if(!m_lstTab[i]->m_bAllowClose) continue;
                
                SWindow *pBtn = SApplication::getSingleton().CreateWindowByName(SImageButton::GetClassName());
                m_lstTab[i]->InsertChild(pBtn);
                pBtn->InitFromXml(xmlCloseBtn);
                pBtn->GetEventSet()->subscribeEvent(EventCmd::EventID,Subscriber(&SChromeTabCtrl::OnBtnCloseTabClick,this));
            }
        }
        return TRUE;
    }

    void SChromeTabCtrl::UpdateChildrenPosition()
    {
        CRect rcClient;
        GetClientRect(&rcClient);
        CSize szBtnNew;
        if(m_pBtnNew) szBtnNew=m_pBtnNew->GetDesiredSize(&rcClient);
        CRect rcTab=rcClient;
        if(m_tabAlign == TDIR_HORZ)
        {
            int nTabWid = 0;
            if(rcClient.Width() - szBtnNew.cx > (int)m_lstTab.GetCount()*m_nDesiredSize)
            {
                nTabWid = m_nDesiredSize;
            }else
            {
                nTabWid = (rcClient.Width()-szBtnNew.cx) / m_lstTab.GetCount();
            }
            rcTab.right = rcTab.left + nTabWid;
            for(UINT i=0;i<m_lstTab.GetCount();i++)
            {
                m_lstTab[i]->MoveTo(rcTab);
                rcTab.OffsetRect(nTabWid,0);
            }
            if(m_pBtnNew)
            {
                CRect rcNewBtn = CRect(rcTab.TopLeft(),szBtnNew);
                m_pBtnNew->MoveTo(rcNewBtn);
            }
            
        }else
        {
            int nTabHei = 0;
            if(rcClient.Height() - szBtnNew.cy > (int)m_lstTab.GetCount()*m_nDesiredSize)
            {
                nTabHei = m_nDesiredSize;
            }else
            {
                nTabHei = (rcClient.Height()-szBtnNew.cx) / m_lstTab.GetCount();
            }
            rcTab.bottom = rcTab.top + nTabHei;
            for(UINT i=0;i<m_lstTab.GetCount();i++)
            {
                m_lstTab[i]->MoveTo(rcTab);
                rcTab.OffsetRect(0,nTabHei);
            }
        }
        if(m_pBtnNew)
        {
            CRect rcNewBtn = CRect(rcTab.TopLeft(),szBtnNew);
            m_pBtnNew->MoveTo(rcNewBtn);
        }

    }

    bool SChromeTabCtrl::OnBtnNewClick( EventArgs *pEvt )
    {
        InsertTab(NULL,-1);
        return true;
    }

    bool SChromeTabCtrl::OnBtnCloseTabClick( EventArgs *pEvt )
    {
        SChromeTab *pTab = (SChromeTab*)pEvt->sender->GetParent();
        int idx = GetTabIndex(pTab);
        if(idx != -1)
        {
            EventChromeTabClose evt(this);
            evt.pCloseTab = pTab;
            evt.iCloseTab = idx;
            FireEvent(evt);

            m_lstTab.RemoveAt(idx);
            DestroyChild(pTab);
            UpdateChildrenPosition();

            if(idx==m_iCurSel)
            {
                m_iCurSel--;
				m_iCurSel = max(0,m_iCurSel);
				
            }else if(idx<m_iCurSel)
            {
                m_iCurSel--;
            }
        }
        return true;
    }

    bool SChromeTabCtrl::OnTabClick( EventArgs *pEvt )
    {
        SChromeTab *pTab = (SChromeTab*)pEvt->sender;
        int idx = GetTabIndex(pTab);
        SASSERT(idx!=-1);
        
        SetCurSel(idx);

        return true;
    }

    BOOL SChromeTabCtrl::InsertTab( LPCTSTR pszTitle,int iPos )
    {
        SChromeTab *pNewTab = new SChromeTab(this);
        SASSERT(pNewTab);
        
        InsertChild(pNewTab);
        pugi::xml_node xmlTabStyle = m_xmlStyle.child(KXmlTabStyle);
        if(xmlTabStyle)
            pNewTab->InitFromXml(xmlTabStyle);
        if(pszTitle)
        {
            pNewTab->SetWindowText(pszTitle);
            pNewTab->SetAttribute(L"tip",S_CT2W(pszTitle));
        }

        pNewTab->GetEventSet()->subscribeEvent(EventCmd::EventID,Subscriber(&SChromeTabCtrl::OnTabClick,this));
        

        if(iPos<0) iPos = m_lstTab.GetCount();
        pNewTab->m_nOrder = iPos;
        m_lstTab.InsertAt(iPos,pNewTab);
        CRect rcClient;
        GetClientRect(&rcClient);
        CRect rcLeft;
        if(iPos>0)
        {
            m_lstTab[iPos-1]->GetWindowRect(&rcLeft);
        }else
        {
            rcLeft=rcClient;
            rcLeft.right=rcLeft.left;
        }

        pugi::xml_node xmlCloseBtn = m_xmlStyle.child(KXmlCloseBtnStyle);
        if(xmlCloseBtn && pNewTab->m_bAllowClose)
        {
            SWindow *pBtn = SApplication::getSingleton().CreateWindowByName(SImageButton::GetClassName());
            pNewTab->InsertChild(pBtn);
            pBtn->InitFromXml(xmlCloseBtn);
            pBtn->GetEventSet()->subscribeEvent(EventCmd::EventID,Subscriber(&SChromeTabCtrl::OnBtnCloseTabClick,this));
        }
        
        //插入到初始位置
        CRect rcInit=rcLeft;
        rcInit.left=rcInit.right;
        rcInit.right=rcInit.left+m_nDesiredSize;
        pNewTab->Move(rcInit);

        UpdateChildrenPosition();

        EventChromeTabNew evt(this);
        evt.pNewTab = pNewTab;
        evt.iNewTab = iPos;
        FireEvent(evt);

        return TRUE;
    }

    int SChromeTabCtrl::GetTabIndex( const SChromeTab* pTab ) const
    {
        for(UINT i=0;i<m_lstTab.GetCount();i++)
        {
            if(pTab == m_lstTab[i])
            {
                return i;
            }
        }
        return -1;
    }

    void SChromeTabCtrl::OnNextFrame()
    {
        for(UINT i=0; i< m_lstTab.GetCount();i++)
        {
            m_lstTab[i]->Update();
        }
        if(m_pBtnNew) m_pBtnNew->Update();
    }

    int SChromeTabCtrl::OnCreate( LPVOID )
    {
        int nRet = __super::OnCreate(NULL);
        if(nRet==0) 
            GetContainer()->RegisterTimelineHandler(this);
        return nRet;
    }

    void SChromeTabCtrl::OnDestroy()
    {
        GetContainer()->UnregisterTimelineHandler(this);
        __super::OnDestroy();
    }

    void SChromeTabCtrl::SetCurSel(int iTab,bool bSendNotify)
    {
        if(iTab != m_iCurSel)
        {
            int oldSel = m_iCurSel;
            if(m_iCurSel!=-1)
            {
                m_lstTab[m_iCurSel]->ModifyState(0,WndState_Check,TRUE);
            }
            
            if(iTab != -1)
            {
                m_lstTab[iTab]->ModifyState(WndState_Check,0,TRUE);
            }
            m_iCurSel = iTab;
            
            
            if(bSendNotify)
            {
                EventChromeTabSelChanged evt(this);
                evt.iOldSel = oldSel;
                evt.iNewSel = iTab;

                FireEvent(evt);
            }
        }
    }

}
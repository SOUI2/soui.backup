/**
* Copyright (C) 2014-2050 SOUI团队
* All rights reserved.
*
* @file       SCalendar.cpp
* @brief      SCalendarCore以及SCalendar类源文件
* @version    v1.0
* @author     soui
* @date       2014-05-25
*
* Describe  时间控件相关函数实现
*/
#include "stdafx.h"
#include <control/scalendar.h>
#include <helper/STime.h>
#include "SCalendar2.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCalendar

namespace SOUI{

	extern WORD gLunarMonthDay[];
	extern BYTE gLunarMonth[];
	extern BYTE gLunarHolDay[];

#define TITLE_HEIGHT 20
#define HEADER_HEIGHT 20
#define FOOTER_HEIGHT 20

	//两个子控件的名字
#define NAME_BTN_TODAY				L"btn_today"
#define NAME_LABEL_TODAY			L"label_today"
#define NAME_LABEL_MONTH		L"label_month"
#define NAME_LABEL_YEAR				L"label_year"
#define NAME_BTN_PREMON			L"btn_prvmon"
#define NAME_BTN_NEXTMON		L"btn_nextmon"
#define NAME_BTN_SELMY				L"btn_selmy"  //选择年月


	//////////////////////////////////////////////////////////////////////////
	//    CCalendar
	//////////////////////////////////////////////////////////////////////////

	SCalendar2::SCalendar2(WORD iYear, WORD iMonth, WORD iDay)
	{
		Init();
		if(!SetDate(iYear, iMonth, iDay))
		{
			CTime tm=CTime::GetCurrentTime();
			SetDate(tm.GetYear(),tm.GetMonth(),tm.GetDay());
		}
	}

	SCalendar2::SCalendar2()
		:m_crBorder(RGB(153,188,232))
	{
		Init();
		CTime tm=CTime::GetCurrentTime();
		SetDate(tm.GetYear(),tm.GetMonth(),tm.GetDay());

	}

	void SCalendar2::Init()
	{
		m_evtSet.addEvent(EventCalendarSelDay::EventID);
		m_nTitleHei=TITLE_HEIGHT;		 
		m_nHeaderHei=HEADER_HEIGHT;
		m_nFooterHei=FOOTER_HEIGHT;
		m_crWeekend=RGBA(255,0,0,255);
		m_crDay=RGBA(255,0,0,255);
		m_crTitleBack=RGBA(0,255,0,255);
		m_crDayBack=RGBA(0,0,255,255);

		m_strBkgSkin=m_strFooterSkin=m_strHeaderSkin=L"";
		m_pTitleSkin=m_pDaySkin=NULL;  
		m_strMonSepYearSkin=m_strPreMonSkin=m_strNxtMonSkin=m_strSelMYSkin=m_strPreYearSkin=m_strNxtYearSkin=L"";
		m_strCRHeader=L"#000000FF";
		m_beInited=FALSE;
		m_iHoverDay=0;
		CTime today=CTime::GetCurrentTime();
		m_selYear=today.GetYear();
		m_selMonth=today.GetMonth();
		TCHAR sztext[][3]={_T("日"),_T("一"),_T("二"),_T("三"),_T("四"),_T("五"),_T("六")};
		for(int i=0;i<7;i++) m_strTitle[i]=sztext[i];
		WORD y=0,m=0,d=0;
		wchar_t iyears[64]={'\0'};
	}


	/////////////////////////////////////////////////////////////////////////////
	// CCalendar message handlers

	
	bool SCalendar2::ShowSelectMonthYear(EventArgs *pArg)
	{
		SWindow *w=FindChildByName(L"wnd_show_selmy");
		CRect rt;
		GetClientRect(&rt);
		wchar_t buff[1024]={'\0'};
		int i=0,j=0;
		UINT yearStart=(m_iYear/10)*10+1;
		if (!_wcsicmp(pArg->nameFrom,L"btn_prvmon"))
		{
			if (m_iMonth==1){
				m_iMonth=12;
				m_iYear-=1;
			}else
				m_iMonth--;
			Invalidate();

		}else if (!_wcsicmp(pArg->nameFrom,L"btn_nextmon"))
		{
			if (m_iMonth==12){
				m_iMonth=1;
				m_iYear+=1;
			}else
				m_iMonth++;
			Invalidate();
		}
		else if (!_wcsicmp(pArg->nameFrom,L"btn_selmy_cancel")){
			w->GetParent()->DestroyChild(w);
		}else if (!_wcsicmp(pArg->nameFrom,L"btn_selmy_ok"))
		{
			SWindow *m=NULL;
			for (i=1;i<=12;i++){
				wmemset(buff,0,sizeof(buff)/sizeof(wchar_t));
				wsprintf(buff,L"_sel_months_%d",i);
				m=FindChildByName(buff);
				if (m->IsChecked()){
					m_iMonth=i;
					break;
				}
			}
			for (i=0;i<10;i++){
				wmemset(buff,0,sizeof(buff)/sizeof(wchar_t));
				wsprintf(buff,L"_sel_years_%d",i);
				m=FindChildByName(buff);
				if (m->IsChecked()){
					swscanf(m->GetWindowText(),L"%d",&j);
					m_iYear=j;
					break;
				}		
			}
			m=FindChildByName(L"btn_month_year");
			wmemset(buff,0,sizeof(buff)/sizeof(wchar_t));
			CCalendarCore::FormatMonth(m_iMonth,buff,FALSE);
			wchar_t wy[7]={'\0'};
			wsprintf(wy,L"  %d",m_iYear);
			wcscat(buff,wy);
			m->SetWindowText(buff);
			w->GetParent()->DestroyChild(w);
		}else if (!w){
			rt.right-=rt.left;rt.bottom-=rt.top;
			rt.left=rt.top=0;
			IRenderTarget *pRT=this->GetRenderTarget();
			CSize strSize;
			pRT->MeasureText(L" 确定 ",wcslen(L" 确定 "),&strSize);
			ReleaseRenderTarget(pRT);																	
			UINT btn_width=strSize.cx+12; 
			UINT btn_hei=strSize.cy+6;
			wchar_t skinSep[128]={'\0'};
			wchar_t skinFooter[128]={'\0'};
			UINT sepWid=1;
			if (m_strFooterSkin.GetLength()>0)
				wsprintf(skinFooter,L"skin='%s' ",m_strFooterSkin);
			if (m_strMonSepYearSkin.GetLength()>0)	{
			   wsprintf(skinSep,L"skin='%s' ",m_strMonSepYearSkin);
			   ISkinObj *sk=SSkinPoolMgr::getSingletonPtr()->GetSkin(m_strMonSepYearSkin);
			   CSize sepSize=sk->GetSkinSize();
			   sepWid=sepSize.cx;
			}
			
			wsprintf(buff, L"<window name='wnd_show_selmy'  pos='%d,%d,%d,%d'  colorBkgnd='#FFFEFEFF'  display='0'>"
				L"<img pos='%d,%d,%d,%d'   %s />"
				L"<img pos='%d,%d,%d,%d'  %s />"
				L"<button  name='btn_selmy_cancel'  pos='%d,%d'    width='%d'  height='%d' > 取消 </button>"
				L"<button  name='btn_selmy_ok'         pos='%d,%d'     width='%d' height='%d'  > 确定 </button>"
				L"</window>"
				,rt.left,rt.top,rt.right,rt.bottom
				,rt.left+(rt.right-rt.left)/2,rt.top,rt.left+(rt.right-rt.left)/2+sepWid,rt.bottom-m_nFooterHei,skinSep
				,rt.left,rt.bottom-m_nFooterHei ,rt.right,rt.bottom,skinFooter
				,rt.left+(UINT)(rt.right-rt.left)/4,    rt.bottom-m_nFooterHei+(UINT)(m_nFooterHei-btn_hei)/2,btn_width,btn_hei
				,rt.left+(UINT)(rt.right-rt.left)*5/8,rt.bottom-m_nFooterHei+(UINT)(m_nFooterHei-btn_hei)/2,btn_width,btn_hei
				);	
			this->CreateChildren((LPCWSTR)buff);
			w=FindChildByName(L"wnd_show_selmy");
			wmemset(buff,0,sizeof(buff)/sizeof(wchar_t));
			wsprintf(buff,
				L"<window name='div_months'  pos='0,1,%d,%d'></window>"
				,rt.Width()/2-2,rt.Height()-m_nFooterHei-2					
				);
			w->CreateChildren((LPCWSTR)buff);
			wmemset(buff,0,sizeof(buff)/sizeof(wchar_t));
			wsprintf(buff,
				L"<window name='div_years'  pos='%d,1,%d,%d'></window>"
				,rt.left+ rt.Width()/2+2,rt.right-1,rt.Height()-m_nFooterHei-2					
				);
			w->CreateChildren((LPCWSTR)buff);	  
			UINT btn_left,btn_top;			
			int offset=0;
			while(true){					   //找一个合适的偏移量 防止除法带来的误差
				if ((rt.Height()-m_nFooterHei-offset-3)%6 ==0 ) break;
				offset++;
				if (offset>=rt.Height()) break;
			}
			btn_hei=(UINT)(rt.Height()-m_nFooterHei-offset-3)/6;
			btn_width=(UINT)rt.Width()/4-6;	  
			SWindow *sm=FindChildByName(L"div_months");
			SWindow *sy=FindChildByName(L"div_years");
			SWindow *m=NULL;
			wchar_t lmon[8];
			for (j=0;j<4;j++) {		  
				for (i=0;i<6;i++){	
					btn_top=rt.top+ btn_hei *i ;			
					wmemset(buff,0,sizeof(buff)/sizeof(wchar_t));
					if (j<2){  //生成月份Radio按钮
						btn_left=btn_width * j +6;
						wmemset(lmon,0,sizeof(lmon)/sizeof(lmon));
						CCalendarCore::FormatMonth(i+6*j+1,lmon,FALSE);
						if (m_strMYSkin.GetLength()>0){
							wsprintf(buff,
								L"<radio2   skin='%s'  focusSkin='%s' imgExpand='1'     name='_sel_months_%d'  pos='%d,%d' width='%d' height='%d'  align='center'  font='adding:-2'>%s</radio2>"
								,m_strMYSkin,m_strMYSkin,i+6*j+1,btn_left,btn_top,btn_width,btn_hei,lmon
								);
						}else{
						wsprintf(buff,
							L"<radio2    skin='btn.noborder'   name='_sel_months_%d'  pos='%d,%d' width='%d' height='%d'  align='center'  font='adding:-2'>%s</radio2>"
							,i+6*j+1,btn_left,btn_top,btn_width,btn_hei,lmon
							);
						}
						sm->CreateChildren(buff);		
						if (m_iMonth==(i+6*j+1)){
							wmemset(buff,0,sizeof(buff)/sizeof(wchar_t));
							wsprintf(buff,L"_sel_months_%d",m_iMonth);
							m=FindChildByName(buff);
							if (m)
								m->SetCheck(TRUE);
						}
					}else{
						btn_left= btn_width * (j-2) +6;
						if (i==0){
							CSize skinSize;
							if (j==2){		 //生成年份前翻按钮
								if (m_strPreYearSkin.GetLength()>0){
									ISkinObj *sk=SSkinPoolMgr::getSingletonPtr()->GetSkin(m_strPreYearSkin);
									if (sk)
										skinSize=sk->GetSkinSize();
									else{
										skinSize.cx=btn_width;
										skinSize.cy=btn_hei;
									}
									wsprintf(buff,
										L"<imgbtn  pos='%d,%d' width='%d' height='%d' name='_btn_sel_prv_year' skin='%s' />"
										,btn_left+(btn_width-skinSize.cx)/2,btn_top+(btn_hei-skinSize.cy)/2,skinSize.cx,skinSize.cy,m_strPreYearSkin
										);
								}
								else {
									wsprintf(buff,
										L"<button  pos='%d,%d' width='%d' height='%d' name='_btn_sel_prv_year'  font='adding:-2' >  &lt;&lt;  </button>"
										,btn_left,btn_top,btn_width,btn_hei
										);
								}
								sy->CreateChildren(buff);
							}else if (j==3){		 //生成年份后翻按钮
								if (m_strNxtYearSkin.GetLength()>0  ){
									ISkinObj *sk=SSkinPoolMgr::getSingletonPtr()->GetSkin(m_strNxtYearSkin);
									if (sk)
										skinSize=sk->GetSkinSize();
									else{
										skinSize.cx=btn_width;
										skinSize.cy=btn_hei;
									}
									wsprintf(buff,
										L"<imgbtn  pos='%d,%d' width='%d' height='%d' name='_btn_sel_next_year'  skin='%s' />"
										,btn_left+(btn_width-skinSize.cx)/2,btn_top+(btn_hei-skinSize.cy)/2,skinSize.cx,skinSize.cy,m_strNxtYearSkin
										);
								}
								else {
									wsprintf(buff,
										L"<button  pos='%d,%d' width='%d' height='%d' name='_btn_sel_next_year'    font='adding:-2' >  &gt;&gt;  </button>"
										,btn_left,btn_top,btn_width,btn_hei
										);
								}
								sy->CreateChildren(buff);	       
							}
						}else{
							if (m_strMYSkin.GetLength()>0){
							wsprintf(buff,
								L"<radio2  skin='%s'  focusSkin='%s' imgExpand='1'    name='_sel_years_%d'    align='center'  pos='%d,%d' width='%d' height='%d' >%d</radio2>"
								,m_strMYSkin,m_strMYSkin,5*(j-2)+i-1,btn_left,btn_top,btn_width,btn_hei,5*(j-2)+yearStart+i-1
								);
							}else{
								wsprintf(buff,
									L"<radio2  skin='btn.noborder'     name='_sel_years_%d'  pos='%d,%d' width='%d' height='%d'   align='center'  >%d</radio2>"
									,5*(j-2)+i-1,btn_left,btn_top,btn_width,btn_hei,5*(j-2)+yearStart+i-1
									);

							}
							sy->CreateChildren(buff);		
							if (m_iYear==(5*(j-2)+yearStart+i-1)){
								wmemset(buff,0,sizeof(buff)/sizeof(wchar_t));
								wsprintf(buff,L"_sel_years_%d",5*(j-2)+i-1);
								m=FindChildByName(buff);
								if (m)
									m->SetCheck(TRUE);
							}
						}
					}
				}




			}
			SWindow *pBtn=FindChildByName(L"btn_selmy_cancel");
			pBtn->GetEventSet()->subscribeEvent(EventCmd::EventID,Subscriber(&SCalendar2::ShowSelectMonthYear,this));
			pBtn=FindChildByName(L"btn_selmy_ok");
			pBtn->GetEventSet()->subscribeEvent(EventCmd::EventID,Subscriber(&SCalendar2::ShowSelectMonthYear,this));
			pBtn=FindChildByName(L"_btn_sel_next_year");
			pBtn->GetEventSet()->subscribeEvent(EventCmd::EventID,Subscriber(&SCalendar2::SelectNextYear,this));
			pBtn=FindChildByName(L"_btn_sel_prv_year");
			pBtn->GetEventSet()->subscribeEvent(EventCmd::EventID,Subscriber(&SCalendar2::SelectPrevYear,this));

			w->BringWindowToTop();
			w->SetVisible(TRUE,TRUE);
		}else{
			w->SetVisible(TRUE,FALSE);
		}
		return true;

	}

	bool SCalendar2::SelectPrevYear(EventArgs *pArg)
	{
		SWindow *m=NULL;
		wchar_t wy[5]={'\0'};
		UINT yearStart=0;
		wchar_t buff[32]={'\0'};
		for (int i=0;i<10;i++){
			wmemset(buff,0,sizeof(buff)/sizeof(wchar_t));
			wsprintf(buff,L"_sel_years_%d",i);
			m=FindChildByName(buff);
			if (i==0){
				swscanf(m->GetWindowText(),L"%d",&yearStart);
				yearStart=(yearStart/10)*10+1;
				yearStart-=10;
				if (yearStart<1900) return false;			 //农历、节气数据到此为止
			}
			if (m->IsChecked()) m->SetCheck(FALSE);
			wsprintf(wy,L"%d", yearStart+i);
			m->SetWindowText(wy);
		}
		return true;
	}

	bool SCalendar2::SelectNextYear(EventArgs *pArg)
	{
		UINT yearStart=0;
		wchar_t buff[32]={'\0'};
		SWindow *m=NULL;
		wchar_t wy[5]={'\0'};
		yearStart=0;
		for (int i=0;i<10;i++){
			wmemset(buff,0,sizeof(buff)/sizeof(wchar_t));
			wsprintf(buff,L"_sel_years_%d",i);
			m=FindChildByName(buff);
			if (i==0){
				swscanf(m->GetWindowText(),L"%d",&yearStart);
				yearStart=(yearStart/10)*10+1;
				yearStart+=10;
				if (yearStart>2060) return false;		 //农历、节气数据到此为止
			}
			if (m->IsChecked()) m->SetCheck(FALSE);
			wsprintf(wy,L"%d", yearStart+i);
			m->SetWindowText(wy);
		} 
	  return true;
	}

	void SCalendar2::DrawBorder(IRenderTarget *pRT)
	{
		CAutoRefPtr<IPen> pPen,oldPen;
		CRect rect ;
		GetClientRect(&rect);
		rect.left--;
		rect.bottom++;
		rect.top--;
		rect.right++;
		pRT->CreatePen( PS_SOLID,m_crBorder,1,&pPen);
		pRT->SelectObject(pPen,(IRenderObj**)&oldPen);
		pRT->DrawRectangle(&rect);    
		pRT->SelectObject(oldPen);
	}

	void SCalendar2::DrawBackGround(IRenderTarget *pRT)
	{
		CRect rect ;
		GetClientRect(&rect);
		if (!m_strBkgSkin.GetLength()) return;
		ISkinObj *sk=SSkinPoolMgr::getSingletonPtr()->GetSkin(m_strBkgSkin);
		sk->Draw(pRT,&rect,0);
	}

	void SCalendar2::DrawHeader(IRenderTarget *pRT)
	{
		CRect rect ;
		CRect rcItem;
		GetClientRect(&rect);
		COLORREF cr_up=RGB(35,67,125);
		COLORREF cr_down=RGB(24,51,101);
		rect.bottom = rect.top + m_nHeaderHei;

		if(m_strHeaderSkin.GetLength()){
			ISkinObj *sk=SSkinPoolMgr::getSingletonPtr()->GetSkin(m_strHeaderSkin);
			sk->Draw(pRT,rect,0);
		}
		else
			pRT->GradientFill(&rect,TRUE,cr_up,cr_down);

		int nWid=rect.Width()/7;
		int nHeight=(m_nHeaderHei/2)-5;
		wchar_t  buffer[520]={'\0'};
		SWindow *w=NULL;
		SImageButton *btn=NULL;
		btn=(SImageButton *)FindChildByName(L"btn_prvmon");
		CSize skinSize;
		if (!btn){
			if (m_strPreMonSkin.GetLength()){
				ISkinObj *sk=SSkinPoolMgr::getSingletonPtr()->GetSkin(m_strPreMonSkin);
				skinSize=sk->GetSkinSize();
				int top=(m_nHeaderHei>skinSize.cy)?(m_nHeaderHei-skinSize.cy)/2:0;
				skinSize.cy=(m_nHeaderHei>skinSize.cy)?skinSize.cy:m_nHeaderHei;
				wsprintf(buffer
					,L"<imgbtn name=\"btn_prvmon\" pos=\"%d,%d,%d,%d\"  skin=\"%s\" />"
					,3,top,3+skinSize.cx,top+skinSize.cy,m_strPreMonSkin
					);
			}else{
				CSize strSize;
				pRT->MeasureText(L" << ",wcslen(L" << "),&strSize);
				wsprintf(buffer
					,L"<button name=\"btn_prvmon\" pos=\"%d,%d,%d,%d\"   > &lt;&lt;</button>"
					,3,(m_nHeaderHei-strSize.cy)/2,3+strSize.cx,(m_nHeaderHei-strSize.cy)/2+strSize.cy
					);
			}

			this->CreateChildren((LPCWSTR)buffer);
			btn=(SImageButton *)FindChildByName(L"btn_prvmon");
			btn->GetEventSet()->subscribeEvent(EventCmd::EventID,Subscriber(&SCalendar2::ShowSelectMonthYear,this));
		}
	   
		wchar_t month[10]={'\0'};
		wchar_t wt[32]={'\0'};
		CTime today=CTime::GetCurrentTime();
		CCalendarCore::FormatMonth(m_iMonth,month,FALSE);
		wsprintf(wt,L"%s  %d",month,m_iYear);
		w=FindChildByName(L"btn_month_year");
		if (!w){
			wmemset(buffer,0,sizeof(buffer)/sizeof(wchar_t));		 
			CSize strSize;
			pRT->MeasureText(wt,wcslen(wt),&strSize);
			UINT btn_width=3*nWid;
			UINT btn_left=2*nWid;
			UINT btn_top=((m_nHeaderHei-strSize.cy)>=0)?(m_nHeaderHei-strSize.cy)/2:0;
			UINT btn_height=((m_nHeaderHei-strSize.cy)>=0)?strSize.cy:m_nHeaderHei;
			wsprintf(buffer
				,L"<imgbtn name=\"btn_month_year\" pos=\"%d,%d,%d,%d\"  skin=\"%s\"     colorText='%s'  />"
				,btn_left,btn_top,btn_left+btn_width,btn_top+btn_height,m_strDaySkin,m_strCRHeader
				);
			this->CreateChildren((LPCWSTR)buffer);
			w=FindChildByName(L"btn_month_year");
			if (w){
				w->SetWindowText(wt);
				SWindow *pBtnSelMY=FindChildByName(L"btn_month_year");
				pBtnSelMY->GetEventSet()->subscribeEvent(EventCmd::EventID,Subscriber(&SCalendar2::ShowSelectMonthYear,this));
			}
		}else{
			if (_wcsicmp(wt,w->GetWindowText()))
				w->SetWindowText(wt);
		}
	   
		btn=(SImageButton *)FindChildByName(L"btn_nextmon");
		if (!btn){
			wmemset(buffer,0,sizeof(buffer)/sizeof(wchar_t));
			if (m_strNxtMonSkin.GetLength()){
				ISkinObj *sk=SSkinPoolMgr::getSingletonPtr()->GetSkin(m_strNxtMonSkin);
				skinSize=sk->GetSkinSize();
				int top=(m_nHeaderHei>skinSize.cy)?(m_nHeaderHei-skinSize.cy)/2:0;
				skinSize.cy=(m_nHeaderHei>skinSize.cy)?skinSize.cy:m_nHeaderHei;				
				wsprintf(buffer
					,L"<imgbtn name=\"btn_nextmon\" pos=\"-%d,%d,-%d,%d\"  skin=\"%s\" />"
					,3+skinSize.cx,top,3,top+skinSize.cy,m_strNxtMonSkin
					);

			}else{
				CSize strSize;
				pRT->MeasureText(L" >> ",wcslen(L" >> "),&strSize);
				wsprintf(buffer
					,L"<button name=\"btn_nextmon\" pos=\"-%d,%d,-%d,%d\"   > &gt;&gt;</button>"
					,3+strSize.cx,(m_nHeaderHei-strSize.cy)/2,3,(m_nHeaderHei-strSize.cy)/2+strSize.cy
					);
			}
			this->CreateChildren((LPCWSTR)buffer);
			btn=(SImageButton *)FindChildByName(L"btn_nextmon");
			btn->GetEventSet()->subscribeEvent(EventCmd::EventID,Subscriber(&SCalendar2::ShowSelectMonthYear,this));

		}


	}

	void SCalendar2::DrawFooter(IRenderTarget *pRT)
	{
		CRect rect ;
		CRect rcItem;
		GetClientRect(&rect);
		rect.top = rect.bottom-m_nFooterHei;
		if(m_strFooterSkin.GetLength()){
			ISkinObj *sk=SSkinPoolMgr::getSingletonPtr()->GetSkin(m_strFooterSkin);
			sk->Draw(pRT,rect,0);
		}
		SWindow *pBtnToday=FindChildByName(L"btn_today");
		if (pBtnToday){
			pBtnToday->GetClientRect(&rcItem);
			pBtnToday->Move(rect.left+(rect.right-rect.left-(rcItem.right-rcItem.left))/2,rect.bottom-m_nFooterHei+(m_nFooterHei-(rcItem.bottom-rcItem.top))/2);
		}

	}

	void SCalendar2::DrawTitle(IRenderTarget *pRT)
	{
		CRect rect ;
		GetClientRect(&rect);
		rect.top+=m_nHeaderHei;
		rect.bottom = rect.top + m_nTitleHei;

		if(m_pTitleSkin)
			m_pTitleSkin->Draw(pRT,rect,0);
		else
			pRT->FillSolidRect(&rect,m_crTitleBack);

		int nWid=rect.Width()/7;
		CRect rcItem=rect;
		rcItem.right=rcItem.left+nWid;

		COLORREF crTxt=pRT->GetTextColor();
		for(int i=0; i <7; i++)
		{
			if(i==0 || i==6 )
				pRT->SetTextColor(m_crWeekend);
			else
				pRT->SetTextColor(crTxt);

			pRT->DrawText(m_strTitle[i],m_strTitle[i].GetLength(),rcItem,DT_SINGLELINE|DT_VCENTER|DT_CENTER);
			rcItem.OffsetRect(nWid,0);
		}
		pRT->SetTextColor(crTxt);
	}

	void SCalendar2::DrawDate(IRenderTarget *pRT)
	{

		int days=CCalendarCore::MonthDays(m_iYear, m_iMonth);

		for(int i=1;i<=days;i++)
		{
			CRect rcDay=GetDayRect(i);
			DrawDay(pRT,rcDay,i);
		}
	}


	void SCalendar2::DrawDay( IRenderTarget *pRT,CRect & rcDay,WORD iDay )
	{
		TCHAR text[3];
		_stprintf(text, _T("%2d"), iDay);
		COLORREF crTxt=pRT->GetTextColor();
		if(iDay==m_iDay)
		{
			if(m_pDaySkin) m_pDaySkin->Draw(pRT,rcDay,2);
			else pRT->FillSolidRect(rcDay,m_crDayBack);
			pRT->SetTextColor(m_crDay);
		}else
		{
			if(m_pDaySkin) m_pDaySkin->Draw(pRT,rcDay,iDay==m_iHoverDay?1:0);
			int iweekday=CCalendarCore::WeekDay(m_iYear,m_iMonth,iDay);
			if(iweekday==0 || iweekday==6)
				pRT->SetTextColor(m_crWeekend);
		}
		pRT->DrawText(text,-1,rcDay,DT_SINGLELINE|DT_VCENTER|DT_CENTER);
		pRT->SetTextColor(crTxt);
	}

	void SCalendar2::RedrawDay(WORD iDay )
	{
		CRect rcDay=GetDayRect(iDay);
		IRenderTarget * pRT=GetRenderTarget(&rcDay,OLEDC_PAINTBKGND);
		DrawDay(pRT,rcDay,iDay);
		ReleaseRenderTarget(pRT);
	}

	void SCalendar2::OnPaint(IRenderTarget * pRT) 
	{
		SPainter painter;
		BeforePaint(pRT,painter);
		DrawBackGround(pRT);	
		SWindow *w=FindChildByName(L"wnd_show_selmy");
		if (!w || w->IsVisible(FALSE)){
			DrawHeader(pRT);			
			DrawTitle(pRT);				
			DrawDate(pRT);				 
			DrawFooter(pRT);			
		}
		DrawBorder(pRT);			
		AfterPaint(pRT,painter);

	}

	void SCalendar2::GetDate(WORD &iYear, WORD &iMonth, WORD &iDay) 
	{
		iYear  = m_iYear;
		iMonth = m_iMonth;
		iDay   = m_iDay;
	}

	BOOL SCalendar2::SetDate(WORD iYear, WORD iMonth, WORD iDay)
	{
		if(iYear < START_YEAR || iYear > END_YEAR || iMonth <1 || iMonth >12)
			return FALSE;

		if(iDay <1 || iDay > CCalendarCore::MonthDays(iYear, iMonth))
			return FALSE;

		m_iYear   = iYear;
		m_iMonth  = iMonth;
		m_iDay    = iDay;

		m_iHoverDay    = 0;
		return TRUE;
	} 


	CRect SCalendar2::GetDayRect( WORD iDay )
	{
		CRect rcClient;
		GetClientRect(&rcClient);
		rcClient.top+=m_nTitleHei+m_nHeaderHei;
		rcClient.bottom-=m_nFooterHei;

		int weeks = CCalendarCore::MonthWeeks(m_iYear,m_iMonth);//计算出iMonth有几周
		int col  = CCalendarCore::WeekDay(m_iYear, m_iMonth,iDay);//计算出iday是星期几
		int row     = CCalendarCore::DayWeek(m_iYear, m_iMonth,iDay);//计算出iday是第几周

		int nWid=(col==6)?(rcClient.Width()+1-rcClient.Width()*6/7):(rcClient.Width()/7);
		int nHei=rcClient.Height()/weeks;

		CRect rc(0,0,nWid,nHei);
		rc.OffsetRect((rcClient.Width()/7)*col,nHei*row);
		rc.OffsetRect(rcClient.TopLeft());
		return rc;
	}

	WORD SCalendar2::HitTest(CPoint pt)
	{
		CRect rcClient;
		GetClientRect(&rcClient);
		rcClient.top+=m_nTitleHei+m_nTitleHei;
		rcClient.bottom-=m_nFooterHei;

		int weeks = CCalendarCore::MonthWeeks(m_iYear,m_iMonth);//计算出iMonth有几周

		int nWid=rcClient.Width()/7;
		int nHei=rcClient.Height()/weeks;

		pt-=rcClient.TopLeft();
		if(pt.x<0 || pt.y<0) return 0;

		int iCol  = pt.x/nWid;
		int iRow  = pt.y/nHei;


		WORD startcol ,endrow, endcol;
		startcol = CCalendarCore::WeekDay(m_iYear, m_iMonth, 1);
		endcol   = CCalendarCore::WeekDay(m_iYear, m_iMonth, CCalendarCore::MonthDays(m_iYear,m_iMonth));

		endrow   = (CCalendarCore::MonthDays(m_iYear, m_iMonth) + startcol -1)/7 ;
		if(iRow == 0 && iCol < startcol || iRow == endrow && iCol > endcol ||  iRow > endrow)
			return 0;
		return iRow *7 + iCol + 1 - startcol ;
	}

	void SCalendar2::OnLButtonDown(UINT nFlags, CPoint point) 
	{
		__super::OnLButtonDown(nFlags,point);
		WORD day = HitTest(point);
		if (day==0) return;
		if(day !=0 && day != m_iDay)
		{
			EventCalendarSelDay evt(this);
			evt.wOldDay=m_iDay;
			evt.wNewDay=day;

			FireEvent(evt);
		}
		m_iDay = day;
		Invalidate();
	}

	void SCalendar2::OnMouseMove( UINT nFlags,CPoint pt )
	{
		int iDay=HitTest(pt);
		if(iDay!=m_iHoverDay)
		{
			WORD oldHover=m_iHoverDay;
			m_iHoverDay=iDay;
			if(m_pDaySkin)
			{
				if(oldHover!=0) RedrawDay(oldHover);
				if(m_iHoverDay!=0) RedrawDay(m_iHoverDay);
			}
		}
	}

	void SCalendar2::OnMouseLeave()
	{
		if(m_iHoverDay!=0)
		{
			WORD oldHover=m_iHoverDay;
			m_iHoverDay=0;
			if(m_pDaySkin) RedrawDay(oldHover);
		}
	}

	BOOL SCalendar2::InitFromXml( pugi::xml_node xmlNode )
	{
		BOOL bLoad=__super::InitFromXml(xmlNode);
		if(bLoad)
		{
		    SASSERT(!m_strDaySkin.IsEmpty());
		    m_pDaySkin = GETSKIN(m_strDaySkin);
		    
			SWindow *pBtnToday=FindChildByName(L"btn_today");
			if(pBtnToday)
			{
				pBtnToday->SetID(100);
				pBtnToday->GetEventSet()->subscribeEvent(EventCmd::EventID,Subscriber(&SCalendar2::OnTodayClick,this));
			}
		}
		return bLoad;
	}

	bool SCalendar2::OnTodayClick( EventArgs *pArg)
	{
		CTime today=CTime::GetCurrentTime();
		SetDate(today.GetYear(),today.GetMonth(),today.GetDay());
		SWindow *w=FindChildByName(L"btn_month_year");
		wchar_t  m[16]={'\0'};
		wchar_t y[5]={'\0'};
		wsprintf(y,L"%d",today.GetYear());
		CCalendarCore::FormatMonth(today.GetMonth(),m,FALSE);
		wcscat(m,L"  ");
		wcscat(m,y);
		w->SetWindowText(m);
		Invalidate();
		return true;
	}

}//end of namespace

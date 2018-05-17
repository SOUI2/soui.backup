// MainDlg.cpp : implementation of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MainDlg.h"
#include <helper/SMenuEx.h>
#include <helper/SAdapterBase.h>

#ifdef DWMBLUR	//win7毛玻璃开关
#include <dwmapi.h>
#pragma comment(lib,"dwmapi.lib")
#endif
	
class CTestMcAdapterFix : public SMcAdapterBase
{
public:
	struct student
	{
		TCHAR  szName[50];
		TCHAR  szGender[10];
		int  age;
		int  score;
	};

	SArray<student> m_studuents;

public:
	CTestMcAdapterFix()
	{
		SStringT strGenders[3] = { S_CW2T(GETSTRING(L"male")),S_CW2T(GETSTRING(L"female")),S_CW2T(GETSTRING(L"unknown"))};
		for (int i = 0; i<100; i++)
		{
			student st;
			_stprintf(st.szName, _T("学生_%d"), i + 1);
			_tcscpy(st.szGender, strGenders[rand() % 3]);
			st.age = rand() % 30;
			st.score = rand() % 60 + 40;
			m_studuents.Add(st);
		}
	}

	virtual int getCount()
	{
		return m_studuents.GetCount();
	}


	virtual void getView(int position, SWindow * pItem, pugi::xml_node xmlTemplate)
	{
		if (pItem->GetChildrenCount() == 0)
		{
			pItem->InitFromXml(xmlTemplate);
		}

		student & st = m_studuents[position];
		pItem->FindChildByName(L"txt_name")->SetWindowText(st.szName);
		pItem->FindChildByName(L"txt_gender")->SetWindowText(st.szGender);
		pItem->FindChildByName(L"txt_score")->SetWindowText(SStringT().Format(_T("%d分"), st.score));
		pItem->FindChildByName(L"txt_age")->SetWindowText(SStringT().Format(_T("%d"), st.age));
	}

	virtual SStringW GetColumnName(int iCol) const override
	{
		return SStringW().Format(L"col%d", iCol + 1);
	}

	struct SORTCTX
	{
		int iCol;
		SHDSORTFLAG stFlag;
	};

	bool OnSort(int iCol, SHDSORTFLAG * stFlags, int nCols)
	{
		if (iCol == 5) //最后一列“操作”不支持排序
			return false;

		SHDSORTFLAG stFlag = stFlags[iCol];
		switch (stFlag)
		{
		case ST_NULL:stFlag = ST_UP; break;
		case ST_DOWN:stFlag = ST_UP; break;
		case ST_UP:stFlag = ST_DOWN; break;
		}
		for (int i = 0; i<nCols; i++)
		{
			stFlags[i] = ST_NULL;
		}
		stFlags[iCol] = stFlag;

		SORTCTX ctx = { iCol,stFlag };
		qsort_s(m_studuents.GetData(), m_studuents.GetCount(), sizeof(student), SortCmp, &ctx);
		return true;
	}

	static int __cdecl SortCmp(void *context, const void * p1, const void * p2)
	{
		SORTCTX *pctx = (SORTCTX*)context;
		const student *pSI1 = (const student*)p1;
		const student *pSI2 = (const student*)p2;
		int nRet = 0;
		switch (pctx->iCol)
		{
		case 0://name
			nRet = _tcscmp(pSI1->szName, pSI2->szName);
			break;
		case 1://gender
			nRet = _tcscmp(pSI1->szGender, pSI2->szGender);
			break;
		case 2://age
			return pSI1->age - pSI2->age;
			break;
		case 3://score;
			return pSI1->score - pSI2->score;
			break;
		}
		if (pctx->stFlag == ST_UP)
			nRet = -nRet;
		return nRet;
	}


};


CMainDlg::CMainDlg() : SHostWnd(_T("LAYOUT:XML_MAINWND"))
{
	m_LangID = R.id.lang_cn;
}

CMainDlg::~CMainDlg()
{
}

int CMainDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	#ifdef DWMBLUR	//win7毛玻璃开关
	MARGINS mar = {5,5,30,5};
	DwmExtendFrameIntoClientArea ( m_hWnd, &mar );
	#endif

	SetMsgHandled(FALSE);
	return 0;
}

const wchar_t * strSrc = L"test input";
BOOL CMainDlg::OnInitDialog(HWND hWnd, LPARAM lParam)
{
	SStringT strTr = S_CW2T(TR(strSrc, L"user"));
	SWindow *wnd = FindChildByID(R.id.txt_test);
	FindChildByID(R.id.txt_test)->SetWindowText(strTr);
	InitListView();
	return 0;
}


void CMainDlg::InitListView()
{
	SMCListView *pListView = FindChildByID2<SMCListView>(R.id.mclv_test);
	IMcAdapter *pAdapter = new CTestMcAdapterFix();
	pListView->SetAdapter(pAdapter);
	pAdapter->Release();
}

HRESULT CMainDlg::OnLanguageChanged()
{
	__super::OnLanguageChanged();
	SStringT strTr = S_CW2T(TR(strSrc, L"user"));
	FindChildByID(R.id.txt_test)->SetWindowText(strTr);
	return 3;
}

void CMainDlg::OnLanguage(int nID)
{
	ITranslatorMgr *pTransMgr =  SApplication::getSingletonPtr()->GetTranslator();
	m_LangID = nID;
	bool bCnLang = nID == R.id.lang_cn;

	pugi::xml_document xmlLang;
	if (SApplication::getSingletonPtr()->LoadXmlDocment(xmlLang, bCnLang?_T("cn"):_T("en"), _T("lang")))
	{
		CAutoRefPtr<ITranslator> lang;
		pTransMgr->CreateTranslator(&lang);
		lang->Load(&xmlLang.child(L"language"), 1);//1=LD_XML
		pTransMgr->SetLanguage(lang->name());
		pTransMgr->InstallTranslator(lang);

		SFontPool::getSingletonPtr()->UpdateFonts();//update fonts that defined by translator
		SDispatchMessage(UM_SETLANGUAGE,0,0);
	}

}


//TODO:消息映射
void CMainDlg::OnClose()
{
	CSimpleWnd::DestroyWindow();
}

void CMainDlg::OnMaximize()
{
	SendMessage(WM_SYSCOMMAND, SC_MAXIMIZE);
}
void CMainDlg::OnRestore()
{
	SendMessage(WM_SYSCOMMAND, SC_RESTORE);
}
void CMainDlg::OnMinimize()
{
	SendMessage(WM_SYSCOMMAND, SC_MINIMIZE);
}

void CMainDlg::OnSize(UINT nType, CSize size)
{
	SetMsgHandled(FALSE);
	
	SWindow *pBtnMax = FindChildByName(L"btn_max");
	SWindow *pBtnRestore = FindChildByName(L"btn_restore");
	if(!pBtnMax || !pBtnRestore) return;
	
	if (nType == SIZE_MAXIMIZED)
	{
		pBtnRestore->SetVisible(TRUE);
		pBtnMax->SetVisible(FALSE);
	}
	else if (nType == SIZE_RESTORED)
	{
		pBtnRestore->SetVisible(FALSE);
		pBtnMax->SetVisible(TRUE);
	}
}

void CMainDlg::OnBtnMenu()
{
	SMenuEx menu;
	menu.LoadMenu(_T("smenu:menu_lang"));
	CPoint pt;
	GetCursorPos(&pt);

	SMenuEx *pLangMenu = menu.GetSubMenu(50);
	pLangMenu->GetMenuItem(m_LangID)->SetAttribute(L"check",L"1");

	menu.TrackPopupMenu(0,pt.x,pt.y,m_hWnd,GetScale());
}

void CMainDlg::OnMenuCmd(UINT uNotifyCode, int nID, HWND wndCtl)
{
	if(nID == R.id.lang_cn || nID == R.id.lang_en)
	{
		FindChildByID(nID)->SetCheck(TRUE);
		OnLanguage(nID);
	}
	else if (nID == 102)
	{
		OnClose();
	}
}

void CMainDlg::OnBtnScale(int nID)
{
	int nScale = 100;
	switch(nID)
	{
	case R.id.btn_scale_100:
		nScale = 100;
		break;
	case R.id.btn_scale_125:
		nScale = 125;
		break;
	case R.id.btn_scale_150:
		nScale = 150;
		break;
	case R.id.btn_scale_200:
		nScale = 200;
		break;
	}
	int nCurScale = GetScale();
	CRect rcWnd = GetWindowRect();
	int OriWid = rcWnd.Width() * 100 / nCurScale;
	int OriHei = rcWnd.Height() * 100/ nCurScale;

	int nNewWid = OriWid * nScale / 100;
	int nNewHei = OriHei * nScale / 100;

	SDispatchMessage(UM_SETSCALE,nScale,0);

	if(!CSimpleWnd::IsZoomed()){
		SetWindowPos(0, 0, 0, nNewWid, nNewHei, SWP_NOZORDER | SWP_NOMOVE);
	}
}


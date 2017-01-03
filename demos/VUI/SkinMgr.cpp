#include "StdAfx.h"
#include "SkinMgr.h"

CSkinMgr::CSkinMgr(void) : SHostWnd(_T("LAYOUT:XML_WINSKINS"))
{
	m_bLayoutInited=FALSE;
}

CSkinMgr::~CSkinMgr(void)
{
}

BOOL CSkinMgr::OnInitDialog( HWND hWnd, LPARAM lParam )
{
	m_bLayoutInited=TRUE;

	return 0;
}

void CSkinMgr::OnBtnMaxspeed()		//极速
{
	SMessageBox(NULL,_T("极速"),_T("haha"),MB_OK|MB_ICONEXCLAMATION);
}

void CSkinMgr::OnBtnDeepblue()		//深湖蓝
{
	SMessageBox(NULL,_T("深湖蓝"),_T("haha"),MB_OK|MB_ICONEXCLAMATION);
}

void CSkinMgr::OnBtnSelfdefine()	//自定义
{
	SMessageBox(NULL,_T("自定义"),_T("haha"),MB_OK|MB_ICONEXCLAMATION);
}

void CSkinMgr::OnBtnBigbang()		//大片风暴
{
	SMessageBox(NULL,_T("大片风暴"),_T("haha"),MB_OK|MB_ICONEXCLAMATION);
}

void CSkinMgr::OnBtnPrev()			//上一个皮肤
{
	SMessageBox(NULL,_T("上一个皮肤"),_T("haha"),MB_OK|MB_ICONEXCLAMATION);
}

void CSkinMgr::OnBtnChoosing1()			//已有的皮肤1
{
	SMessageBox(NULL,_T("已有的皮肤1"),_T("haha"),MB_OK|MB_ICONEXCLAMATION);
}

void CSkinMgr::OnBtnChoosing2()			//已有的皮肤2
{
	SMessageBox(NULL,_T("已有的皮肤2"),_T("haha"),MB_OK|MB_ICONEXCLAMATION);
}

void CSkinMgr::OnBtnNext()				//下一个皮肤
{
	SMessageBox(NULL,_T("下一个皮肤"),_T("haha"),MB_OK|MB_ICONEXCLAMATION);
}

void CSkinMgr::OnBtnColor1()				//配色1
{
	SMessageBox(NULL,_T("配色1"),_T("haha"),MB_OK|MB_ICONEXCLAMATION);
}

void CSkinMgr::OnBtnColor2()				//配色2
{
	SMessageBox(NULL,_T("配色2"),_T("haha"),MB_OK|MB_ICONEXCLAMATION);
}

void CSkinMgr::OnBtnColor3()				//配色3
{
	SMessageBox(NULL,_T("配色3"),_T("haha"),MB_OK|MB_ICONEXCLAMATION);
}

void CSkinMgr::OnBtnColor4()				//配色4
{
	SMessageBox(NULL,_T("配色4"),_T("haha"),MB_OK|MB_ICONEXCLAMATION);
}

void CSkinMgr::OnBtnColor5()				//配色5
{
	SMessageBox(NULL,_T("配色5"),_T("haha"),MB_OK|MB_ICONEXCLAMATION);
}

void CSkinMgr::OnBtnColor6()				//配色6
{
	SMessageBox(NULL,_T("配色6"),_T("haha"),MB_OK|MB_ICONEXCLAMATION);
}

void CSkinMgr::OnBtnColor7()				//配色7
{
	SMessageBox(NULL,_T("配色7"),_T("haha"),MB_OK|MB_ICONEXCLAMATION);
}

void CSkinMgr::OnBtnColor8()				//配色8
{
	SMessageBox(NULL,_T("配色8"),_T("haha"),MB_OK|MB_ICONEXCLAMATION);
}

void CSkinMgr::OnBtnColor9()				//配色9
{
	SMessageBox(NULL,_T("配色9"),_T("haha"),MB_OK|MB_ICONEXCLAMATION);
}

void CSkinMgr::OnBtnColor10()				//配色10
{
	SMessageBox(NULL,_T("配色10"),_T("haha"),MB_OK|MB_ICONEXCLAMATION);
}

void CSkinMgr::OnBtnColor11()				//配色11
{
	SMessageBox(NULL,_T("配色11"),_T("haha"),MB_OK|MB_ICONEXCLAMATION);
}

void CSkinMgr::OnBtnColor12()				//配色12
{
	SMessageBox(NULL,_T("配色12"),_T("haha"),MB_OK|MB_ICONEXCLAMATION);
}
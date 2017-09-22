#include <cstdlib>
#include <time.h>
#include "stdafx.h"
#include "MyHelper.h"

MyHelper::MyHelper()
{
	std::srand(time(0));
	m_pWindow = NULL;
}

MyHelper* MyHelper::Instance()
{
	static MyHelper myHelper;
	return &myHelper;
}

MyHelper::~MyHelper()
{

}

// 初始化窗口信息
void MyHelper::InitWindow(SOUI::SWindow* pWindow)
{
	m_pWindow = pWindow;
}

// 获取随机数
// modular  随机数发生器范围，0开始
// excepts  在随机数发生器范围内的不计入随机运算的数字
int MyHelper::Random(int modular, std::vector<int> excepts)
{
	// 判断：excepts 是否占用了所有的 modular 的值
	if (modular <= excepts.size()) return -1;
	// 产生一个值
	int random = std::rand() % modular;
	// 判断该值是否被剔除不考虑，如果是的话，则再生成一个随机数，直到生成的随机数
	// 值不在被剔除值的范围中之后，才确定返回其值
	while (std::find(excepts.begin(), excepts.end(), random) != excepts.end()) {
		random = std::rand() % modular;
	}
	return random;
}

// 写入日志
void MyHelper::WriteLog(SOUI::SStringW strMsg)
{
	if (m_pWindow != NULL) {
		SOUI::SRichEdit* pEdit = 
			m_pWindow->FindChildByName2<SOUI::SRichEdit>(L"edit_log");
		assert(pEdit);
		SOUI::SStringW strOriginalMsg = pEdit->GetWindowTextW();
		strOriginalMsg.Trim();
		SOUI::SStringW strNowMsg;
		strNowMsg.Format(L"%s\n%s", strOriginalMsg, strMsg);
		pEdit->SetWindowTextW(strNowMsg);
		pEdit->SetScrollPos(TRUE, -1, TRUE);
	}
}

#pragma once

class CSouiSubWnd : public SOUI::SHostWnd
{
public:
	CSouiSubWnd(void);
	~CSouiSubWnd(void);

	void OnBtnOpenSoui();
	EVENT_MAP_BEGIN()
		EVENT_NAME_COMMAND(L"btn_open_soui",OnBtnOpenSoui)
	EVENT_MAP_END()

protected:
	virtual void OnFinalMessage(HWND hWnd);
};

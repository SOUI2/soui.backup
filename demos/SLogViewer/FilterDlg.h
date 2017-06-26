#pragma once
#include <souicoll.h>
class CMainDlg;

interface IFilterChangeListener
{
	virtual void OnTagChange(const SArray<SStringW> & lstTags) PURE;
	virtual void OnPidChange(const SArray<UINT> & lstPids) PURE;
	virtual void OnTidChange(const SArray<UINT> & lstTids) PURE;
};

class CFilterDlg : public SHostWnd , public IFilterChangeListener
{
public:
	CFilterDlg(CMainDlg *pMainDlg);
	~CFilterDlg(void);

	void UpdateTags(const SArray<SStringW> & lstTags);
	void UpdatePids(const SArray<UINT> & lstPids);
	void UpdateTids(const SArray<UINT> & lstTids);
protected:

	void OnInit(EventArgs *e);
	void OnTabChanged(EventArgs *e);
	void OnBtnSelectAll();
	void OnBtnClearAll();

	EVENT_MAP_BEGIN()
		EVENT_ID_COMMAND(R.id.btn_select_all,OnBtnSelectAll)
		EVENT_ID_COMMAND(R.id.btn_clear_all,OnBtnClearAll)
		EVENT_HANDLER(EventTabSelChanged::EventID,OnTabChanged)
		EVENT_HANDLER(EventInit::EventID,OnInit)
	EVENT_MAP_END()

	virtual void OnFinalMessage(HWND hWnd);

	virtual void OnTagChange(const SArray<SStringW> & lstTags);
	virtual void OnPidChange(const SArray<UINT> & lstPid);
	virtual void OnTidChange(const SArray<UINT> & lstTid);

	enum {
		FilterTag=0,
		FilterPid,
		FilterTid,

		FilterCount,
	};
	SListView * m_lvFilters[FilterCount];
	int			m_iTab;
	CMainDlg  * m_pMainDlg;
};

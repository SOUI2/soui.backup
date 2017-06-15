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

	EVENT_MAP_BEGIN()
		EVENT_HANDLER(EventInit::EventID,OnInit)
	EVENT_MAP_END()

	virtual void OnFinalMessage(HWND hWnd);

	virtual void OnTagChange(const SArray<SStringW> & lstTags);
	virtual void OnPidChange(const SArray<UINT> & lstPid);
	virtual void OnTidChange(const SArray<UINT> & lstTid);

	SListView * m_lvTags;
	SListView * m_lvPid;
	SListView * m_lvTid;
	CMainDlg  * m_pMainDlg;
};

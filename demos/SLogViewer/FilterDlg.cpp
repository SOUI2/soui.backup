#include "StdAfx.h"
#include "FilterDlg.h"
#include <helper/SAdapterBase.h>
#include "MainDlg.h"

class CFilterTagAdapter: public SAdapterBase
{
	struct TagCheck
	{
		SStringW tag;
		bool	 bSelected;
	};
public:

	void SetListener(IFilterChangeListener *pListener)
	{
		m_listener = pListener;
	}

	void SetTags(const SArray<SStringW> & tags)
	{
		m_lstTagCheck.RemoveAll();
		for(int i=0;i<tags.GetCount();i++)
		{
			TagCheck obj={tags[i],true};
			m_lstTagCheck.Add(obj);
		}
		qsort(m_lstTagCheck.GetData(),m_lstTagCheck.GetCount(),sizeof(TagCheck),TagCheckCmp);
		notifyDataSetChanged();
	}
protected:
	static int TagCheckCmp(const void * p1, const void*p2)
	{
		const TagCheck *tag1=(const TagCheck*)p1;
		const TagCheck *tag2=(const TagCheck*)p2;
		return tag1->tag.Compare(tag2->tag);
	}

	virtual int getCount()
	{
		return m_lstTagCheck.GetCount();
	}

	virtual void getView(int position, SWindow * pItem, pugi::xml_node xmlTemplate)
	{
		if(pItem->GetChildrenCount() == 0)
		{
			pItem->InitFromXml(xmlTemplate);
		}

		SCheckBox *pCheck = pItem->FindChildByID2<SCheckBox>(R.id.chk_tag);
		pCheck->SetUserData(position);
		pCheck->SetWindowText(m_lstTagCheck[position].tag);
		pCheck->GetEventSet()->setMutedState(true);
		pCheck->SetCheck(m_lstTagCheck[position].bSelected);
		pCheck->GetEventSet()->setMutedState(false);
		pCheck->GetEventSet()->subscribeEvent(EventCmd::EventID,Subscriber(&CFilterTagAdapter::OnCheck,this));
	}

	bool OnCheck(EventArgs *e)
	{
		SCheckBox *pCheck = sobj_cast<SCheckBox>(e->sender);
		int pos = (int)pCheck->GetUserData();
		m_lstTagCheck[pos].bSelected = !!pCheck->IsChecked();
		NotifyListener();
		return true;
	}
private:
	void NotifyListener()
	{
		if(!m_listener) return;
		SArray<SStringW> lstTags;
		for(int i=0;i<m_lstTagCheck.GetCount();i++)
		{
			if(m_lstTagCheck[i].bSelected)
				lstTags.Add(m_lstTagCheck[i].tag);
		}
		m_listener->OnTagChange(lstTags);
	}

	SArray<TagCheck> m_lstTagCheck;

	IFilterChangeListener		* m_listener;
};


class CFilterPidAdapter: public SAdapterBase
{
	struct PidCheck
	{
		UINT pid;
		bool	 bSelected;
	};
public:

	void SetListener(IFilterChangeListener *pListener)
	{
		m_listener = pListener;
	}

	void SetPids(const SArray<UINT> & pids)
	{
		m_lstPidCheck.RemoveAll();
		for(int i=0;i<pids.GetCount();i++)
		{
			PidCheck obj={pids[i],true};
			m_lstPidCheck.Add(obj);
		}
		qsort(m_lstPidCheck.GetData(),m_lstPidCheck.GetCount(),sizeof(PidCheck),PidCheckCmp);

		notifyDataSetChanged();
	}
protected:
	static int PidCheckCmp(const void * p1, const void*p2)
	{
		const PidCheck *pid1=(const PidCheck*)p1;
		const PidCheck *pid2=(const PidCheck*)p2;
		return (int)pid1->pid - (int)pid2->pid;
	}

	virtual int getCount()
	{
		return m_lstPidCheck.GetCount();
	}

	virtual void getView(int position, SWindow * pItem, pugi::xml_node xmlTemplate)
	{
		if(pItem->GetChildrenCount() == 0)
		{
			pItem->InitFromXml(xmlTemplate);
		}

		SCheckBox *pCheck = pItem->FindChildByID2<SCheckBox>(R.id.chk_pid);
		pCheck->SetUserData(position);
		pCheck->SetWindowText(SStringT().Format(_T("%u"),m_lstPidCheck[position].pid));
		pCheck->GetEventSet()->setMutedState(true);
		pCheck->SetCheck(m_lstPidCheck[position].bSelected);
		pCheck->GetEventSet()->setMutedState(false);
		pCheck->GetEventSet()->subscribeEvent(EventCmd::EventID,Subscriber(&CFilterPidAdapter::OnCheck,this));
	}

	bool OnCheck(EventArgs *e)
	{
		SCheckBox *pCheck = sobj_cast<SCheckBox>(e->sender);
		int pos = (int)pCheck->GetUserData();
		m_lstPidCheck[pos].bSelected = !!pCheck->IsChecked();
		NotifyListener();
		return true;
	}
private:
	void NotifyListener()
	{
		if(!m_listener) return;
		SArray<UINT> lstPids;
		for(int i=0;i<m_lstPidCheck.GetCount();i++)
		{
			if(m_lstPidCheck[i].bSelected)
				lstPids.Add(m_lstPidCheck[i].pid);
		}
		m_listener->OnPidChange(lstPids);
	}

	SArray<PidCheck> m_lstPidCheck;

	IFilterChangeListener		* m_listener;
};


class CFilterTidAdapter: public SAdapterBase
{
	struct TidCheck
	{
		UINT tid;
		bool	 bSelected;
	};
public:

	void SetListener(IFilterChangeListener *pListener)
	{
		m_listener = pListener;
	}

	void SetTids(const SArray<UINT> & tids)
	{
		m_lstTidCheck.RemoveAll();
		for(int i=0;i<tids.GetCount();i++)
		{
			TidCheck obj={tids[i],true};
			m_lstTidCheck.Add(obj);
		}
		qsort(m_lstTidCheck.GetData(),m_lstTidCheck.GetCount(),sizeof(TidCheck),TidCheckCmp);
		notifyDataSetChanged();
	}

protected:

	static int TidCheckCmp(const void * p1, const void*p2)
	{
		const TidCheck *tid1=(const TidCheck*)p1;
		const TidCheck *tid2=(const TidCheck*)p2;
		return (int)tid1->tid - (int)tid2->tid;
	}

	virtual int getCount()
	{
		return m_lstTidCheck.GetCount();
	}

	virtual void getView(int position, SWindow * pItem, pugi::xml_node xmlTemplate)
	{
		if(pItem->GetChildrenCount() == 0)
		{
			pItem->InitFromXml(xmlTemplate);
		}

		SCheckBox *pCheck = pItem->FindChildByID2<SCheckBox>(R.id.chk_tid);
		pCheck->SetUserData(position);
		pCheck->SetWindowText(SStringT().Format(_T("%u"),m_lstTidCheck[position].tid));
		pCheck->GetEventSet()->setMutedState(true);
		pCheck->SetCheck(m_lstTidCheck[position].bSelected);
		pCheck->GetEventSet()->setMutedState(false);
		pCheck->GetEventSet()->subscribeEvent(EventCmd::EventID,Subscriber(&CFilterTidAdapter::OnCheck,this));
	}

	bool OnCheck(EventArgs *e)
	{
		SCheckBox *pCheck = sobj_cast<SCheckBox>(e->sender);
		int pos = (int)pCheck->GetUserData();
		m_lstTidCheck[pos].bSelected = !!pCheck->IsChecked();
		NotifyListener();
		return true;
	}
private:
	void NotifyListener()
	{
		if(!m_listener) return;
		SArray<UINT> lstTids;
		for(int i=0;i<m_lstTidCheck.GetCount();i++)
		{
			if(m_lstTidCheck[i].bSelected)
				lstTids.Add(m_lstTidCheck[i].tid);
		}
		m_listener->OnTidChange(lstTids);
	}

	SArray<TidCheck> m_lstTidCheck;

	IFilterChangeListener		* m_listener;
};





CFilterDlg::CFilterDlg(CMainDlg *pMainDlg):SHostWnd(UIRES.LAYOUT.dlg_filter),m_pMainDlg(pMainDlg)
{
}

CFilterDlg::~CFilterDlg(void)
{
}

void CFilterDlg::OnFinalMessage(HWND hWnd)
{
	__super::OnFinalMessage(hWnd);
	delete this;
}

void CFilterDlg::OnInit(EventArgs *e)
{
	{
		m_lvTags = FindChildByID2<SListView>(R.id.lv_tags);
		CFilterTagAdapter *pAdapter = new CFilterTagAdapter;
		pAdapter->SetListener(this);
		m_lvTags->SetAdapter(pAdapter);
		pAdapter->Release();
	}

	{
		m_lvPid = FindChildByID2<SListView>(R.id.lv_pid);
		CFilterPidAdapter *pAdapter = new CFilterPidAdapter;
		pAdapter->SetListener(this);
		m_lvPid->SetAdapter(pAdapter);
		pAdapter->Release();
	}

	{
		m_lvTid = FindChildByID2<SListView>(R.id.lv_tid);
		CFilterTidAdapter *pAdapter = new CFilterTidAdapter;
		pAdapter->SetListener(this);
		m_lvTid->SetAdapter(pAdapter);
		pAdapter->Release();
	}

}

void CFilterDlg::UpdateTags(const SArray<SStringW> & lstTags)
{
	CFilterTagAdapter * pAdapter = (CFilterTagAdapter*)m_lvTags->GetAdapter();
	pAdapter->SetTags(lstTags);
}

void CFilterDlg::UpdatePids(const SArray<UINT> & lstPids)
{
	CFilterPidAdapter * pAdapter = (CFilterPidAdapter*)m_lvPid->GetAdapter();
	pAdapter->SetPids(lstPids);
}

void CFilterDlg::UpdateTids(const SArray<UINT> & lstTids)
{
	CFilterTidAdapter * pAdapter = (CFilterTidAdapter*)m_lvTid->GetAdapter();
	pAdapter->SetTids(lstTids);
}

void CFilterDlg::OnTagChange(const SArray<SStringW> & lstTags)
{
	m_pMainDlg->UpdateFilterTags(lstTags);
}

void CFilterDlg::OnPidChange(const SArray<UINT> & lstPid)
{
	m_pMainDlg->UpdateFilterPids(lstPid);
}

void CFilterDlg::OnTidChange(const SArray<UINT> & lstTid)
{
	m_pMainDlg->UpdateFilterTids(lstTid);
}

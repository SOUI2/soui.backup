#pragma once
#include <vsstyle.h>
#include "ui\VirtualDlgLayout.h"
#include <helper/SAdapterBase.h>
#include <vector>
#include <functional>

class ApproverListAdapter : public SMcAdapterBase
{
protected:
	struct ApproItemInfo
	{
		UINT				uId;							// id
		bool				bChecked;
		SStringT			sName;						// Ãû³Æ
	};
public:
	ApproverListAdapter(std::function<void()> fun)
		: m_funItemClick(&ApproverListAdapter::OnItemClick, this)
		, m_funUpdate(fun)
	{
	
	}

    ~ApproverListAdapter()
	{
		for (size_t i=0; i<m_ItemInfoArr.size(); ++i)
		{
			delete m_ItemInfoArr[i];
		}
		m_ItemInfoArr.clear();
	}
public:
	int Add(UINT uId, LPCTSTR lpName)
	{
		ApproItemInfo* pInfo =  new ApproItemInfo;
		pInfo->uId = uId;
		pInfo->sName = lpName;
		pInfo->bChecked = false;

		m_ItemInfoArr.push_back(pInfo);

		notifyDataSetChanged();

		return m_ItemInfoArr.size() - 1;
	}
	
	bool IsChecked(UINT nIndex)
	{
		if(nIndex >= m_ItemInfoArr.size())
			return false;
		
		return m_ItemInfoArr[nIndex]->bChecked;
	}

	bool Get(UINT nIndex, UINT& uId, SStringT& sName)
	{
		if(nIndex >= m_ItemInfoArr.size())
			return false;

		auto pInfo = m_ItemInfoArr[nIndex];
		uId = pInfo->uId;
		sName = pInfo->sName;
		return true;
	}

	void SetAllSelect(bool bSel)
	{
		for each(auto var in m_ItemInfoArr)
		{
			var->bChecked = bSel;
		}

		notifyDataSetChanged();
	}

	void DeleteAll()
	{
		for (size_t i=0; i<m_ItemInfoArr.size(); ++i)
		{
			delete m_ItemInfoArr[i];
		}
		m_ItemInfoArr.clear();
		notifyDataSetChanged();
	}
public:
	virtual int getCount()
	{
		return static_cast<int>(m_ItemInfoArr.size());
	}
protected:    
	virtual void InitByTemplate(pugi::xml_node xmlTemplate)
	{
		
	}
   

  
	//virtual SIZE getViewDesiredSize(int position, SOUI::SWindow *pItem, LPCRECT prcContainer)
	//{

	//}
	/*virtual int getItemViewType(int position)
	{

	}*/
	virtual SStringT GetColumnName(int iCol) const
	{
		return L"col_nick";
	}
	virtual void getView(int nPosition, SWindow* pItem, pugi::xml_node xmlTemplate)
	{		
		if(0 == pItem->GetChildrenCount())
		{
			pItem->InitFromXml(xmlTemplate);
		}

		int nCount = static_cast<int>(m_ItemInfoArr.size());
		if(nPosition >= nCount)
			return ;

		auto pInfo = m_ItemInfoArr[nPosition];
		if(NULL == pInfo)
			return ;

		SImageWnd* pImg = pItem->FindChildByName2<SImageWnd>(L"ml_appro_check");
		if(NULL != pImg)
		{
			int nCheckIcon = pInfo->bChecked ? CBS_CHECKEDNORMAL : CBS_UNCHECKEDNORMAL;
			--nCheckIcon;
			pImg->SetIcon(nCheckIcon);
		}
			
		SStatic* pName = pItem->FindChildByName2<SStatic>(L"ml_appro_name");
		if (NULL != pName)
		{
			pName->SetWindowText(pInfo->sName);
		}

		pItem->GetEventSet()->subscribeEvent(EVT_ITEMPANEL_CLICK, m_funItemClick);
	}
	
	bool OnItemClick(EventArgs* e)
	{
		EventItemPanelClick* pEvt = sobj_cast<EventItemPanelClick>(e);
		if(NULL == pEvt)
			return true;

		SItemPanel* pItem = sobj_cast<SItemPanel>(e->sender); 

		int nIndex = static_cast<int>(pItem->GetItemIndex());
		int nCount = static_cast<int>(m_ItemInfoArr.size());
		if(nIndex >= nCount) 
			return true;

		auto pInfo = m_ItemInfoArr[nIndex];
		if(NULL == pInfo) return true;

		pInfo->bChecked = !pInfo->bChecked;
		notifyDataSetChanged();

		m_funUpdate();
		return true;
	}
private:
	std::vector<ApproItemInfo*>			m_ItemInfoArr;

	MemberFunctionSlot<ApproverListAdapter, EventArgs>			m_funItemClick;
	std::function<void()> m_funUpdate;
};

typedef std::function<void(__int64, LPCTSTR)>			funApproverCallBack;  // »Øµ÷
// 
class ApproverLayout : public VirtualDlgLayout
{
public:
	ApproverLayout(SWindow* pRoot, funApproverCallBack funBack);

	~ApproverLayout(void);

	void Init(__int64 lBodyId);
	void AddApprover(UINT uId, LPCTSTR lpUserAlias);
	
protected:
	bool OnEventOKCmd(EventCmd* pEvt);
	bool OnEventCheckCmd(EventCmd* pEvt);
	void OnAdapterUpdate();
private:
	__int64								m_lBodyId;
	funApproverCallBack			m_funCallBack;
protected:
	SCheckBox*						m_pCheckAllSel;
	SImageButton*					m_pBtnOK;
	ApproverListAdapter*			m_pAdapter;
};


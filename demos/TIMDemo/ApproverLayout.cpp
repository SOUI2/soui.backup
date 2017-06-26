#include "StdAfx.h"
#include "ApproverLayout.h"

ApproverLayout::ApproverLayout(SWindow* pRoot, funApproverCallBack fun)
	: VirtualDlgLayout(pRoot, _T("layout:layout_approver"))
	, m_funCallBack(fun)
{
	InitWnd(m_pCheckAllSel, L"check_allapprover");
	InitWnd(m_pBtnOK, L"btn_approver_ok");

	subscribeEvent(m_pCheckAllSel, &ApproverLayout::OnEventCheckCmd, this);
	subscribeEvent(m_pBtnOK, &ApproverLayout::OnEventOKCmd, this);

	m_pAdapter = new ApproverListAdapter(std::bind(&ApproverLayout::OnAdapterUpdate, this));
	
	SMCListView* pMcList;
	InitWnd(pMcList, L"mclist_approver");
	pMcList->SetAdapter(m_pAdapter);
	m_pAdapter->Release();
	
	/*m_pAdapter->Add(121, L"aaad");
	m_pAdapter->Add(121, L"aaad");
	m_pAdapter->Add(121, L"aaad");
	m_pAdapter->Add(121, L"aaad");
	m_pAdapter->Add(121, L"aaad");

	m_pAdapter->Add(121, L"aaad");

	m_pAdapter->Add(121, L"aaad");
	m_pAdapter->Add(121, L"aaad");
	m_pAdapter->Add(121, L"aaad");
	m_pAdapter->Add(121, L"aaad");
	m_pAdapter->Add(121, L"aaad");

	m_pAdapter->Add(121, L"aaad");
	m_pAdapter->Add(121, L"aaad");
	m_pAdapter->Add(121, L"aaad");
	m_pAdapter->Add(121, L"aaad");
	m_pAdapter->Add(121, L"aaad");
	m_pAdapter->Add(121, L"aaad");

	m_pAdapter->Add(121, L"aaad");*/
}


ApproverLayout::~ApproverLayout(void)
{
}

void ApproverLayout::Init(__int64 lBodyId)
{
	m_pAdapter->DeleteAll();
	m_pCheckAllSel->SetCheck(FALSE);
	m_pBtnOK->EnableWindow(FALSE, TRUE);

	m_lBodyId = lBodyId;
}

void ApproverLayout::AddApprover(UINT uId, LPCTSTR lpUserAlias)
{
	m_pAdapter->Add(uId, lpUserAlias);
}


void ApproverLayout::OnAdapterUpdate()
{		
	UINT uSelCount = 0;

	UINT uCount = m_pAdapter->getCount();
	for (UINT i=0; i<uCount; ++i)
	{
		if(!m_pAdapter->IsChecked(i))
			continue;
			
		++uSelCount;
	}
		
	if(uSelCount == uCount)
		m_pCheckAllSel->SetCheck(TRUE);
	else 
		m_pCheckAllSel->SetCheck(FALSE);

	if(uSelCount > 0)
	{
		m_pBtnOK->EnableWindow(TRUE, TRUE);
	}
	else
	{
		m_pBtnOK->EnableWindow(FALSE, TRUE);
	}
}

bool ApproverLayout::OnEventCheckCmd(EventCmd* pEvt)
{
	SCheckBox* pCheck = sobj_cast<SCheckBox>(pEvt->sender);
	if(NULL == pCheck) return true;
	
	bool bSel = TRUE == pCheck->IsChecked();

	m_pAdapter->SetAllSelect(bSel);
	OnAdapterUpdate();
	return true;
}

bool ApproverLayout::OnEventOKCmd(EventCmd* pEvt)
{
	if(NULL == pEvt) return true;

	if(0 == m_lBodyId)
	{
		return true;
	}

	SStringT szApproverText;
	UINT uId;
	SStringT sName;
	UINT uCount = m_pAdapter->getCount();
	for (UINT i=0; i<uCount; ++i)
	{
		if(!m_pAdapter->IsChecked(i))
			continue;

		m_pAdapter->Get(i, uId, sName);

		szApproverText.AppendFormat(_T("%d,"), uId);

	}
	szApproverText.TrimRight(',');

	m_funCallBack(m_lBodyId, szApproverText);
		
	ShowLayout(false);
	return true;
}






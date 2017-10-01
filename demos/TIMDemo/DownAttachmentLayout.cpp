#include "StdAfx.h"
#include "DownAttachmentLayout.h"


DownAttachmentLayout::DownAttachmentLayout(SWindow* pRoot)
	: VirtualDlgLayout(pRoot, _T("layout:layout_downattachment"))
{
	InitWnd(m_pImgFile, L"img_icon");
	InitWnd(m_pTextFile, L"text_attachname");
	InitWnd(m_pProgress, L"progress_");
	
	subscribeEvent(L"btn_back", &DownAttachmentLayout::OnEventOKCmd, this);
}


DownAttachmentLayout::~DownAttachmentLayout(void)
{
}


void DownAttachmentLayout::Init(UINT nAttachId, LPCTSTR lpAttachName, int nIconIndex)
{
	m_nAttachId = nAttachId;
	m_pTextFile->SetWindowText(lpAttachName);
	m_pImgFile->SetIcon(nIconIndex);

	m_pProgress->SetValue(0);
}

void DownAttachmentLayout::Update(UINT nAttachId, int nPercent)
{
	if(m_nAttachId != nAttachId)
		return ;

	m_pProgress->SetValue(0);
}

bool DownAttachmentLayout::OnEventOKCmd(EventCmd* pEvt)
{
	if(NULL == pEvt) return true;

	m_nAttachId = 0;
	m_pProgress->SetValue(0);
			
	ShowLayout(false);
	return true;
}






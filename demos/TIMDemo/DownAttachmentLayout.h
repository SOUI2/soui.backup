#pragma once
#include "ui\VirtualDlgLayout.h"

class DownAttachmentLayout : public VirtualDlgLayout
{
public:
	DownAttachmentLayout(SWindow* pRoot);

	~DownAttachmentLayout(void);

	void Init(UINT nAttachId, LPCTSTR lpAttachName, int nIconIndex);
	void Update(UINT nAttachId, int nPercent);
protected:
	bool OnEventOKCmd(EventCmd* pEvt);
private:
	UINT									m_nAttachId;

protected:
	SImageWnd*						m_pImgFile;
	SStatic*								m_pTextFile;
	SProgress*							m_pProgress;
};


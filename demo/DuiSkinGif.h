/********************************************************************
	created:	2012/12/27
	created:	27:12:2012   14:55
	filename: 	DuiSkinGif.h
	file base:	DuiSkinGif
	file ext:	h
	author:		huangjianxiong
	
	purpose:	自定义皮肤对象
*********************************************************************/
#pragma once
#include "../soui/include/DuiSkinBase.h"
#include "../soui/include/duiimage.h"

 class CDuiSkinGif : public CDuiSkinBase
 {
 	SOUI_CLASS_NAME(CDuiSkinGif, "gif")
 public:
 	CDuiSkinGif():m_nFrames(0),m_iFrame(0),m_pFrameDelay(NULL)
 	{
 
 	}
 	virtual ~CDuiSkinGif()
 	{
 		if(m_pFrameDelay) delete [] m_pFrameDelay;
 	}
 	virtual void Draw(HDC dc, CRect rcDraw, DWORD dwState,BYTE byAlpha=0xFF);
 
 	virtual int GetStates(){return m_nFrames;}
 	virtual void OnAttributeChanged(const CDuiStringA & strAttrName,BOOL bLoading,HRESULT hRet);
 	virtual SIZE GetSkinSize()
 	{
 		SIZE sz={0};
 		if(m_pDuiImg)
 		{
 			m_pDuiImg->GetImageSize(sz);
 		}
 		return sz;
 	}
 
 	virtual void SetImage(IDuiImage *pImg)
 	{
 		__super::SetImage(pImg);
 		OnSetImage();
 	}
 
 	long GetFrameDelay(int iFrame=-1);
 	void ActiveNextFrame();
 	void SelectActiveFrame(int iFrame);
 protected:
 	void OnSetImage();
 	int m_nFrames;
 	int m_iFrame;
 	long *m_pFrameDelay;
 	static GUID ms_Guid;
 
 };
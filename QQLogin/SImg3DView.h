//////////////////////////////////////////////////////////////////////////
//   File Name: Dui3DView.h
// Description: SImg3DView
//     Creator: ZhangZhiBin QQ->276883782
//     Version: 2014.02.06 - 1.0 - Create
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "image3d/3dTransform.h"
using namespace IMAGE3D;

namespace SOUI
{

class SImg3DView :
	public SWindow
{
	SOUI_CLASS_NAME(SImg3DView, L"image3dview")
public:
	SImg3DView(void);
	~SImg3DView(void);

	PARAM3DTRANSFORM & Get3dParam(){return m_3dparam;}

	void Update();

	SOUI_ATTRS_BEGIN()
		ATTR_INT(L"rotate-x", m_3dparam.nRotateX,TRUE)
		ATTR_INT(L"rotate-y", m_3dparam.nRotateY,TRUE)
		ATTR_INT(L"rotate-z", m_3dparam.nRotateZ,TRUE)
		ATTR_INT(L"offset-z", m_3dparam.nOffsetZ,TRUE)
	SOUI_ATTRS_END()
protected:
	void OnSize(UINT nType, CSize size);
	void OnPaint(IRenderTarget *pRT);

	SOUI_MSG_MAP_BEGIN()
		MSG_WM_SIZE(OnSize)
		MSG_WM_PAINT_EX(OnPaint)
	SOUI_MSG_MAP_END()

	HBITMAP			m_hBmpOrig;
	HBITMAP			m_hBmpTrans;

	PARAM3DTRANSFORM	m_3dparam;
public:
  void SetBmpOrig(HBITMAP hBmpOrig) { m_hBmpOrig = hBmpOrig; }
};
}
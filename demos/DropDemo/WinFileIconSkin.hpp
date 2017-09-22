#ifndef __WINFILE_ICON_SKIN_HPP_
#define __WINFILE_ICON_SKIN_HPP_

#include "core/SSkinObjBase.h"
#include <commoncontrols.h>
//************************************
// 这个是 系统文件图标 的皮肤控件。 使用SHGetFileInfo 来获取图标索引  
// 使用之前先 注册 theApp->RegisterSkinFactory(TplSkinFactory<SSkinSystemIconList>());		//注册Skin
// 然后在skin.xml 里 添加 资源  <sysiconlist name="sysiconlist" flag="1" />  flag 表示图标大小类型 这个必须要有
// 
//************************************
class SSkinSystemIconList: public SSkinObjBase
{
	SOUI_CLASS_NAME(SSkinSystemIconList, L"sysiconlist")

public:
	SSkinSystemIconList()
		: m_hIconList(NULL)
	{			
		
	}
	virtual ~SSkinSystemIconList()
	{
		if(NULL != m_hIconList)
		{
			((IImageList*)m_hIconList)->Release();
			m_hIconList = NULL;
		}
	}

	virtual SIZE GetSkinSize()
	{
		return m_szSimpleIcon;
	}
protected:
	virtual void _Draw(IRenderTarget *pRT, LPCRECT rcDraw, DWORD dwState,BYTE byAlpha)
	{
		if(NULL == m_hIconList)
			return ;

		HICON hIcon = NULL;
		((IImageList*)m_hIconList)->GetIcon(dwState, ILD_NORMAL, &hIcon);
		if(NULL != hIcon)
		{
			pRT->DrawIconEx(rcDraw->left, rcDraw->top, hIcon, rcDraw->right-rcDraw->left, rcDraw->bottom-rcDraw->top, DI_NORMAL);
			DestroyIcon(hIcon);
			hIcon = NULL;
		}
	}
private:
	HIMAGELIST*		m_hIconList;
	SIZE					m_szSimpleIcon;

	SOUI_ATTRS_BEGIN()
		ATTR_CUSTOM(L"flag", OnAttrFlag)   //XML文件中指定的图片资源名,(type:name) flag 表示图标类型 有small large 
	SOUI_ATTRS_END()
protected:
	LRESULT OnAttrFlag(const SStringW &strValue,BOOL bLoading)
	{
		int nRet=0;   
		::StrToIntExW(strValue, STIF_SUPPORT_HEX, &nRet);

		//#define SHIL_LARGE          0   // normally 32x32
		//#define SHIL_SMALL          1   // normally 16x16
		//SHIL_EXTRALARGE获取48 * 48的图标， SHIL_JUMBO 获取256 * 256的图标。
		HRESULT hResult = ::SHGetImageList(nRet , IID_IImageList, (void**)&m_hIconList);
		if(S_OK != hResult)
			return S_FALSE;

		//计算图标大小 
		int nX = 0;
		int nY = 0;
		((IImageList*)m_hIconList)->GetIconSize(&nX, &nY);
		m_szSimpleIcon.cx = nX;
		m_szSimpleIcon.cy = nY;

		return S_OK;
	}
};
//////////////////////////////////////////////////////////////////////////
#endif // __WINFILE_ICON_SKIN_HPP_


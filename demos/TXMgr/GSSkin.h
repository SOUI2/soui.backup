/* $Copyright (c) 2006-2017 Green Net World
 * 
 * Author	:	cherish  version 1.0
 * DateTime	:	2017-3-31 14:34:31
 *
 *
 * GSSkin  功能
 */

#ifndef __GSSKIN_C0C4E8DA_8432_4BA3_9871_36AE94D85E84__
#define __GSSKIN_C0C4E8DA_8432_4BA3_9871_36AE94D85E84__

/** CGSSkin */
class  GSSkinImgList : public SSkinImgFrame
{
	SOUI_CLASS_NAME(GSSkinImgList, L"gsimglist")

public:
	GSSkinImgList();
	virtual ~GSSkinImgList();

	virtual SIZE GetSkinSize();

public:
	HRESULT			OnSetImgList(const SStringW & strValue, BOOL bLoading);
	HRESULT			OnSetImgPath(const SStringW & strValue, BOOL bLoading);
	void			OnSetSkinState(int nState){ m_nSkinState = nState; }
	
protected:
	virtual void	_Draw(IRenderTarget *pRT, LPCRECT rcDraw, DWORD dwState, BYTE byAlpha);
	HRESULT			OnAttrMask(const SStringW & strValue, BOOL bLoading);
	BOOL			_DrawAttrImg(IRenderTarget *pRT, LPCRECT rcDraw, const RECT& rcSrc, BYTE byAlpha);

	IBitmap*		GetImage(DWORD dwState);
	HRESULT			OnAttrOffset(const SStringW & strValue, BOOL bLoading);

	SOUI_ATTRS_BEGIN()
		ATTR_INT(L"size-x", m_nSizeX, FALSE)  //子图固定宽
		ATTR_INT(L"size-y", m_nSizeX, FALSE)  //子图固定高
		ATTR_CUSTOM(L"mask", OnAttrMask)	  //image.a
		ATTR_CUSTOM(L"imglist", OnSetImgList)	// 使用多文件方式
		ATTR_CUSTOM(L"imgpath", OnSetImgPath)	// 使用文件目录方式
		ATTR_CUSTOM(L"offset", OnAttrOffset)
	SOUI_ATTRS_END()

private:
	SIZE	GetOffsetSize(DWORD dwState);

private:
	int						m_nSizeX;
	int						m_nSizeY;
	int						m_nSkinState;

	CAutoRefPtr<IBitmap>    m_bmpMask;
	CAutoRefPtr<IBitmap>    m_bmpAttrImg;

	SArray<IBitmap *>		m_arrBitMap;				// imglist存储
	bool					m_bImgList;				// 使用imglist方式
	float fOffsetX, fOffsetY;    /**< 窗口坐标偏移量, x += fOffsetX * width, y += fOffsetY * height  */

};


#endif //__GSSKIN_C0C4E8DA_8432_4BA3_9871_36AE94D85E84__



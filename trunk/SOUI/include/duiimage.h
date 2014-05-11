/********************************************************************
	created:	2012/12/27
	created:	27:12:2012   16:45
	filename: 	duiimage.h
	file base:	duiimage
	file ext:	h
	author:		huangjianxiong

	purpose:
*********************************************************************/
#pragma once

#include "duiimage-i.h"
#include <GdiPlus.h>

namespace SOUI
{


	class SOUI_EXP CDuiBitmap : public IDuiImage
	{
	public:
		CDuiBitmap();
		virtual ~CDuiBitmap()
		{
			Clear();
		}

		virtual BOOL LoadFromResource(HINSTANCE hInst,LPCTSTR pszType,LPCTSTR pszName);
		virtual BOOL LoadFromFile(LPCTSTR pszPath);
		virtual BOOL LoadFromMemory(LPVOID pBuf,DWORD dwSize);

		CDuiBitmap(HBITMAP hBitmap);

		operator HBITMAP()  const;

		virtual void Clear();

		virtual BOOL IsEmpty() const;
		virtual BOOL GetImageSize(SIZE & sz);

		virtual int GetWidth();

		virtual int GetHeight();

		virtual BOOL BitBlt(HDC hdc,int x,int y,int nWid,int nHei,int xSrc,int ySrc,BYTE byAlpha=0xFF);

		virtual BOOL StretchBlt(HDC hdc,int x,int y,int nWid,int nHei,int xSrc,int ySrc,int nWidSrc,int nHeiSrc,BYTE byAlpha=0xFF);

		virtual BOOL TileBlt(HDC hdc,int x,int y,int nWid,int nHei,int xSrc,int ySrc,int nWidSrc,int nHeiSrc,BYTE byAlpha=0xFF);

		virtual void SetAttributes(pugi::xml_node xmlNode);

		void SetMaskColor(COLORREF crMask)
		{
			m_crMask=crMask;
		}

		HBITMAP Attach(HBITMAP hBmp)
		{
			HBITMAP hRet=m_hBitmap;
			m_hBitmap=hBmp;
			return hRet;
		}

		HBITMAP Detatch()
		{
			return Attach(NULL);
		}

		virtual void OnFinalRelease()
		{
			delete this;
		}
	protected:
		HBITMAP m_hBitmap;
		COLORREF	m_crMask;
	};

class SOUI_EXP CDuiImgX :public IDuiImage
{
public:
    CDuiImgX();

    CDuiImgX(Gdiplus::Image* pImg);

    virtual ~CDuiImgX()
    {
        Clear();
    }

    virtual BOOL LoadFromResource(HINSTANCE hInst,LPCTSTR pszType,LPCTSTR pszName);
    virtual BOOL LoadFromFile(LPCTSTR pszPath);
    virtual BOOL LoadFromMemory(LPVOID pBuf,DWORD dwSize);

    virtual void Clear();
    virtual BOOL IsEmpty() const;
    virtual BOOL GetImageSize(SIZE &sz);

    virtual int GetWidth();

    virtual int GetHeight();

    virtual BOOL BitBlt(HDC hdc,int x,int y,int nWid,int nHei,int xSrc,int ySrc,BYTE byAlpha=0xFF);

    virtual BOOL StretchBlt(HDC hdc,int x,int y,int nWid,int nHei,int xSrc,int ySrc,int nWidSrc,int nHeiSrc,BYTE byAlpha=0xFF);
    virtual BOOL TileBlt(HDC hdc,int x,int y,int nWid,int nHei,int xSrc,int ySrc,int nWidSrc,int nHeiSrc,BYTE byAlpha=0xFF);

    Gdiplus::Image * GetImage() const
    {
        return m_pImg;
    }

    virtual void OnFinalRelease()
    {
        delete this;
    }

    static BOOL GdiplusStartup();
    static void GdiplusShutdown();
protected:
    Gdiplus::Image *m_pImg;
    static ULONG_PTR s_gdiplusToken;
};

}//namespace SOUI
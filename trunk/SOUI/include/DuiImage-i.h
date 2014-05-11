#pragma once

#include "duiref.h"

namespace SOUI
{

class SOUI_EXP IDuiImage : public CDuiRef
{
public:
    virtual BOOL LoadFromResource(HINSTANCE hInst,LPCTSTR pszType,LPCTSTR pszResName)=NULL;
    virtual BOOL LoadFromFile(LPCTSTR pszPath)=NULL;
    virtual BOOL LoadFromMemory(LPVOID pBuf,DWORD dwSize)=NULL;

    virtual BOOL GetImageSize(SIZE & sz)=NULL;
    virtual int GetWidth()=NULL;
    virtual int GetHeight()=NULL;
    virtual BOOL IsEmpty() const
    {
        return TRUE;
    }
    virtual BOOL BitBlt(HDC hdc,int x,int y,int nWid,int nHei,int xSrc,int ySrc,BYTE byAlpha=0xFF)=NULL;
    virtual BOOL StretchBlt(HDC hdc,int x,int y,int nWid,int nHei,int xSrc,int ySrc,int nWidSrc,int nHeiSrc,BYTE byAlpha=0xFF)=NULL;
    virtual BOOL TileBlt(HDC hdc,int x,int y,int nWid,int nHei,int xSrc,int ySrc,int nWidSrc,int nHeiSrc,BYTE byAlpha=0xFF)=NULL;
    virtual void SetAttributes(pugi::xml_node xmlNode) {}

	BOOL operator!() const
	{
		return IsEmpty();
	}
};


class SOUI_EXP IDuiImgDecoder
{
public:
    virtual IDuiImage * CreateDuiImage(LPCTSTR pszType)=NULL;

    virtual void DestoryDuiImage(IDuiImage* pImg)=NULL;

    virtual LPCTSTR GetSupportTypes()= NULL;
};

}
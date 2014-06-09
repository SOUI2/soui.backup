#pragma once

class CDuiColor
{
public:
	static COLORREF RGBA(BYTE r,BYTE g,BYTE b,BYTE a=0xFF)
	{
		CDuiColor color(r,g,b,a);
		return color;
	}

	CDuiColor(BYTE r,BYTE g,BYTE b,BYTE a=0xFF):a(a),b(b),g(g),r(r)
	{

	}

	CDuiColor(COLORREF cr)
	{
		memcpy(this,&cr,4);
	}

	operator const COLORREF() const
	{
		DWORD cr;
		memcpy(&cr,this,4);
		return cr;
	}

	DWORD b:8;
	DWORD g:8;
	DWORD r:8;
	DWORD a:8;
};
//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft shared
// source or premium shared source license agreement under which you licensed
// this source code. If you did not accept the terms of the license agreement,
// you are not authorized to use this source code. For the terms of the license,
// please see the license agreement between you and Microsoft or, if applicable,
// see the SOURCE.RTF on your install media or the root of your tools installation.
// THE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES.
//
/*
 *	_DEVDSC.H
 *	
 *	Purpose:
 *		CDevDesc (Device Descriptor) class
 *	
 *	Authors:
 *		Original RichEdit code: David R. Fulmer
 *		Christian Fortini
 *		Murray Sargent
 */

#ifndef _DEVDSC_H
#define _DEVDSC_H


class CTxtEdit;

// device descriptor
class CDevDesc
{
protected:
	CTxtEdit * _ped;        // used to GetDC and ReleaseDC
	
	HDC 	_hdc;			// hdc for rendering device

	HDC		_hdcMeasure;	// hdc for measuring (used for metafiles).
	
	SHORT	_xPerInch;		// device units per horizontal "inch"
	SHORT	_yPerInch;		// device units per vertical "inch"

	HDC		GetScreenDC () const;
	void	ReleaseScreenDC (HDC hdc) const;

public:
	CDevDesc(CTxtEdit * ped) 		{_ped = ped; _xPerInch = 0; _yPerInch = 0;}
	~CDevDesc();

    // Test validity of device descriptor 
    // (whether SetDC has been properly called)
    BOOL    IsValid() const         {return _xPerInch != 0 && _yPerInch != 0;}

	BOOL 	IsMetafile() const;
	
	BOOL	SetDC(HDC hdc);

	void	SetMetafileDC(
				HDC hdcMetafile, 
				HDC hdcMeasure,
				LONG xMeasurePerInch,
				LONG yMeasurePerInch);

	HDC		GetMeasureDC(LONG *pyMeasurePerInch) const;

	void	ReleaseMeasureDC(HDC hdc) const;

	void 	ResetDC() { SetDC(NULL); }

	HDC	 	GetDC() const
	{
		if(_hdc)
			return _hdc;
		return GetScreenDC();
	}

	void	ReleaseDC(HDC hdc) const
	{
		if(!_hdc)
			ReleaseScreenDC(hdc);
	}

	// Methods for converting between pixels and himetric
	LONG 	HimetricXtoDX(LONG xHimetric) const { return W32->HimetricXtoDX(xHimetric, _xPerInch); }
	LONG 	HimetricYtoDY(LONG yHimetric) const { return W32->HimetricYtoDY(yHimetric, _yPerInch); }
	LONG	DXtoHimetricX(LONG dx)  const { return W32->DXtoHimetricX(dx, _xPerInch); }
	LONG	DYtoHimetricY(LONG dy) const { return W32->DYtoHimetricY(dy, _yPerInch); }

	LONG 	DXtoLX(LONG x) const;
	LONG 	DYtoLY(LONG y) const;
	void 	DPtoLP(POINT &ptDest, const POINT &ptSrc) const;
	void 	DRtoLR(RECT &rcDest, const RECT &rcSrc) const;
	
#ifdef DEBUG
	LONG	LXtoDX(LONG x) const;
	LONG	LYtoDY(LONG y) const;
#else
	LONG	LXtoDX(LONG x) const	{return ((x * _xPerInch) + LX_PER_INCH / 2) / LX_PER_INCH;}
	LONG	LYtoDY(LONG y) const	{return ((y * _yPerInch) + LY_PER_INCH / 2) / LY_PER_INCH;}
#endif
	void 	LPtoDP(POINT &ptDest, const POINT &ptSrc) const;
	void 	LRtoDR(RECT &rcDest, const RECT &rcSrc) const;

	BOOL 	SameDevice(const CDevDesc *pdd) const
	{
		return (_xPerInch == pdd->_xPerInch) && (_yPerInch == pdd->_yPerInch)
			? TRUE : FALSE;
	}

	LONG	ConvertXToDev(LONG x, const CDevDesc *pdd) const
	{
		return MulDiv(x, _xPerInch, pdd->_xPerInch);
	}

	LONG	ConvertYToDev(LONG y, const CDevDesc *pdd) const
	{
		return MulDiv(y, _yPerInch, pdd->_xPerInch);
	}

	// Assignment
	CDevDesc& 	operator = (const CDevDesc& dd)
	{
		_hdc = dd._hdc;
		_xPerInch = dd._xPerInch;
		_yPerInch = dd._yPerInch;
		return *this;
	}

	// Compares two device descriptors
	BOOL 	operator == (const CDevDesc& dd) const
	{
		return 	_hdc == dd._hdc;
	}

	BOOL 	operator != (const CDevDesc& dd) const
	{
		return !(*this == dd);
	}

	LONG	GetXPerInch() const
	{
#ifndef MACPORT
		AssertSz(_xPerInch != 0, "CDevDesc::GetXPerInch _xPerInch is 0");
#endif
		return _xPerInch;
	}

	LONG	GetYPerInch() const
	{
#ifndef MACPORT
		AssertSz(_yPerInch != 0, "CDevDesc::GetYPerInch _yPerInch is 0");
#endif
		return _yPerInch;
	}
};


#endif


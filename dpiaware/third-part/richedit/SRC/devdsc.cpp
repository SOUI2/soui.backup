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
 *	DEVDSC.C
 *	
 *	Purpose:
 *		CDevDesc (Device Descriptor) class
 *	
 *	Owner:
 *		Original RichEdit code: David R. Fulmer
 *		Christian Fortini
 *		Murray Sargent
 */

#include "_common.h"
#include "_devdsc.h"
#include "_edit.h"
#include "_font.h"

ASSERTDATA

CDevDesc::~CDevDesc()
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDevDesc::~CDevDesc");

}


BOOL CDevDesc::SetDC(HDC hdc)
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDevDesc::SetDC");

	AssertSz((NULL == hdc) || (GetDeviceCaps(hdc, TECHNOLOGY) != DT_METAFILE),
		"CDevDesc::SetDC attempting to set a metafile");

	_hdc = hdc;

	if(!_hdc)
	{
		_hdcMeasure = NULL;

	    if(!_ped->_fInPlaceActive || !(hdc = _ped->TxGetDC()))
        {
            _xPerInch = _yPerInch = 0;
    	    return FALSE;
        }
    }

	// get device metrics - these should both succeed
	_xPerInch = GetDeviceCaps(hdc, LOGPIXELSX);

	AssertSz(_xPerInch != 0, 
		"CDevDesc::SetDC _xPerInch is 0");

	_yPerInch = GetDeviceCaps(hdc, LOGPIXELSY);

	AssertSz(_yPerInch != 0, 
		"CDevDesc::SetDC _yPerInch is 0");

	if( !_xPerInch || !_yPerInch )
	{
		return FALSE;
	}


	// release DC if we got the window DC
	if(!_hdc)
	{
		_ped->TxReleaseDC(hdc);
	}

	return TRUE;
}


void CDevDesc::SetMetafileDC(
	HDC hdcMetafile,
	HDC hdcMeasure,
	LONG xMeasurePerInch,
	LONG yMeasurePerInch)
{
	_hdc = hdcMetafile;
	_hdcMeasure = hdcMeasure;
	_xPerInch = (SHORT) xMeasurePerInch;
	_yPerInch = (SHORT) yMeasurePerInch;
}

HDC CDevDesc::GetMeasureDC(LONG *pyMeasurePerInch) const
{
	HDC hdcRet = _hdc;

	if (hdcRet != NULL)
	{
		if (_hdcMeasure != NULL)
		{
			hdcRet = _hdcMeasure;
		}

		*pyMeasurePerInch = _yPerInch;
	}
	else
	{
		// The measure DC should not be set without a real DC
		AssertSz(_hdcMeasure == NULL,
			"CDevDesc::GetMeasureDC Measure DC without real DC");
		hdcRet = _ped->fInOurHost()
			? _ped->TxGetDC() 
			: CreateIC (TEXT("DISPLAY"), NULL, NULL, NULL);
		// Make the y per inch what we got at DLL load
		*pyMeasurePerInch = sysparam.GetYPerInchScreenDC();
	}

	return hdcRet;
}

void CDevDesc::ReleaseMeasureDC(
	HDC hdc) const
{
	if (NULL == _hdc)
	{
		if (_ped->fInOurHost())
		{
			_ped->TxReleaseDC(hdc);
		}
		else
		{
			DeleteDC(hdc);
		}
	}
#ifdef DEBUG
	else
	{
		// If the DC is not NULL then this DC must be either a measure
		// DC or the drawing DC.
		if (_hdcMeasure != NULL)
		{
			AssertSz(hdc == _hdcMeasure, 
				"CDevDesc::ReleaseMeasureDC Measure DC mismatch");
		}
		else
		{
			AssertSz(hdc == _hdc, 
				"CDevDesc::ReleaseMeasureDC Draw DC mismatch");
		}
	}
#endif // DEBUG
}

BOOL CDevDesc::IsMetafile() const
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDevDesc::IsMetafile");

	if(!_hdc)
        return FALSE;

	AssertSz((GetDeviceCaps(_hdc, TECHNOLOGY) != DT_METAFILE)
		|| (_hdcMeasure != NULL), 
			"CDevDesc::IsMetafile _hdcMeasure not specified for metafile");

	return _hdcMeasure != NULL;
}

HDC CDevDesc::GetScreenDC() const
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDevDesc::GetScreenDC");

	Assert(!_hdc);
	Assert(_ped);
	return _ped->TxGetDC();
}

VOID CDevDesc::ReleaseScreenDC(HDC hdc) const
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDevDesc::ReleaseScreenDC");

	Assert(!_hdc);
	Assert(_ped);
	_ped->TxReleaseDC(hdc);
}

LONG CDevDesc::DXtoLX(LONG x) const	
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDevDesc::DXtoLX");

    AssertSz(_xPerInch, "CDevDesc::DXtoLX() - hdc has not been set");
    return ((x * LX_PER_INCH) * 2 + _xPerInch) / (2 * _xPerInch);
}

LONG CDevDesc::DYtoLY(LONG y) const	
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDevDesc::DYtoLY");

    AssertSz(_yPerInch, "CDevDesc::DYtoLY() - hdc has not been set");
    return ((y * LY_PER_INCH) * 2 + _yPerInch) / (2 * _yPerInch);
}

void CDevDesc::DPtoLP(POINT &ptDest, const POINT &ptSrc) const
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDevDesc::DPtoLP");

    ptDest.x = DXtoLX(ptSrc.x);    
    ptDest.y = DYtoLY(ptSrc.y);    
}

void CDevDesc::DRtoLR(RECT &rcDest, const RECT &rcSrc) const
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDevDesc::DRtoLR");

    rcDest.left = DXtoLX(rcSrc.left);    
    rcDest.right = DXtoLX(rcSrc.right);    
    rcDest.top = DYtoLY(rcSrc.top);    
    rcDest.bottom = DYtoLY(rcSrc.bottom);    
}

#ifdef DEBUG
LONG CDevDesc::LXtoDX(LONG x) const	
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDevDesc::LXtoDX");

    AssertSz(_xPerInch, "CDevDesc::LXtoDX() - hdc has not been set");
    return ((x * _xPerInch) + LX_PER_INCH / 2) / LX_PER_INCH;
}

LONG CDevDesc::LYtoDY(LONG y) const	
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDevDesc::LYtoDY");

    AssertSz(_yPerInch, "CDevDesc::LYtoDY() - hdc has not been set");
    return ((y * _yPerInch) + LY_PER_INCH / 2) / LY_PER_INCH;
}
#endif  // DEBUG

void CDevDesc::LPtoDP(POINT &ptDest, const POINT &ptSrc) const
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDevDesc::LPtoDP");

    ptDest.x = LXtoDX(ptSrc.x);    
    ptDest.y = LYtoDY(ptSrc.y);    
}

void CDevDesc::LRtoDR(RECT &rcDest, const RECT &rcSrc) const
{
	TRACEBEGIN(TRCSUBSYSDISP, TRCSCOPEINTERN, "CDevDesc::LRtoDR");

    rcDest.left = LXtoDX(rcSrc.left);    
    rcDest.right = LXtoDX(rcSrc.right);    
    rcDest.top = LYtoDY(rcSrc.top);    
    rcDest.bottom = LYtoDY(rcSrc.bottom);    
}


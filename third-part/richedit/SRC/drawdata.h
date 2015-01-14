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
class CDrawData 
{
public:

						CDrawData(

						~CDrawData(

	HDC					GetDC();

	HDC					GetTargetDevice();

	DWORD				GetDrawAspect();

	LONG				GetLindex();

	const DVTARGETDEVICE *GetTargetDeviceDesc();

	void				Push(CDrawData *pdd);

	CDrawData *			Pop();

private:

	DWORD 				_dwDrawAspect;

	LONG  				_lindex;

	const DVTARGETDEVICE *_ptd;

	HDC 				_hdcDraw;

	HDC 				_hicTargetDev;

	CDrawData *			_pddNext;
};

inline void CDrawData::Push(CDrawData *pdd)
{
	_pddNext = pdd;
}

inline CDrawData *CDrawData::Pop()
{
	return _pNext;
}
	


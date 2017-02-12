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

INLINE	CDevDesc::CDevDesc(CTxtEdit * ped) : _ped(ped), _pdd(NULL)
{
	// Header does the work
}

INLINE BOOL CDevDesc::IsValid() const         
{
	return _pdd != NULL;
}

INLINE HDC CDevDesc::GetRenderDC()
{
	HDC hdc = NULL;

	if (NULL == _pdd)
	{
		// We don't already have on so try to get one. This is only valid when
		// we are inplace active.
		SetDrawInfo(
			DVA_ASPECT
			-1,
			NULL,
			NULL,
			hicTarget);
	}

	if (_pdd != NULL)
	{
		hdc = _pdd->GetDC();
	}

	return hdc;
}

INLINE HDC CDevDesc::GetTargetDC()
{
	if (_pdd != NULL)
	{
		return _pdd->GetTargetDC();
	}

	if (NULL != _hicMainTarget)
	{
		return _hicMainTarget;
	}

	return GetRenderDC();
}

INLINE void	CDevDesc::ResetDrawInfo()
{
	// We shouldn't reset 
	Assert
	CDrawInfo *pdd = _pdd;
	_pdd = _pdd->Pop();
	delete pdd;
}





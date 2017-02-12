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
 *	UUID.C
 *
 *	Purpose:
 *		provide definitions for locally used GUID's
 *
 */
#include "_common.h"

//set these two GUIDs up for export in our file

// BUGBUG: New compiler is generating these. x86 compiler doesn't complain
// about this stuff...
#pragma warning ( disable : 4502 4518 )

#undef IID_RichEditOle
#undef IID_IRichEditOleCallback
__declspec(dllexport)DEFINE_GUID(IID_IRichEditOle,         0x00020D00, 0, 0, 
	0xC0,0,0,0,0,0,0,0x46);
__declspec(dllexport)DEFINE_GUID(IID_IRichEditOleCallback, 0x00020D03, 0, 0, 
	0xC0,0,0,0,0,0,0,0x46);
	
#pragma warning ( default : 4502 4518 )

#undef DEFINE_GUID
#undef DEFINE_OLEGUID

#define DEFINE_GUID(name, l, w1, w2, b1, b2, b3, b4, b5, b6, b7, b8) \
    const IID name \
        = { l, w1, w2, { b1, b2,  b3,  b4,  b5,  b6,  b7,  b8 } }
#define DEFINE_OLEGUID(name, l, w1, w2) \
    DEFINE_GUID(name, l, w1, w2, 0xC0,0,0,0,0,0,0,0x46)


DEFINE_OLEGUID(IID_IDispatch,				0x00020400, 0, 0);
DEFINE_OLEGUID(IID_IUnknown,				0x00000000, 0, 0);
DEFINE_OLEGUID(IID_IEnumFORMATETC,          0x00000103, 0, 0);
DEFINE_OLEGUID(IID_IDataObject,             0x0000010e, 0, 0);
DEFINE_OLEGUID(IID_IDropSource,             0x00000121, 0, 0);
DEFINE_OLEGUID(IID_IDropTarget,             0x00000122, 0, 0);
DEFINE_OLEGUID(IID_IViewObject,	 			0x0000010d, 0, 0);
DEFINE_OLEGUID(IID_IViewObject2,			0x00000127, 0, 0);
DEFINE_OLEGUID(IID_IAdviseSink,				0x0000010f, 0, 0);
DEFINE_OLEGUID(IID_IOleClientSite, 			0x00000118, 0, 0);
DEFINE_OLEGUID(IID_IOleWindow,				0x00000114, 0, 0);
DEFINE_OLEGUID(IID_IOleInPlaceSite,			0x00000119, 0, 0);
DEFINE_OLEGUID(IID_IOleLink,	 			0x0000011d, 0, 0);
DEFINE_OLEGUID(IID_IOleCache,	 			0x0000011e, 0, 0);
DEFINE_OLEGUID(IID_IOleObject, 				0x00000112, 0, 0);
DEFINE_OLEGUID(IID_IPersistStorage,			0x0000010a, 0, 0);
DEFINE_OLEGUID(IID_IOleInPlaceObject,	   	0x00000113, 0, 0);
DEFINE_GUID(IID_IRichEditOle,				0x00020D00, 0, 0, 0xC0,0,0,0,0,0,0,0x46);
DEFINE_GUID(IID_IRichEditOleCallback,		0x00020D03, 0, 0, 0xC0,0,0,0,0,0,0,0x46);

DEFINE_OLEGUID(CLSID_Picture_EnhMetafile,	0x00000319, 0, 0);
DEFINE_OLEGUID(CLSID_StaticMetafile,		0x00000315, 0, 0);
DEFINE_OLEGUID(CLSID_StaticDib,				0x00000316, 0, 0); 			

// REMARK: presumably TOM should have official MS GUIDs
// To make pre-compiled headers work better, we just copy the
// guid definitions here.  Make sure they don't change!

DEFINE_GUID(LIBID_TOM,			0x8CC497C9,	0xA1DF,0x11ce,0x80,0x98,0x00,0xAA,
											0x00,0x47,0xBE,0x5D);
DEFINE_GUID(IID_ITextDocument,	0x8CC497C0,	0xA1DF,0x11CE,0x80,0x98,0x00,0xAA,
											0x00,0x47,0xBE,0x5D);
DEFINE_GUID(IID_ITextSelection,	0x8CC497C1,	0xA1DF,0x11CE,0x80,0x98,0x00,0xAA,
											0x00,0x47,0xBE,0x5D);
DEFINE_GUID(IID_ITextRange,		0x8CC497C2,	0xA1DF,0x11CE,0x80,0x98,0x00,0xAA,
											0x00,0x47,0xBE,0x5D);
DEFINE_GUID(IID_ITextFont,		0x8CC497C3,	0xA1DF,0x11CE,0x80,0x98,0x00,0xAA,
											0x00,0x47,0xBE,0x5D);
DEFINE_GUID(IID_ITextPara,		0x8CC497C4,	0xA1DF,0x11CE,0x80,0x98,0x00,0xAA,
											0x00,0x47,0xBE,0x5D);


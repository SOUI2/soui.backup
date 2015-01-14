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
 *	_undoid.h
 *
 *	Purpose:
 *		Undo ID defintions.  These ID's are used to lookup string names
 *		in our resources for the various undo operations
 *
 *	Author:
 *		AlexGo  4/13/95
 */

#ifndef __UNDOID_H__
#define __UNDOID_H__

//
//	typing operations
//

#define UID_TYPING			1
#define	UID_REPLACESEL		2
#define UID_DELETE			3

//
//	data transfer operations
//

#define	UID_DRAGDROP		4
#define UID_CUT				5
#define UID_PASTE			6
#define UID_LOAD			7

#endif // __UNDOID_H__


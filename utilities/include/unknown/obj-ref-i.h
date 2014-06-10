#pragma once

#ifndef OR_API
#define OR_API
#endif

struct OR_API IObjRef
{
	virtual ~IObjRef(){}

	virtual void __stdcall AddRef() = 0;

	virtual void __stdcall Release() = 0;
};
#pragma once

#ifndef OR_API
#define OR_API
#endif

struct OR_API IObjRef
{
	virtual ~IObjRef(){}

	virtual void AddRef() = 0;

	virtual void Release() = 0;
	
	virtual void OnFinalRelease() =0;
};
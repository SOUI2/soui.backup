#pragma once

struct IObjRef
{
	virtual ~IObjRef(){}

	virtual void __stdcall AddRef() = 0;

	virtual void __stdcall Release() = 0;
};
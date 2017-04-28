#pragma once

#include "sdpiawareobject.h"

namespace SOUI
{
	class SOUI_EXP SDpiAwareFont: public SDpiAwareObject
	{
	public:
		SDpiAwareFont(void);
		~SDpiAwareFont(void);


		IFont * GetFontPtr();

	protected:
		virtual IObjRef * OnGetObject(const SStringW & strDesc,int nScale);
	};

}

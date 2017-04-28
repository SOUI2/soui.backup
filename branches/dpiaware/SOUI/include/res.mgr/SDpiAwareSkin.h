#pragma once

#include "sdpiawareobject.h"

namespace SOUI
{
	class SOUI_EXP SDpiAwareSkin: public SDpiAwareObject
	{
	public:
		SDpiAwareSkin(void);
		~SDpiAwareSkin(void);


		ISkinObj * GetSkinPtr();
	protected:

		virtual IObjRef * OnGetObject(const SStringW & strDesc,int nScale);
	};

}

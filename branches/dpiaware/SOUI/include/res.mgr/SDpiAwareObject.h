#pragma once

#include "interface/sdpiaware-i.h"

namespace SOUI
{
	class SOUI_EXP SDpiAwareObject
	{
	public:
		SDpiAwareObject(void);
		~SDpiAwareObject(void);


		IObjRef * GetObjectPtr();

		void SetObjectDesc(const SStringW & strDesc,int nScale);

		void SetScale(int nScale);
	protected:

		virtual IObjRef * OnGetObject(const SStringW & strDesc,int nScale) = 0;

		SStringW m_strDesc;
		CAutoRefPtr<IObjRef> m_object;
	};

}

#pragma once
#include "core/SSingleton.h"
#include <unknown/obj-ref-i.h>
#include <unknown/obj-ref-impl.hpp>
#include "interface/SResProvider-i.h"

#include "res.mgr/SSkinPool.h"
#include "res.mgr/SStylePool.h"
#include "res.mgr/SFontPool.h"
#include "res.mgr/SObjDefAttr.h"
#include "res.mgr/SNamedValue.h"

namespace SOUI
{

	struct IUiDefInfo : IObjRef
	{
		virtual SSkinPool * GetSkinPool() =0;
		virtual SStylePool * GetStylePool() =0;
		virtual SNamedColor & GetNamedColor()  =0;
		virtual SNamedString & GetNamedString()  =0;
	};


	class SOUI_EXP SUiDef : public SSingleton<SUiDef>
	{
	public:
		SUiDef(void);
		~SUiDef(void);

		static IUiDefInfo * CreateUiDefInfo(IResProvider *pResProvider, LPCTSTR pszUiDef);

		
		IUiDefInfo * GetUiDef(){return m_pCurUiDef;}
		
		void SetUiDef(IUiDefInfo* pUiDefInfo){m_pCurUiDef = pUiDefInfo;}
	protected:
		CAutoRefPtr<IUiDefInfo> m_pCurUiDef;
	};


}

#pragma once
#include "DUISingleton.h"

#define DUI_MAX_NAME	30	//控件名字的最大长度

namespace DuiEngine
{
	class DUI_EXP DuiName2ID : public Singleton<DuiName2ID>
	{
		class CNamedID
		{
		public:
			CNamedID() {}
			CNamedID(LPCSTR name,UINT id)
			{
				DUIASSERT(strlen(name)<=DUI_MAX_NAME);
				strcpy_s(strName,DUI_MAX_NAME,name);
				uID=id;
			}

			static int Compare( __in const void * id1, __in const void * id2 )
			{
				CNamedID *pid1=(CNamedID*)id1;
				CNamedID *pid2=(CNamedID*)id2;
				return strcmp(pid1->strName,pid2->strName);
			}

			char 		strName[DUI_MAX_NAME+1];
			UINT		uID;
		};

	public:
		DuiName2ID(void);
		~DuiName2ID(void);

		UINT Name2ID(LPCSTR pszName);
		int Init(pugi::xml_node xmlNode);
	protected:
		CNamedID *m_pBuf;
		int		  m_nCount;
	};
}


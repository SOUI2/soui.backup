#pragma once
#include "helper/SAdapterBase.h"


#define DEFAULT_LINE 10
class CBaseMcAdapterFix : public SMcAdapterBase
{
	SArray<SStringT> m_colNames;
public:
	CBaseMcAdapterFix()
	{		
	}

	virtual int getCount()
	{
		return DEFAULT_LINE;
	}

	virtual void getView(int position, SWindow * pItem, pugi::xml_node xmlTemplate);

	SStringW GetColumnName(int iCol) const;

	void IniColNames(pugi::xml_node xmlTemplate);

	virtual void InitByTemplate(pugi::xml_node xmlTemplate);
};


class CBaseAdapterFix : public SAdapterBase
{	
	const wchar_t*  KAttrName_Height[3];
	SArray<SStringT> m_TemplateNames;
	int m_nItemHeight[3];
public:

	CBaseAdapterFix()
	{
		KAttrName_Height[0] = L"oddHeight";
		KAttrName_Height[1] = L"evenHeight";
		KAttrName_Height[2] = L"evenSelHeight";
	}

	~CBaseAdapterFix()
	{	
	}

	virtual int getCount()
	{
		return DEFAULT_LINE;
	}
	
	int IniTemplateNames(pugi::xml_node xmlTemplate);

	virtual void InitByTemplate(pugi::xml_node xmlTemplate);

	virtual int getViewTypeCount();

	virtual int getItemViewType(int position, DWORD dwState);

	virtual SIZE getViewDesiredSize(int position, SWindow *pItem, LPCRECT prcContainer);

	virtual void getView(int position, SWindow * pItem, pugi::xml_node xmlTemplate);
};



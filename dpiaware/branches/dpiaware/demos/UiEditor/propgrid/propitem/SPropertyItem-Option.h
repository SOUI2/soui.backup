#pragma once

#include "../SPropertyItemBase.h"
#include <souicoll.h>

namespace SOUI
{
    class SPropertyItemOption : public SPropertyItemBase
    {
        SOUI_CLASS_NAME(SPropertyGroup,L"propoption")
    public:
        virtual void DrawItem(IRenderTarget *pRT,CRect rc);
        
		//add
        virtual void SetStringOnly(const SStringT & strValue);

        virtual void SetString(const SStringT & strValue);
        virtual SStringT GetString() const;

        SOUI_ATTRS_BEGIN()
            ATTR_CUSTOM(L"options",OnAttrOptions)
            ATTR_INT(L"dropHeight",m_nDropHeight,FALSE)
            ATTR_INT(L"value",m_nValue,FALSE)
			ATTR_INT(L"CanEmpty",m_bCanEmpty, 1,FALSE)
        SOUI_ATTRS_END()

    protected:
        virtual void OnInplaceActive(bool bActive);
    protected:
        HRESULT OnAttrOptions(const SStringW & strValue,BOOL bLoading);


        int      m_nDropHeight;
        int      m_nValue;
		SStringT      m_strValue;
		bool     m_bCanEmpty:  1;

		SStringT m_strDisplay;
        //SArray<SStringT>    m_options;     
		SMap<SStringT, SStringT> m_options;

        SComboBox  * m_pCombobox;
        
    public:
        static IPropertyItem * CreatePropItem(SPropertyGrid *pOwner)
        {
            return new SPropertyItemOption(pOwner);
        }

		bool OnCBSelChange(EventArgs *pEvt);
    protected:
        SPropertyItemOption(SPropertyGrid *pOwner):SPropertyItemBase(pOwner),m_pCombobox(NULL),m_nValue(-1),m_nDropHeight(200)
        {
        }
    };
}

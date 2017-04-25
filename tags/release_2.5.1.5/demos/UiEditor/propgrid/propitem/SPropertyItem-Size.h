#pragma once

#include "SPropertyItem-Text.h"

namespace SOUI
{
    class SPropertyItemSize : public SPropertyItemText
    {
        SOUI_CLASS_NAME(SPropertyGroup,L"propsize")
    public:

        virtual void SetString(const SStringT & strValue);
		//add
		virtual void SetStringOnly( const SStringT & strValue );
        
        virtual SStringT GetString() const {
            SStringT strValue;
            strValue.Format(_T("%d,%d"),m_szValue.cx,m_szValue.cy);
            return strValue;
        }
        virtual void OnValueChanged();
        virtual void OnChildValueChanged(IPropertyItem *pChild);
        
        SOUI_ATTRS_BEGIN()
            ATTR_CUSTOM(L"value",OnAttrValue)
            ATTR_CUSTOM(L"childrenNames",OnAttrChildrenNames)
        SOUI_ATTRS_END()

    protected:
        HRESULT OnAttrValue(const SStringW & strValue,BOOL bLoading);
        HRESULT OnAttrChildrenNames(const SStringW & strValue,BOOL bLoading);
        
        SIZE    m_szValue;
        
        BOOL    m_bChildChanged;
    public:
        static IPropertyItem * CreatePropItem(SPropertyGrid *pOwner)
        {
            return new SPropertyItemSize(pOwner);
        }
    protected:
        SPropertyItemSize(SPropertyGrid *pOwner);
    };
}

#include "StdAfx.h"
#include "SPropertyItem-Option.h"
#include "../SPropertyEmbedWndHelper.hpp"
#include "../SPropertyGrid.h"
#include <helper/SplitString.h>

namespace SOUI
{
    class SPropCombobox: public SComboBox
                   , public IPropInplaceWnd
    {
    public:
        SPropCombobox(IPropertyItem *pOwner):m_pOwner(pOwner)
        {
            SASSERT(m_pOwner);
        }
        
        void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
        {
            SComboBox::OnKeyDown(nChar,nRepCnt,nFlags); 
        }
        
        SOUI_MSG_MAP_BEGIN()
            MSG_WM_KEYDOWN(OnKeyDown)
        SOUI_MSG_MAP_END()
        
        virtual IPropertyItem* GetOwner(){return m_pOwner;}
        
        virtual void UpdateData()
        {
//             SStringT strValue=GetWindowText();
//             m_pOwner->SetValue(&strValue);
        }

    protected:
        CAutoRefPtr<IPropertyItem> m_pOwner;

    };
    
    void SPropertyItemOption::DrawItem( IRenderTarget *pRT,CRect rc )
    {
        SStringT strValue=GetValue();
        pRT->DrawText(strValue,strValue.GetLength(),rc,DT_SINGLELINE|DT_VCENTER);
    }
    
    void SPropertyItemOption::OnInplaceActive(bool bActive)
    {
        if(bActive)
        {
            SASSERT(!m_pCombobox);
            m_pCombobox = new TplPropEmbedWnd<SPropCombobox>(this);
            m_pCombobox->SetAttribute(L"colorBkgnd",L"#ffffff",TRUE);
            m_pCombobox->SetAttribute(L"dropDown",L"1",TRUE);
            SStringW strHei;
            strHei.Format(L"%d",m_nDropHeight);
            m_pCombobox->SetAttribute(L"dropHeight",strHei,TRUE);
            m_pOwner->OnInplaceActiveWndCreate(this,m_pCombobox);
            for(UINT i=0;i<m_options.GetCount();i++)
            {
                m_pCombobox->InsertItem(i,m_options[i],0,i);
            }
        }else
        {
            if(m_pCombobox)
            {
                m_pOwner->OnInplaceActiveWndDestroy(this,m_pCombobox);
                m_pCombobox->Release();
                m_pCombobox = NULL;
            }
        }
    }

    void SPropertyItemOption::SetValue( void *pValue,UINT uType/*=0*/ )
    {
        m_nValue = *(int*)pValue;
    }

    SStringT SPropertyItemOption::GetValue() const
    {
        if(m_nValue<0 || m_nValue>=(int)m_options.GetCount()) return _T("");
        return m_options[m_nValue];
    }

    HRESULT SPropertyItemOption::OnAttrOptions( const SStringW & strValue,BOOL bLoading )
    {
        SStringT strValueT = S_CW2T(strValue);
        SplitString(strValueT,_T('|'),m_options);
        return S_FALSE;
    }

}

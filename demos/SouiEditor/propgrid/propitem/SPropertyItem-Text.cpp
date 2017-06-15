#include "StdAfx.h"
#include "SPropertyItem-Text.h"
#include "../SPropertyEmbedWndHelper.hpp"
#include "../SPropertyGrid.h"

namespace SOUI
{
    class SPropEdit: public SEdit
                   , public IPropInplaceWnd
    {
    public:
        SPropEdit(IPropertyItem *pOwner):m_pOwner(pOwner)
        {
            SASSERT(m_pOwner);
        }
        
        void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
        {
            if(nChar==VK_RETURN)
            {
                GetParent()->SetFocus();
            }else
            {
                SEdit::OnKeyDown(nChar,nRepCnt,nFlags);            
            }
        }
        
        SOUI_MSG_MAP_BEGIN()
            MSG_WM_KEYDOWN(OnKeyDown)
        SOUI_MSG_MAP_END()
        
        virtual IPropertyItem* GetOwner(){return m_pOwner;}
        
        virtual void UpdateData()
        {
            SStringT strValue=GetWindowText();
            m_pOwner->SetString(strValue);
        }

    protected:
        CAutoRefPtr<IPropertyItem> m_pOwner;

    };
    
    void SPropertyItemText::DrawItem( IRenderTarget *pRT,CRect rc )
    {
        SStringT strValue = GetString();
		rc.left += 5;
        pRT->DrawText(strValue,strValue.GetLength(),rc,DT_SINGLELINE|DT_VCENTER);
    }
    
    void SPropertyItemText::OnInplaceActive(bool bActive)
    {
        __super::OnInplaceActive(bActive);
        if(bActive)
        {
            SASSERT(!m_pEdit);
            m_pEdit = new TplPropEmbedWnd<SPropEdit>(this);
            pugi::xml_document xmlDoc;
            pugi::xml_node xmlNode=xmlDoc.append_child(L"root");
            xmlNode.append_attribute(L"colorBkgnd").set_value(L"#000000");
            m_pOwner->OnInplaceActiveWndCreate(this,m_pEdit,xmlNode);
            m_pEdit->SetWindowText(GetString());
			m_pEdit->SetFocus();
        }else
        {
            if(m_pEdit)
            {
                m_pOwner->OnInplaceActiveWndDestroy(this,m_pEdit);
                m_pEdit->Release();
                m_pEdit = NULL;
            }
        }
    }


    void SPropertyItemText::SetString( const SStringT & strValue )
    {
		//如果值没有改变，就不发送通知
		if (m_strValue.CompareNoCase(strValue) != 0)
		{
			m_strValue = strValue;
			OnValueChanged();
		}
		
    }

	void SPropertyItemText::SetStringOnly( const SStringT & strValue )
	{
		m_strValue = strValue;
	}


    void SPropertyItemText::OnButtonClick()
	{
		GetOwner()->OnItemButtonClick(this, m_strButtonType);
	}

    BOOL SPropertyItemText::HasButton() const 
	{
		if (m_strButtonType.IsEmpty())
		{
			return FALSE;
		}
		
		return TRUE;
	}
}

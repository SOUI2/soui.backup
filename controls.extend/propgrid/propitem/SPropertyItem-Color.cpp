#include "StdAfx.h"
#include "SPropertyItem-Color.h"
#include "../SPropertyEmbedWndHelper.hpp"
#include "../SPropertyGrid.h"
#include <commdlg.h>

const int KColorWidth   = 20;

namespace SOUI
{
    class SPropColorEdit: public SEdit
                        , public IPropInplaceWnd
    {
    public:
        SPropColorEdit(SPropertyItemColor *pOwner):m_pOwner(pOwner)
        {
            SASSERT(m_pOwner);
            m_rcInsetPixel.left = KColorWidth;
            m_cr = m_pOwner->m_crValue;
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
        
        void OnPaint(IRenderTarget *pRT)
        {
            SEdit::OnPaint(pRT);
            CRect rcColor;
            GetClientRect(&rcColor);
            rcColor.right = rcColor.left +KColorWidth;
            pRT->FillSolidRect(&rcColor,m_cr);
        }
        
        int OnCreate(void *)
        {
            SEdit::OnCreate(NULL);
            LRESULT lr=SSendMessage(EM_SETEVENTMASK,0,ENM_CHANGE);
            GetEventSet()->subscribeEvent(EventRENotify::EventID,Subscriber(&SPropColorEdit::OnReNotify,this));
            return 0;
        }
        
        bool OnReNotify(EventArgs *pEvt)
        {
            EventRENotify *pReEvt = (EventRENotify*)pEvt;
            if(pReEvt->iNotify == EN_CHANGE)
            {
                SStringT strValue=GetWindowText();
                int r,g,b,a;
                int nGet=_stscanf(strValue,m_pOwner->m_strFormat,&r,&g,&b,&a);
                if(nGet==4)
                {
                    m_cr = RGBA(r,g,b,a);
                    CRect rcColor;
                    GetClientRect(&rcColor);
                    rcColor.right= rcColor.left +KColorWidth;
                    InvalidateRect(&rcColor);
                }
            }
            return true;
        }
        
        SOUI_MSG_MAP_BEGIN()
            MSG_WM_KEYDOWN(OnKeyDown)
            MSG_WM_PAINT_EX(OnPaint)
            MSG_WM_CREATE(OnCreate)
        SOUI_MSG_MAP_END()
        
        virtual IPropertyItem* GetOwner(){return m_pOwner;}
        
        virtual void UpdateData()
        {
            m_pOwner->SetValue(&m_cr);
        }

    protected:
        CAutoRefPtr<SPropertyItemColor> m_pOwner;
        COLORREF    m_cr;
    };
    
    void SPropertyItemColor::DrawItem( IRenderTarget *pRT,CRect rc )
    {
        CRect rcColor = rc;
        rcColor.right = rcColor.left + KColorWidth;
        pRT->FillSolidRect(&rcColor,m_crValue);
        CRect rcValue = rc;
        rcValue.left += KColorWidth;
        SStringT strValue = GetString();
        pRT->DrawText(strValue,strValue.GetLength(),&rcValue,DT_SINGLELINE|DT_VCENTER);
    }
    
    void SPropertyItemColor::OnInplaceActive(bool bActive)
    {
        if(bActive)
        {
            SASSERT(!m_pEdit);
            m_pEdit = new TplPropEmbedWnd<SPropColorEdit>(this);
            pugi::xml_document xmlDoc;
            pugi::xml_node xmlNode=xmlDoc.append_child(L"root");
            xmlNode.append_attribute(L"colorBkgnd").set_value(L"#ffffff");
            m_pOwner->OnInplaceActiveWndCreate(this,m_pEdit,xmlNode);
            m_pEdit->SetWindowText(GetString());
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

    void SPropertyItemColor::SetValue( void *pValue)
    {
        m_crValue = *(COLORREF*)pValue;
        OnValueChanged();
    }

    const void* SPropertyItemColor::GetValue()
    {
        return &m_crValue;
    }

    void SPropertyItemColor::SetString( const SStringT & strValue )
    {
        int r,g,b,a;
        if(_stscanf(strValue,m_strFormat,&r,&g,&b,&a)==4)
        {
            m_crValue = RGBA(r,g,b,a);
            OnValueChanged();
        }
    }

    void SPropertyItemColor::OnButtonClick()
    {
        CHOOSECOLOR cc;                 // common dialog box structure 
        static COLORREF acrCustClr[16]; // array of custom colors 

        // Initialize CHOOSECOLOR 
        ZeroMemory(&cc, sizeof(cc));
        cc.lStructSize = sizeof(cc);
        cc.hwndOwner = GetOwner()->GetContainer()->GetHostHwnd();
        cc.lpCustColors = (LPDWORD) acrCustClr;
        cc.rgbResult = m_crValue;
        cc.Flags = CC_FULLOPEN | CC_RGBINIT;
        
        if (ChooseColor(&cc))
        {
            m_crValue = cc.rgbResult|0xff000000;
            OnValueChanged();
            CRect rc=GetOwner()->GetItemRect(this);
            GetOwner()->InvalidateRect(&rc);
        }
    }

}

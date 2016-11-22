#pragma once

struct ISetSkinHandler
{
    virtual void OnSetSkin(int iSkin) = 0;
};

class CSetSkinWnd : public SHostWnd
{
public:
    CSetSkinWnd(ISetSkinHandler * pSetSkinHandler);
    ~CSetSkinWnd(void);
    
protected:
    virtual void OnFinalMessage(HWND hWnd){
        __super::OnFinalMessage(hWnd);
        delete this;
    }
    
protected:
    void OnActivate(UINT nState, BOOL bMinimized, HWND wndOther);
    BEGIN_MSG_MAP_EX(CSetSkinWnd)
        MSG_WM_ACTIVATE(OnActivate)
        CHAIN_MSG_MAP(SHostWnd)
    END_MSG_MAP();
    
    void OnSetSkin(EventArgs *e);
    EVENT_MAP_BEGIN()
        EVENT_ID_RANGE_HANDLER(R.id.img_skin_0,R.id.img_skin_7,EVT_CMD,OnSetSkin)
    EVENT_MAP_END()
    
    
    ISetSkinHandler * m_pSetSkinHandler;
};


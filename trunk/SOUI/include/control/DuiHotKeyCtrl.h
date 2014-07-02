#pragma once
#include "duiwnd.h"
#include "Accelerator.h"

namespace SOUI
{

class SOUI_EXP SHotKeyCtrl 
    : public SWindow
    , public CAccelerator
{
    SOUI_CLASS_NAME(SHotKeyCtrl, L"hotkey")
public:
    SHotKeyCtrl(void);
    virtual ~SHotKeyCtrl(void);

    void SetRule(WORD wInvalidComp,WORD wModifier);

    void SetHotKey(WORD vKey,WORD wModifiers);

    void GetHotKey(WORD & vKey,WORD &wModifers);


protected:
    virtual UINT OnGetDlgCode()
    {
        return (DUIC_WANTALLKEYS|DUIC_WANTSYSKEY) & (~DUIC_WANTTAB);
    }

    int OnCreate(LPVOID);
    
    void OnLButtonDown(UINT nFlags,CPoint pt);

    void OnPaint(IRenderTarget *pRT);

    void OnSetFocus();

    void OnKillFocus();

    void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

    void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);

    void OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

    void OnSysKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);

    void UpdateModifier();

    void UpdateCaret();

    LRESULT OnWindowPosChanged(LPRECT lpRcContainer);

    SOUI_ATTRS_BEGIN()
        ATTR_WORD(L"invalidcomb",m_wInvalidComb,FALSE)
        ATTR_WORD(L"defcombkey",m_wInvalidModifier,FALSE)
        ATTR_WORD(L"combkey",m_wModifier,FALSE)
        ATTR_WORD(L"hotkey",m_wVK,FALSE)
    SOUI_ATTRS_END()

    SOUI_MSG_MAP_BEGIN()
        MSG_WM_CREATE(OnCreate)
        MSG_WM_PAINT_EX(OnPaint)
        MSG_WM_LBUTTONDOWN(OnLButtonDown)
        MSG_WM_SETFOCUS_EX(OnSetFocus)
        MSG_WM_KILLFOCUS_EX(OnKillFocus)
        MSG_WM_KEYDOWN(OnKeyDown)
        MSG_WM_KEYUP(OnKeyUp)
        MSG_WM_SYSKEYDOWN(OnSysKeyDown)
        MSG_WM_SYSKEYUP(OnSysKeyUp)
        MSG_WM_WINPOSCHANGED_EX(OnWindowPosChanged)
    SOUI_MSG_MAP_END()
    WORD     m_wInvalidComb;        //无效的组合键
    WORD     m_wInvalidModifier; //对无效组合键的替换方案,默认方案

    BOOL    m_bInSetting;        //正在设置中
    CAutoRefPtr<IFont> m_curFont;//当前字体，用于计算文字大小
};

}//namespace SOUI

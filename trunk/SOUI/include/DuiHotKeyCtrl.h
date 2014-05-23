#pragma once
#include "duiwnd.h"
#include "Accelerator.h"

namespace SOUI
{

class SOUI_EXP CDuiHotKeyCtrl 
    : public CDuiWindow
    , public CAccelerator
{
    SOUI_CLASS_NAME(CDuiHotKeyCtrl, "hotkey")
public:
    CDuiHotKeyCtrl(void);
    virtual ~CDuiHotKeyCtrl(void);

    void SetRule(WORD wInvalidComp,WORD wModifier);

    void SetHotKey(WORD vKey,WORD wModifiers);

    void GetHotKey(WORD & vKey,WORD &wModifers);

    SOUO_ATTRIBUTES_BEGIN()
    DUIWIN_WORD_ATTRIBUTE("invalidcomb",m_wInvalidComb,FALSE)
    DUIWIN_WORD_ATTRIBUTE("defcombkey",m_wInvalidModifier,FALSE)
    DUIWIN_WORD_ATTRIBUTE("combkey",m_wModifier,FALSE)
    DUIWIN_WORD_ATTRIBUTE("hotkey",m_wVK,FALSE)
    SOUI_ATTRIBUTES_END()

protected:
    virtual UINT OnGetDuiCode()
    {
        return (DUIC_WANTALLKEYS|DUIC_WANTSYSKEY) & (~DUIC_WANTTAB);
    }

    int OnCreate(LPVOID);

    void OnLButtonDown(UINT nFlags,CPoint pt);

    void OnPaint(CDCHandle dc);

    void OnSetDuiFocus();

    void OnKillDuiFocus();

    void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

    void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);

    void OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

    void OnSysKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);

    void UpdateModifier();

    void UpdateCaret();

    LRESULT OnWindowPosChanged(LPRECT lpRcContainer);

    WND_MSG_MAP_BEGIN()
    MSG_WM_CREATE(OnCreate)
    MSG_WM_PAINT(OnPaint)
    MSG_WM_LBUTTONDOWN(OnLButtonDown)
    MSG_WM_SETFOCUS_EX(OnSetDuiFocus)
    MSG_WM_KILLFOCUS_EX(OnKillDuiFocus)
    MSG_WM_KEYDOWN(OnKeyDown)
    MSG_WM_KEYUP(OnKeyUp)
    MSG_WM_SYSKEYDOWN(OnSysKeyDown)
    MSG_WM_SYSKEYUP(OnSysKeyUp)
    MSG_WM_DUIWINPOSCHANGED(OnWindowPosChanged)
    WND_MSG_MAP_END()
    WORD     m_wInvalidComb;        //无效的组合键
    WORD     m_wInvalidModifier; //对无效组合键的替换方案,默认方案

    HFONT    m_hTxtFont;
    BOOL    m_bInSetting;        //正在设置中
};

}//namespace SOUI

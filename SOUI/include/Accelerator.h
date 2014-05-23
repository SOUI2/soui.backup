#pragma once

namespace SOUI
{

    class SOUI_EXP CAccelerator
    {
    public:
        CAccelerator(DWORD dwAccel);
        CAccelerator(UINT vKey=0,bool bCtrl=false,bool bAlt=false,bool bShift=false);
        ~CAccelerator(void);

        CDuiStringT GetKeyName(WORD vk);

        CDuiStringT FormatHotkey();

        WORD GetModifier() const {return m_wModifier;}
        WORD GetKey() const {return m_wVK;}

        static DWORD TranslateAccelKey(LPCTSTR pszKeyName);
    protected:
        WORD     m_wModifier;
        WORD    m_wVK;
    };

    // 想要注册键盘加速键的类需要实现本接口.
    struct IAcceleratorTarget
    {
        // 如果加速键被处理, 方法应该返回true.
        virtual bool OnAcceleratorPressed(const CAccelerator& accelerator) = 0;
    };

    struct IAcceleratorMgr
    {
        // Register a keyboard accelerator for the specified target. If multiple
        // targets are registered for an accelerator, a target registered later has
        // higher priority.
        // Note that we are currently limited to accelerators that are either:
        // - a key combination including Ctrl or Alt
        // - the escape key
        // - the enter key
        // - any F key (F1, F2, F3 ...)
        // - any browser specific keys (as available on special keyboards)
        virtual void RegisterAccelerator(const CAccelerator& accelerator,
            IAcceleratorTarget* target)=NULL;

        // Unregister the specified keyboard accelerator for the specified target.
        virtual void UnregisterAccelerator(const CAccelerator& accelerator,
            IAcceleratorTarget* target)=NULL;

        // Unregister all keyboard accelerator for the specified target.
        virtual void UnregisterAccelerators(IAcceleratorTarget* target)=NULL;
    };
}//end of namespace SOUI

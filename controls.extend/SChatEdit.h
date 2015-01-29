#pragma once

namespace SOUI
{
    #define EVT_CHATEDIT_KEYRETURN (EVT_EXTERNAL_BEGIN+500)
    
    class EventChatEditKeyReturn : public EventArgs
    {
    public:
    EventChatEditKeyReturn(SWindow *pSender):EventArgs(pSender),bCancel(false){}
    enum {EventID=EVT_CHATEDIT_KEYRETURN};
    virtual UINT GetEventID() { return EventID;}
    bool bCancel;
    };
    
    class SChatEdit : public SRichEdit
    {
    SOUI_CLASS_NAME(SChatEdit,L"chatedit")
    public:
        SChatEdit(void);
        ~SChatEdit(void);
        
        //************************************
        // Method:    AppendFormatText
        // FullName:  SOUI::SChatEdit::AppendFormatText
        // Access:    public 
        // Returns:   BOOL -- success:TRUE
        // Qualifier: 
        // Parameter: const SStringW & strMsg
        // remark: strMsg :一个XML格式的正文，支持标签：<color value="#ff0000">abc<link color="#0000ff">home</link><underline>efg<italic>abc<bold>abc<strike>afc<font value="宋体">abc<smiley id="1" path="c:\a.gif"/></font></strike></bold></italic></underline></color>
        //************************************
        BOOL AppendFormatText(const SStringW & strMsg);

        BOOL AppendFormatText(const pugi::xml_node xmlMsg);

        SStringW GetFormatText();
        
    protected:
        int     OnCreate(LPVOID);
        void    OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
        
        SOUI_MSG_MAP_BEGIN()
            MSG_WM_CREATE(OnCreate)
            MSG_WM_KEYDOWN(OnKeyDown)
        SOUI_MSG_MAP_END()
        
    protected:
        int _AppendFormatText(int iCaret,CHARFORMAT cf,pugi::xml_node xmlText);
    };

}

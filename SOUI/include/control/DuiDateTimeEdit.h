#pragma once

#include "DuiRichEdit.h"
#include "DuiTime.h"

namespace SOUI{

//===========================================================================
// Summary:
//     CDuiMaskEdit is a CXTPEdit derived class. It allows text masking to be
//     applied to the control to format it for special editing restrictions.
//===========================================================================
class SOUI_EXP SMaskEdit : public SEdit
{
    SOUI_CLASS_NAME(SMaskEdit, L"maskedit")

public:
    SMaskEdit();

    BOOL            CanUseMask() const;
    void            SetUseMask(BOOL bUseMask);

    BOOL            CanOverType() const;
    void            SetOverType(BOOL bOverType);

    BOOL            PosInRange(int nPos) const;

    TCHAR           GetPromptChar() const;
    SStringT         GetPromptString(int nLength) const;
    void            SetPromptChar(TCHAR ch, BOOL bAutoReplace = TRUE);

    BOOL            MaskCut();
    BOOL            MaskCopy();
    void            MaskReplaceSel(LPCTSTR lpszNewText);
    BOOL            MaskPaste();
    void            MaskDeleteSel();
    BOOL            MaskClear();
    void            MaskSelectAll();
    BOOL            IsModified() const;
    void            SetMaskedText(LPCTSTR lpszMaskedText, int nPos = 0, BOOL bUpdateWindow = TRUE);
    virtual BOOL    SetEditMask(LPCTSTR lpszMask, LPCTSTR lpszLiteral, LPCTSTR lpszDefault=NULL);
    TCHAR           ConvertUnicodeAlpha(TCHAR nChar, BOOL bUpperCase) const;
    virtual BOOL    CheckChar(TCHAR& nChar, int nPos);
    virtual BOOL    ProcessMask(TCHAR& nChar, int nEndPos);

    void            DeleteCharAt(int nPos);
    void            InsertCharAt(int nPos, TCHAR nChar);

    SStringT         GetMaskedText(int nStartPos = 0, int nEndPos = -1) const;

    void            GetMaskState(BOOL bCorrectSelection = TRUE);
    void            SetMaskState();

    void            MaskGetSel();

    BOOL            CopyToClipboard(const SStringT& strText);

    BOOL            IsPromptPos(int nPos) const;
    BOOL            IsPromptPos(const SStringT& strLiteral, int nPos) const;

    BOOL            CorrectPosition(int& nPos, BOOL bForward = TRUE);

    void            CorrectWindowText();

    virtual BOOL    IsPrintChar(TCHAR nChar);
    virtual BOOL    IsAlphaChar(TCHAR nChar);
    virtual void    NotifyPosNotInRange();
    virtual void    NotifyInvalidCharacter(TCHAR /*nChar*/, TCHAR /*chMask*/);
    void            ProcessChar(TCHAR nChar);

protected:
    int             OnCreate(LPVOID);
    void            OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
    void            OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
    void            OnSetDuiFocus();

    SStringT        GetWindowText();
protected:
    int             m_nStartChar;       // Current position of the first character in the current selection.
    int             m_nEndChar;         // Current position of the first non-selected character past the end of the current selection.
    BOOL            m_bUseMask;         // TRUE to use the edit mask.
    BOOL            m_bOverType;        // TRUE to over type the text, set with VK_INSERT key press.
    BOOL            m_bModified;        // TRUE if mask edit has been modified.
    TCHAR           m_chPrompt;         // Prompt character used to identify the text entry.
    SStringT         m_strMask;          // Buffer that holds the actual edit mask value.
    SStringT         m_strDefault;       // Contains the edit controls default display text.
    SStringT         m_strWindowText;    // Buffer that holds the actual edit text.
    SStringT         m_strLiteral;       // Literal format that restricts where the user can enter text.

protected:
   SOUI_MSG_MAP_BEGIN()
        MSG_WM_CREATE(OnCreate)
        MSG_WM_CHAR(OnChar)
        MSG_WM_KEYDOWN(OnKeyDown)
        MSG_WM_SETFOCUS_EX(OnSetDuiFocus)
   SOUI_MSG_MAP_END()
};

//===========================================================================
//  CDxDateEdit
class SOUI_EXP SDateEdit : public SMaskEdit
{
    SOUI_CLASS_NAME(SDateEdit, L"dateedit")

public:
    SDateEdit();

    virtual void    SetDateTime(LPCTSTR strDate);
    virtual void    SetDateTime(CTime tm);

    virtual SStringT GetWindowDateTime();
    virtual BOOL    ProcessMask(TCHAR& nChar, int nEndPos);
protected:
    int             OnCreate(LPVOID);

protected:
   SOUI_MSG_MAP_BEGIN()
        MSG_WM_CREATE(OnCreate)
   SOUI_MSG_MAP_END()
};

//===========================================================================
//  CDxTimeEdit
class SOUI_EXP STimeEdit : public SDateEdit
{
    SOUI_CLASS_NAME(STimeEdit, L"timeedit")

public:
    STimeEdit();

public:
    virtual void    SetHours(int nHours);
    virtual void    SetMins(int nMins);
    virtual void    SetTime(int nHours, int nMins);
    int             GetHours() const;
    int             GetMins() const;
    virtual BOOL    ProcessMask(TCHAR& nChar, int nEndPos);
    void            SetMilitary(BOOL bMilitary = TRUE);

protected:
    int             m_nHours;
    int             m_nMins;
    BOOL            m_bMilitary;

protected:
    int             OnCreate(LPVOID);

protected:
   SOUI_MSG_MAP_BEGIN()
        MSG_WM_CREATE(OnCreate)
   SOUI_MSG_MAP_END()
};

//////////////////////////////////////////////////////////////////////
inline int STimeEdit::GetHours() const
{
    return m_nHours;
}

inline int STimeEdit::GetMins() const
{
    return m_nMins;
}

inline void STimeEdit::SetMilitary(BOOL bMilitary)
{
    m_bMilitary = bMilitary;

}


}//end of namespace

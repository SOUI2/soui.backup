#pragma once

#include <core/simplewnd.h>

class CScintillaModule
{
public:
	CScintillaModule();
	~CScintillaModule();

	BOOL operator !() const
	{
		return m_hModule==NULL;
	}
protected:
	HINSTANCE m_hModule;
};

class CScintillaWnd;
typedef void (CSimpleWnd::*SCIWND_FN_CALLBACK)(CScintillaWnd*, int, SStringT);

// CScintillaWnd
class CScintillaWnd : public CSimpleWnd
{
public:
	CScintillaWnd();
	virtual ~CScintillaWnd();
	BOOL Create (LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, HWND hParentWnd, UINT nID,HINSTANCE hInst);
	BOOL OpenFile(LPCTSTR lpFileName);
	void DoSave();

	void SetDirty(bool bDirty);

	BOOL SaveFile(LPCTSTR lpFileName);

	LPCTSTR GetOpenedFileName(){return m_strFileName;}
	void SetOpenedFileName(LPCTSTR pszFileName){m_strFileName=pszFileName;}

	LRESULT SendEditor(UINT Msg, WPARAM wParam=0, LPARAM lParam=0) {
		return SendMessage(Msg, wParam, lParam);
	}
	void SetSaveCallback(SCIWND_FN_CALLBACK fun)
	{
		m_fnCallback = fun;
	}

	bool m_bDirty;		//指示文档是否已修改

protected:
	// 显示行号
	void UpdateLineNumberWidth(void);
	void InitScintillaWnd(void);
	void SetAStyle(int style, COLORREF fore, COLORREF back = RGB(0xff,0xff,0xff), int size = 0, const char* face = NULL);
	void SetFold();
	// 设置XML的语法规则
	void SetXmlLexer(COLORREF bkColor);

	void findMatchingBracePos(int & braceAtCaret, int & braceOpposite);
	bool doMatch(); //匹配括号并加亮缩进向导;
	
	void GetRange(int start, int end, char* text);

	void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);

	LRESULT OnNotify(int idCtrl, LPNMHDR pnmh);

	BEGIN_MSG_MAP_EX(CScintillaWnd)
		MSG_OCM_NOTIFY(OnNotify)
		MSG_WM_KEYDOWN(OnKeyDown)
	END_MSG_MAP()

	SStringT m_strFileName;
	SCIWND_FN_CALLBACK m_fnCallback;
};

// ScintillaWnd.cpp : 实现文件
//

#include "stdafx.h"
#include "ScintillaWnd.h"
#include <shlwapi.h>

#include "Scintilla.h"
#include "SciLexer.h"
#include "DesignerView.h"
#include "xpm_icons.h"
#include "SysdataMgr.h"
#include "MainDlg.h"

#define STR_SCINTILLAWND _T("Scintilla")
#define STR_SCINTILLADLL _T("SciLexer.dll")

extern CMainDlg* g_pMainDlg;
//////////////////////////////////////////////////////////////////////////
CScintillaModule::CScintillaModule()
{
	//m_hModule=LoadLibrary(STR_SCINTILLADLL);
}

CScintillaModule::~CScintillaModule()
{
	//if(m_hModule) FreeLibrary(m_hModule);
}

//////////////////////////////////////////////////////////////////////////

// CScintillaWnd
const COLORREF black = RGB(0, 0, 0);
const COLORREF white = RGB(0xff, 0xff, 0xff);
const COLORREF orange = RGB(0x80, 0x80, 0);
const COLORREF darkGreen = RGB(0, 0x80, 0);
const COLORREF darkBlue = RGB(0x80, 0x80, 0);
const COLORREF red = RGB(1, 0, 0);
const COLORREF yellow = RGB(1, 1, 0);
const COLORREF liteBlue = RGB(0x80, 0x80, 1);

static const int MARGIN_SCRIPT_FOLD_INDEX = 1;

CScintillaWnd::CScintillaWnd()
{
	m_fnCallback = NULL;
}

CScintillaWnd::~CScintillaWnd()
{
}

BOOL CScintillaWnd::Create(LPCTSTR lpszWindowName, DWORD dwStyle, const RECT& rect, HWND hParent, UINT nID, HINSTANCE hInst)
{
	SetDirty(false);
	HWND hWnd = CreateWindowEx(WS_EX_CLIENTEDGE, STR_SCINTILLAWND, lpszWindowName, dwStyle, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, hParent, (HMENU)nID, hInst, NULL);
	if (!hWnd) return FALSE;
	SubclassWindow(hWnd);
	InitScintillaWnd();
	return TRUE;
}

// 显示行号
void CScintillaWnd::UpdateLineNumberWidth(void)
{
	char tchLines[32];
	int  iLineMarginWidthNow;
	int  iLineMarginWidthFit;

	sprintf((tchLines), (" %i "),
		SendMessage(SCI_GETLINECOUNT, 0, 0));

	iLineMarginWidthNow = SendMessage(
		SCI_GETMARGINWIDTHN, 0, 0);
	iLineMarginWidthFit = SendMessage(
		SCI_TEXTWIDTH, STYLE_LINENUMBER, (LPARAM)tchLines);

	if (iLineMarginWidthNow != iLineMarginWidthFit)
	{
		SendMessage(SCI_SETMARGINWIDTHN, 0,
			iLineMarginWidthFit);
	}
}

/***********************************************************************
 *             map_fileW
 *
 * Helper function to map a file to memory:
 *  name			-	file name
 *  [RETURN] ptr		-	pointer to mapped file
 *  [RETURN] filesize           -       pointer size of file to be stored if not NULL
 */
static const LPBYTE map_file(LPCTSTR name, LPDWORD filesize)
{
	HANDLE hFile, hMapping;
	LPBYTE ptr = NULL;

	hFile = CreateFile(name, GENERIC_READ, FILE_SHARE_READ, NULL,
		OPEN_EXISTING, FILE_FLAG_RANDOM_ACCESS, 0);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		hMapping = CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL);
		if (hMapping)
		{
			ptr = (LPBYTE)MapViewOfFile(hMapping, FILE_MAP_READ, 0, 0, 0);
			CloseHandle(hMapping);
			if (filesize)
				*filesize = GetFileSize(hFile, NULL);
		}
		CloseHandle(hFile);
	}
	return ptr;
}

BOOL CScintillaWnd::OpenFile(LPCTSTR lpFileName)
{
	DWORD dwSize = 0;
	const LPBYTE pbuf = map_file(lpFileName, &dwSize);
	if (!pbuf) return FALSE;
	SStringA str;
	if (dwSize > 2 && pbuf[0] == 0xFF && pbuf[1] == 0xFE)
	{//utf16
		SStringW strBuf((LPCWSTR)(pbuf + 2), dwSize / 2 - 1);
		str = S_CW2A(strBuf, CP_UTF8);
	}
	else if (dwSize > 3 && pbuf[0] == 0xEF && pbuf[1] == 0xBB && pbuf[2] == 0xBF)
	{//utf8有签名
		str = SStringA((LPCSTR)(pbuf + 3), dwSize - 3);
	}
	else
	{//utf8无签名
		str = SStringA((LPCSTR)pbuf, dwSize);
	}
	UnmapViewOfFile(pbuf);
	SendEditor(SCI_CLEARALL);
	SendEditor(SCI_MARKERDELETEALL, (WPARAM)-1, 0);

	SendEditor(SCI_ADDTEXT, str.GetLength(),
		reinterpret_cast<LPARAM>((LPCSTR)str));

	SendEditor(SCI_SETUNDOCOLLECTION, 1, 0);
	SendEditor(EM_EMPTYUNDOBUFFER, 0, 0);
	SendEditor(SCI_SETSAVEPOINT, 0, 0);
	SendEditor(SCI_GOTOPOS, 0, 0);
	SendEditor(SCI_CHOOSECARETX, 0, 0);
	::SetFocus(m_hWnd);
	UpdateLineNumberWidth();

	m_strFileName = lpFileName;
	SetDirty(false);

	return TRUE;
}

void CScintillaWnd::DoSave()
{
	if (m_strFileName.IsEmpty())
	{
		return;
	}

	if (SaveFile(m_strFileName))
		SetDirty(false);
}

void CScintillaWnd::SetDirty(bool bDirty)
{
	m_bDirty = bDirty;
	if (!::IsWindow(m_hWnd))
		return;

	if (bDirty)
	{
		SetXmlLexer(RGB(255, 235, 240));
	}
	else
	{
		SetXmlLexer(white);
	}

	if (m_fnCallback && !m_strFileName.IsEmpty())
	{
		(this->*m_fnCallback)(this, 1, bDirty ? _T("dirty") : _T(""));
	}
}

void CScintillaWnd::GetRange(int start, int end, char* text)
{
	TEXTRANGEA tr;
	tr.chrg.cpMin = start;
	tr.chrg.cpMax = end;
	tr.lpstrText = (text);
	SendMessage(EM_GETTEXTRANGE, 0, reinterpret_cast<LPARAM>(&tr));
}

#define BLOCKSIZE	1024
BOOL CScintillaWnd::SaveFile(LPCTSTR lpFileName)
{
	FILE *f = _tfopen(lpFileName, _T("wb"));
	if (!f) return FALSE;
	BYTE bom[3] = { 0xef,0xbb,0xbf };
	fwrite(bom, 3, 1, f);

	char data[BLOCKSIZE + 1];
	int lengthDoc = SendMessage(SCI_GETLENGTH);
	for (int i = 0; i < lengthDoc; i += BLOCKSIZE) {
		int grabSize = lengthDoc - i;
		if (grabSize > BLOCKSIZE)
			grabSize = BLOCKSIZE;
		GetRange(i, i + grabSize, data);
		fwrite(data, 1, grabSize, f);
	}
	fclose(f);
	return TRUE;
}


void CScintillaWnd::InitScintillaWnd(void)
{
	SendMessage(SCI_SETCODEPAGE, SC_CP_UTF8);		//UTF8
	//SendMessage(SCI_USEPOPUP,0,0);				//关闭右键菜单，改由父窗口view类响应

	SendMessage(SCI_SETWRAPMODE, SC_WRAP_WORD);		//自动换行
	
	//设置全局默认style，默认style只在没有明确的选择时会被应用
	SetAStyle(STYLE_DEFAULT, black, white, 11, "宋体");
	SendMessage(SCI_STYLECLEARALL);	// 将全局默认style应用到所有

	//清空所有默认的Ctrl快捷键消息,避免产生乱码
	// byte key = 'A';
	// while (key <= 'Z')
	// {
	// 	int keyDefinition = key + (SCMOD_CTRL << 16);
	// 	SendMessage(SCI_ASSIGNCMDKEY,(WPARAM)keyDefinition,(LPARAM)SCI_NULL);
	// 	keyDefinition = key + ((SCMOD_CTRL+SCMOD_ALT) << 16);
	// 	SendMessage(SCI_CLEARCMDKEY,(WPARAM)keyDefinition,(LPARAM)SCI_NULL);
	// 	keyDefinition = key + ((SCMOD_CTRL+SCMOD_SHIFT) << 16);
	// 	SendMessage(SCI_CLEARCMDKEY,(WPARAM)keyDefinition,(LPARAM)SCI_NULL);
	// 	key += 1;
	// }

	SendEditor(SCI_ASSIGNCMDKEY, (WPARAM)('S' + (SCMOD_CTRL << 16)), (LPARAM)SCI_NULL);

	//自动完成
	SendEditor(SCI_AUTOCSETSEPARATOR, static_cast<WPARAM>(' '), 0);	//设置自动完成列表单词分隔符
	SendEditor(SCI_AUTOCSETMAXHEIGHT, static_cast<WPARAM>(15), 0);
	SendEditor(SCI_AUTOCSETMAXWIDTH, static_cast<WPARAM>(0), 0);
	SendEditor(SCI_AUTOCSETIGNORECASE, static_cast<WPARAM>(1), 0);

	
	//显示当前行的背景
	SendEditor(SCI_SETCARETLINEVISIBLE, TRUE);
	SendEditor(SCI_SETCARETLINEVISIBLEALWAYS, TRUE);
	SendEditor(SCI_SETCARETLINEBACK, 0xa0ffff);
	//SendEditor(SCI_SETCARETLINEBACKALPHA, 100, 0);

	// 括号匹配颜色
	SendEditor(SCI_STYLESETFORE, STYLE_BRACELIGHT, RGB(0,255,0));       //代码框.置风格前景色 (#代码编辑框常量.风格_匹配括号, #红色)
	SendEditor(SCI_STYLESETBOLD, STYLE_BRACELIGHT, true);           //风格.粗体

	SetFold();
	SetXmlLexer(white);
	UpdateLineNumberWidth();
}

void CScintillaWnd::SetAStyle(int style, COLORREF fore, COLORREF back, int size, const char* face)
{
	SendMessage(SCI_STYLESETFORE, style, fore);
	SendMessage(SCI_STYLESETBACK, style, back);
	if (size >= 1)
		SendMessage(SCI_STYLESETSIZE, style, size);
	if (face)
		SendMessage(SCI_STYLESETFONT, style, reinterpret_cast<LPARAM>(face));
}

void CScintillaWnd::SetFold()
{
	// 设置自定义图标
	SendMessage(SCI_MARKERDEFINEPIXMAP, SC_MARKNUM_FOLDER, (sptr_t)boxplus_xpm);
	SendMessage(SCI_MARKERDEFINEPIXMAP, SC_MARKNUM_FOLDEROPEN, (sptr_t)boxminus_xpm);
	SendMessage(SCI_MARKERDEFINEPIXMAP, SC_MARKNUM_FOLDEREND, (sptr_t)boxplus_xpm);
	SendMessage(SCI_MARKERDEFINEPIXMAP, SC_MARKNUM_FOLDEROPENMID, (sptr_t)boxminus_xpm);

	// 设置折叠标签样式
	SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDER, SC_MARK_PIXMAP);	 //折叠时显示 +
	SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDEROPEN, SC_MARK_PIXMAP);	 //打开时显示 -
	SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDEREND, SC_MARK_PIXMAP);
	SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDEROPENMID, SC_MARK_PIXMAP);
	SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDERMIDTAIL, SC_MARK_TCORNERCURVE);
	SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDERSUB, SC_MARK_VLINE);
	SendEditor(SCI_MARKERDEFINE, SC_MARKNUM_FOLDERTAIL, SC_MARK_LCORNERCURVE);

	// 折叠标签颜色 
	SendEditor(SCI_MARKERSETBACK, SC_MARKNUM_FOLDERSUB, 0xa0a0a0);
	SendEditor(SCI_MARKERSETBACK, SC_MARKNUM_FOLDERMIDTAIL, 0xa0a0a0);
	SendEditor(SCI_MARKERSETBACK, SC_MARKNUM_FOLDERTAIL, 0xa0a0a0);

	// 设置代码折叠规则
	SendMessage(SCI_SETPROPERTY, (WPARAM)"fold", (LPARAM)"1");
	SendMessage(SCI_SETPROPERTY, (WPARAM)"fold.compact", (LPARAM)"0");
	SendMessage(SCI_SETPROPERTY, (WPARAM)"fold.html", (LPARAM)"1");
	SendMessage(SCI_SETPROPERTY, (WPARAM)"fold.html.preprocessor", (LPARAM)"1");
	SendMessage(SCI_SETMARGINWIDTHN, MARGIN_SCRIPT_FOLD_INDEX, 0);
	SendMessage(SCI_SETMARGINTYPEN, MARGIN_SCRIPT_FOLD_INDEX, SC_MARGIN_SYMBOL);
	SendMessage(SCI_SETMARGINMASKN, MARGIN_SCRIPT_FOLD_INDEX, SC_MASK_FOLDERS);
	SendMessage(SCI_SETMARGINWIDTHN, MARGIN_SCRIPT_FOLD_INDEX, 20);


	SendMessage(SCI_SETFOLDFLAGS, 16, 0); // 16  折叠完后画一条线 | 4后上下各有一条线

	SendMessage(SCI_SETMARGINSENSITIVEN, MARGIN_SCRIPT_FOLD_INDEX, 1);
	//---------代码折叠设置完毕-----------
}

void CScintillaWnd::SetXmlLexer(COLORREF bkColor)
{
	SendMessage(SCI_SETLEXER, SCLEX_XML);
	SendMessage(SCI_SETSTYLEBITS, 7);

	// 设置全局style. 这些属性会在无其它选择时被应用.
	SetAStyle(STYLE_DEFAULT, black, bkColor, 9, "Verdana");
	SendMessage(SCI_STYLECLEARALL);
	const COLORREF CR_RED = RGB(0xFF, 0, 0);
	const COLORREF CR_OFFWHITE = RGB(0xFF, 0xFB, 0xF0);
	const COLORREF CR_DARKGREEN = RGB(0, 0x80, 0);
	const COLORREF CR_DARKBLUE = RGB(0, 0, 0x80);

	SendMessage(SCI_STYLESETBOLD, STYLE_CONTROLCHAR, 1);
	// Hypertext default is used for all the document's text
	SetAStyle(SCE_H_DEFAULT, black, bkColor, 9, "宋体");

	SetAStyle(SCE_H_TAG, 128);
	SetAStyle(SCE_H_TAGEND, 128);
	SetAStyle(SCE_H_ATTRIBUTE, CR_RED);
	SetAStyle(SCE_H_NUMBER, RGB(0x80, 0, 0x80));
	SetAStyle(SCE_H_DOUBLESTRING, RGB(0, 0, 0x80));
	SetAStyle(SCE_H_SINGLESTRING, RGB(0, 0, 0x80));
	SetAStyle(SCE_H_OTHER, RGB(0x80, 0, 0x80));
	SetAStyle(SCE_H_COMMENT, RGB(0, 0x80, 0));
	SetAStyle(SCE_H_ENTITY, RGB(0x80, 0, 0x80));

	SetAStyle(SCE_H_XMLSTART, CR_DARKBLUE);	// <?
	SetAStyle(SCE_H_XMLEND, CR_DARKBLUE);		// ?>

	SetAStyle(SCE_HB_DEFAULT, black);
	SetAStyle(SCE_HB_COMMENTLINE, CR_DARKGREEN);
	SetAStyle(SCE_HB_NUMBER, RGB(0, 0x80, 0x80));
	SetAStyle(SCE_HB_WORD, CR_DARKBLUE);
	SendMessage(SCI_STYLESETBOLD, SCE_HB_WORD, 1);
	SetAStyle(SCE_HB_STRING, RGB(0x80, 0, 0x80));
	SetAStyle(SCE_HB_IDENTIFIER, black);

	const COLORREF lightBlue = RGB(0xA6, 0xCA, 0xF0);

	SendMessage(SCI_STYLESETBACK, SCE_HB_STRINGEOL, RGB(0x7F, 0x7F, 0xFF));
	SendMessage(SCI_STYLESETFONT, SCE_HB_COMMENTLINE, reinterpret_cast<LPARAM>("宋体"));
}

void CScintillaWnd::findMatchingBracePos(int & braceAtCaret, int & braceOpposite)
{
	int caretPos = int(SendEditor(SCI_GETCURRENTPOS));
	braceAtCaret = -1;
	braceOpposite = -1;
	TCHAR charBefore = '\0';

	int lengthDoc = int(SendEditor(SCI_GETLENGTH));

	if ((lengthDoc > 0) && (caretPos > 0))
	{
		charBefore = TCHAR(SendEditor(SCI_GETCHARAT, caretPos - 1, 0));
	}
	// Priority goes to character before caret
	if (charBefore && _tcschr(TEXT("<>[](){}"), charBefore))
	{
		braceAtCaret = caretPos - 1;
	}

	if (lengthDoc > 0 && (braceAtCaret < 0))
	{
		// No brace found so check other side
		TCHAR charAfter = TCHAR(SendEditor(SCI_GETCHARAT, caretPos, 0));
		if (charAfter && _tcschr(TEXT("<>[](){}"), charAfter))
		{
			braceAtCaret = caretPos;
		}
	}
	if (braceAtCaret >= 0)
		braceOpposite = int(SendEditor(SCI_BRACEMATCH, braceAtCaret, 0));
}

bool CScintillaWnd::doMatch()           //匹配括号并加亮缩进向导 
{
	int braceAtCaret = -1;
	int braceOpposite = -1;
	findMatchingBracePos(braceAtCaret, braceOpposite);

	if ((braceAtCaret != -1) && (braceOpposite == -1))
	{
		SendEditor(SCI_BRACEBADLIGHT, braceAtCaret);
		SendEditor(SCI_SETHIGHLIGHTGUIDE, 0);
	}
	else
	{
		SendEditor(SCI_BRACEHIGHLIGHT, braceAtCaret, braceOpposite);

		//if (_pEditView->isShownIndentGuide())
		{
			int columnAtCaret = int(SendEditor(SCI_GETCOLUMN, braceAtCaret));
			int columnOpposite = int(SendEditor(SCI_GETCOLUMN, braceOpposite));
			SendEditor(SCI_SETHIGHLIGHTGUIDE, (columnAtCaret < columnOpposite) ? columnAtCaret : columnOpposite);
		}
	}

	return (braceAtCaret != -1);
}

SStringT CScintillaWnd::GetHtmlTagname()
{
	int caretPos = int(SendEditor(SCI_GETCURRENTPOS));
	TCHAR charBefore = '\0';
	SStringT tagname;
	int lengthDoc = int(SendEditor(SCI_GETLENGTH));

	if ((lengthDoc > 0) && (caretPos > 0))
	{
		int namestart = caretPos;
		int nameend = 0;
		do
		{
			charBefore = TCHAR(SendEditor(SCI_GETCHARAT, --namestart, 0));
		} while (charBefore != '<' && namestart >= 0);

		if (namestart >= 0)
		{
			nameend = namestart + 1;
			while (nameend <= caretPos)
			{
				charBefore = TCHAR(SendEditor(SCI_GETCHARAT, nameend++, 0));
				if (charBefore == ' ')
					break;

				tagname += charBefore;
			}
			tagname.Trim();
		}
	}

	return tagname;
}

SStringA CScintillaWnd::GetNotePart(int curPos)
{
	// int(SendEditor(SCI_GETCURRENTPOS))
	int startPos = SendEditor(SCI_WORDSTARTPOSITION, curPos, true);
	SStringA tagname;
	if (curPos == startPos)
		return tagname;

	const int wordMaxSize = 64;
	char name[wordMaxSize] = {0};
	int len = (curPos > startPos) ? (curPos - startPos) : (startPos - curPos);
	if (len < wordMaxSize)
	{
		Sci_TextRange sci_tr;
		if (curPos > startPos)
		{
			sci_tr.chrg.cpMin = startPos;
			sci_tr.chrg.cpMax = curPos;
		}
		else
		{
			sci_tr.chrg.cpMin = curPos;
			sci_tr.chrg.cpMax = startPos;
		}

		sci_tr.lpstrText = name;
		SendEditor(SCI_GETTEXTRANGE, 0, reinterpret_cast<LPARAM>(&sci_tr));

		tagname = sci_tr.lpstrText;
	}

	tagname.Trim();

	return tagname;
}

void CScintillaWnd::ShowAutoComplete(const char ch)
{
	if (SendEditor(SCI_AUTOCACTIVE, 0, 0) != 0)
		return;

	long lStart = SendEditor(SCI_GETCURRENTPOS, 0, 0);
	int startPos = SendEditor(SCI_WORDSTARTPOSITION, lStart, true);

	if (ch == '.')
	{
		SStringA str = g_pMainDlg->m_UIResFileMgr.GetSkinAutos();
		if (!str.IsEmpty())
		{
			SendEditor(SCI_AUTOCSHOW, lStart - startPos, (LPARAM)(LPCSTR)str);
		}
	}
	else if (ch == '/')
	{
		//int startPos = SendEditor(SCI_WORDSTARTPOSITION, lStart-1, true);
		SStringA clsName = GetNotePart(lStart - 1);
		SStringA str;
		if (clsName.IsEmpty())
			str = g_pMainDlg->m_UIResFileMgr.GetStyleAutos();
		else if (clsName.CompareNoCase("color") == 0)
			str = g_pMainDlg->m_UIResFileMgr.GetColorAutos();
		else if (clsName.CompareNoCase("string") == 0)
			str = g_pMainDlg->m_UIResFileMgr.GetStringAutos();

		if (!str.IsEmpty())
		{
			SendEditor(SCI_AUTOCSHOW, lStart - startPos, (LPARAM)(LPCSTR)str);
		}
	}
	else if (ch == '<')
	{
		SStringA str = g_SysDataMgr.GetCtrlAutos();
		if (!str.IsEmpty())
		{
			SendEditor(SCI_AUTOCSHOW, lStart - startPos, (LPARAM)(LPCSTR)str);
		}
	}
	else if (ch >= 'a' && ch <= 'z')
	{
		SStringT tagname = GetHtmlTagname();
		if (!tagname.IsEmpty())
		{
			SStringA str = g_SysDataMgr.GetCtrlAttrAutos(tagname);
			if (!str.IsEmpty())
			{	// 自动完成字串要进行升充排列, 否则功能不正常
				SendEditor(SCI_AUTOCSHOW, lStart - startPos, (LPARAM)(LPCSTR)str);
			}
		}		
	}
}

LRESULT CScintillaWnd::OnNotify(int idCtrl, LPNMHDR pnmh)
{
	if (pnmh->hwndFrom != m_hWnd) return 0;
	LRESULT lRet = 0;
	SCNotification *pSCNotification = (SCNotification*)pnmh;
	switch (pnmh->code)
	{
	case SCN_MARGINCLICK:
	{
		int nLine = SendMessage(SCI_LINEFROMPOSITION, (WPARAM)pSCNotification->position);
		SendMessage(SCI_TOGGLEFOLD, (WPARAM)nLine);
	}
	break;

	case SCN_MODIFIED:
	{
		const char *pp = pSCNotification->text;
		if (pp)	//判断是否是文字改变
		{
			UpdateLineNumberWidth();
			SetDirty(true);
		}
	}
	break;

#ifdef SCN_AUTOCCOMPLETED
	// SOUI内核带Scintilla没实现这个, 需要 Scintilla 3.6.0 以上版本
	case SCN_AUTOCCOMPLETED:
	{
		const char *pp = pSCNotification->text;
		if (pp)
		{
			char name[8] = { 0 };
			Sci_TextRange sci_tr;
			sci_tr.chrg.cpMin = pSCNotification->position - 2;
			sci_tr.chrg.cpMax = pSCNotification->position;
			sci_tr.lpstrText = name;
			SendEditor(SCI_GETTEXTRANGE, 0, reinterpret_cast<LPARAM>(&sci_tr));
			if ((strcmp(name, "\".") == 0) || (strcmp(name, "\"/") == 0))
			{
				SendEditor(SCI_DELETERANGE, pSCNotification->position - 1, 1);
			}
		}
	}
	break;
#endif // SCN_AUTOCCOMPLETED

	
	case SCN_CHARADDED:
	{
		BOOL bReadonly = (BOOL)SendEditor(SCI_GETREADONLY);
		if (!bReadonly)
			SetDirty(true);

		char pp = tolower(pSCNotification->ch);
		ShowAutoComplete(pp);
	}
	break;

	case SCN_UPDATEUI:
	{
		doMatch();
	}
	break;

	default:
		break;
	}
	return lRet;
}

BOOL IsCtrlPressed()
{
	return ((GetKeyState(VK_CONTROL) & (1 << (sizeof(SHORT) * 8 - 1))) != 0);
}

BOOL IsAltPressed()
{
	return ((GetKeyState(VK_MENU) & (1 << (sizeof(SHORT) * 8 - 1))) != 0);
}

BOOL IsShiftPressed()
{
	return ((GetKeyState(VK_SHIFT) & (1 << (sizeof(SHORT) * 8 - 1))) != 0);
}

void CScintillaWnd::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (nChar == 'S' && IsCtrlPressed())
	{
		if (m_bDirty)
		{
			if (m_fnCallback)
			{
				(this->*m_fnCallback)(this, 0, m_strFileName);
			}
		}
	}
	// 不加这个, 默认是Handled消息 Scintilla功能会不正常
	SetMsgHandled(FALSE);
}

void CScintillaWnd::GotoFoundLine()
{
	int currentPos = SendEditor(SCI_GETCURRENTPOS);
	int lno = SendEditor(SCI_LINEFROMPOSITION, currentPos);
	int start = SendEditor(SCI_POSITIONFROMLINE, lno);
	int end = SendEditor(SCI_GETLINEENDPOSITION, lno);
	if (start + 2 >= end) return; // avoid empty lines

	if (SendEditor(SCI_GETFOLDLEVEL, lno) & SC_FOLDLEVELHEADERFLAG)
	{
		SendEditor(SCI_TOGGLEFOLD, lno);
		return;
	}

	displaySectionCentered(start, end);
	displaySectionCentered(start, end);
}

void CScintillaWnd::displaySectionCentered(int posStart, int posEnd, bool isDownwards)
{
	int testPos = (isDownwards) ? posEnd : posStart;
	SendEditor(SCI_SETCURRENTPOS, testPos);
	int currentlineNumberDoc = (int)SendEditor(SCI_LINEFROMPOSITION, testPos);
	int currentlineNumberVis = (int)SendEditor(SCI_VISIBLEFROMDOCLINE, currentlineNumberDoc);
	SendEditor(SCI_ENSUREVISIBLE, currentlineNumberDoc);	// make sure target line is unfolded

	int firstVisibleLineVis = (int)SendEditor(SCI_GETFIRSTVISIBLELINE);
	int linesVisible = (int)SendEditor(SCI_LINESONSCREEN) - 1;	//-1 for the scrollbar
	int lastVisibleLineVis = linesVisible + firstVisibleLineVis;

	//if out of view vertically, scroll line into (center of) view
	int linesToScroll = 0;
	if (currentlineNumberVis <= firstVisibleLineVis)
	{
		linesToScroll = currentlineNumberVis - firstVisibleLineVis;
		//use center
		linesToScroll -= linesVisible / 2;
	}
	else if (currentlineNumberVis >= lastVisibleLineVis)
	{
		linesToScroll = currentlineNumberVis - lastVisibleLineVis;
		//use center
		linesToScroll += linesVisible / 2;
	}
	SendEditor(SCI_LINESCROLL, 0, linesToScroll);
	//Make sure the caret is visible, scroll horizontally (this will also fix wrapping problems)
	SendEditor(SCI_GOTOPOS, posStart);
	//SendEditor(SCI_GOTOPOS, posEnd);

	SendEditor(SCI_SETANCHOR, posStart);
}
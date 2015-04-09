/****************************************************************************
*  功    能：LRC歌词文件解析                                                *
*  添 加 人：小可                                                           *
*  添加时间：2015.04.10 04：27                                              *
*  版本类型：初始版本                                                       *
*  联系方式：QQ-1035144170                                                  *
****************************************************************************/
#pragma once

#include <vector>
#include "atlstr.h"//使用CString或其他
#include <tchar.h>
#include<algorithm>
#include <atlconv.h>
using namespace std;

#define MAX_LINE_LENGHT 100

typedef struct tagTime
{
	int     nNum;
	float   fTimeValu;
	int     ndd;//横向距离(用于水平滚动模式)
}TIMETAG;

class CLrcParse
{
public:
	CLrcParse(void);
	~CLrcParse(void);
public:
	std::vector<TIMETAG> m_vTimeTags;  //存储时间标签
	std::vector<CString> m_vWords;     //存储每句歌词

	BOOL   ReadFile(LPCSTR lpszFile);  //读取歌词文件(.lrc)
	int    GetLine(FILE* fp,char* szLine) const;
	BOOL   ParseLine( LPCSTR lpsz, int lineNumber);
	void   AddWord(LPCSTR lpszWord);
	void   AddTimeTag(LPCSTR lpszTag, int Number);
	void   SortForTime();
	double StringToDouble( LPCTSTR str );
};
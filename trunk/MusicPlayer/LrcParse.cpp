#include "stdafx.h"
#include "LrcParse.h"


CLrcParse::CLrcParse(void)
{

}
CLrcParse::~CLrcParse(void)
{

}


/*===================================================================
*  函 数 名：CompareTimeRise                                          
*  功能描述: 将标签按时间降序排序
*  输入参数：TIMETAG& tag1, TIMETAG& tag2     
*  输出参数: 无
*  返 回 值：true  执行成功
* 		     false 执行失败
*  抛出异常：无
====================================================================*/
bool CompareTimeLand(TIMETAG& tag1, TIMETAG& tag2)
{
	return tag1.fTimeValu > tag2.fTimeValu;
}


/*===================================================================
*  函 数 名：CompareTimeLand                                         
*  功能描述: 将标签按时间升序排序
*  输入参数：TIMETAG& tag1, TIMETAG& tag2     
*  输出参数: 无
*  返 回 值：true  执行成功
* 		     false 执行失败
*  抛出异常：无
====================================================================*/
bool CompareTimeRise(TIMETAG& tag1, TIMETAG& tag2)
{
	return tag1.fTimeValu < tag2.fTimeValu;
}


/*===================================================================
*  函 数 名：StrToSS                                          
*  功能描述: 将字符串的时间转化为以秒为单位的浮点值
*  输入参数：LPCSTR lpszTag  
*  输出参数: 无
*  返 回 值：float
*  抛出异常：无
====================================================================*/
float StrToSS(LPCSTR lpszTag)
{
	int paramNums = 0;
	int minutes;
	float seconds;
	USES_CONVERSION;
	paramNums = sscanf(lpszTag, "%d:%f", &minutes, &seconds);
	if(paramNums != 2)
	{
		return 1000000.f;
	}
	return 60*minutes + seconds;
}


/*===================================================================
*  函 数 名：GetLine                                        
*  功能描述: 从文本文件中取一行文本地函数 
*  输入参数：fp:     文件指针，输入时保证非空
*            szLine: 返回行的指针，长度为MAX_LINE_LENGTH 
*  输出参数: 无
*  返 回 值：int 该行的字符数 
*  抛出异常：无
====================================================================*/
int CLrcParse::GetLine(FILE* fp,char* szLine) const 
{ 
	int ch = fgetc(fp); 
	int nIndex = 0; 
	while(!feof(fp) && (ch != '\n') && (nIndex < MAX_LINE_LENGHT)) 
	{ 
		szLine[nIndex++] = (char)ch; 
		ch = fgetc(fp); 
	} 
	szLine[nIndex] = 0; 
	return nIndex; 
}  


/*===================================================================
*  函 数 名：ReadFile                                        
*  功能描述: 根据文件路径读取歌词字符 
*  输入参数：LPCSTR lpszFile
*  输出参数: 无
*  返 回 值：TRUE  执行成功
* 		     FALSE 执行失败
*  抛出异常：无
====================================================================*/
BOOL CLrcParse::ReadFile(LPCSTR lpszFile)
{
	FILE *stream; 
	char line[100];
	//CString line;
	int i=0;

	m_vTimeTags.clear();
	m_vWords.clear();

	if( (stream = fopen( "E:\\Project\\Lrc_Soui滚动歌词\\Lrc_Soui\\Debug\\张学友 - 如果这都不算爱.lrc", "r" )) != NULL ) 
	{ 

		while( fgets( line, 100, stream ) != NULL) 
		{
			//printf( "%s \n" ,line); 
			if(ParseLine(line,i))//不是空行
			{
				i++;

			}
		}
		fclose( stream ); 

	}
	STRACE(_T("TimeTag Count: %d, Word Count: %d \n"), m_vTimeTags.size(), m_vWords.size());

	////最后增加一个无歌词的空行
	//m_vWords.push_back("");
	//TIMETAG tm;
	//tm.nNum = m_vWords.size()-1;
	//tm.fTimeValu = ((CLrc_TestDlg*)AfxGetMainWnd())->GetSongTotalTime(); //时间值为文件的总时间长度
	//m_vTimeTags.push_back(tm);

	SortForTime();

	//TRACE("TimeTag Count: %d, Word Count: %d \n", m_vTimeTags.size(), m_vWords.size());


	//SetHorzData();

	return TRUE;
}


/*===================================================================
*  函 数 名：ParseLine                                        
*  功能描述: 解析传入的每行字符的时间和歌词字符单元。时间标签存储在数  
*  组m_vTimeTags里,每句歌词存储在数组m_vWords里，两个数组通过nNumber
*  做映射，多个时间标签可以对应m_vWords的相同的歌词内容。
*  输入参数： LPCSTR lpsz, int lineNumber
*  输出参数: 无
*  返 回 值：TRUE  执行成功
* 		     FALSE 执行失败
*  抛出异常：无
====================================================================*/
BOOL CLrcParse::ParseLine( LPCSTR lpsz, int lineNumber)
{
	CStringA str(lpsz);
	CStringA sz_time_tag;
	CStringA strHead;

	str.TrimLeft();
	str.TrimRight();

	int pos, pos2;
	pos = str.Find("[");
	if(pos == -1)
		return FALSE;

	bool bAddStr = true;

	while(pos !=-1 )
	{

		str = str.Mid(pos+1);
		pos = str.Find("]");
		if(pos!=-1)
		{
			if(lineNumber<5)
			{
				if( (pos2 = str.Find(":")) != -1)
				{
					strHead = str.Left(pos2);

					if( strHead== "ti" || strHead== "ar" || strHead=="by" || strHead=="al" || strHead == "offset" )
					{
						str = str.Left(str.GetLength()-1);
						bAddStr = false;
						break;
					}
				}  
			}
			USES_CONVERSION;
			AddTimeTag(str.Left(pos),lineNumber);
			str = str.Mid(pos+1);
		}
		else
		{
			bAddStr = false;
			return bAddStr;
		}

		pos = str.Find("[");

	}//end while


	if(bAddStr)
	{
		str.TrimLeft();
		str.TrimRight();
		USES_CONVERSION;
		AddWord(str);
	}

	//STRACE("%d, %d \n", m_vTimeTags.size(), m_vWords.size());

	return bAddStr;
}


/*===================================================================
*  函 数 名：AddWord                                        
*  功能描述: 保存一句歌词到容器 
*  输入参数：LPCSTR lpszWord
*  输出参数: 无
*  返 回 值：无
*  抛出异常：无
====================================================================*/
void CLrcParse::AddWord(LPCSTR lpszWord)
{
	m_vWords.push_back(lpszWord);
	//	STRACE("%s\n", lpszWord);
}


/*===================================================================
*  函 数 名：AddTimeTag                                        
*  功能描述: 保存一个时间标签到容器
*  输入参数：LPCSTR lpszTag, int Number
*  输出参数: 无
*  返 回 值：无
*  抛出异常：无
====================================================================*/
void CLrcParse::AddTimeTag(LPCSTR lpszTag, int Number)
{
	TIMETAG tag;
	tag.nNum   = Number;
	//tag.fTimeValu = _tstof(lpszTag.GetBuffer()); 
	//tag.fTimeValu=(float)_ttoi(lpszTag);
	//tag.fTimeValu=(float)atof(lpszTag);
	//tag.fTimeValu=_wtof(lpszTag);
	tag.fTimeValu = StrToSS(lpszTag);
	m_vTimeTags.push_back(tag);

	//STRACE( "%f\n", tag.fTimeValu*1000 );
}


/*===================================================================
*  函 数 名：SortForTime                                        
*  功能描述: 按时间大小排序
*  输入参数：无
*  输出参数: 无
*  返 回 值：无
*  抛出异常：无
====================================================================*/
void CLrcParse::SortForTime()
{
	//使用stl排序算法按时间大小排序
	std::sort(m_vTimeTags.begin(),m_vTimeTags.end(), CompareTimeRise);
}


/*===================================================================
*  函数原型: double CLrcParse::StringToDouble( LPCTSTR str )
*  函 数 名：StringToDouble                                        
*  功能描述: CString 转换到 double ( 支持Unicode )
*  输入参数：LPCTSTR str
*  输出参数: 无
*  返 回 值：double
*  抛出异常：无
====================================================================*/
double CLrcParse::StringToDouble( LPCTSTR str )
{
	TCHAR szBuff[ 20 ], *p;
	p = szBuff;

#ifdef _UNICODE
	return wcstod( str, &p );   
#else
	return strtod( str, &p );   
#endif
}
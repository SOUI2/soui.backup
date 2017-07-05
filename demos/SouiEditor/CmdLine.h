
#pragma once

/*
	CCmdLine cmdLine(GetCommandLine());
	if (cmdLine.GetParamCount() > 0)
	{
		cmdLine.GetParam(0);	//0 放的是进程路径
		cmdLine.GetParam(1);	//从1开始才是参数
	}
*/

class CCmdLine
{
private:
	enum {MAX_PARAM_COUNT = 25};//最多处理25个参数
	LPTSTR m_szStrBuf;
	int m_nBufSize;
	LPTSTR m_szParams[MAX_PARAM_COUNT];
	int m_nParamCount;
	//工具函数,分析命令行
	void ProcessCmdLine()
	{
		if(m_szStrBuf == NULL) return;
		
		bool blInQt = false;//是否在一对引号内,"引号内状态"标志
		bool blInParam = false;//是否在一个参数内
		//循环遍历这个Buf
		for (LPTSTR p = m_szStrBuf; *p != 0; p++)
		{
			switch(*p)
			{
			case _T(' '): //---------------------空格
				{
					//忽略引号范围内的空格
					if(blInQt) break;
					if(blInParam)//参数结束了
					{
						*p = _T('\0');
						blInParam = false;
					}
					break;
				}
			case _T('\"'): //---------------------引号
				{
					//翻转"引号内状态"标志
					blInQt = !blInQt;
					if(blInParam)//参数结束了
					{
						*p = _T('\0');
						blInParam = false;
					}
					else  //一个参数开始了
					{
						//一个参数开始了
						blInParam = true;
						if(m_nParamCount >= MAX_PARAM_COUNT || _T('\0') == *(p + 1)) return;
						m_szParams[m_nParamCount++] = ++p;
					}
					break;
				}
			default:  //---------------------其他字符
				{
					if(!blInParam)
					{
						//一个参数开始了
						blInParam = true;
						if(m_nParamCount >= MAX_PARAM_COUNT) return;
						m_szParams[m_nParamCount++] = p;
					}
					break;
				}
			}
		}
	}
public:
	//构造函数
	CCmdLine(LPCTSTR szCmdLine)
		:m_szStrBuf(NULL)
		,m_nBufSize(0)
		,m_nParamCount(0)
	{
		//初始化指针数组
		memset(m_szParams, 0, MAX_PARAM_COUNT * sizeof(TCHAR));
		int m_nBufSize = lstrlen(szCmdLine) + 1;
		//分配内存,保存通过参数传递近来的字符串
		if(m_nBufSize > 1) m_szStrBuf = new TCHAR[m_nBufSize];
		lstrcpyn(m_szStrBuf, szCmdLine, m_nBufSize);
		m_szStrBuf[m_nBufSize - 1] = 0;//保证文本正常结束
		//分析命令行
		ProcessCmdLine();
	}
	~CCmdLine()
	{
		delete[] m_szStrBuf;
	}
	//取得命令行的参数个数
	int GetParamCount(void) const
	{
		return m_nParamCount;
	}
	//取得某个命令行参数
	LPCTSTR GetParam(int nIndex)
	{
		if(nIndex >= MAX_PARAM_COUNT) return NULL;
		return m_szParams[nIndex];
	}
};


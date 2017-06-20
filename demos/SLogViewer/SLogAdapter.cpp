#include "StdAfx.h"
#include "SLogAdapter.h"
#include "SColorizeText.h"

namespace SOUI
{

	SLogBuffer::SLogBuffer(ILogParse *pLogParser) : m_logParser(pLogParser)
	{

	}

	SLogBuffer::~SLogBuffer()
	{
		Clear();
	}

	void SLogBuffer::Insert(const SLogBuffer & src)
	{
		m_lstLogs.InsertArrayAt(0,&src.m_lstLogs);
		for(int i=0;i<src.m_lstLogs.GetCount();i++)
		{
			m_lstLogs[i]->AddRef();
		}
		for(int i=src.m_lstLogs.GetCount();i<m_lstLogs.GetCount();i++)
		{
			SLogInfo *pLogInfo = m_lstLogs[i];
			pLogInfo->iLine += src.m_nLineCount;
		}
		m_nLineCount += src.m_nLineCount;

		CopyMap(m_mapTags , src.m_mapTags);
		CopyMap(m_mapPids , src.m_mapPids);
		CopyMap(m_mapTids , src.m_mapTids);
	}

	void SLogBuffer::Append(const SLogBuffer & src)
	{
		for(int i=0;i<src.m_lstLogs.GetCount();i++)
		{
			SLogInfo *pLogInfo = src.m_lstLogs[i];
			pLogInfo->iLine += m_nLineCount;
			m_lstLogs.Add(pLogInfo);
			pLogInfo->AddRef();
		}
		m_nLineCount += src.m_nLineCount;

		CopyMap(m_mapTags , src.m_mapTags);
		CopyMap(m_mapPids , src.m_mapPids);
		CopyMap(m_mapTids , src.m_mapTids);
	}

	SLogBuffer & SLogBuffer::operator=(const SLogBuffer & src)
	{
		Clear();
		m_lstLogs.Copy(src.m_lstLogs);
		for(int i=0;i<src.m_lstLogs.GetCount();i++)
		{
			m_lstLogs[i]->AddRef();
		}
		m_nLineCount = src.m_nLineCount;
		CopyMap(m_mapTags , src.m_mapTags);
		CopyMap(m_mapPids , src.m_mapPids);
		CopyMap(m_mapTids , src.m_mapTids);

		m_logParser = src.m_logParser;
		return *this;
	}

	SLogInfo * SLogBuffer::ParseLine(LPCWSTR pszLine)
	{
		SASSERT(m_logParser);
		SLogInfo * pLogInfo=NULL;
		if(!m_logParser->ParseLine(pszLine,&pLogInfo))
		{
			if(!m_lstLogs.IsEmpty())
			{//将不满足一个LOG格式化的行加入上一个LOG行中。
				pLogInfo = m_lstLogs.GetAt(m_lstLogs.GetCount()-1);
				pLogInfo->strContent += SStringW(L"\\n")+pszLine;
			}
			return NULL;
		}else
		{
			m_lstLogs.Add(pLogInfo);
			m_mapTags[pLogInfo->strTag]=true;
			m_mapPids[pLogInfo->dwPid]= true;
			m_mapTids[pLogInfo->dwTid]=true;
			return pLogInfo;
		}
	}

	void SLogBuffer::ParseLog(LPWSTR pszBuffer)
	{
		Clear();

		int nLines=0;
		LPWSTR pLine = pszBuffer;
		for(;;)
		{
			nLines ++;
			WCHAR *pNextLine = wcschr(pLine,0x0A);
			if(pNextLine)
			{
				if(pNextLine-pLine>1 && *(pNextLine-1) == 0x0D)
				{
					*(pNextLine-1)=0;
				}else
				{
					*pNextLine = 0;
				}
			}
			SLogInfo * logInfo = ParseLine(pLine);
			if(logInfo) logInfo->iLine = nLines;

			if(!pNextLine) break;
			pLine = pNextLine+1;
		}
		if(wcslen(pLine)==0) //处理最后一行是空行的问题.
			nLines--;
		m_nLineCount = nLines;
	}

	void SLogBuffer::Clear()
	{
		for(int i=0;i<m_lstLogs.GetCount();i++)
		{
			m_lstLogs[i]->Release();
		}
		m_lstLogs.RemoveAll();
		m_nLineCount = 0;

		m_mapTags.RemoveAll();
		m_mapPids.RemoveAll();
		m_mapTids.RemoveAll();
	}


	//////////////////////////////////////////////////////////////////////////
	SLogAdapter::SLogAdapter(void):m_lstFilterResult(NULL),m_filterLevel(-1),m_pScilexer(NULL)
	{
		m_crLevels[Verbose]=m_crLevels[Debug]=m_crLevels[Info]=RGBA(0,0,0,255);
		m_crLevels[Warn]=RGBA(255,255,0,255);
		m_crLevels[Error]=m_crLevels[Assert]=RGBA(255,0,0,255);
	}

	SLogAdapter::~SLogAdapter(void)
	{
		clear();
	}

	int SLogAdapter::getCount()
	{
		return (int)GetLogList()->GetCount();
	}


	void SLogAdapter::clear()
	{
		SLogBuffer::Clear();

		if(m_lstFilterResult) delete m_lstFilterResult;
		m_lstFilterResult = NULL;
		m_filterLevel = -1;
		m_filterKeyInfo.Clear();
		m_filterTags.RemoveAll();
		m_filterTids.RemoveAll();
		m_filterPids.RemoveAll();
	}

	void SLogAdapter::getView(int position, SWindow * pItem,pugi::xml_node xmlTemplate)
	{
		if(pItem->GetChildrenCount()==0)
		{
			pItem->InitFromXml(xmlTemplate);
		}

		SLogInfo * pLogInfo = GetLogList()->GetAt(position);
		pItem->FindChildByID(R.id.txt_line)->SetWindowText(SStringT().Format(_T("%d"),pLogInfo->iLine));
		pItem->FindChildByID(R.id.txt_time)->SetWindowText(S_CW2T(pLogInfo->strTime));
		pItem->FindChildByID(R.id.txt_pid)->SetWindowText(SStringT().Format(_T("%u"),pLogInfo->dwPid));
		pItem->FindChildByID(R.id.txt_tid)->SetWindowText(SStringT().Format(_T("%u"),pLogInfo->dwTid));
		pItem->FindChildByID(R.id.txt_module)->SetWindowText(S_CW2T(pLogInfo->strModule));
		pItem->FindChildByID(R.id.txt_function)->SetWindowText(S_CW2T(pLogInfo->strFunction));
		pItem->FindChildByID(R.id.txt_source_file)->SetWindowText(S_CW2T(pLogInfo->strSourceFile));
		pItem->FindChildByID(R.id.txt_source_line)->SetWindowText(SStringT().Format(_T("%d"),pLogInfo->iSourceLine));

		SWindow * pTxtLevel = pItem->FindChildByID(R.id.txt_level);
		pTxtLevel->SetWindowText(S_CW2T(pLogInfo->strLevel));
		if(pLogInfo->iLevel>=Verbose && pLogInfo->iLevel< Level_Count)
		{
			pTxtLevel->GetStyle().SetTextColor(0,m_crLevels[pLogInfo->iLevel]);
		}
		pItem->FindChildByID(R.id.txt_tag)->SetWindowText(S_CW2T(pLogInfo->strTag));
		SColorizeText *pColorizeText = pItem->FindChildByID2<SColorizeText>(R.id.txt_content);
		pColorizeText->SetWindowText(S_CW2T(pLogInfo->strContent));
		pColorizeText->ClearColorizeInfo();

		SArray<SRange> hilightRange;
		m_filterKeyInfo.FindKeyRange(pLogInfo->strContent,hilightRange);

		for(int i=0;i<hilightRange.GetCount();i++)
		{
			pColorizeText->AddColorizeInfo(hilightRange[i].iBegin,hilightRange[i].iEnd,RGBA(255,0,0,255));
		}

		pItem->GetEventSet()->subscribeEvent(EventItemPanelDbclick::EventID,Subscriber(&SLogAdapter::OnItemDblClick,this));
	}

	SStringW SLogAdapter::GetColumnName(int iCol) const
	{
		wchar_t * colNames[]={
			L"col_line_index",
			L"col_time",
			L"col_pid",
			L"col_tid",
			L"col_level",
			L"col_tag",
			L"col_module",
			L"col_source_file",
			L"col_source_line",
			L"col_function",
			L"col_content"
		};
		return colNames[iCol];
	}


	BOOL SLogAdapter::Load(const TCHAR *szFileName)
	{
		FILE *file = _tfopen(szFileName,_T("rb"));
		if(file)
		{
			fseek(file,0,SEEK_END);
			int len = ftell(file);
			fseek(file,0,SEEK_SET);
			char *pBuf = (char*)malloc(len+1);
			fread(pBuf,1,len,file);
			fclose(file);
			pBuf[len]=0;
			
			
			CAutoRefPtr<ILogParse> pMatchParser;

			for(int i=0;i<m_parserFactory->GetLogParserCount() && !pMatchParser;i++)
			{
				ILogParse *pLogParser = m_parserFactory->CreateLogParser(i);
				if(pLogParser->TestLogBuffer(pBuf,len))
				{
					pMatchParser = pLogParser;
				}
				pLogParser->Release();
			}

			if(!pMatchParser)
			{
				free(pBuf);
				return FALSE;
			}

			//目前只支持多字节的log
			int uniLen = MultiByteToWideChar(pMatchParser->GetCodePage(),0,pBuf,len,NULL,0);
			WCHAR* pUniBuf = (WCHAR*) malloc((uniLen+1)*sizeof(WCHAR));
			MultiByteToWideChar(pMatchParser->GetCodePage(),0,pBuf,len,pUniBuf,uniLen);
			free(pBuf);
			pUniBuf[uniLen]=0;

			SStringA bufUtf8 = S_CW2A(pUniBuf,CP_UTF8);

			SLogBuffer logBuffer(pMatchParser);
			logBuffer.ParseLog(pUniBuf);
			free(pUniBuf);

			if(logBuffer.m_lstLogs.IsEmpty())
				return FALSE;

			if(m_logParser)
			{
				if(m_logParser->GetName()!=logBuffer.m_logParser->GetName())
				{
					Clear();
					m_pScilexer->SendMessage(SCI_SETTEXT,0,(LPARAM)"");
				}
			}


			if(m_lstLogs.IsEmpty())
			{//原有LOG为空
				*(SLogBuffer*)this = logBuffer;
				m_pScilexer->SendMessage(SCI_SETTEXT,0,(LPARAM)(LPCSTR)bufUtf8);
			}else
			{
				SLogInfo *pLine1 = m_lstLogs[0];
				SLogInfo *pLine2 = logBuffer.m_lstLogs[0];
				if(pLine1->time<pLine2->time)
				{//原有log的时间大于新LOG的时间，原log加(append)到新log后
					Append(logBuffer);
					m_pScilexer->SendMessage(SCI_APPENDTEXT,bufUtf8.GetLength(),(LPARAM)(LPCSTR)bufUtf8);
				}else
				{
					Insert(logBuffer);
					m_pScilexer->SendMessage(SCI_INSERTTEXT,0,(LPARAM)(LPCSTR)bufUtf8);
				}
			}

			doFilter();
			return TRUE;
		}
		return FALSE;
	}

	void SLogAdapter::SetFilter(const SStringT& str)
	{
		m_filterKeyInfo.SetFilterKeys(S_CT2W(str));
		doFilter();
	}

	const SArray<SLogInfo*> * SLogAdapter::GetLogList() const
	{
		if(m_lstFilterResult!=NULL)
			return m_lstFilterResult;
		return &m_lstLogs;
	}


	void SLogAdapter::SetLevel(int nCurSel)
	{
		m_filterLevel = nCurSel;
		doFilter();
	}

	void SLogAdapter::doFilter()
	{
		if(m_filterKeyInfo.IsEmpty() && m_filterLevel == -1 
			&& m_filterTags.IsEmpty() 
			&& m_filterPids.IsEmpty() 
			&& m_filterTids.IsEmpty())
		{
			if(m_lstFilterResult)
			{
				delete m_lstFilterResult;
				m_lstFilterResult = NULL;
			}
		}else
		{
			if(m_lstFilterResult)
			{
				m_lstFilterResult->RemoveAll();
			}else
			{
				m_lstFilterResult = new SArray<SLogInfo*>;
			}

			for(int i=0;i<m_lstLogs.GetCount();i++)
			{
				if(m_filterKeyInfo.TestExclude(m_lstLogs[i]->strContent) || !m_filterKeyInfo.TestInclude(m_lstLogs[i]->strContent) )
					continue;
				if(m_lstLogs[i]->iLevel < m_filterLevel)
					continue;
				if(!m_filterTags.IsEmpty() && !m_filterTags.Lookup(m_lstLogs[i]->strTag))
					continue;
				if(!m_filterPids.IsEmpty() && !m_filterPids.Lookup(m_lstLogs[i]->dwPid))
					continue;
				if(!m_filterTids.IsEmpty() && !m_filterTids.Lookup(m_lstLogs[i]->dwTid))
					continue;

				m_lstFilterResult->Add(m_lstLogs[i]);
			}
		}
		notifyDataSetChanged();

	}

	int SLogAdapter::GetTags(SArray<SStringW> &tags) const
	{
		SPOSITION pos = m_mapTags.GetStartPosition();
		while(pos)
		{
			SStringW tag = m_mapTags.GetNextKey(pos);
			tags.Add(tag);
		}
		return tags.GetCount();
	}

	int SLogAdapter::GetPids(SArray<UINT> &pids) const
	{
		SPOSITION pos = m_mapPids.GetStartPosition();
		while(pos)
		{
			UINT tag = m_mapPids.GetNextKey(pos);
			pids.Add(tag);
		}
		return pids.GetCount();

	}

	int SLogAdapter::GetTids(SArray<UINT> &tids) const
	{
		SPOSITION pos = m_mapTids.GetStartPosition();
		while(pos)
		{
			UINT tag = m_mapTids.GetNextKey(pos);
			tids.Add(tag);
		}
		return tids.GetCount();
	}


	void SLogAdapter::SetParserFactory(IParserFactory *pParserFactory)
	{
		m_parserFactory = pParserFactory;
	}

	SLogInfo* SLogAdapter::GetLogInfo(int iItem) const
	{
		const SArray<SLogInfo*> *pLogInfos = GetLogList();
		if(pLogInfos->GetCount()<=iItem) return NULL;
		return pLogInfos->GetAt(iItem);
	}

	void SLogAdapter::SetFilterTags(const SArray<SStringW> & tags)
	{
		m_filterTags.RemoveAll();
		for(int i=0;i<tags.GetCount();i++)
		{
			m_filterTags[tags[i]]=true;
		}
		doFilter();
	}

	void SLogAdapter::SetFilterPids(const SArray<UINT> & lstPid)
	{
		m_filterPids.RemoveAll();
		for(int i=0;i<lstPid.GetCount();i++)
		{
			m_filterPids[lstPid[i]]=true;
		}
		doFilter();

	}

	void SLogAdapter::SetFilterTids(const SArray<UINT> & lstTid)
	{
		m_filterTids.RemoveAll();
		for(int i=0;i<lstTid.GetCount();i++)
		{
			m_filterTids[lstTid[i]]=true;
		}
		doFilter();
	}

	bool SLogAdapter::IsColumnVisible(int iCol) const
	{
		if(!m_logParser) return true;
		return m_logParser->IsFieldValid(Field(iCol));
	}

	void SLogAdapter::SetScintillaWnd(CScintillaWnd *pWnd)
	{
		m_pScilexer = pWnd;
	}

	bool SLogAdapter::OnItemDblClick(EventArgs *e)
	{
		SItemPanel * pItem = sobj_cast<SItemPanel>(e->sender);
		int index = pItem->GetItemIndex();
		SLogInfo * logInfo = GetLogList()->GetAt(index);
		m_pScilexer->SendMessage(SCI_GOTOLINE,logInfo->iLine-1,0);
		m_pScilexer->SetFocus();
		return true;
	}



}

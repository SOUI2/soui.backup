#pragma once

#include <helper/SAdapterBase.h>
#include <helper/STime.h>
#include <unknown/obj-ref-impl.hpp>
#include <souicoll.h>

namespace SOUI
{
	class SLogInfo : public TObjRefImpl<IObjRef>
	{
	public:
		SLogInfo():iSourceLine(0){}
		CTime    time;
		SStringW strTime;
		DWORD	 dwPid;
		DWORD	 dwTid;
		SStringW strPackage;
		SStringW strLevel;
		int	     iLevel;
		SStringW strTag;
		SStringW strContent;
		SStringW strModule;
		SStringW strSourceFile;
		int      iSourceLine;
		SStringW strFunction;
	};

	#define MAX_LEVEL_LENGTH  50

	enum Levels
	{
		Verbose=0,
		Debug,
		Info,
		Warn,
		Error,
		Assert,

		Level_Count,
	};

	enum Field
	{
		col_line_index=0,
		col_time,
		col_pid,
		col_tid,
		col_level,
		col_tag,
		col_moduel,
		col_source_file,
		col_source_line,
		col_function,
		col_content
	};

	struct ILogParse : public IObjRef
	{
		virtual SStringW GetName() const PURE;
		virtual int GetLevels() const PURE;			
		virtual void GetLevelText(wchar_t szLevels[][MAX_LEVEL_LENGTH]) const PURE;
		virtual BOOL ParseLine(LPCWSTR pszLine,SLogInfo **ppLogInfo) const PURE;
		virtual bool IsFieldValid(Field field) const PURE;
	};
	
	struct IParserFactory: public IObjRef
	{
		virtual ILogParse * CreateLogParser(int iParser) const PURE;
		virtual int GetLogParserCount() const PURE;
	};


	struct SRange
	{
		SRange(int a=0,int b=0):iBegin(a),iEnd(b){}
		int iBegin;
		int iEnd;
	};

	class SFilterKeyInfo
	{
	public:

		void Clear()
		{
			m_lstExclude.RemoveAll();
			m_lstInclude.RemoveAll();
		}

		bool IsEmpty() const{
			return m_lstExclude.IsEmpty() && m_lstInclude.IsEmpty();
		}

		void SetFilterKeys(const SStringW & szFilter)
		{
			Clear();
			SStringWList keys;
			int nKeys = SplitString(szFilter,L'|',keys);
			for(int i=0;i<nKeys;i++)
			{
				if(keys[i].GetAt(0)==_T('-'))
				{
					SStringW key = keys[i].Right(keys[i].GetLength()-1);
					if(!key.IsEmpty()) m_lstExclude.Add(key);
				}
				else
					m_lstInclude.Add(keys[i]);
			}
		}

		bool TestExclude(const SStringW &strContent) const
		{
			for(int i=0;i<m_lstExclude.GetCount();i++)
			{
				if(strContent.Find(m_lstExclude[i])!=-1) return true;
			}
			return false;
		}

		bool TestInclude(const SStringW &strContent) const
		{
			if(m_lstInclude.IsEmpty()) return true;

			for(int i=0;i<m_lstInclude.GetCount();i++)
			{
				SStringT key = m_lstInclude[i];
				if(-1 != strContent.Find(key)) return true;
			}
			return false;
		}

		int FindKeyRange(const SStringW &strContent, SArray<SRange> &outRange) const
		{
			outRange.RemoveAll();
			for(int i=0;i<m_lstInclude.GetCount();i++)
			{
				SStringW key = m_lstInclude[i];
				int iEnd=0;
				for(;;)
				{
					int iBegin= strContent.Find(key,iEnd);
					if(iBegin == -1) break;
					iEnd = iBegin + key.GetLength();
					outRange.Add(SRange(iBegin,iEnd));
				}
			}
			if(outRange.IsEmpty()) return 0;
			qsort(outRange.GetData(),outRange.GetCount(),sizeof(SRange),SRangeCmp);		
			return outRange.GetCount();
		}
	private:
		static int SRangeCmp(const void* p1, const void* p2){
			const SRange *r1 = (const SRange *)p1;
			const SRange *r2 = (const SRange *)p2;
			return r1->iBegin - r2->iBegin;
		}

		SStringWList m_lstInclude;
		SStringWList m_lstExclude;
	};

	class SLogAdapter : public SMcAdapterBase
	{
	public:
		SLogAdapter(void);
		~SLogAdapter(void);

		BOOL Load(const TCHAR *szFileName);
		void SetFilter(const SStringT& str);
		void SetLevel(int nCurSel);

		ILogParse * GetLogParse() const {
			return m_logParser;
		}

		int GetTags(SArray<SStringW> &tags) const;
		int GetPids(SArray<UINT> &pids) const;
		int GetTids(SArray<UINT> &tids) const;
		void SetFilterTags(const SArray<SStringW> & tags);
		void SetFilterPids(const SArray<UINT> & lstPid);
		void SetFilterTids(const SArray<UINT> & lstTid);

		void SetParserFactory(IParserFactory *pParserFactory);
		SLogInfo* GetLogInfo(int iItem) const;

	protected:
		BOOL AddLine(LPCWSTR pszLine);		
		const SArray<SLogInfo*> * GetLogList() const;

		void doFilter();
		void clear();
	protected:
		virtual void getView(int position, SWindow * pItem,pugi::xml_node xmlTemplate);
		virtual SStringW GetColumnName(int iCol) const;
		virtual bool IsColumnVisible(int iCol) const;
		virtual int getCount();
	private:
		SArray<SLogInfo*> m_lstLogs;
		SArray<SLogInfo*> *m_lstFilterResult;

		CAutoRefPtr<ILogParse> m_logParser;
	
		SMap<SStringW,bool> m_mapTags;
		SMap<UINT,bool> m_mapPids;
		SMap<UINT,bool> m_mapTids;

		SFilterKeyInfo m_filterKeyInfo;

		int		m_filterLevel;
		
		SMap<SStringW,bool> m_filterTags;
		SMap<UINT,bool> m_filterPids;
		SMap<UINT,bool> m_filterTids;

		COLORREF m_crLevels[Level_Count];

		CAutoRefPtr<IParserFactory> m_parserFactory;
	};

}

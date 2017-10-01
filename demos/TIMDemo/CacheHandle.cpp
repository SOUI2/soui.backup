#include "StdAfx.h"
#include "CacheHandle.h"
#include <ShlObj.h>

#define  Cache_Save_Num				20
#define ChatRecordTable				_T("ChatRecord")

CacheHandle	theCache;
CacheHandle::CacheHandle(void)
{

}

CacheHandle::~CacheHandle(void)
{
	m_Sqlite.Close();
	
	// 清理 map
	for each(auto var in m_mapChatRecord)
	{		
		for each(auto vbr in *var.second)
		{
			delete vbr;
		}

		delete var.second;
	}
}

void CacheHandle::Init(LPCTSTR lpDataPath, LPCTSTR lpUserName)
{
	// IM 缓存 目录
	m_szUserCachePath.Format(_T("%scache\\%s\\"), lpDataPath, lpUserName);
	// IM 图片 缓存 目录
	m_szImgCachePath = m_szUserCachePath + _T("imgcache\\");
	
	// 最近聊天列表 xml
	m_szRecentTalkXml = m_szUserCachePath + _T("RecentList.xml");


	SHCreateDirectoryEx(NULL, m_szImgCachePath, NULL);


	// 聊天记录 db 
	SStringT szChatRecordDbPath = m_szUserCachePath + _T("IMChatRecord.db");
	if(m_Sqlite.Open(szChatRecordDbPath))
	{
		TCHAR lpSQL[1024] = {0};
		_stprintf_s(lpSQL, _T("select name from sqlite_master where type='table' and name='%s'"), ChatRecordTable);
				
		SQLite3Query sQuery = m_Sqlite.execQuery(lpSQL);
						
		if(sQuery.IsEof())
		{
			ZeroMemory(lpSQL, 1024*sizeof(TCHAR));

			_stprintf_s(lpSQL, L"CREATE TABLE %s (id INTEGER, "		//主键id
									L"userid INTEGER, "									// 保存用户的id
									L"type INTEGER, "									// 消息 类型 
									L"bodyid INTEGER, "								// 消息 id
									L"content TEXT, "										// 消息 正文
									L"time INTEGER, "							// 时间
									L"PRIMARY KEY (id ASC))",
									ChatRecordTable);

			int nRet = m_Sqlite.execDML(lpSQL);
		}
	}
}

void CacheHandle::InitRecentTalkInfo(UserList& recentList)
{
	if(!PathFileExists(m_szRecentTalkXml))
		return ;

	if(!m_docRecentTalk.load_file(m_szRecentTalkXml))
	{
		return ;
	}

	auto nodeTalk = m_docRecentTalk.child(L"Talk");
	if(!nodeTalk)
	{
		return ;
	}

	auto node = nodeTalk.child(L"user");
	for ( ; node; node=node.next_sibling(L"user"))
	{
		UserInfo info;
		info.uId = node.attribute(L"id").as_uint();
		info.sName = node.attribute(L"name").as_string();
		info.sContent = node.attribute(L"content").as_string();
		recentList.push_back(info);
		//recentList.emplace_back(info);
	}
}

void CacheHandle::NewTalkInfo(UINT uId, LPCTSTR lpUserAlias, LPCTSTR lpContent)
{
	SStringT szId;
	szId.Format(_T("%d"), uId);

	auto nodeTalk = m_docRecentTalk.child(L"Talk");
	if(!nodeTalk)
	{
		nodeTalk = m_docRecentTalk.append_child(L"Talk");
	}

	auto node = nodeTalk.find_child_by_attribute(L"id", szId, false);
	if(node)
	{
		nodeTalk.remove_child(node);
	}

	auto nodeFirst = nodeTalk.first_child();
	if(!nodeFirst)
		node = nodeTalk.append_child(L"user");
	else
		node = nodeTalk.insert_child_before(L"user", nodeFirst);

	node.append_attribute(L"id") = uId;
	node.append_attribute(L"name") = lpUserAlias;
	node.append_attribute(L"content") = lpContent;
	
	m_docRecentTalk.save_file(m_szRecentTalkXml);
}

void CacheHandle::UpdateTalk(UINT uId, LPCTSTR lpUserAlias, LPCTSTR lpContent/*=NULL*/)
{
	if(NULL == lpUserAlias && NULL == lpContent)
		return ;

	SStringT szId;
	szId.Format(_T("%d"), uId);

	auto nodeTalk = m_docRecentTalk.child(L"Talk");
	if(!nodeTalk)
	{
		nodeTalk = m_docRecentTalk.append_child(L"Talk");
	}

	auto node = nodeTalk.find_child_by_attribute(L"id", szId, false);
	if(node)
	{
		if(NULL != lpUserAlias)
			node.attribute(L"name") = lpUserAlias;

		if(NULL != lpContent)
			node.attribute(L"content") = lpContent;

		m_docRecentTalk.save_file(m_szRecentTalkXml);
	}
}

void CacheHandle::MoveTalkTo(UINT uId)
{
	SStringT szId;
	szId.Format(_T("%d"), uId);

	auto nodeTalk = m_docRecentTalk.child(L"Talk");
	if(!nodeTalk)
	{
		nodeTalk = m_docRecentTalk.append_child(L"Talk");
	}

	auto node = nodeTalk.find_child_by_attribute(L"id", szId, false);
	if(node)
	{
		
		auto nodeBegin = nodeTalk.insert_child_before(L"user", nodeTalk.first_child());

		nodeBegin.append_copy(node.attribute(L"id"));
		nodeBegin.append_copy(node.attribute(L"name"));
		nodeBegin.append_copy(node.attribute(L"content"));
		nodeTalk.remove_child(node);
		
		m_docRecentTalk.save_file(m_szRecentTalkXml);
	}
}

const SendingChatList* CacheHandle::GetSendingChatList(UINT uUserId)
{
	auto ite = m_mapStateChat.find(uUserId);
	if(ite == m_mapStateChat.end())
	{
		return NULL;
	}

	return ite->second;
}

bool CacheHandle::GetSendChatInfo(UINT uUserId, LPCTSTR lpChatId, AsyncSendMsgParam& info)
{
	auto ite = m_mapStateChat.find(uUserId);
	if(ite == m_mapStateChat.end())
	{
		return false;
	}

	for each(auto var in *(ite->second))
	{
		if(0 == var->szChatId.CompareNoCase(lpChatId))
		{
			info.szChatId = var->szChatId;
			info.uRecipientId = var->uRecipientId;
			info.szContent = var->szContent;
			break;
		}
	}

	return true;
}

void CacheHandle::AddSendingChat(UINT uUserId, LPCTSTR lpChatContent, const AsyncSendMsgParam& info)
{
	SendingChatList* pList = NULL;
	auto ite = m_mapStateChat.find(uUserId);
	if(ite == m_mapStateChat.end())
	{
		pList = new SendingChatList;
		m_mapStateChat[uUserId] = pList;
	}
	else
		pList = ite->second;

	SendingChatInfo* pInfo = new SendingChatInfo(eCST_Waiting, lpChatContent, info);
	
	pList->push_back(pInfo);
}

void CacheHandle::UpdateStateChat(UINT uUserId, LPCTSTR lpChatId, EnChatState eState)
{
	auto ite = m_mapStateChat.find(uUserId);
	if(ite == m_mapStateChat.end())
		return ;

	if(NULL == ite->second)
		return ;

	for each(auto var in *(ite->second))
	{
		if(0 == var->szChatId.CompareNoCase(lpChatId))
		{
			var->eState = eState;
			break;
		}
	}
}

void CacheHandle::DelStateChatAndSave(UINT uUserId, LPCTSTR lpChatId, __int64 lBodyId, __int64 lTime)
{
	auto ite = m_mapStateChat.find(uUserId);
	if(ite == m_mapStateChat.end())
		return ;

	if(NULL == ite->second)
		return ;

	for each(auto var in *(ite->second))
	{
		if(0 == var->szChatId.CompareNoCase(lpChatId))
		{
			AddChatRecord(uUserId, eChT_Right, lBodyId, var->szChatContent, lTime);
			ite->second->remove(var);
			break;
		}
	}
}



const ChatRecordList* CacheHandle::GetChatRecordList(UINT uUserId)
{
	auto ite = m_mapChatRecord.find(uUserId);
	if(ite == m_mapChatRecord.end())
	{
		ChatRecordList* pList = new ChatRecordList;
		SelectRecord(uUserId, *pList);
		m_mapChatRecord[uUserId] = pList;
		return pList;
	}
	
	return ite->second;
}

void CacheHandle::AddChatRecord(UINT uUserId, EnChatType eType, __int64 lBodyId, LPCTSTR lpContent, __int64 lTime)
{
	ChatRecordList* pList = NULL;
	auto ite = m_mapChatRecord.find(uUserId);
	if(ite == m_mapChatRecord.end())
	{
		pList = new ChatRecordList;
		m_mapChatRecord[uUserId] = pList;
	}
	else
		pList = ite->second;

	int n = pList->size() - Cache_Save_Num;
	for (int i=0; i<n; ++i)			// 清除 多余项 
	{
		delete pList->front();
		pList->pop_front();
	}

	ChatRecord* pInfo = new ChatRecord;
	pInfo->eType = eType;
	pInfo->lBodyId = lBodyId;
	pInfo->szContent = lpContent;
	pInfo->lTime = lTime;

	pList->push_back(pInfo);

	InsertSqlite(uUserId, *pInfo);
}

SStringT CacheHandle::GetBodyContentBy(UINT uUserId, __int64 lBodyId)
{
	auto ite = m_mapChatRecord.find(uUserId);
	if(ite != m_mapChatRecord.end())
	{
		for each(auto var in *ite->second)
		{
			if(lBodyId == var->lBodyId)
			{
				return var->szContent;
			}
		}
	}

	SStringT szContent;

	TCHAR lpSQL[1024] = {0};
	_stprintf_s(lpSQL, _T("Select content, From %s Where userid=%d and bodyid=%I64d;"), 
		ChatRecordTable,
		uUserId,
		lBodyId);

	SQLite3Query sqlQuery = m_Sqlite.execQuery(lpSQL);
	if(!sqlQuery.IsEof())
	{		
		szContent = sqlQuery.GetText16(0);
	}

	return szContent;
}

void CacheHandle::UpdateBodyContent(UINT uUserId, __int64 lBodyId, LPCTSTR lpContent)
{
	auto ite = m_mapChatRecord.find(uUserId);
	if(ite != m_mapChatRecord.end())
	{
		for each(auto var in *ite->second)
		{
			if(lBodyId == var->lBodyId)
			{
				var->szContent = lpContent;
			}
		}
	}
	
	SStringT sSql;
	sSql.Format(_T("update %s set content='%s' where userid=%d and bodyid=%I64d;"), 
		ChatRecordTable,
		lpContent,
		uUserId,
		lBodyId);
	
	int n = m_Sqlite.execDML(sSql);
	if(SQLITE_OK == n)
	{
		OutputDebugString(_T("update error %d"));
	}
}
void CacheHandle::InsertSqlite(UINT uUserId, const ChatRecord& chatInfo)
{
	SStringT sSql;
	sSql.Format(_T("insert or replace into %s values (?,?,?,?,?,?);"), ChatRecordTable);
	SQLite3Statement Statement = m_Sqlite.CompileStatement(sSql);
	Statement.bind(2, (int)uUserId);
	Statement.bind(3, (int)chatInfo.eType);
	Statement.bind(4, chatInfo.lBodyId);
	Statement.bind(5, chatInfo.szContent);
	Statement.bind(6, chatInfo.lTime);
	int nRow = Statement.execDML();
}

void CacheHandle::SelectRecord(UINT uUserId, ChatRecordList& chatList)
{
	SStringT sSql;
	sSql.Format(_T("Select type, bodyid, content, time  From %s Where userid=%d order by id ASC;"), 
		ChatRecordTable,
		uUserId);

	SQLite3Query sqlQuery = m_Sqlite.execQuery(sSql);
	while (!sqlQuery.IsEof())
	{
		ChatRecord* pInfo = new ChatRecord;
		pInfo->eType = (EnChatType)sqlQuery.GetInt(0);
		pInfo->lBodyId = sqlQuery.GetInt64(1);
		pInfo->szContent = sqlQuery.GetText16(2);
		pInfo->lTime = sqlQuery.GetInt64(3);
	
		chatList.push_back(pInfo);
		
		sqlQuery.NextRow();
	}	
}






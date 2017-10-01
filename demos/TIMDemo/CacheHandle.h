#pragma once
#include "Typedef.h"
#include "SQLiteBase.h"
#include <map>
//IM  的 缓存 处理

class CacheHandle
{
public:
	CacheHandle(void);
	~CacheHandle(void);
	void Init(LPCTSTR lpDataPath, LPCTSTR lpUserName);
	SStringT GetImgCachePath() const{return m_szImgCachePath;}
public:
	//  最近 会话 信息 
	void InitRecentTalkInfo(UserList& recentList);
	void NewTalkInfo(UINT uId, LPCTSTR lpUserAlias, LPCTSTR lpContent);
	void UpdateTalk(UINT uId, LPCTSTR lpUserAlias, LPCTSTR lpContent=NULL);
	void MoveTalkTo(UINT uId);

	// 正在发送的消息
	const SendingChatList* GetSendingChatList(UINT uUserId);
	void AddSendingChat(UINT uUserId, LPCTSTR lpChatContent, const AsyncSendMsgParam& info);
	bool GetSendChatInfo(UINT uUserId, LPCTSTR lpChatId, AsyncSendMsgParam& info);
	void UpdateStateChat(UINT uUserId, LPCTSTR lpChatId, EnChatState eState);
	void DelStateChatAndSave(UINT uUserId, LPCTSTR lpChatId, __int64 lBodyId, __int64 lTime);
	
	//  聊天记录
	const ChatRecordList* GetChatRecordList(UINT uUserId);
	void AddChatRecord(UINT uUserId, EnChatType eType, __int64 lBodyId, LPCTSTR lpContent, __int64 lTime);
	SStringT GetBodyContentBy(UINT uUserId, __int64 lBodyId);
	void UpdateBodyContent(UINT uUserId, __int64 lBodyId, LPCTSTR lpContent);

protected:
	void InsertSqlite(UINT uUserId, const ChatRecord& chatInfo);
	void SelectRecord(UINT uUserId, ChatRecordList& pList);
private:
	
	std::map<UINT, SendingChatList*>				m_mapStateChat;			// 有状态的消息 
	std::map<UINT, ChatRecordList*>				m_mapChatRecord;			// 聊天记录  

	SStringT														m_szUserCachePath;		// 当前用户的cachepath
	
	pugi::xml_document									m_docRecentTalk;
	SStringT														m_szRecentTalkXml;

	SStringT														m_szImgCachePath;			// 图片的缓存文件夹

	SQLite3DB													m_Sqlite;

};


extern CacheHandle		theCache;				// 单例对象 
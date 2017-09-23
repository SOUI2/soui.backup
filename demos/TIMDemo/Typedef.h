#pragma once
#include <list>

#pragma pack(push, 1)

struct UserInfo
{
	UINT				uId;
	SStringT			sName;
	SStringT			sContent;
};

typedef std::list<UserInfo>				UserList;

//服务端 的消息 结构 
struct IMBodyContent
{
public:
	__int64			lBodyId;					// id  每条消息 是唯一的
	//std::wstring	sBodyContent;		// body 内容			// 用wstring  是想 用 move  因为这个有可能 很大 
	SStringT			sBodyContent;		// body 内容	以后 卡了 再优化吧  
	__int64			lTime;					// 每条消息 都有时间 
};

typedef std::list<IMBodyContent*>		IMBodyContentList;

// 聊天msg 类型
enum EnChatType
{
	eChT_Left = 0,
	eChT_Center = 1,
	eChT_Right = 2,
	eChT_More = 3,
	eChT_Split = 4,
	eChT_CenterWithoutBk = 5,
};

//聊天 记录  保存 结构 
struct ChatRecord
{
	EnChatType		eType;
	__int64			lBodyId;
	SStringT			szContent;
	__int64			lTime;
};
typedef std::list<ChatRecord*>		ChatRecordList;


// soap 发送消息 结构体
struct SendMsgParam
{
	UINT				uRecipientId;				// 接收人 id
	SStringT			szContent;					// 正文内容 
};

// 异步 发送 消息 参数
struct AsyncSendMsgParam : public SendMsgParam		
{
	SStringT			szChatId;				// 这个是 rich 消息 id 用来 查找的
};

// 正在发送 的消息状态  成功发送 消息后 就不需要这个了 
enum EnChatState
{
	eCST_Waiting = 0,
	eCST_Sending = 1,
	eCST_Error = 2,
};

// 正在 发送的 消息 结构体 
struct SendingChatInfo : public AsyncSendMsgParam				
{
	EnChatState			eState;						// 状态
	SStringT					szChatContent;			// 界面 
	SendingChatInfo(EnChatState e, LPCTSTR lpChatContent, const AsyncSendMsgParam& info)
		: AsyncSendMsgParam(info)
		, eState(e)
		, szChatContent(lpChatContent)
	{

	}
};

typedef std::list<SendingChatInfo*>		SendingChatList;


struct SendMsgResult
{
	__int64		lBodyId;
	bool			bApprove;
	__int64		lTime;
};

#pragma pack(pop)
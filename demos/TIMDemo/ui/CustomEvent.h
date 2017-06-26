
#ifndef __CUSTOM_EVENT_H__
#define __CUSTOM_EVENT_H__

//#include "core/SWnd.h"
namespace SOUI
{

	/////////////////////////////////通知中心 异步 事件   /////////////////////////////////////////

	// 一开始的初始化 
	class EventStartInit : public TplEventArgs<EventStartInit>
	{
		SOUI_CLASS_NAME(EventStartInit, L"on_event_startinit")
	public:
		EventStartInit(SObject *pSender)
			: TplEventArgs<EventStartInit>(pSender)
			, bSuccess(false)
		{
			
		}
		enum{EventID=EVT_EXTERNAL_BEGIN+10001};
		bool bSuccess;
		
		SStringT szErrorText;
	};
	
	//通知中心  异步获取 未读消息 事件 
	class EventGetUnreadMsg : public TplEventArgs<EventGetUnreadMsg>
	{
		SOUI_CLASS_NAME(EventGetUnreadMsg, L"on_event_gerunread")
	public:
		EventGetUnreadMsg(SObject *pSender)
			: TplEventArgs<EventGetUnreadMsg>(pSender)
			, bSuccess(false)
			, uSenderId(0)
		{
		
		}
		~EventGetUnreadMsg()
		{
			for each(auto var in bodyList)
			{
				delete var;
			}
		}
		enum{EventID=EVT_EXTERNAL_BEGIN+10003};
		bool bSuccess;
		IMBodyContentList bodyList;
		UINT uSenderId;
		SStringT szErrorText;
	};

	// 通知中心 发送 消息 状态通知
	class EventSendMsg : public TplEventArgs<EventSendMsg>
	{
		SOUI_CLASS_NAME(EventSendMsg, L"on_event_sendmsg")
	public:
		EventSendMsg(SObject *pSender)
			: TplEventArgs<EventSendMsg>(pSender)
			, uRecipierId(0)
		{
			bSuccess = false;
			
			lTime = 0;
			lBodyId = 0;
		}
		enum{EventID=EVT_EXTERNAL_BEGIN+10004};
		bool bSuccess;
		UINT uRecipierId;
		SStringT szRichObjId;
		__int64 lBodyId;
		__int64 lTime;
		
	};

	// 通知中心 上传附件 更新状态 
	class EventHttpUploadFile : public TplEventArgs<EventHttpUploadFile>
	{
		SOUI_CLASS_NAME(EventHttpUploadFile, L"on_event_httpuploadfile")
	public:
		EventHttpUploadFile(SObject *pSender)
			: TplEventArgs<EventHttpUploadFile>(pSender)
			, uRecipierId(0)
			, nFileId(0)
		{
		}
		enum{EventID=EVT_EXTERNAL_BEGIN+10005};
		UINT uRecipierId;
		int nFileId;				// 大于0  成功 文件id  负数  -100 - 0  进度   -101 开始 表示错误
	};

	// 通知中心 下载附件 更新 状态
	class EventHttpDownFile : public TplEventArgs<EventHttpDownFile>
	{
		SOUI_CLASS_NAME(EventHttpDownFile, L"on_event_httpdownfile")
	public:
		EventHttpDownFile(SObject *pSender)
			: TplEventArgs<EventHttpDownFile>(pSender)
			, uRecipientId(0)
			, lBodyId(0)
			, nAttachId(0)
			, nPercent(0)
		{
		}
		enum{EventID=EVT_EXTERNAL_BEGIN+10006};
		UINT uRecipientId;
		__int64 lBodyId;			// 那个  用来查找 界面元素 要更新界面
		UINT nAttachId;
		int nPercent;				// <0  错误  > 100  成功
	};

	// 通知中心 保存审批员 
	class EventSaveApprover : public TplEventArgs<EventSaveApprover>
	{
		SOUI_CLASS_NAME(EventHttpUploadFile, L"on_event_saveapprover")
	public:
		EventSaveApprover(SObject *pSender)
			: TplEventArgs<EventSaveApprover>(pSender)
		{
		}
		enum{EventID=EVT_EXTERNAL_BEGIN+10007};
		
	};

}



#endif	//__PATHBAR_H__
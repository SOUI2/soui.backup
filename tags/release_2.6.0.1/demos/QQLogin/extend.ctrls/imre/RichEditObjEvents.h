// ------------------------------------------------------------------------------
//
// RichEditObjEvents.h : 
//
// 各RichEdit对象的事件定义。有ole类型，也有自定义的RichEditObj类型
//
// 该头文件定义的宏对应 EventRichEditObj.SubEventId 字段, 事件接受者通过
// SubEventId可知事件是由哪类对象发出。
//
// ------------------------------------------------------------------------------


#pragma once

#define RICHOBJ_EVENT_BASE      0

#define DBLCLICK_IMAGEOLE               (RICHOBJ_EVENT_BASE+0)     // 双击图片ole
#define CLICK_RICH_FETCHMORE            (RICHOBJ_EVENT_BASE+1)     // 点击查看更多ole
#define DBLCLICK_RICH_METAFILE          (RICHOBJ_EVENT_BASE+2)     // 双击文件图标ole，在输入框
#define CLICK_ARTICLEOLE                (RICHOBJ_EVENT_BASE+3)     // 点击图文消息
#define CLICK_FILEOLE                   (RICHOBJ_EVENT_BASE+4)     // 点击了文件OLE的某个按钮
#define CLICK_FETCHMOREOLE_MORE_MSG     (RICHOBJ_EVENT_BASE+5)     // 点击获取更多OL的"查看更多消息"
#define CLICK_FETCHMOREOLE_OPEN_LINK    (RICHOBJ_EVENT_BASE+6)     // 点击了获取更多OLE的"打开消息记录"
#define CLICK_LINK                      (RICHOBJ_EVENT_BASE+7)     // 点击了自己实现的链接
#define CLICK_VOICEOLE                  (RICHOBJ_EVENT_BASE+8)     // 点击了语音OLE
#define CLICK_BK_ELE                    (RICHOBJ_EVENT_BASE+9)     // 点击了背景元素
#define CLICK_EXTMSG_ELE                (RICHOBJ_EVENT_BASE+10)    // 点击了扩展消息OLE

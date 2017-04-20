#include "stdafx.h"
#include "RichEditObjFactory.h"
#include "RichEditObj.h"
#include "RichEditOleCtrls.h"

RichEditObjFactory::RichEditObjFactory()
{
    m_mapCreater[RichEditText::GetClassName()]          = RichEditText::CreateObject;
    m_mapCreater[RichEditBkImg::GetClassName()]         = RichEditBkImg::CreateObject;
    m_mapCreater[RichEditBubble::GetClassName()]        = RichEditBubble::CreateObject;
    m_mapCreater[RichEditAvatar::GetClassName()]        = RichEditAvatar::CreateObject;
    m_mapCreater[RichEditPara::GetClassName()]          = RichEditPara::CreateObject;
    m_mapCreater[RichEditContent::GetClassName()]       = RichEditContent::CreateObject;
    m_mapCreater[RichEditImageOle::GetClassName()]      = RichEditImageOle::CreateObject;
    m_mapCreater[RichEditFileOle::GetClassName()]       = RichEditFileOle::CreateObject;
    m_mapCreater[RichEditFetchMoreOle::GetClassName()]  = RichEditFetchMoreOle::CreateObject;
    m_mapCreater[RichEditSplitLineOle::GetClassName()]  = RichEditSplitLineOle::CreateObject;
    m_mapCreater[RichEditNewsOle::GetClassName()]       = RichEditNewsOle::CreateObject;
    m_mapCreater[RichEditReminderOle::GetClassName()]   = RichEditReminderOle::CreateObject;
}

RichEditObjFactory::~RichEditObjFactory()
{
}

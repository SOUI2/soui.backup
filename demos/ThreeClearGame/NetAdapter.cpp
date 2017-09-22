#include "stdafx.h"
#include "NetAdapter.h"

CNetAdapter::CNetAdapter(std::vector<std::vector<Grid>> vecNet)
{
	m_vecNet = vecNet;
	m_event = NULL;
	m_nClickCount = 0;
}

CNetAdapter::~CNetAdapter()
{

}

// 更新显示
void CNetAdapter::UpdateNet(std::vector<std::vector<Grid>> vecNet)
{
	m_vecNet = vecNet;
}

// 设置网格事件
void CNetAdapter::SetEvent(ChangeEvent* pEvent)
{
	if (pEvent != NULL) m_event = pEvent;
}

// 元素个数
int CNetAdapter::getCount()
{
	return NET_ROW_NUMBER * NET_COL_NUMBER;
}

// 设置界面样式
void CNetAdapter::getView(int position, SWindow * pItem,
	pugi::xml_node xmlTemplate)
{
	if (pItem->GetChildrenCount() == 0) {
		pItem->InitFromXml(xmlTemplate);
	}
	// 绑定按钮事件
	SButton* pButton = pItem->FindChildByName2<SButton>(L"btn_grid");
	assert(pItem);
	pItem->GetEventSet()->subscribeEvent(SOUI::EVT_CMD,
		Subscriber(&CNetAdapter::OnButtonClick, this));
	// 记录当前按钮信息
	assert(pItem->GetRoot());
	pItem->GetRoot()->SetUserData(position);
	// 显示图像按钮
	PosPoint point = covertPostion2Grid(position);
	switch (m_vecNet[point.row][point.col].status)
	{
	// 星
	case Grid_Star:
		pButton->SetAttribute(L"skin", SKIN_STAR);
	break;
	// 心
	case Grid_Heart:
		pButton->SetAttribute(L"skin", SKIN_HEART);
	break;
	// 剑
	case Grid_Sword:
		pButton->SetAttribute(L"skin", SKIN_SWORD);
	break;
	// 盾
	case Grid_Shield:
		pButton->SetAttribute(L"skin", SKIN_SHIELD);
	break;
	// 删除
	case Grid_Delete:
		pButton->SetAttribute(L"skin", SKIN_DELETE);
	break;
	}
	pButton->RequestRelayout();
	// 清空按钮选中
	pButton->SetCheck(FALSE);
}

// 按钮点击
bool CNetAdapter::OnButtonClick(EventArgs* pEvt)
{
	m_nClickCount++;
	SWindow* pTemplate = sobj_cast<SWindow>(pEvt->sender);
	assert(pTemplate);
	SButton* pButton = pTemplate->FindChildByName2<SButton>(L"btn_grid");
	assert(pButton);
	// 设置选中
	pButton->SetCheck(TRUE);
	// 获取坐标信息
	assert(pButton->GetRoot());
	INT position = pButton->GetRoot()->GetUserData();
	PosPoint point = covertPostion2Grid(position);
	SOUI::SStringW strPos;
	strPos.Format(L"坐标（%d，%d）被点击", point.row, point.col);
	MyHelper::Instance()->WriteLog(strPos);
	if (m_nClickCount == 2) {
		// 调用消除函数
		m_event->Change(m_preGrid, point);
		// 清空计数
		m_nClickCount = 0;
		// 刷新显示
		notifyDataSetChanged();
	} else {
		m_preGrid = point;
	}
	return true;
}

// 将 tileview 的 position 转化为 Grid 坐标
PosPoint CNetAdapter::covertPostion2Grid(int position)
{
	return PosPoint(position / NET_COL_NUMBER, position % NET_COL_NUMBER);
}
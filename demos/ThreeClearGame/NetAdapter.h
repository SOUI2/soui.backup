/*******************************************************************   
 *  文件名称: NetAdatper.h
 *  简要描述: 实现界面格子内容适配显示
 *   
 *  创建日期: 2017-9-18
 *  作　　者: 王莹
 *  说　　明: 
 *   
 *  修改日期: 
 *  作　　者: 
 *  说　　明: 
 ******************************************************************/  
#pragma once
#include "MyHelper.h"

class CNetAdapter : public SAdapterBase
{
public:
	CNetAdapter(std::vector<std::vector<Grid>> vecNet);
	virtual ~CNetAdapter();

	// 更新显示
	void UpdateNet(std::vector<std::vector<Grid>> vecNet);

	// 设置网格事件
	void SetEvent(ChangeEvent* pEvent);
	
	// 元素个数
	int getCount() override;

	// 设置界面样式
	void getView(int position, SWindow * pItem, 
		pugi::xml_node xmlTemplate) override;

	// 按钮点击
	bool OnButtonClick(EventArgs* pEvt);

protected:
	// 将 tileview 的 position 转化为 Grid 坐标
	PosPoint covertPostion2Grid(int position);

private:
	// 网格
	std::vector<std::vector<Grid>> m_vecNet;
	// 事件
	ChangeEvent* m_event;
	// 选中计数
	INT m_nClickCount;
	// 上一个点
	PosPoint m_preGrid;
};
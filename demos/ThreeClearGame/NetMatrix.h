/*******************************************************************   
 *  文件名称: NetMatrix.h
 *  简要描述: 用来管理游戏中的网格信息的类
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

class NetMatrix : public ChangeEvent
{
public:
	NetMatrix();
	virtual ~NetMatrix();

public:
	// 获取内部数据结构
	std::vector<std::vector<Grid>> GetNet();

	// 设置数据结构
	void SetNet(std::vector<std::vector<Grid>> vecNet);

	// 初始化随机阵列
	// 1. 随机排列四种图像按钮
	// 2. 不能有 3 个及其以上个数相连
	void Init();

	// 主动尝试消除
	bool Change(PosPoint pre, PosPoint cur) override;

	// 被动产生消除（重力降落）
	// 返回是否产生了自动消除
	bool AutoDelete();

	// 设置刷新事件指针
	void SetEvent(RefreshEvent* event);

	// 获取分数
	int GetScore();

	// 重力降落一个格子
	// 如果需要重力降落，则进行一次重力降落并返回 true
	// 如果不需要则返回 false
	bool LandOneGrid();

	// 随机补充格子
	void AddRandomGrid();

protected:
	// 随机产生一个阵列
	void RandomNet(std::vector<std::vector<Grid>>& vecNet);

	// 是否合法阵列
	bool ValidNet(std::vector<std::vector<Grid>> vecNet);

	// 计算消除点
	// point 基于 point 点的状态查找相同点
	// vecNet 网格状态
	std::vector<PosPoint> GetCancelPoints(PosPoint point, std::vector<std::vector<Grid>> vecNet);

private:
	// 存储矩阵
	std::vector<std::vector<Grid>> m_vecNet;
	// 刷新事件
	RefreshEvent* m_event;
	// 当前分数
	INT m_nScore;
};
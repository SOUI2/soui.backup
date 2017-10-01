#include "stdafx.h"
#include "NetMatrix.h"
#include "MyHelper.h"

NetMatrix::NetMatrix() : m_nScore(0)
{
	for (int i = 0; i < NET_ROW_NUMBER; ++i) {
		std::vector<Grid> row;
		for (int j = 0; j < NET_COL_NUMBER; ++j) {
			Grid grid(i, j);
			row.push_back(grid);
		}
		m_vecNet.push_back(row);
	}
	m_event = NULL;
}

NetMatrix::~NetMatrix()
{
	m_vecNet.clear();
}

// 获取内部数据结构
std::vector<std::vector<Grid>> NetMatrix::GetNet()
{
	return m_vecNet;
}

// 设置数据结构
void NetMatrix::SetNet(std::vector<std::vector<Grid>> vecNet)
{
	m_vecNet = vecNet;
}

// 初始化随机阵列
void NetMatrix::Init()
{
	m_nScore = 0;
	RandomNet(m_vecNet);
	while (!ValidNet(m_vecNet)) RandomNet(m_vecNet);
}

// 尝试消除
bool NetMatrix::Change(PosPoint pre, PosPoint cur)
{
	// 临时缓存处理内容
	std::vector<std::vector<Grid>> vecNet = m_vecNet;
	// 如果这两个点中有一个点是删除状态，直接返回不处理
	if (vecNet[pre.row][pre.col].status == Grid_Delete ||
		vecNet[cur.row][cur.col].status == Grid_Delete) {
		MyHelper::Instance()->WriteLog(L"删除按钮不能消除");
		return false;
	}
	// 交换这两个点
	std::swap(vecNet[pre.row][pre.col], vecNet[cur.row][cur.col]);
	// 检查是否能够消去
	if (!ValidNet(vecNet)) 
		MyHelper::Instance()->WriteLog(L"消除成功!");
	else {
		MyHelper::Instance()->WriteLog(L"消除失败!");
		return false;
	}
	// 消除成功，输出消除信息：两个点都需要进行查询
	std::vector<PosPoint> prePoints = GetCancelPoints(pre, vecNet);
	std::vector<PosPoint> curPoints = GetCancelPoints(cur, vecNet);
	SOUI::SStringW strPoint, strPointsMsg, strCancelMsg;
	for (std::vector<PosPoint>::iterator it = prePoints.begin(); it != prePoints.end(); ++it)
		strPointsMsg += strPoint.Format(L"(%d, %d) ", it->row, it->col);
	for (std::vector<PosPoint>::iterator it = curPoints.begin(); it != curPoints.end(); ++it)
		strPointsMsg += strPoint.Format(L"(%d, %d) ", it->row, it->col);
	strCancelMsg.Format(L"消除了以下点：%s", strPointsMsg);
	MyHelper::Instance()->WriteLog(strCancelMsg);
	// 计算分数：每消除一个得 1 分
	m_nScore += (prePoints.size() + curPoints.size());
	// 设置删除的点为删除皮肤
	for (int i = 0; i < prePoints.size(); ++i) {
		vecNet[prePoints[i].row][prePoints[i].col].status = Grid_Delete;
	}
	for (int i = 0; i < curPoints.size(); ++i) {
		vecNet[curPoints[i].row][curPoints[i].col].status = Grid_Delete;
	}
	m_vecNet = vecNet;
	// 刷新界面
	m_event->RefreshNet(m_vecNet);
	return true;
}

// 被动产生消除（重力降落）
bool NetMatrix::AutoDelete()
{
	// 记录有连续情况的点
	std::vector<PosPoint> points;
	// 行检查是否有连续 3 个
	for (int i = 0; i < NET_ROW_NUMBER; ++i) {
		for (int j = 0; j < NET_COL_NUMBER - 2; ++j) {
			if (m_vecNet[i][j].status == m_vecNet[i][j + 1].status	   &&
				m_vecNet[i][j + 1].status == m_vecNet[i][j + 2].status &&
				m_vecNet[i][j].status != Grid_Delete) {
				points.push_back(PosPoint(i, j));
				j += 3;
			}
		}
	}
	// 列检查是否有连续 3 个
	for (int j = 0; j < NET_COL_NUMBER; ++j) {
		for (int i = 0; i < NET_ROW_NUMBER - 2; ++i) {
			if (m_vecNet[i][j].status == m_vecNet[i + 1][j].status	   &&
				m_vecNet[i + 1][j].status == m_vecNet[i + 2][j].status &&
				m_vecNet[i][j].status != Grid_Delete) {
				points.push_back(PosPoint(i, j));
				i += 3;
			}
		}
	}
	// 如果没有找到这样的点，则证明没有被动产生消除
	if (points.size() == 0) return false;
	// 根据点获取消除点
	std::set<PosPoint> cancelPoints;
	for (int i = 0; i < points.size(); ++i) {
		std::vector<PosPoint> tempPoints = GetCancelPoints(points[i], m_vecNet);
		cancelPoints.insert(tempPoints.begin(), tempPoints.end());
	}
	// 计算分数
	m_nScore += cancelPoints.size();
	// 输出自动消除点的信息
	SOUI::SStringW strAutoCancelPoints, strAutoCancelMsg;
	for (std::set<PosPoint>::iterator it = cancelPoints.begin(); it != cancelPoints.end(); ++it) {
		SOUI::SStringW strPoint;
		strPoint.Format(L"(%d, %d)", it->row, it->col);
		strAutoCancelPoints += strPoint;
	}
	strAutoCancelMsg.Format(L"执行自动消除：%s", strAutoCancelPoints);
	MyHelper::Instance()->WriteLog(strAutoCancelMsg);
	// 设置以上点为删除状态
	for (std::set<PosPoint>::iterator it = cancelPoints.begin(); it != cancelPoints.end(); ++it) {
		m_vecNet[it->row][it->col].status = Grid_Delete;
	}
	m_event->RefreshNet(m_vecNet);
	return true;
}

// 设置刷新事件指针
void NetMatrix::SetEvent(RefreshEvent* event)
{
	if (event != NULL) m_event = event;
}

// 获取分数
int NetMatrix::GetScore()
{
	return m_nScore;
}

// 重力降落
bool NetMatrix::LandOneGrid()
{
	// 标记是否发生了重力降落
	bool bNeedLand = false;
	// 遍历每列
	for (int col = 0; col < NET_COL_NUMBER; ++col) {
		// 从下开始遍历每行
		for (int row = NET_ROW_NUMBER - 1; row >= 1; --row) {
			// 让删除点冒泡上去
			if (m_vecNet[row][col].status == Grid_Delete   &&
				m_vecNet[row - 1][col].status != Grid_Delete) {
				// 标记发生了重力沉降
				bNeedLand = true;
				// 依次挪动删除点上去
				int count = row;
				do {
					std::swap(m_vecNet[count][col], m_vecNet[count - 1][col]);
				} while (--count >= 1 && m_vecNet[count - 1][col].status != Grid_Delete);
				break;
			}
		}
	}
	return bNeedLand;
}

// 随机补充格子
void NetMatrix::AddRandomGrid()
{
	// m_vecNet 已经成为了一个删除按钮全部在上面，
	// 图像按钮全部在下面的矩阵了，此时需要做的是：
	// 1. 在首行随机插入点
	// 2. 执行一次沉降
	// 遍历首行
	for (int col = 0; col < NET_COL_NUMBER; ++col) {
		if (m_vecNet[0][col].status == Grid_Delete) {
			int random = MyHelper::Instance()->Random(4);
			switch (random)
			{
			case 0: m_vecNet[0][col].status = Grid_Star; break;
			case 1: m_vecNet[0][col].status = Grid_Heart; break;
			case 2: m_vecNet[0][col].status = Grid_Sword; break;
			case 3: m_vecNet[0][col].status = Grid_Shield; break;
			}
		}
	}
	m_event->RefreshNet(m_vecNet);
}

// 随机产生一个阵列
void NetMatrix::RandomNet(std::vector<std::vector<Grid>>& vecNet)
{
	for (int i = 0; i < NET_ROW_NUMBER; ++i) {
		for (int j = 0; j < NET_COL_NUMBER; ++j) {
			int random = MyHelper::Instance()->Random(4);
			switch (random)
			{
			case 0: vecNet[i][j].status = Grid_Star; break;
			case 1: vecNet[i][j].status = Grid_Heart; break;
			case 2: vecNet[i][j].status = Grid_Sword; break;
			case 3: vecNet[i][j].status = Grid_Shield; break;
			}
		}
	}
}

// 是否合法阵列
bool NetMatrix::ValidNet(std::vector<std::vector<Grid>> vecNet)
{
	// 行检查是否有连续 3 个
	for (int i = 0; i < NET_ROW_NUMBER; ++i) {
		for (int j = 0; j < NET_COL_NUMBER - 2; ++j) {
			if (vecNet[i][j].status == vecNet[i][j + 1].status		&&
				vecNet[i][j + 1].status == vecNet[i][j + 2].status  &&
				vecNet[i][j].status != Grid_Delete) {
				return false;
			}
		}
	}
	// 列检查是否有连续 3 个
	for (int j = 0; j < NET_COL_NUMBER; ++j) {
		for (int i = 0; i < NET_ROW_NUMBER - 2; ++i) {
			if (vecNet[i][j].status == vecNet[i + 1][j].status	   &&
				vecNet[i + 1][j].status == vecNet[i + 2][j].status &&
				vecNet[i][j].status != Grid_Delete) {
				return false;
			}
		}
	}
	// 检查通过
	return true;
}

// 计算消除点
std::vector<PosPoint> NetMatrix::GetCancelPoints(PosPoint point, std::vector<std::vector<Grid>> vecNet)
{
	// 由此起点开始发散查询
	std::vector<PosPoint> horizontalPoints, verticalPoints;
	GridStatus status = vecNet[point.row][point.col].status;
	// 如果查询状态为删除状态，则直接返回（已经消除了的点不能再次消除）
	if (status == Grid_Delete) return std::vector<PosPoint>();
	// 向左
	for (int left = point.col - 1; left >= 0; --left) {
		if (vecNet[point.row][left].status == status)
			horizontalPoints.push_back(PosPoint(point.row, left));
		else break;
	}
	// 向右
	for (int right = point.col + 1; right < NET_COL_NUMBER; ++right) {
		if (vecNet[point.row][right].status == status)
			horizontalPoints.push_back(PosPoint(point.row, right));
		else break;
	}
	// 向上
	for (int up = point.row - 1; up >= 0; --up) {
		if (vecNet[up][point.col].status == status)
			verticalPoints.push_back(PosPoint(up, point.col));
		else break;
	}
	// 向下
	for (int down = point.row + 1; down < NET_ROW_NUMBER; ++down) {
		if (vecNet[down][point.col].status == status)
			verticalPoints.push_back(PosPoint(down, point.col));
		else break;
	}
	// 检查结果是否合理
	// 1. 水平或竖直方向上的个数小于等于 2，则该方向数值舍弃（之所以是 2，是因为
	// 基准点在最后加上）
	if (horizontalPoints.size() < 2) horizontalPoints.clear();
	if (verticalPoints.size() < 2) verticalPoints.clear();
	// 2. 将两个集合的点合并
	std::vector<PosPoint> results;
	for (std::vector<PosPoint>::iterator h = horizontalPoints.begin(); h < horizontalPoints.end(); ++h)
			results.push_back(*h);
	for (std::vector<PosPoint>::iterator v = verticalPoints.begin(); v < verticalPoints.end(); ++v)
			results.push_back(*v);
	if (horizontalPoints.size() >= 2 || verticalPoints.size() >= 2)
		results.push_back(point);
	return results;
}

/*******************************************************************   
 *  文件名称: ThreeClearHelper.h
 *  简要描述: 用来将一些常用的帮助函数放到这里
 *   
 *  创建日期: 2017-9-18
 *  作　　者: 王莹
 *  说　　明: 此类的实现将作为一个单例模式进行
 *   
 *  修改日期: 
 *  作　　者: 
 *  说　　明: 
 ******************************************************************/  
#pragma once


//////////////////////////////////////////////////////////////////////////
// 数据结构定义

// 一个格子的一种状态
enum GridStatus {
	Grid_None = 0,
	Grid_Star = 1,
	Grid_Heart = 2,
	Grid_Sword = 3,
	Grid_Shield = 4,
	Grid_Delete = 5
};

// 一个坐标点
struct PosPoint
{
	PosPoint() : row(0), col(0) {}
	PosPoint(int row, int col) : row(row), col(col) {}
	PosPoint SetPos(int row, int col) { 
		row = row, col = col; 
		return PosPoint(row, col);
	}
	// 要想使用 std::set::insert 正确比较必须重载 operator== 操作符
	bool operator==(const PosPoint& r) const {
		return row == r.row && col == r.col;
	}
	// 要想使用 std::set 必须重载 operator< 操作符
	bool operator<(const PosPoint& r) const {
		return (row + col) < (r.row + r.col);
	}
	int row;
	int col;
};

// 一个格子
struct Grid {
	Grid() : point(0, 0), status(Grid_None) {}
	Grid(int row, int col, GridStatus status = Grid_None) : 
		point(row, col), status(status) {}
	PosPoint point;
	GridStatus status;
};

//////////////////////////////////////////////////////////////////////////
// 事件

// 消除事件
class ChangeEvent {
public:
	// 尝试消除
	virtual bool Change(PosPoint pre, PosPoint cur) = 0;
};

// 刷新网格事件
class RefreshEvent {
public:
	// 重新刷新网格
	virtual void RefreshNet(std::vector<std::vector<Grid>> vecNet) = 0;
};

//////////////////////////////////////////////////////////////////////////
// 单例帮助类

class MyHelper
{
protected:
	MyHelper();

public:
	static MyHelper* Instance();
	virtual ~MyHelper();

	// 初始化窗口信息
	void InitWindow(SOUI::SWindow* pWindow);

public:
	// 获取随机数
	// modular  随机数发生器范围，0开始
	// excepts  在随机数发生器范围内的不计入随机运算的数字
	int Random(int modular, std::vector<int> excepts = std::vector<int>());

	// 写入日志
	void WriteLog(SOUI::SStringW strMsg);

private:
	SOUI::SWindow* m_pWindow;
};
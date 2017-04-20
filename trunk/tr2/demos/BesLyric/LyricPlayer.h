/*
	BesLyric  一款 操作简单、功能实用的 专门用于制作网易云音乐滚动歌词的 歌词制作软件。
    Copyright (C) 2017  BensonLaur

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/**
* @file       LyricPlayer.h
* @version    v1.0      
* @author     BensonLaur   
* @date       2017/01/08
* 
* Describe    LyricPlayer类，定义了 歌词滚动播放页面处理 歌词播放事务的接口
*/

#pragma once
#include "stdafx.h"
#include "MusicPlayer.h"

#include <vector>
using namespace std;

class TimeLineInfo;

/*
*	@brief 歌词播放器，存储和处理 歌词滚动预览过程 中使用到的数据
*/
class LyricPlayer
{
public:
	LyricPlayer();

	//设置各个路径
	//设置音乐路径时，传入播放音乐需要的 消息宿主窗口的句柄
	void setMusicPath(LPCTSTR pathName,HWND hostWnd);
	
	void setLyricPath(LPCTSTR pathName);

	//重置 LyricPlayer的 歌词数据为空
	void reloadPlayer();

	//播放歌曲与滚动歌词 预览开始
	void playingStart(SHostWnd *wnd);

	//预览结束
	void playingEnd(SHostWnd *wnd);

	//设置startPointF 初始值
	void setStartPoint();

	//得到与startPointF 的毫秒差值
	// note: 本来用于滚动歌词，利用和播放起点的 本地时间的差值来计算出当前的 绝对时间偏移，由此来决定是否滚动新的一行歌词
	//		 但在加入了暂停 、前进、后退等功能后，该时间差值已无法用来判断 歌词是否应该滚动，故暂时启用该函数
	int getMsDiffFromStartPoint();

	//如果快进或者后退都会导致，当前行发生变化，故需要先更新再取值
	void updateCurLine();

private:
	//播放音乐
	void playMusic();

	//停止音乐
	void stopMusic();

public:
	TCHAR m_szMusicPathName[_MAX_PATH];			/*存放 选择 的2个从界面选择的路径名*/
	TCHAR m_szLyricPathName[_MAX_PATH];

	vector<TimeLineInfo> m_vLineInfo;		/* 储存带时间信息的歌词的每一行的具体信息 （第一行储存位置为 0 不是 1*/

	int				m_nCurLine;				/* 当前时间行 所在行数 (第一行为：1 不是 0)*/
	int				m_nTotalLine;			/* 带时间信息的 总行数（不包括空行，但包括有时间但是没歌词的行） */
	
	MusicPlayer		m_musicPlayer;			/* 负责歌词滚动预览过程中音乐的播放 */
private:
	ULARGE_INTEGER  startPointF;			/* 对应的 FILETIME ，为了得到时间差，使用FILETIME(单位100ns)*/ 
											/* 使用 函数 setStartPoint 设置startPointF 初始值*/
											/*使用函数 getMsDiffFromStartPoint 得到与startPointF 的毫秒差值 */
};


/*
*	@brief 储存处理一行歌词文件； 处理lrc文件（带时间轴的歌词文件）的辅助类
*/
class TimeLineInfo
{
public:
	TimeLineInfo(SStringT timeLine)
	{
		//初始化类的基本成员的信息
		m_strTimeLine = timeLine;
		int pos = m_strTimeLine.Find(_T(']'));
		SStringT timeStr = m_strTimeLine.Left(pos+1);

		m_strLine = m_strTimeLine.Right(m_strTimeLine.GetLength()-pos-1);
		m_nmSesonds = TimeStringToMSecond(timeStr,timeStr.GetLength());

		if(m_strLine.GetLength()==0)
			m_bIsEmptyLine = true;
		else
			m_bIsEmptyLine = false;
	}
private:
	//从时间标签字符串得到对应的毫秒时间
	int TimeStringToMSecond(LPCTSTR timeStr, int length)
	{
		//TODO：异常抛出处理

		TCHAR szMinute[5];	//分钟
		TCHAR szSecond[5];	//秒
		TCHAR szMSecond[5];	//毫秒

		int i,j;
		//得到: 和. 的位置
		int pos1 = -1,pos2 = -1,pos3 = length-1;
		for(i=0; i<length; i++)
		{
			if(_T(':') == timeStr[i])
				pos1 = i;
			if(_T('.') == timeStr[i])
				pos2 = i;
		}

		//得到三个时间段的字符串
		for(j=0,i=1; i < pos1; i++,j++)
			szMinute[j] = timeStr[i];
		szMinute[j] = _T('\0');

		for(j=0, i = pos1+1; i<pos2; i++, j++)
			szSecond[j] = timeStr[i];
		szSecond[j] = _T('\0');

		for(j=0, i = pos2+1; i<pos3; i++,j++ )
			szMSecond[j] = timeStr[i];
		szMSecond[j] = _T('\0');

		int millisecond = DecStrToDecimal(szMinute) * 60000 + DecStrToDecimal(szSecond)*1000 + DecStrToDecimal(szMSecond);
		return millisecond;
	}

	//返回无符号十进制串对应的数字（十进制串可以是 023、12、04 等形式，数值为0到999）
	int DecStrToDecimal(LPCTSTR timeStr)
	{
		int bit = _tcslen(timeStr);
		int result = 0;
		for(int i=0; i< bit; i++)
		{
			result *= 10;
			result += timeStr[i]-_T('0');
		}
		return result;
	}

public:
	SStringT m_strTimeLine;		/* 直接存储 从文件读取的一整行 */
	SStringT m_strLine;			/* 存储去除时间标记之后的内容 */
	int	m_nmSesonds;			/* 存储时间标记对应的毫秒时间 */
	bool m_bIsEmptyLine;		/* 是否为空行（只有时间标记） */
};
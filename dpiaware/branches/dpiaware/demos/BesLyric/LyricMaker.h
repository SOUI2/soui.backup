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
* @file       LyricMaker.h
* @version    v1.0      
* @author     BensonLaur   
* @date       2017/01/08
* 
* Describe    LyricMaker类，定义了 歌词制作页面处理 歌词制作事务的  LyricMaker类 的接口	
*/

#pragma once
#include "stdafx.h"
#include "MusicPlayer.h"

#include <vector>
using namespace std;

/*
*	@brief 歌词制作器，存储和处理 歌词制作过程中使用到的数据
*/
class LyricMaker
{
public:
	LyricMaker();

	//设置各个路径
	//设置音乐路径时，传入播放音乐需要的 消息宿主窗口的句柄
	void setMusicPath(LPCTSTR pathName,HWND hostWnd);
	
	void setLyricPath(LPCTSTR pathName);
	
	void setm_szOutputPath(LPCTSTR pathName);
	
	//重置 LyricMaker的 歌词数据为空, 生成输出的文件名
	void reloadMaker();

	//制作开始,记录开始制作的时间
	void makingStart();

	//为下一行歌词 标记上 网易云音乐要求的 时间轴格式，写入m_vLyricOutput中
	void markNextLine();

	
	//如果上一行不是空白行的话,添加
	void markSpaceLine();

	//将 m_vLyricOutput 写入输出文件m_szOutputPathName 中
	void makingEnd();

	//获得当前的输出 文件名
	void getOutputFileName(TCHAR* name, int lenth = _MAX_FNAME);

	//获得当前的输出 路径文件名
	void getm_szOutputPathName(TCHAR* name, int lenth = _MAX_PATH);
	
	//结束音乐播放
	void stopMusic();

	//设置最后的一行的状态：是否为空白行
	void setLastLineSpace(bool value);

	//上一行是否为空白行
	bool isLastLineSpace();

private:
	//根据m_szMusicPathName 的文件名得到歌词文件名，并更新outputFileName 和 m_szOutputPathName的值
	void generateOutputFileName();

	//将毫秒差值时间 转换为歌词时间格式 “[00:33.490] Look at the stars”
	//以 [00:33.490] 格式输出到 timeBuf
	void msToLyricTimeString(int ms, LPTSTR timeBuf);

	//播放音乐
	void playMusic();
	
public:
	TCHAR m_szMusicPathName[_MAX_PATH];			/*存放 选择 的三个从界面选择的路径名*/
	TCHAR m_szLyricPathName[_MAX_PATH];
	TCHAR m_szOutputPath[_MAX_PATH];

	TCHAR m_szOutputPathName[_MAX_PATH];		/*输出文件的文件 路径和名字 */

	vector<SStringT> m_vLyricOrigin;			/* 储存原歌词文件 （第一行储存位置为 0 不是 1）*/
	vector<SStringT> m_vLyricOutput;			/* 储存输出歌词文件 */

	int				m_nCurLine;				/* 当前歌词所在行 (第一行为：1 不是 0)*/
	int				m_nTotalLine;			/* 歌词总行数（不包括空行） */
	
	MusicPlayer		m_musicPlayer;			/* 负责歌词制作过程中音乐的播放 */
private:
	ULARGE_INTEGER  startPointF;			/* 对应的 FILETIME ，为了得到时间差，使用FILETIME(单位100ns)*/ 
	TCHAR outputFileName[_MAX_FNAME];		/* 输出文件的文件名 */

	
	bool			m_bLastLineSpace;		/* 当前行是否是空空白行，允许添加空白行，但是非空行之间对多只能有一行空白行 */

};
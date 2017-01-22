/*
*	Copyright (C) 2017  BensonLaur
*	note: Looking up header file for license detail
*/

// LyricPlyer.cpp :  实现  LyricPlayer类 的接口	
//
/////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LyricPlayer.h"
#include "FileHelper.h"
#include <mmsystem.h> 
#pragma comment (lib, "winmm.lib")

//构造函数
LyricPlayer::LyricPlayer():
		 m_nCurLine(0),	
		 m_nTotalLine(0)
{
	m_szMusicPathName[0]= _T('\0');		
	m_szLyricPathName[0]= _T('\0');
}

//设置各个路径
void LyricPlayer::setMusicPath(LPCTSTR pathName, HWND hostWnd)
{
	_tcscpy(m_szMusicPathName,pathName);
	m_musicPlayer.init(m_szMusicPathName,hostWnd);
}
	
void LyricPlayer::setLyricPath(LPCTSTR pathName)
{
	_tcscpy(m_szLyricPathName,pathName);
}

//重置 LyricPlayer的 歌词数据为空
void LyricPlayer::reloadPlayer()
{
	m_vLineInfo.clear();
}

//播放歌曲与滚动歌词 预览开始
void LyricPlayer::playingStart(SHostWnd *wnd)
{
	//更新基本的行数记录的数据
	m_nTotalLine = this->m_vLineInfo.size();
	m_nCurLine = 1;	//初值设为1，定时器循环的需要

	//启动间隔为1毫秒的Timer来更新页面的显示
	wnd->SetTimer(102,1);
	//设置参照的时间起点
	setStartPoint();

	playMusic();
}

//预览结束
void LyricPlayer::playingEnd(SHostWnd *wnd)
{
	//关闭Timer
	wnd->KillTimer(102);
	stopMusic();
}

//播放音乐
void LyricPlayer::playMusic()
{
	m_musicPlayer.openStart();
}

//停止音乐
void LyricPlayer::stopMusic()
{
	m_musicPlayer.closeStop();
}

//设置startPointF 初始值
void LyricPlayer::setStartPoint()
{
	SYSTEMTIME		currentPoint;				/* 记录当前的时间点 */
	GetLocalTime(&currentPoint);
	
	//转换得到得到 startPointF
	SystemTimeToFileTime(&currentPoint,(FILETIME*)&startPointF);
}

//得到与startPointF 的毫秒差值
int LyricPlayer::getMsDiffFromStartPoint()
{
	SYSTEMTIME		currentPoint;				/* 记录当前的时间点 */
	ULARGE_INTEGER  currentPointF;				/* 对应的 FILETIME ，为了得到时间差，使用FILETIME*/ 
	GetLocalTime(&currentPoint);
	
	SystemTimeToFileTime(&currentPoint,(FILETIME*)&currentPointF); 
	
	unsigned __int64 dft=currentPointF.QuadPart-startPointF.QuadPart; 
	int ms = (int)(dft/10000);//得到相差的毫秒数
	
	return ms;
}

//如果快进或者后退都会导致，当前行发生变化，故需要先更新再取值
void LyricPlayer::updateCurLine()
{
	//根据当前歌曲播放的位置，判断当前在哪一行
	int pos = this->m_musicPlayer.getPosition();
	vector<TimeLineInfo>::iterator i= m_vLineInfo.begin();
	int j;
	for(j=1; i != m_vLineInfo.end(); i++,j++)
		if( i->m_nmSesonds > pos)//歌词时间 大于当前播放位置时
		{
			//取前一个位置，则为 m_nCurLine 的值
			this->m_nCurLine = j-1; 
			break;
		}
}
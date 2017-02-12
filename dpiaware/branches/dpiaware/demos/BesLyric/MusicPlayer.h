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
* @file       MusicPlayer.h
* @version    v1.0      
* @author     BensonLaur   
* @date       2017/01/08
* 
* Describe    MusicPlayer类，可以进行打开声音设备、音乐的播放、声音设置等常规操作
*/

#pragma once
#include "stdafx.h"
#include "digitalv.h"

class MusicPlayer
{
public:
	//构造函数
	MusicPlayer();

	//设置音乐路径 和 目标窗口句柄
	void init(LPCTSTR musicPathName, HWND hostWnd);

	//打开并播放音乐
	void openStart();

	//结束并关闭音乐
	void closeStop();

	//从某个位置开始播放音乐
	void play(int milliSecondPosition);

	//seek后状态为stop，但是位置发生了改变此时可获得该最新位置并播放
	void playAfterSeek();

	//暂停音乐
	void pause();
	
	//从暂停的状态恢复播放
	void resume();

	//停止音乐
	void stop();

	//前进或后退一定时间
	void shift(int milliSecond);

	//得到歌曲的长度
	int getLength();

	//返回当前的位置（毫秒）
	int getPosition();

	//获得当前模式状态   返回状态查看 https://msdn.microsoft.com/en-us/library/dd798405(v=vs.85).aspx#MCI_STATUS_MODE
	DWORD_PTR  getModeStatus();

	//打开音乐文件
	MCIERROR openDevice();

	//关闭音乐文件
	MCIERROR closeDevice();

	//设置音量大小(0-1000)
	int setVolumn(int volumn);

	//获得当前音量平均值（默认初始播放音量为最大值1000）
	int getVolumn();

private:
	bool isParamReady(){return  _tcslen(m_szMusicPathName)==0?false:true;}

	//到达指定的位置
	void seek(int position);

private:
	TCHAR m_szMusicPathName[_MAX_PATH];		/* 储存音乐的文件路径名 */
	HWND m_hdlHostWnd;						/* 消息发送到 的窗口的句柄*/
	bool m_bIsParamReady;					/* 记录参数（文件路径名和目标窗口句柄）是否准备好 */
	
	int m_nVolumn;							/* 当前音量 0 ~ 1000 */

	//用于mciSendCommand函数的的参数
	MCI_OPEN_PARMS m_mciOpen;		//打开参数
	MCI_PLAY_PARMS m_mciPlay;		//播放参数
	MCI_GENERIC_PARMS m_mciStop;	//停止参数
	MCI_GENERIC_PARMS m_mciClose;	//关闭参数
	MCI_GENERIC_PARMS m_mciPause;	//暂停参数
	MCI_GENERIC_PARMS m_mciResume;	//暂停恢复参数
	MCI_STATUS_PARMS m_mciStatus;	//查询状态参数 参数具体内容 https://msdn.microsoft.com/en-us/library/dd798405(v=vs.85).aspx
	MCI_SEEK_PARMS m_mciSeek;		//定位参数
	MCI_SET_PARMS  m_mciSet;		//设置参数
	MCI_DGV_SETAUDIO_PARMS m_mciSetVolumn;//音量参数
	MCI_STATUS_PARMS m_mciGetVolumn;//查询音量参数
};
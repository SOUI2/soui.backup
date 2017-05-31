/*
*	Copyright (C) 2017  BensonLaur
*	note: Looking up header file for license detail
*/

// MusicPlayer.cpp :  实现  MusicPlayer类 的接口	
//
/////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "MusicPlayer.h"

MusicPlayer::MusicPlayer()
{
	this->m_hdlHostWnd = NULL;
	this->m_szMusicPathName[0]=_T('\0');
	m_bIsParamReady = false;

	this->m_nVolumn = 1000;
}

//设置音乐路径 和 目标窗口句柄
void MusicPlayer::init(LPCTSTR musicPathName, HWND hostWnd)
{
	_tcscpy(this->m_szMusicPathName,musicPathName);
	this->m_hdlHostWnd = hostWnd;
	m_bIsParamReady = true;
}

//打开并播放音乐
void MusicPlayer::openStart()
{
	if( isParamReady() )
	{
		MCIERROR mcierror = openDevice();
		if(mcierror)
		{
			 TCHAR buf[128]={0};
			 mciGetErrorString(mcierror, buf,128);
			 SMessageBox(this->m_hdlHostWnd,buf,_T("提示"),0);
		}
		else
		{
			//重头开始播放
			play(0);
		}
	}
	else
		SMessageBox(this->m_hdlHostWnd,_T("播放参数还没设置好！"),_T("提示"),0);

}

//结束并关闭音乐
void MusicPlayer::closeStop()
{
	stop();
	closeDevice();
}

//播放音乐
void MusicPlayer::play(int milliSecondPosition)
{	
	//设置音量再播放
	this->setVolumn(this->m_nVolumn);

	m_mciPlay.dwCallback=(DWORD)this->m_hdlHostWnd;
	m_mciPlay.dwFrom = (DWORD)milliSecondPosition; //播放起始位置ms为单位
	mciSendCommand(m_mciOpen.wDeviceID, MCI_PLAY, MCI_NOTIFY|MCI_FROM, (DWORD)(LPVOID)&m_mciPlay); 
}

//seek后状态为stop，但是位置发生了改变此时可获得该最新位置并播放
void MusicPlayer::playAfterSeek()
{
	play(getPosition());
}

//暂停音乐
void MusicPlayer::pause()
{
	m_mciPause.dwCallback = (DWORD)this->m_hdlHostWnd;
	mciSendCommand(m_mciOpen.wDeviceID, MCI_PAUSE, MCI_WAIT|MCI_FROM, (DWORD)(LPVOID)&m_mciPause); 
}

//从暂停的状态恢复播放
void MusicPlayer::resume()
{
	m_mciResume.dwCallback = (DWORD)this->m_hdlHostWnd;
	mciSendCommand(m_mciOpen.wDeviceID, MCI_RESUME, MCI_WAIT|MCI_FROM, (DWORD)(LPVOID)&m_mciResume); 
}

//停止音乐
void MusicPlayer::stop()
{
	m_mciStop.dwCallback = (DWORD_PTR)this->m_hdlHostWnd;	//接收消息的窗口的句柄
	mciSendCommand(m_mciOpen.wDeviceID, MCI_STOP, MCI_WAIT, (DWORD)(LPMCI_GENERIC_PARMS)&m_mciStop); 
}

//前进或后退一定时间
void MusicPlayer::shift(int milliSecond)
{
	int pos = getPosition();
	int len = getLength();
	int nextPos = pos + milliSecond;
	nextPos = min(max(0,nextPos),len);
	seek(nextPos);
}

//得到歌曲的长度
int MusicPlayer::getLength()
{
	m_mciStatus.dwCallback =  (DWORD_PTR)this->m_hdlHostWnd;
	m_mciStatus.dwItem = MCI_STATUS_LENGTH;
	m_mciStatus.dwReturn = 0;
	mciSendCommand(m_mciOpen.wDeviceID, MCI_STATUS, MCI_STATUS_ITEM, (DWORD)&m_mciStatus); 
	return m_mciStatus.dwReturn;
}

//返回当前的位置
int MusicPlayer::getPosition()
{
	m_mciStatus.dwCallback =  (DWORD_PTR)this->m_hdlHostWnd;
	m_mciStatus.dwItem = MCI_STATUS_POSITION;
	m_mciStatus.dwReturn = 0;
	// 指定 MCI_STATUS_MODE 时，需要同时指定MCI_STATUS_ITEM，以用来确保m_mciStatus 中dwItem的设置有效
	mciSendCommand(m_mciOpen.wDeviceID, MCI_STATUS, MCI_STATUS_MODE | MCI_STATUS_ITEM, (DWORD)&m_mciStatus); 
	return m_mciStatus.dwReturn;
}

//获得当前模式状态
DWORD_PTR  MusicPlayer::getModeStatus()
{
	m_mciStatus.dwCallback =  (DWORD_PTR)this->m_hdlHostWnd;
	m_mciStatus.dwItem = MCI_STATUS_MODE;
	m_mciStatus.dwReturn = 0;
	// 指定 MCI_STATUS_MODE 时，需要同时指定MCI_STATUS_ITEM，以用来确保m_mciStatus 中dwItem的设置有效
	mciSendCommand(m_mciOpen.wDeviceID, MCI_STATUS, MCI_STATUS_ITEM, (DWORD)&m_mciStatus); 
	return m_mciStatus.dwReturn;
}

//打开音乐文件
MCIERROR MusicPlayer::openDevice()
{
	m_mciOpen.lpstrDeviceType = _T("mpegvideo"); //要操作的文件类型
	m_mciOpen.lpstrElementName = m_szMusicPathName; //要操作的文件路径

	return mciSendCommand( 0,MCI_OPEN,MCI_OPEN_TYPE | MCI_OPEN_ELEMENT ,(DWORD)&m_mciOpen); //打开文件命令
}

//关闭音乐文件
MCIERROR MusicPlayer::closeDevice()
{
	m_mciClose.dwCallback = (DWORD_PTR)this->m_hdlHostWnd;	//接收消息的窗口的句柄
	return mciSendCommand( m_mciOpen.wDeviceID, MCI_CLOSE, MCI_WAIT,(DWORD)(LPMCI_STATUS_PARMS)&m_mciClose);
}

//设置音量大小(0-1000)
int MusicPlayer::setVolumn(int volumn)
{	
	this->m_nVolumn = min(max(0,volumn),1000);

	m_mciSetVolumn.dwCallback=(DWORD_PTR)this->m_hdlHostWnd;	//接收消息的窗口的句柄
	m_mciSetVolumn.dwItem=MCI_DGV_SETAUDIO_VOLUME;
	m_mciSetVolumn.dwValue= this->m_nVolumn;

	mciSendCommand(m_mciOpen.wDeviceID,MCI_SETAUDIO,MCI_DGV_SETAUDIO_ITEM  |MCI_DGV_SETAUDIO_VALUE,(DWORD)&m_mciSetVolumn);

	return this->m_nVolumn;
}

//获得当前音量平均值（左右平均？）
int MusicPlayer::getVolumn()
{
	m_mciGetVolumn.dwCallback =  (DWORD_PTR)this->m_hdlHostWnd;
	m_mciGetVolumn.dwItem = MCI_DGV_STATUS_VOLUME;
	m_mciGetVolumn.dwReturn = 0;

	mciSendCommand(m_mciOpen.wDeviceID, MCI_STATUS, MCI_DGV_STATUS_NOMINAL | MCI_STATUS_ITEM, (DWORD)&m_mciGetVolumn); 
	return m_mciGetVolumn.dwReturn;
}

//到达指定的位置
void MusicPlayer::seek(int position)
{
	m_mciSet.dwCallback =  (DWORD_PTR)this->m_hdlHostWnd;
	m_mciSet.dwTimeFormat = MCI_FORMAT_MILLISECONDS;  //设置时间格式为 毫秒
	mciSendCommand(m_mciOpen.wDeviceID,MCI_SET, MCI_SET_TIME_FORMAT, (DWORD)&m_mciSeek);//指定设备识别的时间格式

	m_mciSeek.dwCallback =  (DWORD_PTR)this->m_hdlHostWnd;
	m_mciSeek.dwTo = position;
	mciSendCommand(m_mciOpen.wDeviceID,MCI_SEEK, MCI_TO, (DWORD)&m_mciSeek);
}


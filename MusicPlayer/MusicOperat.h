/****************************************************************************
*  功    能：音频文件操作类                                                 *
*  添 加 人：小可                                                           *
*  添加时间：2015.01.17 12：27                                              *
*  版本类型：初始版本                                                       *
*  联系方式：QQ-1035144170                                                  *
****************************************************************************/

#pragma once
#include "DropEx.h"
#include "BassMusic.h"

//通知消息ID
#define MSG_USER_REDRAW	(WM_USER+2000)

class CMusicOpreat:public IMusicState,public SHostWnd
{
public:
	CMusicOpreat(HWND m_PWnd);
	~CMusicOpreat(void);

	//static CMusicOpreat *GetInstance();
private:
	 HWND m_ParenhWnd;
	//接口继承
public:
	//正在播放
	virtual void OnPlaying(){}
	//暂停状态
	virtual void OnPause(){}
	//停止播放
	virtual void OnStop();
	//加/慢 速播放
	virtual void OnStalled(){}

	//自定义
public:
	int                 nIndex;         //播放索引
	HSTREAM             hStream;        //播放流
	CMusicManagerMap	m_MusicManager;
	CBassMusicEngine    *m_pBassMusic;
	IMusicState			*m_pMainState;
	
	void InitDatas();
	void OnButPrev();
	void OnButPlay();
	void OnButPause();
	void OnButPlayNext();

	void InsertMapInfo(int nNum, CString strPath, tagMusicInfo &pMuInfo);
};
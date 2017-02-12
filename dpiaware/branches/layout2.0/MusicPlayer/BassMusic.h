#pragma once

using namespace std;
//////////////////////////////////////////////////////////////////////////
//	
//	
//	音乐引擎
//	
//////////////////////////////////////////////////////////////////////////

//播放状态接口
interface IMusicState
{
	//正在播放
	virtual void OnPlaying() = NULL;
	//暂停状态
	virtual void OnPause() = NULL;
	//停止播放
	virtual void OnStop() = NULL;
	//加/慢 速播放
	virtual void OnStalled() = NULL;
};

//音频信息
struct tagMusicInfo 
{
	TCHAR			szArtist[30];
	TCHAR			szTitle[30];
	DWORD			dwTime;
	HSTREAM			hStream;
};

typedef map<HSTREAM,tagMusicInfo>		CMusicEngineMap;
//////////////////////////////////////////////////////////////////////////

class CBassMusicEngine
{
public:
	tagMusicInfo		m_tagInfo;					//音乐信息
	CMusicEngineMap		m_MusicEngineMap;			//音乐列表
	HSTREAM				m_hStream;					//当前播放
	IMusicState			*m_pMusicState;				//接口指针

	//单例模式
public:
	CBassMusicEngine(void);
	~CBassMusicEngine(void);

	//创建单例
	static CBassMusicEngine *GetInstance();

	//初始化
	void Init(HWND hWnd,IMusicState *pMusicState=NULL);
	//错误处理
	int ShowError(LPCTSTR lpError);
	//播放
	BOOL Play(HSTREAM hStream,bool bRestart = false);
	//暂停
	BOOL Pause(HSTREAM hStream);
	//停止
	BOOL Stop(HSTREAM hStream);
	//释放
	BOOL Free(HSTREAM hStream);
	//是否在播放
	BOOL IsPlaying(HSTREAM hStream);
	//加载文件
	HSTREAM LoadFile(LPCTSTR lpszFileName);
	//获取音量
	int GetVolume();
	//设置音量
	BOOL SetVolume(int nVolume);
	//获取音频信息
	tagMusicInfo *GetInfo(HSTREAM hStream);

protected:
	//回调函数---设置接口播放状态
	static void CALLBACK MySyncProc(HSYNC handle, DWORD channel, DWORD data, void *user);
};

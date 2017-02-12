/****************************************************************************
*  功    能：音频文件操作类                                                 *
*  添 加 人：小可                                                           *
*  添加时间：2015.01.17 12：27                                              *
*  版本类型：初始版本                                                       *
*  联系方式：QQ-1035144170                                                  *
****************************************************************************/

#include "StdAfx.h"
#include "MusicOperat.h"

CMusicOpreat::CMusicOpreat(HWND m_PWnd)
{
	m_ParenhWnd=m_PWnd;
	nIndex=0;         //播放索引
	hStream=NULL;     //播放流
	m_pBassMusic=NULL;
	m_pMainState=NULL;

	//测试：
	CLrcParse lrcPar;
	lrcPar.ReadFile("");

}

CMusicOpreat::~CMusicOpreat(void)
{
	
	if (hStream)
	{
		BASS_ChannelStop(hStream);
		hStream=NULL;
	}
}
//CMusicOpreat * CMusicOpreat::GetInstance()
//{
//	static CMusicOpreat _Instance;
//
//	return &_Instance;
//}

void CMusicOpreat::InitDatas()
{
	//初始化声音组件
	m_pBassMusic = CBassMusicEngine::GetInstance();
	if ( m_pBassMusic == NULL )
	{
		if ( SMessageBox(NULL,TEXT("声音引擎初始化失败"),_T("警告"),MB_OK|MB_ICONEXCLAMATION) == IDOK )
		{
			PostQuitMessage(0);
		}
	}
	m_pBassMusic->Init(m_hWnd,this);

}

void CMusicOpreat::InsertMapInfo(int nNum, CString strPath, tagMusicInfo &pMuInfo)
{
	//加载文件
	HSTREAM hStream = m_pBassMusic->LoadFile(strPath);
	if ( hStream == -1 ) return;

	//获取媒体标签
	tagMusicInfo *pInfo = m_pBassMusic->GetInfo(hStream);

	//通过map和ListBox结合，一起管理播放列表
	tagMusicInfo *pMusicInfo = new tagMusicInfo;

	pMusicInfo->dwTime = pInfo->dwTime;
	pMusicInfo->hStream = pInfo->hStream;
	lstrcpyn(pMusicInfo->szArtist,pInfo->szArtist,CountArray(pMusicInfo->szArtist));
	lstrcpyn(pMusicInfo->szTitle,pInfo->szTitle,CountArray(pMusicInfo->szTitle));

	pMuInfo=*pMusicInfo;
	m_MusicManager.insert(pair<int,tagMusicInfo*>(nNum,pMusicInfo));
}

void CMusicOpreat::OnButPrev()         // 上一曲
{
	m_pBassMusic->Stop(hStream);

	nIndex--;
	if (nIndex<0)
	{
		nIndex=m_MusicManager.size()-1;
	}

	CMusicManagerMap::iterator iter = m_MusicManager.find(nIndex);
	if ( iter == m_MusicManager.end() ) return;

	hStream = iter->second->hStream;

	if( m_pBassMusic->Play(hStream,true) )
	{
		int i=0;
	}
}

void CMusicOpreat::OnButPlay()         // 播放
{

	m_pBassMusic->Stop(hStream);

	CMusicManagerMap::iterator iter = m_MusicManager.find(nIndex);
	if ( iter == m_MusicManager.end() )
	{
		return;
	}else
	{
		hStream = iter->second->hStream;
		if( m_pBassMusic->Play(hStream,/*(++nIndex!= nIndex) ? false : true)*/true ))
		{
			int i=0;
		}
	}

}

void CMusicOpreat::OnButPause()        // 暂停
{
	if ( m_pBassMusic->IsPlaying(hStream) == FALSE ) return;

	if( m_pBassMusic->Pause(hStream) )
	{
		int i=0;

	}
}

void CMusicOpreat::OnButPlayNext()     // 下一曲
{
	m_pBassMusic->Stop(hStream);

	nIndex++;
	if (nIndex>=m_MusicManager.size())
	{
		nIndex=0;
	}

	CMusicManagerMap::iterator iter = m_MusicManager.find(nIndex);
	if ( iter == m_MusicManager.end() ) return;

	hStream = iter->second->hStream;

	if( m_pBassMusic->Play(hStream,true) )
	{
		int i=0;
	}
}

void CMusicOpreat::OnStop()
{
	//自动切换下一首歌
	OnButPlayNext();
	//::PostMessage(GetContainer()->GetHostHwnd(), MSG_USER_SEARCH_DMTASKDLG, 0, 0);
	::PostMessage(m_ParenhWnd,MSG_USER_REDRAW,0,0);
}
#include "StdAfx.h"
#include "BassMusic.h"

CBassMusicEngine::CBassMusicEngine(void)
{
	ZeroMemory(&m_tagInfo,sizeof m_tagInfo);

	m_MusicEngineMap.clear();

	m_hStream = 0;
	m_pMusicState = NULL;
}

CBassMusicEngine::~CBassMusicEngine(void)
{
	BASS_Free();
}

CBassMusicEngine * CBassMusicEngine::GetInstance()
{
	static CBassMusicEngine _Instance;

	return &_Instance;
}

void CBassMusicEngine::Init( HWND hWnd,IMusicState *pMusicState )
{
	if (!BASS_Init(-1,44100,0,hWnd,NULL)) 
	{
		if( ShowError(TEXT("初始化Bass组件失败，程序需要退出！")) == IDOK )
		{
			PostQuitMessage(0);
		}
	}

	m_pMusicState = pMusicState;
}

int CBassMusicEngine::ShowError( LPCTSTR lpError )
{
	CString StrError;
	StrError.Format(TEXT("%s 错误码: %d"),lpError,BASS_ErrorGetCode());

	//return AfxMessageBox(StrError);
	return SMessageBox(NULL,StrError,_T("警告"),MB_OK|MB_ICONEXCLAMATION);	
}

HSTREAM CBassMusicEngine::LoadFile( LPCTSTR lpszFileName )
{
	tagMusicInfo _Info;
	ZeroMemory(&_Info,sizeof _Info);

	if (!(_Info.hStream = BASS_StreamCreateFile(FALSE, lpszFileName,0,0,BASS_SAMPLE_MONO)))
	{
		ShowError(TEXT("不能打开该文件"));
		return -1;
	}

	_Info.dwTime = BASS_ChannelBytes2Seconds(_Info.hStream,BASS_ChannelGetLength(_Info.hStream,BASS_POS_BYTE) );

	TAG_ID3 *pTag = (TAG_ID3 *)BASS_ChannelGetTags(_Info.hStream,BASS_TAG_ID3);
	if ( pTag != NULL )
	{
#ifdef UNICODE
		MultiByteToWideChar(CP_ACP, 0, pTag->artist, -1, _Info.szArtist, 30);
		MultiByteToWideChar(CP_ACP, 0, pTag->title, -1, _Info.szTitle, 30);

		if ( lstrlen(_Info.szArtist) == 0 )
		{
			lstrcpyn(_Info.szArtist,TEXT("未知"),CountArray(_Info.szArtist));
		}

		if ( lstrlen(_Info.szTitle) == 0 )
		{
			CString StrTitle(lpszFileName);

			StrTitle = StrTitle.Right(StrTitle.GetLength()-StrTitle.ReverseFind('\\')-1);
			StrTitle = StrTitle.Left(StrTitle.Find('.'));
			lstrcpyn(_Info.szTitle,StrTitle,CountArray(_Info.szTitle));
		}
#else
		lstrcpyn(_Info.szArtist,CString(pTag->artist),CountArray(_Info.szArtist));
		lstrcpyn(_Info.szTitle,CString(pTag->title),CountArray(_Info.szTitle));
#endif
	}
	else
	{
		CString StrTitle(lpszFileName);

		StrTitle = StrTitle.Right(StrTitle.GetLength()-StrTitle.ReverseFind('\\')-1);
		StrTitle = StrTitle.Left(StrTitle.Find('.'));

		lstrcpyn(_Info.szArtist,TEXT("未知"),CountArray(_Info.szArtist));
		lstrcpyn(_Info.szTitle,StrTitle,CountArray(_Info.szTitle));
	}

	m_MusicEngineMap.insert(pair<HSTREAM,tagMusicInfo>(_Info.hStream,_Info));

	//Play(_Info.hStream);

	return _Info.hStream;
}

BOOL CBassMusicEngine::Play(HSTREAM hStream,bool bRestart/* = false*/)
{
	Stop(m_hStream);

	m_hStream = hStream;

	//开始播放
	BOOL bResult = BASS_ChannelPlay(hStream, bRestart);

	//if ( m_pMusicState != NULL ) m_pMusicState->OnPlaying();

	BASS_ChannelSetSync(hStream, BASS_SYNC_END, (QWORD)MAKELONG(10,0), &CBassMusicEngine::MySyncProc, 0);

	return bResult;
}

tagMusicInfo * CBassMusicEngine::GetInfo( HSTREAM hStream )
{
	CMusicEngineMap::iterator iter = m_MusicEngineMap.find(hStream);

	//if ( iter == m_MusicEngineMap.end()) return NULL;
	
	return &iter->second;
}

BOOL CBassMusicEngine::Pause( HSTREAM hStream )
{
	//if ( m_pMusicState != NULL ) m_pMusicState->OnPause();

	return BASS_ChannelPause(hStream);
}

BOOL CBassMusicEngine::Stop( HSTREAM hStream )
{
	//if ( m_pMusicState != NULL ) m_pMusicState->OnStop();

	return BASS_ChannelStop(m_hStream);
}

BOOL CBassMusicEngine::Free( HSTREAM hStream )
{
	//关闭之前
	return BASS_StreamFree(m_hStream);
}

BOOL CBassMusicEngine::IsPlaying( HSTREAM hStream )
{
	if( BASS_ChannelIsActive(hStream) == BASS_ACTIVE_PLAYING )
		return TRUE;
	
	return FALSE;
}

void CALLBACK CBassMusicEngine::MySyncProc( HSYNC handle, DWORD channel, DWORD data, void *user )
{
	CBassMusicEngine *pBassMusicEngine = CBassMusicEngine::GetInstance();

	if ( (pBassMusicEngine == NULL) || (pBassMusicEngine->m_pMusicState == NULL) ) return;
	
	DWORD dwActive = BASS_ChannelIsActive(channel);

	if( dwActive == BASS_ACTIVE_STOPPED )
	{
		pBassMusicEngine->m_pMusicState->OnStop();
	}
	if( dwActive == BASS_ACTIVE_PAUSED )
	{
		pBassMusicEngine->m_pMusicState->OnPause();
	}
	if( dwActive == BASS_ACTIVE_PLAYING )
	{
		pBassMusicEngine->m_pMusicState->OnPlaying();
	}
	if( dwActive == BASS_ACTIVE_STALLED )
	{
		pBassMusicEngine->m_pMusicState->OnStalled();
	}
}

int CBassMusicEngine::GetVolume()
{
	float fVolume = BASS_GetVolume();

	if ( fVolume == -1 )
	{
		ShowError(TEXT("获取声音大小错误"));
		return 0;
	}

	return int(fVolume*100);
}

BOOL CBassMusicEngine::SetVolume( int nVolume )
{
	if ( nVolume < 0 ) nVolume = 0;
	if ( nVolume > 100 ) nVolume = 100;

	return BASS_SetVolume(nVolume/100.0f);
}



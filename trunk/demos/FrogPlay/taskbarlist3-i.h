#pragma once

#include <Shobjidl.h>

#ifndef __ITaskbarList3_INTERFACE_DEFINED__
#define __ITaskbarList3_INTERFACE_DEFINED__

typedef /* [v1_enum] */ 
enum TBPFLAG
{	TBPF_NOPROGRESS	= 0,
TBPF_INDETERMINATE	= 0x1,
TBPF_NORMAL	= 0x2,
TBPF_ERROR	= 0x4,
TBPF_PAUSED	= 0x8
} 	TBPFLAG;


typedef /* [v1_enum] */ 
enum THUMBBUTTONFLAGS
{	THBF_ENABLED	= 0,
THBF_DISABLED	= 0x1,
THBF_DISMISSONCLICK	= 0x2,
THBF_NOBACKGROUND	= 0x4,
THBF_HIDDEN	= 0x8,
THBF_NONINTERACTIVE	= 0x10
} 	THUMBBUTTONFLAGS;

//DEFINE_ENUM_FLAG_OPERATORS(THUMBBUTTONFLAGS)
typedef /* [v1_enum] */ 
enum THUMBBUTTONMASK
{	THB_BITMAP	= 0x1,
THB_ICON	= 0x2,
THB_TOOLTIP	= 0x4,
THB_FLAGS	= 0x8
} 	THUMBBUTTONMASK;

//DEFINE_ENUM_FLAG_OPERATORS(THUMBBUTTONMASK)
#include <pshpack8.h>
typedef struct THUMBBUTTON
{
	THUMBBUTTONMASK dwMask;
	UINT iId;
	UINT iBitmap;
	HICON hIcon;
	WCHAR szTip[ 260 ];
	THUMBBUTTONFLAGS dwFlags;
} 	THUMBBUTTON;

typedef struct THUMBBUTTON *LPTHUMBBUTTON;

#include <poppack.h>
#define THBN_CLICKED        0x1800


MIDL_INTERFACE("ea1afb91-9e28-4b86-90e9-9e9f8a5eefaf")
ITaskbarList3 : public ITaskbarList2
{
public:
	virtual HRESULT STDMETHODCALLTYPE SetProgressValue( 
		/* [in] */ __RPC__in HWND hwnd,
		/* [in] */ ULONGLONG ullCompleted,
		/* [in] */ ULONGLONG ullTotal) = 0;

	virtual HRESULT STDMETHODCALLTYPE SetProgressState( 
		/* [in] */ __RPC__in HWND hwnd,
		/* [in] */ TBPFLAG tbpFlags) = 0;

	virtual HRESULT STDMETHODCALLTYPE RegisterTab( 
		/* [in] */ __RPC__in HWND hwndTab,
		/* [in] */ __RPC__in HWND hwndMDI) = 0;

	virtual HRESULT STDMETHODCALLTYPE UnregisterTab( 
		/* [in] */ __RPC__in HWND hwndTab) = 0;

	virtual HRESULT STDMETHODCALLTYPE SetTabOrder( 
		/* [in] */ __RPC__in HWND hwndTab,
		/* [in] */ __RPC__in HWND hwndInsertBefore) = 0;

	virtual HRESULT STDMETHODCALLTYPE SetTabActive( 
		/* [in] */ __RPC__in HWND hwndTab,
		/* [in] */ __RPC__in HWND hwndMDI,
		/* [in] */ DWORD dwReserved) = 0;

	virtual HRESULT STDMETHODCALLTYPE ThumbBarAddButtons( 
		/* [in] */ __RPC__in HWND hwnd,
		/* [in] */ UINT cButtons,
		/* [size_is][in] */ __RPC__in_ecount_full(cButtons) LPTHUMBBUTTON pButton) = 0;

	virtual HRESULT STDMETHODCALLTYPE ThumbBarUpdateButtons( 
		/* [in] */ __RPC__in HWND hwnd,
		/* [in] */ UINT cButtons,
		/* [size_is][in] */ __RPC__in_ecount_full(cButtons) LPTHUMBBUTTON pButton) = 0;

	virtual HRESULT STDMETHODCALLTYPE ThumbBarSetImageList( 
		/* [in] */ __RPC__in HWND hwnd,
		/* [in] */ __RPC__in_opt HIMAGELIST himl) = 0;

	virtual HRESULT STDMETHODCALLTYPE SetOverlayIcon( 
		/* [in] */ __RPC__in HWND hwnd,
		/* [in] */ __RPC__in HICON hIcon,
		/* [string][unique][in] */ __RPC__in_opt_string LPCWSTR pszDescription) = 0;

	virtual HRESULT STDMETHODCALLTYPE SetThumbnailTooltip( 
		/* [in] */ __RPC__in HWND hwnd,
		/* [string][unique][in] */ __RPC__in_opt_string LPCWSTR pszTip) = 0;

	virtual HRESULT STDMETHODCALLTYPE SetThumbnailClip( 
		/* [in] */ __RPC__in HWND hwnd,
		/* [in] */ __RPC__in RECT *prcClip) = 0;

};

#endif //__ITaskbarList3_INTERFACE_DEFINED__

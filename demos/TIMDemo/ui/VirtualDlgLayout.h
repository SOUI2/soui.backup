#pragma once

// 模拟 模态对话框 的布局 
class VirtualDlgLayout
{
public:
	VirtualDlgLayout(SWindow* pRoot, LPCTSTR pszResName);
	virtual ~VirtualDlgLayout(void);
		
public:
	virtual void ShowLayout(bool bShow);
	
protected:
	template<class T>
	inline void InitWnd(T*& pWnd, LPCTSTR lpWndName)
	{
		pWnd = m_pLayout->FindChildByName2<T>(lpWndName);
	}
	
	template<typename T, typename A>
	bool subscribeEvent(SWindow* pWin, bool (T::* pFn)(A *), T* pObject) 
	{
		if(NULL == pWin) return false;

		return pWin->GetEventSet()->subscribeEvent(pFn, pObject);
	}

	template<typename T, typename A>
	bool subscribeEvent(LPCTSTR lpWinName, bool (T::* pFn)(A *), T* pObject) 
	{
		SWindow* pWin = m_pLayout->FindChildByName(lpWinName);
		if(NULL == pWin) return false;

		return pWin->GetEventSet()->subscribeEvent(pFn, pObject);
	}
protected:
	SWindow*				m_pLayout;
	SWND					m_hFocus;
};


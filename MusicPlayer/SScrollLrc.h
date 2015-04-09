/****************************************************************************
*  功    能：STreeBox扩展的歌词面板控件                                     *
*  作    者：小可                                                           *
*  添加时间：2015.02.07 17：00                                              *
*  版本类型：初始版本                                                       *
*  联系方式：QQ-1035144170                                                  *
****************************************************************************/
#pragma once

namespace SOUI
{
	typedef BOOL(__stdcall *pCallBackLrc)(void *pUser);// 回调函数原型

    class SScrollLrc : public STreeBox
    {
        SOUI_CLASS_NAME(SScrollLrc,L"scrolllrc")
    public:
        SScrollLrc(void);
        ~SScrollLrc(void);

    protected:
        void OnTimer(char cTimer);
        void OnSize(UINT nType, CSize size);
      
        SOUI_MSG_MAP_BEGIN()
            MSG_WM_TIMER_EX(OnTimer)
            MSG_WM_SIZE(OnSize)

        SOUI_MSG_MAP_END()
        
        SOUI_ATTRS_BEGIN()
            ATTR_INT(L"speed",m_nSpeed,FALSE)
        SOUI_ATTRS_END()

	private:
        int m_nSpeed;
		bool bTurnFlag;
		void *pParen;
		pCallBackLrc pCallSetlrc;

	public:
		static SScrollLrc *GetInstance();

		void StarsRollLrc();
		void LoadLrc(const SStringT strLrc);

		void SetWindowText(const SStringT & strText);
		BOOL GetSetLrcFun(pCallBackLrc funCall,void *pUser);
    };

}

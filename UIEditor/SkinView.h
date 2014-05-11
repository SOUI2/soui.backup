#pragma once

#include <duiimage.h>

namespace SOUI
{
	class CSkinView_Base :
		public CDuiWindow
	{
	public:
		CSkinView_Base(void);
		virtual ~CSkinView_Base(void);

		BOOL SetImageFile(LPCTSTR pszFileName);

	protected:
		virtual CDuiSkinBase * GetSkin() =0;

		void OnPaint(CDCHandle dc);

		DUIWIN_BEGIN_MSG_MAP()
			MSG_WM_PAINT(OnPaint)
		DUIWIN_END_MSG_MAP()

		DUIWIN_DECLARE_ATTRIBUTES_BEGIN()
			DUIWIN_COLOR_ATTRIBUTE("crsep", m_crSep, TRUE)
		DUIWIN_DECLARE_ATTRIBUTES_END()

		COLORREF m_crSep;

		CDuiImgX	m_img;
	};

	class CSkinView_ImgLst : public CSkinView_Base
	{
		DUIOBJ_DECLARE_CLASS_NAME(CSkinView_ImgLst, "skinview_imglst")
	public:
		CSkinView_ImgLst();
		virtual ~CSkinView_ImgLst(){
			delete m_skin;
		}

		void SetVertical(BOOL bVertical)
		{
			CDuiSkinImgList * pSkin= dynamic_cast<CDuiSkinImgList *>(GetSkin());
			if (NULL != pSkin)
			{
				pSkin->SetVertical(bVertical);
			}
		}
		void SetStates(int nStates)
		{
			CDuiSkinImgList * pSkin= dynamic_cast<CDuiSkinImgList *>(GetSkin());
			if (NULL != pSkin)
			{
				pSkin->SetStates(nStates);
			}
		}

		void SetTile(BOOL bTile)
		{
			CDuiSkinImgList * pSkin= dynamic_cast<CDuiSkinImgList *>(GetSkin());
			if (NULL != pSkin)
			{
				pSkin->SetTile(bTile);
			}
		}
	
	protected:
		virtual CDuiSkinBase * GetSkin(){return m_skin;}

		void OnPaint(CDCHandle dc);

		DUIWIN_BEGIN_MSG_MAP()
			MSG_WM_PAINT(OnPaint)
		DUIWIN_END_MSG_MAP()

		CDuiSkinImgList *m_skin;
	};

	class CSkinView_ImgFrame : public CSkinView_ImgLst
	{
		DUIOBJ_DECLARE_CLASS_NAME(CSkinView_ImgFrame, "skinview_imgframe")
	public:
		CSkinView_ImgFrame():m_crFrame(128)
		{
			m_skin=new CDuiSkinImgFrame;
			m_skin->SetImage(&m_img);
		}
		virtual ~CSkinView_ImgFrame(){
			delete m_skin;
		}

		void SetMargin( CRect rcMargin )
		{
			m_skin->SetMargin(rcMargin);
		}

	protected:
		virtual CDuiSkinBase * GetSkin(){return m_skin;}
		void OnPaint(CDCHandle dc);

		DUIWIN_BEGIN_MSG_MAP()
			MSG_WM_PAINT(OnPaint)
		DUIWIN_END_MSG_MAP()

		DUIWIN_DECLARE_ATTRIBUTES_BEGIN()
			DUIWIN_COLOR_ATTRIBUTE("crframe", m_crFrame, TRUE)
		DUIWIN_DECLARE_ATTRIBUTES_END()


		COLORREF m_crFrame;
		CDuiSkinImgFrame *m_skin;
	};

	class CSkinView_Button : public CSkinView_Base
	{
		DUIOBJ_DECLARE_CLASS_NAME(CSkinView_Button, "skinview_button")
	public:
		CSkinView_Button()
		{
			m_skin=new CDuiSkinButton;
		}
		virtual ~CSkinView_Button(){
			delete m_skin;
		}

		CDuiSkinButton * GetButtonSkin(){return m_skin;}
		
	protected:
		virtual CDuiSkinBase * GetSkin(){return m_skin;}

		CDuiSkinButton *m_skin;
	};

	class CSkinView_Gradation : public CSkinView_Base
	{
		DUIOBJ_DECLARE_CLASS_NAME(CSkinView_Gradation, "skinview_gradation")
	public:
		CSkinView_Gradation()
		{
			m_skin=new CDuiSkinGradation;
		}
		virtual ~CSkinView_Gradation(){
			delete m_skin;
		}

		CDuiSkinGradation * GetGradationSkin(){return m_skin;}

	protected:
		virtual CDuiSkinBase * GetSkin(){return m_skin;}

		CDuiSkinGradation *m_skin;
	};

	inline void RegSkinViewClass()
	{
		DuiSystem::getSingleton().RegisterWndFactory(TplDuiWindowFactory<CSkinView_ImgLst>());
		DuiSystem::getSingleton().RegisterWndFactory(TplDuiWindowFactory<CSkinView_ImgFrame>());
		DuiSystem::getSingleton().RegisterWndFactory(TplDuiWindowFactory<CSkinView_Button>());
		DuiSystem::getSingleton().RegisterWndFactory(TplDuiWindowFactory<CSkinView_Gradation>());
	}
}

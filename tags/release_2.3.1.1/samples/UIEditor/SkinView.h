#pragma once

namespace SOUI
{
	class CSkinView_Base : public SWindow
	{
	public:
		CSkinView_Base(void);
		virtual ~CSkinView_Base(void);

		BOOL SetImageFile(LPCTSTR pszFileName);

	protected:
		virtual ISkinObj * GetSkin() =0;

		void OnPaint(IRenderTarget *pRT);

		SOUI_MSG_MAP_BEGIN()
			MSG_WM_PAINT_EX(OnPaint)
		SOUI_MSG_MAP_END()

		SOUI_ATTRS_BEGIN()
			ATTR_COLOR(L"crsep", m_crSep, TRUE)
		SOUI_ATTRS_END()

		COLORREF m_crSep;

        CAutoRefPtr<IBitmap> m_img;
	};

	class CSkinView_ImgList : public CSkinView_Base
	{
		SOUI_CLASS_NAME(CSkinView_ImgList, L"skinview_imglist")
	public:
		CSkinView_ImgList();
		virtual ~CSkinView_ImgList(){
			delete m_skin;
		}
        
		void SetVertical(BOOL bVertical)
		{
			SSkinImgList * pSkin= GetSkinImgList();
			if (NULL != pSkin)
			{
				pSkin->SetVertical(bVertical);
			}
		}
		void SetStates(int nStates)
		{
            SSkinImgList * pSkin= GetSkinImgList();
			if (NULL != pSkin)
			{
				pSkin->SetStates(nStates);
			}
		}

		void SetTile(BOOL bTile)
		{
            SSkinImgList * pSkin= GetSkinImgList();
			if (NULL != pSkin)
			{
				pSkin->SetTile(bTile);
			}
		}
	
	protected:
		virtual ISkinObj * GetSkin(){return m_skin;}

        SSkinImgList * GetSkinImgList(){
            if(!m_skin->IsClass(SSkinImgList::GetClassName())) return NULL;
            return (SSkinImgList*)m_skin;
        }

		SSkinImgList *m_skin;
	};

	class CSkinView_ImgFrame : public CSkinView_ImgList
	{
		SOUI_CLASS_NAME(CSkinView_ImgFrame, L"skinview_imgframe")
	public:
		CSkinView_ImgFrame():m_crFrame(128)
		{
			m_skin=new SSkinImgFrame;
			m_skin->SetImage(m_img);
		}
		virtual ~CSkinView_ImgFrame(){
			delete m_skin;
		}

		void SetMargin( CRect rcMargin )
		{
			m_skin->SetMargin(rcMargin);
		}

	protected:
		virtual ISkinObj * GetSkin(){return m_skin;}
		void OnPaint(IRenderTarget *pRT);

		SOUI_MSG_MAP_BEGIN()
			MSG_WM_PAINT_EX(OnPaint)
		SOUI_MSG_MAP_END()

		SOUI_ATTRS_BEGIN()
			ATTR_COLOR(L"crframe", m_crFrame, TRUE)
		SOUI_ATTRS_END()


		COLORREF m_crFrame;
		SSkinImgFrame *m_skin;
	};

	class CSkinView_Button : public CSkinView_Base
	{
		SOUI_CLASS_NAME(CSkinView_Button, L"skinview_button")
	public:
		CSkinView_Button()
		{
			m_skin=new SSkinButton;
		}
		virtual ~CSkinView_Button(){
			delete m_skin;
		}

		SSkinButton * GetButtonSkin(){return m_skin;}
		
	protected:
		virtual ISkinObj * GetSkin(){return m_skin;}

		SSkinButton *m_skin;
	};

	class CSkinView_Gradation : public CSkinView_Base
	{
		SOUI_CLASS_NAME(CSkinView_Gradation, L"skinview_gradation")
	public:
		CSkinView_Gradation()
		{
			m_skin=new SSkinGradation;
		}
		virtual ~CSkinView_Gradation(){
			delete m_skin;
		}

		SSkinGradation * GetGradationSkin(){return m_skin;}

	protected:
		virtual ISkinObj * GetSkin(){return m_skin;}

		SSkinGradation *m_skin;
	};

	inline void RegSkinViewClass(SApplication *theApp)
	{
		theApp->RegisterWndFactory(TplSWindowFactory<CSkinView_ImgList>());
		theApp->RegisterWndFactory(TplSWindowFactory<CSkinView_ImgFrame>());
		theApp->RegisterWndFactory(TplSWindowFactory<CSkinView_Button>());
		theApp->RegisterWndFactory(TplSWindowFactory<CSkinView_Gradation>());
	}
}

#pragma once

namespace SOUI
{
    class SImgCanvas : public SWindow
    {
        SOUI_CLASS_NAME(SImgCanvas,L"imgcanvas")
    public:
        SImgCanvas(void);
        ~SImgCanvas(void);
        
        BOOL AddFile(LPCWSTR pszFileName);
        void Clear();
		BOOL Save2IconFile(LPCWSTR pszFileName);
        BOOL Save2File(LPCWSTR pszFileName,int nSplit=1);
        void SetVertical(BOOL bVert);		
		bool IsCanSave() { return !m_lstImg.IsEmpty(); }
    protected:
        void OnPaint(IRenderTarget *pRT);
        
        SOUI_MSG_MAP_BEGIN()
            MSG_WM_PAINT_EX(OnPaint)
        SOUI_MSG_MAP_END()
        
        BOOL            m_bVert;
        SList<IBitmap*> m_lstImg;
    };
}

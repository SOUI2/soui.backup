#pragma once
#include "droptarget.h"
#include "SImgCanvas.h"

class CImageMergerHandler : public IFileDropHandler
{
friend class CMainDlg;
public:
    CImageMergerHandler(void);
    ~CImageMergerHandler(void);
    
    void OnInit(SOUI::SWindow *pRoot);
    
    void AddFile(LPCWSTR pszFileName);
protected:
    virtual void OnFileDropdown(HDROP hDrop);

    void OnSave();
	void OnSaveToIco();
    void OnClear();
    void OnModeHorz();
    void OnModeVert();
    void OnSplit();
	EVENT_MAP_BEGIN()
		EVENT_CHECK_SENDER_ROOT(m_pPageRoot)
		EVENT_NAME_COMMAND(L"btn_save", OnSave)
		EVENT_NAME_COMMAND(L"btn_clear", OnClear)
		EVENT_NAME_COMMAND(L"btn_save_ico",OnSaveToIco)
        EVENT_NAME_COMMAND(L"radio_horz", OnModeHorz)
        EVENT_NAME_COMMAND(L"radio_vert", OnModeVert)   
        EVENT_NAME_COMMAND(L"btn_split", OnSplit)     
    EVENT_MAP_BREAK()
    
    SOUI::SWindow *m_pPageRoot;
    SOUI::SImgCanvas *m_pImgCanvas;
};

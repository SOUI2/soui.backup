#pragma once
#include "droptarget.h"
#include "SFolderList.h"
#include "SEdit2.h"

class CFolderScanHandler : public IFileDropHandler
{
    friend class CMainDlg;
    friend class CDropTarget_Dir2;
public:
    CFolderScanHandler(void);
    ~CFolderScanHandler(void);
    
    void OnInit(SOUI::SWindow *pRoot);
protected:
    virtual void OnFileDropdown(HDROP hDrop);

	void OnGo(SOUI::EventArgs *pEvt);
	bool OnTreeDbclick(SOUI::EventArgs *pEvt);
	BOOL EnumFiles(SOUI::SStringT strPath, HSTREEITEM hParent);
    BOOL DoSomething();
	void InitDir(const SOUI::SStringT & strPath);

    EVENT_MAP_BEGIN()
        EVENT_CHECK_SENDER_ROOT(m_pPageRoot)
		EVENT_NAME_HANDLER(L"btn_go", SOUI::EventCmd::EventID, OnGo)
		EVENT_NAME_HANDLER(L"edit_dir", SOUI::EventKeyEnter::EventID, OnGo)
    EVENT_MAP_BREAK()

	SOUI::SWindow *m_pPageRoot;
	SOUI::SFolderTreeList *m_pTreelist;
};


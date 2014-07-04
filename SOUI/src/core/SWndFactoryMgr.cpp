#include "souistd.h"
#include "core/SWnd.h"
#include "core/SWndFactoryMgr.h"
#include "control/souictrls.h"

namespace SOUI
{

SWindowFactoryMgr::SWindowFactoryMgr(void)
{
    m_pFunOnKeyRemoved=OnWndFactoryRemoved;
    AddStandardWindowFactory();
}

void SWindowFactoryMgr::AddStandardWindowFactory()
{
    AddKeyObject(SWindow::GetClassName(),new TplDuiWindowFactory<SWindow>);
    AddKeyObject(SPanel::GetClassName(),new TplDuiWindowFactory<SPanel>);
    AddKeyObject(SStatic::GetClassName(),new TplDuiWindowFactory<SStatic>);
    AddKeyObject(SButton::GetClassName(),new TplDuiWindowFactory<SButton>);
    AddKeyObject(SImageWnd::GetClassName(),new TplDuiWindowFactory<SImageWnd>);
    AddKeyObject(SProgress::GetClassName(),new TplDuiWindowFactory<SProgress>);
    AddKeyObject(SImageButton::GetClassName(),new TplDuiWindowFactory<SImageButton>);
    AddKeyObject(SLine::GetClassName(),new TplDuiWindowFactory<SLine>);
    AddKeyObject(SCheckBox::GetClassName(),new TplDuiWindowFactory<SCheckBox>);
    AddKeyObject(SIconWnd::GetClassName(),new TplDuiWindowFactory<SIconWnd>);
    AddKeyObject(SRadioBox::GetClassName(),new TplDuiWindowFactory<SRadioBox>);
    AddKeyObject(SLink::GetClassName(),new TplDuiWindowFactory<SLink>);
    AddKeyObject(SGroup::GetClassName(),new TplDuiWindowFactory<SGroup>);
    AddKeyObject(SAnimateImgWnd::GetClassName(),new TplDuiWindowFactory<SAnimateImgWnd>);
    AddKeyObject(SScrollView::GetClassName(),new TplDuiWindowFactory<SScrollView>);
    AddKeyObject(SRealWnd::GetClassName(),new TplDuiWindowFactory<SRealWnd>);
    AddKeyObject(SToggle::GetClassName(),new TplDuiWindowFactory<SToggle>);
    AddKeyObject(SCaption::GetClassName(),new TplDuiWindowFactory<SCaption>);
    AddKeyObject(STabCtrl::GetClassName(),new TplDuiWindowFactory<STabCtrl>);
    AddKeyObject(SActiveX::GetClassName(),new TplDuiWindowFactory<SActiveX>);
    AddKeyObject(SFlashCtrl::GetClassName(),new TplDuiWindowFactory<SFlashCtrl>);
    AddKeyObject(SMediaPlayer::GetClassName(),new TplDuiWindowFactory<SMediaPlayer>);
    AddKeyObject(SSplitWnd::GetClassName(),new TplDuiWindowFactory<SSplitWnd>);
    AddKeyObject(SSplitWnd_Col::GetClassName(),new TplDuiWindowFactory<SSplitWnd_Col>);
    AddKeyObject(SSplitWnd_Row::GetClassName(),new TplDuiWindowFactory<SSplitWnd_Row>);
    AddKeyObject(SSliderBar::GetClassName(),new TplDuiWindowFactory<SSliderBar>);
    AddKeyObject(STreeCtrl::GetClassName(),new TplDuiWindowFactory<STreeCtrl>);
    AddKeyObject(SScrollBar::GetClassName(),new TplDuiWindowFactory<SScrollBar>);
    AddKeyObject(SHeaderCtrl::GetClassName(),new TplDuiWindowFactory<SHeaderCtrl>);
    AddKeyObject(SListCtrl::GetClassName(),new TplDuiWindowFactory<SListCtrl>);
    AddKeyObject(SListBox::GetClassName(),new TplDuiWindowFactory<SListBox>);
    AddKeyObject(SRichEdit::GetClassName(),new TplDuiWindowFactory<SRichEdit>);
    AddKeyObject(SEdit::GetClassName(),new TplDuiWindowFactory<SEdit>);
    AddKeyObject(SHotKeyCtrl::GetClassName(),new TplDuiWindowFactory<SHotKeyCtrl>);
    AddKeyObject(STreeBox::GetClassName(),new TplDuiWindowFactory<STreeBox>);
    AddKeyObject(SListBoxEx::GetClassName(),new TplDuiWindowFactory<SListBoxEx>);
    AddKeyObject(SComboBox::GetClassName(),new TplDuiWindowFactory<SComboBox>);
    AddKeyObject(SComboBoxEx::GetClassName(),new TplDuiWindowFactory<SComboBoxEx>);
    
    AddKeyObject(SCalendar::GetClassName(),new TplDuiWindowFactory<SCalendar>);
    AddKeyObject(SMaskEdit::GetClassName(),new TplDuiWindowFactory<SMaskEdit>);
    AddKeyObject(SDateEdit::GetClassName(),new TplDuiWindowFactory<SDateEdit>);
    AddKeyObject(STimeEdit::GetClassName(),new TplDuiWindowFactory<STimeEdit>);
    /*
    AddKeyObject(CDuiItemBox::GetClassName(),new TplDuiWindowFactory<CDuiItemBox>);
    //*/
}

void SWindowFactoryMgr::OnWndFactoryRemoved( const SWindowFactoryPtr & obj )
{
    delete obj;
}

SWindow * SWindowFactoryMgr::CreateWindowByName( LPCWSTR pszClassName )
{
    if(!HasKey(pszClassName))
    {
        DuiTraceW(L"CreateWindowByName,Warning: no window type:%s in DuiSystem!!",pszClassName);
        return NULL;
    }
    return GetKeyObject(pszClassName)->NewWindow();
}

LPCWSTR SWindowFactoryMgr::BaseClassNameFromClassName( LPCWSTR pszClassName )
{
    if(!HasKey(pszClassName))
    {
        DuiTraceW(L"BaseClassNameFromClassName, Warning: no window type:%s in DuiSystem!!",pszClassName);
        return NULL;
    }
    LPCWSTR pszBaseClassName=GetKeyObject(pszClassName)->DuiWindowBaseName();
    if(!pszBaseClassName) return NULL;
    //注意，baseClassName可能与ClassName相同，为了避免死循环，这里需要判断一下。
    if(wcscmp(pszBaseClassName,pszClassName)==0) return NULL;
    return pszBaseClassName;
}
}//namesspace SOUI
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
    AddKeyObject(SWindow::GetClassName(),new TplSWindowFactory<SWindow>);
    AddKeyObject(SPanel::GetClassName(),new TplSWindowFactory<SPanel>);
    AddKeyObject(SStatic::GetClassName(),new TplSWindowFactory<SStatic>);
    AddKeyObject(SButton::GetClassName(),new TplSWindowFactory<SButton>);
    AddKeyObject(SImageWnd::GetClassName(),new TplSWindowFactory<SImageWnd>);
    AddKeyObject(SProgress::GetClassName(),new TplSWindowFactory<SProgress>);
    AddKeyObject(SImageButton::GetClassName(),new TplSWindowFactory<SImageButton>);
    AddKeyObject(SLine::GetClassName(),new TplSWindowFactory<SLine>);
    AddKeyObject(SCheckBox::GetClassName(),new TplSWindowFactory<SCheckBox>);
    AddKeyObject(SIconWnd::GetClassName(),new TplSWindowFactory<SIconWnd>);
    AddKeyObject(SRadioBox::GetClassName(),new TplSWindowFactory<SRadioBox>);
    AddKeyObject(SLink::GetClassName(),new TplSWindowFactory<SLink>);
    AddKeyObject(SGroup::GetClassName(),new TplSWindowFactory<SGroup>);
    AddKeyObject(SAnimateImgWnd::GetClassName(),new TplSWindowFactory<SAnimateImgWnd>);
    AddKeyObject(SScrollView::GetClassName(),new TplSWindowFactory<SScrollView>);
    AddKeyObject(SRealWnd::GetClassName(),new TplSWindowFactory<SRealWnd>);
    AddKeyObject(SToggle::GetClassName(),new TplSWindowFactory<SToggle>);
    AddKeyObject(SCaption::GetClassName(),new TplSWindowFactory<SCaption>);
    AddKeyObject(STabCtrl::GetClassName(),new TplSWindowFactory<STabCtrl>);
    AddKeyObject(SActiveX::GetClassName(),new TplSWindowFactory<SActiveX>);
    AddKeyObject(SFlashCtrl::GetClassName(),new TplSWindowFactory<SFlashCtrl>);
    AddKeyObject(SMediaPlayer::GetClassName(),new TplSWindowFactory<SMediaPlayer>);
    AddKeyObject(SSplitWnd::GetClassName(),new TplSWindowFactory<SSplitWnd>);
    AddKeyObject(SSplitWnd_Col::GetClassName(),new TplSWindowFactory<SSplitWnd_Col>);
    AddKeyObject(SSplitWnd_Row::GetClassName(),new TplSWindowFactory<SSplitWnd_Row>);
    AddKeyObject(SSliderBar::GetClassName(),new TplSWindowFactory<SSliderBar>);
    AddKeyObject(STreeCtrl::GetClassName(),new TplSWindowFactory<STreeCtrl>);
    AddKeyObject(SScrollBar::GetClassName(),new TplSWindowFactory<SScrollBar>);
    AddKeyObject(SHeaderCtrl::GetClassName(),new TplSWindowFactory<SHeaderCtrl>);
    AddKeyObject(SListCtrl::GetClassName(),new TplSWindowFactory<SListCtrl>);
    AddKeyObject(SListBox::GetClassName(),new TplSWindowFactory<SListBox>);
    AddKeyObject(SRichEdit::GetClassName(),new TplSWindowFactory<SRichEdit>);
    AddKeyObject(SEdit::GetClassName(),new TplSWindowFactory<SEdit>);
    AddKeyObject(SHotKeyCtrl::GetClassName(),new TplSWindowFactory<SHotKeyCtrl>);
    AddKeyObject(STreeBox::GetClassName(),new TplSWindowFactory<STreeBox>);
    AddKeyObject(SListBoxEx::GetClassName(),new TplSWindowFactory<SListBoxEx>);
    AddKeyObject(SComboBox::GetClassName(),new TplSWindowFactory<SComboBox>);
    AddKeyObject(SComboBoxEx::GetClassName(),new TplSWindowFactory<SComboBoxEx>);
    
    AddKeyObject(SCalendar::GetClassName(),new TplSWindowFactory<SCalendar>);
}

void SWindowFactoryMgr::OnWndFactoryRemoved( const SWindowFactoryPtr & obj )
{
    delete obj;
}

SWindow * SWindowFactoryMgr::CreateWindowByName( LPCWSTR pszClassName )
{
    if(!HasKey(pszClassName))
    {
        STraceW(L"Warning: no window type:%s in SOUI!!",pszClassName);
        return NULL;
    }
    return GetKeyObject(pszClassName)->NewWindow();
}

LPCWSTR SWindowFactoryMgr::BaseClassNameFromClassName( LPCWSTR pszClassName )
{
    if(!HasKey(pszClassName))
    {
        STraceW(L"BaseClassNameFromClassName, Warning: no window type:%s in SOUI!!",pszClassName);
        return NULL;
    }
    LPCWSTR pszBaseClassName=GetKeyObject(pszClassName)->SWindowBaseName();
    if(!pszBaseClassName) return NULL;
    //注意，baseClassName可能与ClassName相同，为了避免死循环，这里需要判断一下。
    if(wcscmp(pszBaseClassName,pszClassName)==0) return NULL;
    return pszBaseClassName;
}
}//namesspace SOUI
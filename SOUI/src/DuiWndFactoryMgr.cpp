#include "duistd.h"
#include "duiwnd.h"
#include "DuiWndFactoryMgr.h"
#include "duictrls.h"

namespace SOUI
{

DuiWindowFactoryMgr::DuiWindowFactoryMgr(void)
{
    m_pFunOnKeyRemoved=OnWndFactoryRemoved;
    AddStandardWindowFactory();
}

void DuiWindowFactoryMgr::AddStandardWindowFactory()
{
    AddKeyObject(CDuiWindow::GetClassName(),new TplDuiWindowFactory<CDuiWindow>);
    AddKeyObject(CDuiPanel::GetClassName(),new TplDuiWindowFactory<CDuiPanel>);
    AddKeyObject(CDuiScrollView::GetClassName(),new TplDuiWindowFactory<CDuiScrollView>);
    AddKeyObject(CDuiStatic::GetClassName(),new TplDuiWindowFactory<CDuiStatic>);
    AddKeyObject(CDuiButton::GetClassName(),new TplDuiWindowFactory<CDuiButton>);
    AddKeyObject(CDuiSpacing::GetClassName(),new TplDuiWindowFactory<CDuiSpacing>);
    AddKeyObject(CDuiImageWnd::GetClassName(),new TplDuiWindowFactory<CDuiImageWnd>);
    AddKeyObject(CDuiProgress::GetClassName(),new TplDuiWindowFactory<CDuiProgress>);
    AddKeyObject(CDuiImageBtnWnd::GetClassName(),new TplDuiWindowFactory<CDuiImageBtnWnd>);
    AddKeyObject(CDuiRealWnd::GetClassName(),new TplDuiWindowFactory<CDuiRealWnd>);
    AddKeyObject(CDuiLine::GetClassName(),new TplDuiWindowFactory<CDuiLine>);
    AddKeyObject(CDuiCheckBox::GetClassName(),new TplDuiWindowFactory<CDuiCheckBox>);
    AddKeyObject(CDuiIconWnd::GetClassName(),new TplDuiWindowFactory<CDuiIconWnd>);
    AddKeyObject(CDuiTabCtrl::GetClassName(),new TplDuiWindowFactory<CDuiTabCtrl>);
    AddKeyObject(CDuiRadioBox::GetClassName(),new TplDuiWindowFactory<CDuiRadioBox>);
    AddKeyObject(CDuiLink::GetClassName(),new TplDuiWindowFactory<CDuiLink>);
    AddKeyObject(CDuiGroup::GetClassName(),new TplDuiWindowFactory<CDuiGroup>);
    AddKeyObject(CDuiAnimateImgWnd::GetClassName(),new TplDuiWindowFactory<CDuiAnimateImgWnd>);
    AddKeyObject(CDuiScrollBar::GetClassName(),new TplDuiWindowFactory<CDuiScrollBar>);
    AddKeyObject(CDuiListBox::GetClassName(),new TplDuiWindowFactory<CDuiListBox>);
    AddKeyObject(CDuiListBoxEx::GetClassName(),new TplDuiWindowFactory<CDuiListBoxEx>);
    AddKeyObject(CDuiItemBox::GetClassName(),new TplDuiWindowFactory<CDuiItemBox>);
    AddKeyObject(CDuiToggle::GetClassName(),new TplDuiWindowFactory<CDuiToggle>);
    AddKeyObject(CDuiTreeBox::GetClassName(),new TplDuiWindowFactory<CDuiTreeBox>);
    AddKeyObject(CDuiTreeCtrl::GetClassName(),new TplDuiWindowFactory<CDuiTreeCtrl>);
    AddKeyObject(CDuiRichEdit::GetClassName(),new TplDuiWindowFactory<CDuiRichEdit>);
    AddKeyObject(CDuiEdit::GetClassName(),new TplDuiWindowFactory<CDuiEdit>);
    AddKeyObject(CDuiSplitWnd::GetClassName(),new TplDuiWindowFactory<CDuiSplitWnd>);
    AddKeyObject(CDuiSplitWnd_Col::GetClassName(),new TplDuiWindowFactory<CDuiSplitWnd_Col>);
    AddKeyObject(CDuiSplitWnd_Row::GetClassName(),new TplDuiWindowFactory<CDuiSplitWnd_Row>);
    AddKeyObject(CDuiComboBox::GetClassName(),new TplDuiWindowFactory<CDuiComboBox>);
    AddKeyObject(CDuiComboBoxEx::GetClassName(),new TplDuiWindowFactory<CDuiComboBoxEx>);
    AddKeyObject(CDuiHotKeyCtrl::GetClassName(),new TplDuiWindowFactory<CDuiHotKeyCtrl>);
    AddKeyObject(CDuiCaption::GetClassName(),new TplDuiWindowFactory<CDuiCaption>);
    AddKeyObject(CDuiSliderBar::GetClassName(),new TplDuiWindowFactory<CDuiSliderBar>);
     AddKeyObject(CDuiActiveX::GetClassName(),new TplDuiWindowFactory<CDuiActiveX>);
     AddKeyObject(CDuiFlashCtrl::GetClassName(),new TplDuiWindowFactory<CDuiFlashCtrl>);
     AddKeyObject(CDuiMediaPlayer::GetClassName(),new TplDuiWindowFactory<CDuiMediaPlayer>);
    AddKeyObject(CDuiMaskEdit::GetClassName(),new TplDuiWindowFactory<CDuiMaskEdit>);
    AddKeyObject(CDuiDateEdit::GetClassName(),new TplDuiWindowFactory<CDuiDateEdit>);
    AddKeyObject(CDuiTimeEdit::GetClassName(),new TplDuiWindowFactory<CDuiTimeEdit>);
    AddKeyObject(CDuiCalendar::GetClassName(),new TplDuiWindowFactory<CDuiCalendar>);
    AddKeyObject(CDuiHeaderCtrl::GetClassName(),new TplDuiWindowFactory<CDuiHeaderCtrl>);
    AddKeyObject(CDuiListCtrl::GetClassName(),new TplDuiWindowFactory<CDuiListCtrl>);
}

void DuiWindowFactoryMgr::OnWndFactoryRemoved( const CDuiWindowFactoryPtr & obj )
{
    delete obj;
}

CDuiWindow * DuiWindowFactoryMgr::CreateWindowByName( LPCSTR pszClassName )
{
    if(!HasKey(pszClassName))
    {
        DuiTraceA("CreateWindowByName,Warning: no window type:%s in DuiSystem!!",pszClassName);
        return NULL;
    }
    return GetKeyObject(pszClassName)->NewWindow();
}

LPCSTR DuiWindowFactoryMgr::BaseClassNameFromClassName( LPCSTR pszClassName )
{
    if(!HasKey(pszClassName))
    {
        DuiTraceA("BaseClassNameFromClassName, Warning: no window type:%s in DuiSystem!!",pszClassName);
        return NULL;
    }
    LPCSTR pszBaseClassName=GetKeyObject(pszClassName)->DuiWindowBaseName();
    if(!pszBaseClassName) return NULL;
    //注意，baseClassName可能与ClassName相同，为了避免死循环，这里需要判断一下。
    if(strcmp(pszBaseClassName,pszClassName)==0) return NULL;
    return pszBaseClassName;
}
}//namesspace SOUI
#include "duistd.h"
#include "auto_reset.h"
#include "FocusManager.h"
#include "DuiWindowMgr.h"

# pragma warning(disable:4800)

namespace SOUI
{

    FocusSearch::FocusSearch( SWindow * root, bool cycle )
        : root_(root),cycle_(cycle)
    {

    }

    SWindow* FocusSearch::FindNextFocusableView( SWindow* starting_view, bool reverse, bool check_starting_view )
    {
        if(root_->GetChildrenCount()==0) return NULL;

        if(!starting_view)
        {
            if(reverse)
            {
                starting_view=root_->GetWindow(GDUI_LASTCHILD);
                while(starting_view->GetChildrenCount()) starting_view=starting_view->GetWindow(GDUI_LASTCHILD);
            }else
            {
                starting_view=root_->GetWindow(GDUI_FIRSTCHILD);
            }
            check_starting_view=true;
        }

        SWindow *pRet=NULL;
        SWindow *pStartGroupOwner=NULL;
        if(starting_view && starting_view->IsSiblingsAutoGroupped()) 
            pStartGroupOwner=starting_view->GetParent();

        if(reverse)
        {
            bool can_go_down=!IsFocusable(starting_view);
            pRet=FindPreviousFocusableViewImpl(starting_view,check_starting_view,true,can_go_down,pStartGroupOwner);
        }
        else
        {
            pRet= FindNextFocusableViewImpl(starting_view,check_starting_view,true,true,pStartGroupOwner);
        }
        if(!pRet && cycle_ && starting_view)
        {
            cycle_=false;
            pRet=FindNextFocusableView(NULL,reverse,false);
        }
        return pRet;
    }

    // Strategy for finding the next focusable view:
    // - keep going down the first child, stop when you find a focusable view or
    //   a focus traversable view (in that case return it) or when you reach a view
    //   with no children.
    // - go to the right sibling and start the search from there (by invoking
    //   FindNextFocusableViewImpl on that view).
    // - if the view has no right sibling, go up the parents until you find a parent
    //   with a right sibling and start the search from there.
    SWindow* FocusSearch::FindNextFocusableViewImpl( SWindow* starting_view, bool check_starting_view, bool can_go_up, bool can_go_down , SWindow * pSkipGroupOwner)
    {
        if(check_starting_view)
        {
            if(IsViewFocusableCandidate(starting_view, pSkipGroupOwner))
            {
                SWindow* v = FindSelectedViewForGroup(starting_view);
                // The selected view might not be focusable (if it is disabled for
                // example).
                if(IsFocusable(v))
                {
                    return v;
                }
            }

        }

        // First let's try the left child.
        if(can_go_down)
        {
            SWindow *pChild=starting_view->GetWindow(GDUI_FIRSTCHILD);
            if(pChild)
            {
                SWindow* v = FindNextFocusableViewImpl(
                    pChild,
                    true, false, true, pSkipGroupOwner);
                if(v )
                {
                    return v;
                }
            }
        }

        // Then try the right sibling.
        SWindow* sibling = starting_view->GetWindow(GDUI_NEXTSIBLING);
        if(sibling)
        {
            SWindow* v = FindNextFocusableViewImpl(sibling,
                true, false, true, pSkipGroupOwner);
            if(v )
            {
                return v;
            }
        }

        // Then go up to the parent sibling.
        if(can_go_up)
        {
            SWindow* parent = starting_view->GetParent();
            while(parent)
            {
                sibling = parent->GetWindow(GDUI_NEXTSIBLING);
                if(sibling)
                {
                    return FindNextFocusableViewImpl(sibling,
                        true, true, true,
                        pSkipGroupOwner);
                }
                parent = parent->GetParent();
            }
        }

        // We found nothing.
        return NULL;
    }

    // Strategy for finding the previous focusable view:
    // - keep going down on the right until you reach a view with no children, if it
    //   it is a good candidate return it.
    // - start the search on the left sibling.
    // - if there are no left sibling, start the search on the parent (without going
    //   down).
    SWindow* FocusSearch::FindPreviousFocusableViewImpl( SWindow* starting_view, bool check_starting_view, bool can_go_up, bool can_go_down, SWindow * pSkipGroupOwner )
    {
        if(can_go_down)
        {//find the last focusable window
            SWindow *pChild=starting_view->GetWindow(GDUI_LASTCHILD);
            if(pChild)
            {
                SWindow *pRet=FindPreviousFocusableViewImpl(pChild,true,false,true,pSkipGroupOwner);
                if(pRet) return pRet;
            }
        }

        if(check_starting_view && IsViewFocusableCandidate(starting_view,pSkipGroupOwner))
        {
            SWindow* v = FindSelectedViewForGroup(starting_view);
            // The selected view might not be focusable (if it is disabled for example).
            if(IsFocusable(v))
            {
                return v;
            }
        }

        SWindow *pPrevSibling=starting_view->GetWindow(GDUI_PREVSIBLING);
        if(pPrevSibling)
        {
            return FindPreviousFocusableViewImpl(pPrevSibling,true,true,true,pSkipGroupOwner);
        }
        if(can_go_up)
        {
            SWindow *pParent=starting_view->GetWindow(GDUI_PARENT);
            if(pParent) return FindPreviousFocusableViewImpl(pParent,true,true,false,pSkipGroupOwner);
        }

        return NULL;
    }

    bool FocusSearch::IsViewFocusableCandidate( SWindow* v,SWindow *pGroupOwner )
    {
        if(! IsFocusable(v) ) return false;
        if(pGroupOwner && v->IsSiblingsAutoGroupped() && v->GetParent()==pGroupOwner) return false;
        return true;
    }

    bool FocusSearch::IsFocusable( SWindow* view )
    {
        if(!view) return false;
        return view->IsTabStop() && view->IsVisible(TRUE) && !view->IsDisabled(TRUE);
    }

    SWindow* FocusSearch::FindSelectedViewForGroup( SWindow* view )
    {
        if(!view->IsSiblingsAutoGroupped()) return view;
        if(view->IsChecked()) return view;
        SWindow *pParent=view->GetParent();
        SWindow *pSibling=pParent->GetWindow(GDUI_FIRSTCHILD);
        while(pSibling)
        {
            if(pSibling->IsSiblingsAutoGroupped())
            {
                if(pSibling->IsChecked()) return pSibling;
            }
            pSibling=pSibling->GetWindow(GDUI_NEXTSIBLING);
        }
        return view;
    }

    //////////////////////////////////////////////////////////////////////////
    CFocusManager::CFocusManager(SWindow *pOwner):m_pOwner(pOwner)
    {
    }

    CFocusManager::~CFocusManager(void)
    {
    }

    BOOL CFocusManager::IsTabTraversalKey( UINT vKey )
    {
        if(vKey!=VK_TAB) return FALSE;
        SWindow *pFocus=DuiWindowMgr::GetWindow(focused_view_);
        if(pFocus && pFocus->OnGetDlgCode()&DUIC_WANTTAB) return FALSE;
        if(GetKeyState(VK_CONTROL)&0x8000) return FALSE;
        else return TRUE;
    }

    BOOL CFocusManager::OnKeyDown(UINT vKey)
    {
        //tab traversal
        if(IsTabTraversalKey(vKey))
        {
            AdvanceFocus(GetKeyState(VK_SHIFT)&0x8000);
            return TRUE;
        }

        // Intercept arrow key messages to switch between grouped views.
        SWindow *pFocusWnd=DuiWindowMgr::GetWindow(focused_view_);
        if(pFocusWnd && pFocusWnd->IsSiblingsAutoGroupped() && (vKey==VK_LEFT || vKey==VK_RIGHT || vKey==VK_UP || vKey==VK_DOWN))
        {
            UINT ucode= (vKey == VK_RIGHT || vKey == VK_DOWN)?GDUI_NEXTSIBLING:GDUI_PREVSIBLING;
            SWindow *pNext=pFocusWnd->GetWindow(ucode);
            while(pNext)
            {
                if(pNext->IsSiblingsAutoGroupped())
                {
                    SetFocusedHwndWithReason(pNext->GetSwnd(),kReasonFocusTraversal);
                    break;
                }
                pNext=pNext->GetWindow(ucode);
            }
            if(!pNext)
            {
                pNext=pFocusWnd->GetParent()->GetWindow(ucode==GDUI_NEXTSIBLING? GDUI_FIRSTCHILD : GDUI_LASTCHILD);
                while(pNext)
                {
                    if(pNext->IsSiblingsAutoGroupped())
                    {
                        SetFocusedHwndWithReason(pNext->GetSwnd(),kReasonFocusTraversal);
                        break;
                    }
                    pNext=pNext->GetWindow(ucode);
                }
            }
            return TRUE;
        }
        // Process keyboard accelerators.
        CAccelerator accelerator(vKey,GetKeyState(VK_CONTROL)&0x8000,GetKeyState(VK_MENU)&0x8000,GetKeyState(VK_SHIFT)&0x8000);
        if(ProcessAccelerator(accelerator))
        {
            // If a shortcut was activated for this keydown message, do not propagate
            // the event further.
            return TRUE;
        }

        return FALSE;
    }

    void CFocusManager::AdvanceFocus( bool reverse )
    {
        // Let's revalidate the focused view.
        ValidateFocusedView();
        SWindow *pFocus=DuiWindowMgr::GetWindow(focused_view_);
        SWindow *pDuiWnd=GetNextFocusableView(pFocus,reverse,true);
        if(pDuiWnd)
        {
            SetFocusedHwndWithReason(pDuiWnd->GetSwnd(),kReasonFocusTraversal);
        }
    }

    SWindow * CFocusManager::GetNextFocusableView( SWindow* original_starting_view, bool bReverse, bool bLoop )
    {
        
        FocusSearch fs(m_pOwner,bLoop);
        return fs.FindNextFocusableView(original_starting_view,bReverse,false);
    }

    void CFocusManager::SetFocusedHwndWithReason( SWND hDuiWnd, FocusChangeReason reason )
    {
        if(hDuiWnd == focused_view_)
        {
            return;
        }
        
        AutoReset<bool> auto_changing_focus(&is_changing_focus_, true);
        // Update the reason for the focus change (since this is checked by
        // some listeners), then notify all listeners.
        focus_change_reason_ = reason;
        SWindow *pOldFocus=DuiWindowMgr::GetWindow(focused_view_);
        SWindow *pNewFocus=DuiWindowMgr::GetWindow(hDuiWnd);
        if(pOldFocus)
        {
            pOldFocus->SendMessage(WM_KILLFOCUS,(WPARAM)pNewFocus);
        }
        if(pNewFocus && !pNewFocus->IsDisabled(TRUE))
        {
            pNewFocus->SendMessage(WM_SETFOCUS,(WPARAM)pOldFocus);
            focused_view_ = hDuiWnd;
        }else
        {
            focused_view_=0;
        }
    }

    void CFocusManager::ValidateFocusedView()
    {
        if(focused_view_)
        {
            SWindow *pFocus=DuiWindowMgr::GetWindow(focused_view_);
            if(pFocus)
            {
                pFocus=pFocus->GetTopLevelParent();
                if(pFocus!=m_pOwner)
                {
                    focused_view_=0;
                }
            }else
            {
                focused_view_=0;
            }
        }
    }

    void CFocusManager::ClearFocus()
    {
        SetFocusedHwnd(NULL);
    }

    void CFocusManager::StoreFocusedView()
    {
        ValidateFocusedView();
        focused_backup_ = focused_view_;
        focused_view_=0;
        SWindow *pWnd=DuiWindowMgr::GetWindow(focused_backup_);

        if(pWnd)
        {
            pWnd->SendMessage(WM_KILLFOCUS);
        }
    }

    void CFocusManager::RestoreFocusedView()
    {
        SWindow *pWnd=DuiWindowMgr::GetWindow(focused_backup_);
        if(pWnd && !pWnd->IsDisabled(TRUE))
        {
            focused_view_=focused_backup_;
            pWnd->SendMessage(WM_SETFOCUS);
        }
        focused_backup_=0;
    }

    void CFocusManager::RegisterAccelerator( const CAccelerator& accelerator, IAcceleratorTarget* target )
    {
        AcceleratorTargetList& targets = accelerators_[accelerator];
        targets.AddHead(target);
    }

    void CFocusManager::UnregisterAccelerator( const CAccelerator& accelerator, IAcceleratorTarget* target )
    {
        if(!accelerators_.Lookup(accelerator)) return;
        AcceleratorTargetList* targets=&accelerators_[accelerator];
        POSITION pos = targets->Find(target);
        if(pos) targets->RemoveAt(pos);
    }

    void CFocusManager::UnregisterAccelerators( IAcceleratorTarget* target )
    {
        POSITION pos=accelerators_.GetStartPosition();
        while(pos)
        {
            AcceleratorTargetList & targets=accelerators_.GetValueAt(pos);
            POSITION pos2=targets.Find(target);
            if(pos2) targets.RemoveAt(pos2);
            accelerators_.GetNext(pos);
        }
    }

    bool CFocusManager::ProcessAccelerator( const CAccelerator& accelerator )
    {
        if(!accelerators_.Lookup(accelerator)) return false;

        // We have to copy the target list here, because an AcceleratorPressed
        // event handler may modify the list.
        AcceleratorTargetList targets;
        CopyDuiList(accelerators_[accelerator],targets);
        
        POSITION pos=targets.GetHeadPosition();

        while(pos)
        {
            IAcceleratorTarget *pTarget=targets.GetNext(pos);
            if(pTarget->OnAcceleratorPressed(accelerator))
            {
                return true;
            }
        }
        return false;
    }
}//end of namespace SOUI

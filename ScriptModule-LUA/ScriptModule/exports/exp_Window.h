#include <core/swnd.h>

BOOL ExpLua_Window(lua_State *L)
{
	try{
		lua_tinker::class_add<SWindow>(L,"SWindow");
		lua_tinker::class_con<SWindow>(L,lua_tinker::constructor<SWindow>);
		lua_tinker::class_def<SWindow>(L,"GetParent",&SWindow::GetParent);
		lua_tinker::class_def<SWindow>(L,"SetParent",&SWindow::SetParent);
		lua_tinker::class_def<SWindow>(L,"GetTopLevelParent",&SWindow::GetTopLevelParent);
		lua_tinker::class_def<SWindow>(L,"DestroyChild",&SWindow::DestroyChild);
		lua_tinker::class_def<SWindow>(L,"GetChildrenCount",&SWindow::GetChildrenCount);
		lua_tinker::class_def<SWindow>(L,"GetChild",&SWindow::GetChild);
		lua_tinker::class_def<SWindow>(L,"FindChildByID",&SWindow::FindChildByID);
		lua_tinker::class_def<SWindow>(L,"FindChildByName",&SWindow::FindChildByName);
 		lua_tinker::class_def<SWindow>(L,"CreateChildrenFromString",(SWindow* (SWindow::*)(LPCWSTR))&SWindow::CreateChildren);
		lua_tinker::class_def<SWindow>(L,"GetTextAlign",&SWindow::GetTextAlign);
		lua_tinker::class_def<SWindow>(L,"GetRect",&SWindow::GetRect);
		lua_tinker::class_def<SWindow>(L,"GetClient",&SWindow::GetClient);
		lua_tinker::class_def<SWindow>(L,"GetWindowText",&SWindow::GetWindowText);
		lua_tinker::class_def<SWindow>(L,"SetWindowText",&SWindow::SetWindowText);
		lua_tinker::class_def<SWindow>(L,"SendSwndMessage",&SWindow::SendSwndMessage);
		lua_tinker::class_def<SWindow>(L,"GetID",&SWindow::GetID);
		lua_tinker::class_def<SWindow>(L,"SetID",&SWindow::SetID);
		lua_tinker::class_def<SWindow>(L,"GetUserData",&SWindow::GetUserData);
		lua_tinker::class_def<SWindow>(L,"SetUserData",&SWindow::SetUserData);
		lua_tinker::class_def<SWindow>(L,"GetName",&SWindow::GetName);
		lua_tinker::class_def<SWindow>(L,"GetSwnd",&SWindow::GetSwnd);
		lua_tinker::class_def<SWindow>(L,"InsertChild",&SWindow::InsertChild);
		lua_tinker::class_def<SWindow>(L,"RemoveChild",&SWindow::RemoveChild);
		lua_tinker::class_def<SWindow>(L,"IsChecked",&SWindow::IsChecked);
		lua_tinker::class_def<SWindow>(L,"IsDisabled",&SWindow::IsDisabled);
		lua_tinker::class_def<SWindow>(L,"IsVisible",&SWindow::IsVisible);
		lua_tinker::class_def<SWindow>(L,"SetVisible",&SWindow::SetVisible);
		lua_tinker::class_def<SWindow>(L,"EnableWindow",&SWindow::EnableWindow);
		lua_tinker::class_def<SWindow>(L,"SetCheck",&SWindow::SetCheck);
		lua_tinker::class_def<SWindow>(L,"GetLinkUrl",&SWindow::GetLinkUrl);
		lua_tinker::class_def<SWindow>(L,"SetOwner",&SWindow::SetOwner);
		lua_tinker::class_def<SWindow>(L,"GetOwner",&SWindow::GetOwner);
		lua_tinker::class_def<SWindow>(L,"Invalidate",&SWindow::Invalidate);
        lua_tinker::class_def<SWindow>(L,"InvalidateRect",(void (SWindow::*)(LPRECT))&SWindow::InvalidateRect);
		lua_tinker::class_def<SWindow>(L,"AnimateWindow",&SWindow::AnimateWindow);

		return TRUE;
	}catch(...)
	{
		return FALSE;
	}
}
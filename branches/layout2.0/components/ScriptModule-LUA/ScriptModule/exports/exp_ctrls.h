#include <control/souictrls.h>


SComboBoxBase * toSComboboxBase(SObject *pObj)
{
    return sobj_cast<SComboBoxBase>(pObj);
}

SComboBox * toSCombobox(SObject *pObj)
{
    return sobj_cast<SComboBox>(pObj);
}

SComboBoxEx * toSComboboxEx(SObject *pObj)
{
    return sobj_cast<SComboBoxEx>(pObj);
}


BOOL ExpLua_Ctrls(lua_State *L)
{
	try{
		lua_tinker::class_add<SComboBoxBase>(L,"SComboBoxBase");
		lua_tinker::class_inh<SComboBoxBase,SWindow>(L);
//         lua_tinker::class_def<SComboBoxBase>(L,"toSWindow",&SComboBoxBase::toSWindow);
		lua_tinker::class_def<SComboBoxBase>(L,"GetCurSel",&SComboBoxBase::GetCurSel);
		lua_tinker::class_def<SComboBoxBase>(L,"GetCount",&SComboBoxBase::GetCount);
		lua_tinker::class_def<SComboBoxBase>(L,"SetCurSel",&SComboBoxBase::SetCurSel);
		lua_tinker::class_def<SComboBoxBase>(L,"GetLBText",&SComboBoxBase::GetLBText);
		lua_tinker::class_def<SComboBoxBase>(L,"FindString",&SComboBoxBase::FindString);
		lua_tinker::class_def<SComboBoxBase>(L,"DropDown",&SComboBoxBase::DropDown);
		lua_tinker::class_def<SComboBoxBase>(L,"CloseUp",&SComboBoxBase::CloseUp);
		lua_tinker::def(L,"toComboboxBase",toSComboboxBase);
		
        lua_tinker::class_add<SComboBox>(L,"SComboBox");
        lua_tinker::class_inh<SComboBox,SComboBoxBase>(L);
        lua_tinker::class_def<SComboBox>(L,"InsertItem",&SComboBox::InsertItem);
        lua_tinker::class_def<SComboBox>(L,"DeleteString",&SComboBox::DeleteString);
        lua_tinker::class_def<SComboBox>(L,"ResetContent",&SComboBox::ResetContent);
        lua_tinker::class_def<SComboBox>(L,"GetLBText",&SComboBox::GetLBText);
        lua_tinker::class_def<SComboBox>(L,"GetListBox",&SComboBox::GetListBox);
        lua_tinker::class_def<SComboBox>(L,"GetItemData",&SComboBox::GetItemData);
        lua_tinker::class_def<SComboBox>(L,"SetItemData",&SComboBox::SetItemData);
        lua_tinker::def(L,"toCombobox",toSCombobox);


        lua_tinker::class_add<SComboBoxEx>(L,"SComboBoxEx");
        lua_tinker::class_inh<SComboBoxEx,SComboBoxBase>(L);
        lua_tinker::class_def<SComboBoxEx>(L,"InsertItem",&SComboBoxEx::InsertItem);
        lua_tinker::class_def<SComboBoxEx>(L,"DeleteString",&SComboBoxEx::DeleteString);
        lua_tinker::class_def<SComboBoxEx>(L,"ResetContent",&SComboBoxEx::ResetContent);
        lua_tinker::class_def<SComboBoxEx>(L,"GetLBText",&SComboBoxEx::GetLBText);
        lua_tinker::class_def<SComboBoxEx>(L,"GetListBox",&SComboBoxEx::GetListBox);
        lua_tinker::class_def<SComboBoxEx>(L,"GetItemData",&SComboBoxEx::GetItemData);
        lua_tinker::class_def<SComboBoxEx>(L,"SetItemData",&SComboBoxEx::SetItemData);
        lua_tinker::def(L,"toComboboxEx",toSComboboxEx);
        
		return TRUE;
	}catch(...)
	{
		return FALSE;
	}
}
#include <core/sobject.h>

void SetObjAttr(SObject *pObj,LPCSTR pszAttr,LPCSTR pszValue)
{
    pObj->SetAttribute(pszAttr,pszValue,FALSE);
}

BOOL ExpLua_SObject(lua_State *L)
{
	try{
		lua_tinker::class_add<SObject>(L,"SObject");
		lua_tinker::class_def<SObject>(L,"IsClass",&SObject::IsClass);
        lua_tinker::class_def<SObject>(L,"GetObjectClass",&SObject::GetObjectClass);
        lua_tinker::class_def<SObject>(L,"InitFromXml",&SObject::InitFromXml);
        lua_tinker::class_def<SObject>(L,"SetAttributeA",(HRESULT (SObject::*)(const SStringA &, const SStringA &, BOOL))&SObject::SetAttribute);
        lua_tinker::class_def<SObject>(L,"SetAttributeW",(HRESULT (SObject::*)(const SStringW &, const SStringW &, BOOL))&SObject::SetAttribute);
        lua_tinker::class_def<SObject>(L,"tr",&SObject::tr);
        lua_tinker::class_def<SObject>(L,"GetID",&SObject::GetID);
        lua_tinker::class_def<SObject>(L,"GetName",&SObject::GetName);
        lua_tinker::def(L,"SetObjAttr",SetObjAttr);
#ifdef _DEBUG
        lua_tinker::class_mem<SObject>(L,"m_strXml",&SObject::m_strXml);
#endif
		return TRUE;
	}catch(...)
	{
		return FALSE;
	}
}
#include <interface/SResProvider-i.h>
#include <res.mgr/SResProvider.h>

BOOL ExpLua_ResProvider(lua_State *L)
{
	try{
		lua_tinker::class_add<IResProvider>(L,"IResProvider");
		lua_tinker::class_def<IResProvider>(L,"HasResource",&IResProvider::HasResource);
		lua_tinker::class_def<IResProvider>(L,"LoadIcon",&IResProvider::LoadIcon);
 		lua_tinker::class_def<IResProvider>(L,"LoadBitmap",&IResProvider::LoadBitmap);
 		lua_tinker::class_def<IResProvider>(L,"LoadImage",&IResProvider::LoadImage);
 		lua_tinker::class_def<IResProvider>(L,"GetRawBufferSize",&IResProvider::GetRawBufferSize);
 		lua_tinker::class_def<IResProvider>(L,"GetRawBuffer",&IResProvider::GetRawBuffer);
 
 		lua_tinker::class_add<SResProviderPE>(L,"SResProviderPE");
 		lua_tinker::class_inh<SResProviderPE,IResProvider>(L);
 		lua_tinker::class_con<SResProviderPE>(L,lua_tinker::constructor<SResProviderPE,HINSTANCE>);
 
 		lua_tinker::class_add<SResProviderFiles>(L,"DuiResProviderFiles");
 		lua_tinker::class_inh<SResProviderFiles,IResProvider>(L);
 		lua_tinker::class_con<SResProviderFiles>(L,lua_tinker::constructor<SResProviderFiles>);
 		lua_tinker::class_def<SResProviderFiles>(L,"Init",&SResProviderFiles::Init);
	
		return TRUE;
	}catch(...)
	{
		return FALSE;
	}
}
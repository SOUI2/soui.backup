include_directories(${PROJECT_SOURCE_DIR}/config)
include_directories(${PROJECT_SOURCE_DIR}/utilities/include)
include_directories(${PROJECT_SOURCE_DIR}/SOUI/include)
include_directories(${PROJECT_SOURCE_DIR}/third-part/lua-52/src)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)

set (lua_tinker_src
    
    )

set(
    ScriptModule-lua_src
    src/ScriptModule-Lua.cpp
    src/exports/exp_soui.cpp
    lua_tinker/lua_tinker.cpp
    )

if (NOT ENABLE_SOUI_COM_LIB)
    set (ScriptModule-Lua_src  ${ScriptModule-lua_src} ScriptModule-lua.rc)
    add_library(ScriptModule-Lua SHARED ${ScriptModule-lua_src})
    target_link_libraries(ScriptModule-Lua lua utilities soui)
    cotire(ScriptModule-Lua)
else()
    add_library(ScriptModule-Lua STATIC ${ScriptModule-lua_src})
    cotire(ScriptModule-Lua)
endif()
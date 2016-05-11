# 
# SOUI cmake配置文件
# 

#
# lijinggang@021.com
#

add_definitions(-DSOUI_EXPORTS -D_CRT_SECURE_NO_WARNINGS)

include_directories(${PROJECT_SOURCE_DIR}/config)
include_directories(${PROJECT_SOURCE_DIR}/utilities/include)
include_directories(include)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})



aux_source_directory(src         src)
aux_source_directory(src/core    core)
aux_source_directory(src/control control)
aux_source_directory(src/activex activex)
aux_source_directory(src/event   event)
aux_source_directory(src/helper  helper)
aux_source_directory(src/res.mgr res_mgr)
aux_source_directory(src/updatelayeredwindow updatelayeredwindow)

set(source ${src} ${core} ${control} ${activex} ${event} ${helper} ${res_mgr} ${updatelayeredwindow})

if (NOT ENABLE_SOUI_CORE_LIB)
    set (source  ${source} soui.rc)
    add_library(soui SHARED ${source})
    target_link_libraries(soui utilities)
else()
    add_library(soui STATIC ${source})
endif()
set_target_properties(soui PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "include/souistd.h")
cotire(soui)
# 
# SOUI cmake配置文件
#

include_directories(${PROJECT_SOURCE_DIR}/config)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(include)

set ( SRCS_MAIN DllMain.cpp soui-sys-resource.rc)
set ( HDRS_MAIN resource.h)
add_library (soui-sys-resource SHARED ${SRCS_MAIN} ${HDRS_MAIN} )

# add_library(soui-sys-resource SHARED soui-sys-resource.rc)
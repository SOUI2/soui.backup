# 
# SOUI cmake配置文件
#
include_directories(${PROJECT_SOURCE_DIR}/config)
include_directories(${PROJECT_SOURCE_DIR}/utilities/include)
include_directories(${PROJECT_SOURCE_DIR}/SOUI/include)

set(translator_src 
    dllmain.cpp
    translator.cpp)

if (NOT ENABLE_SOUI_COM_LIB)
    set (translator_src  ${translator_src} translator.rc)
    add_library(translator SHARED ${translator_src})
    target_link_libraries(translator utilities soui)
else()
    add_library(translator STATIC ${translator_src})
endif()

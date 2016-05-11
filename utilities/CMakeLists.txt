# 
# SOUI cmake配置文件
#

add_definitions(-DUTILITIES_EXPORTS -D_CRT_SECURE_NO_WARNINGS)

include_directories(${PROJECT_SOURCE_DIR}/config)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(include)

aux_source_directory(src         utilities_src)
aux_source_directory(src/pugixml utilities_pugixml)
aux_source_directory(src/string  utilities_string)

set(source ${utilities_src} ${utilities_pugixml} ${utilities_string})

if (NOT ENABLE_SOUI_CORE_LIB)
    set (source  ${source} utilities.rc)
    add_library(utilities SHARED ${source})
    target_link_libraries(utilities png zlib)
else()
    add_library(utilities STATIC ${source})
endif()

# add_library(utilities SHARED ${source})

set_target_properties(utilities PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")
cotire(utilities)

include_directories(${PROJECT_SOURCE_DIR}/config)
include_directories(${PROJECT_SOURCE_DIR}/utilities/include)
include_directories(${PROJECT_SOURCE_DIR}/SOUI/include)

add_definitions(-D_CRT_SECURE_NO_WARNINGS)

set (
    log4z_src
    log4z.cpp)

if (NOT ENABLE_SOUI_COM_LIB)
    set (log4z_src  ${log4z_src} log4z.rc)
    add_library(log4z SHARED ${log4z_src})
else()
    add_library(log4z STATIC ${log4z_src})
endif()
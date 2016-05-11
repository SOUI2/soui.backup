include_directories(${PROJECT_SOURCE_DIR}/config)
include_directories(${PROJECT_SOURCE_DIR}/utilities/include)
include_directories(${PROJECT_SOURCE_DIR}/SOUI/include)

set(imgdecoder-gdip_src 
    imgdecoder-gdip.cpp)

if (NOT ENABLE_SOUI_COM_LIB)
    set (imgdecoder-gdip_src  ${imgdecoder-gdip_src} imgdecoder-gdip.rc)
    add_library(imgdecoder-gdip SHARED ${imgdecoder-gdip_src})
else()
    add_library(imgdecoder-gdip STATIC ${imgdecoder-gdip_src})
endif()
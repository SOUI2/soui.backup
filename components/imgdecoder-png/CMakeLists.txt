# 
# SOUI cmake配置文件
#
include_directories(${PROJECT_SOURCE_DIR}/third-part/zlib)
include_directories(${PROJECT_SOURCE_DIR}/third-part/png)

include_directories(${PROJECT_SOURCE_DIR}/config)
include_directories(${PROJECT_SOURCE_DIR}/utilities/include)
include_directories(${PROJECT_SOURCE_DIR}/SOUI/include)


# imgdecoder-png

set(imgdecoder-png_src
    decoder-apng.cpp 
    imgdecoder-png.cpp
    imgdecoder-png.rc)
        
if (NOT ENABLE_SOUI_COM_LIB)
    set (imgdecoder-png_src  ${imgdecoder-png_src} imgdecoder-png.rc)
    add_library(imgdecoder-png SHARED ${imgdecoder-png_src})
    target_link_libraries(imgdecoder-png png zlib)
else()
    add_library(imgdecoder-png STATIC ${imgdecoder-png_src})
endif()

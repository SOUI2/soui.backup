include_directories(${PROJECT_SOURCE_DIR}/config)
include_directories(${PROJECT_SOURCE_DIR}/utilities/include)
include_directories(${PROJECT_SOURCE_DIR}/SOUI/include)


set(imgdecoder-stb_src
    imgdecoder-stb.cpp)

if (NOT ENABLE_SOUI_COM_LIB)
    set (imgdecoder-stb_src  ${imgdecoder-stb_src} imgdecoder-stb.rc)
    add_library(imgdecoder-stb SHARED ${imgdecoder-stb_src})
else()
    add_library(imgdecoder-stb STATIC ${imgdecoder-stb_src})
endif()
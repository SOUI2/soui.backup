include_directories(${PROJECT_SOURCE_DIR}/config)
include_directories(${PROJECT_SOURCE_DIR}/utilities/include)
include_directories(${PROJECT_SOURCE_DIR}/SOUI/include)


set(imgdecoder-wic_src
    imgdecoder-wic.cpp)

if (NOT ENABLE_SOUI_COM_LIB)
    set (imgdecoder-wic_src  ${imgdecoder-wic_src} imgdecoder-wic.rc)
    add_library(imgdecoder-wic SHARED ${imgdecoder-wic_src})
else()
    add_library(imgdecoder-wic STATIC ${imgdecoder-wic_src})
endif()
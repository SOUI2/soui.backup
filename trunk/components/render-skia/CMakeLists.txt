include_directories(${PROJECT_SOURCE_DIR}/config)
include_directories(${PROJECT_SOURCE_DIR}/utilities/include)
include_directories(${PROJECT_SOURCE_DIR}/SOUI/include)

include_directories(${PROJECT_SOURCE_DIR}/third-part/skia)
include_directories(${PROJECT_SOURCE_DIR}/third-part/skia/include)
include_directories(${PROJECT_SOURCE_DIR}/third-part/skia/include/config)
include_directories(${PROJECT_SOURCE_DIR}/third-part/skia/include/core)

set(render-skia_src 
    drawtext-skia.cpp
    render-skia.cpp
    render-skia.rc
    Render-Skia2.cpp
    skia2rop2.cpp)

if (NOT ENABLE_SOUI_COM_LIB)
    set (render-skia_src  ${render-skia_src} render-skia.rc)
    add_library(render-skia SHARED ${render-skia_src})
    target_link_libraries(render-skia Usp10 opengl32 skia utilities)
else()
    add_library(render-skia STATIC ${render-skia_src})
endif()
include_directories(${PROJECT_SOURCE_DIR}/config)
include_directories(${PROJECT_SOURCE_DIR}/utilities/include)
include_directories(${PROJECT_SOURCE_DIR}/SOUI/include)


set(render-gdi_src 
    GradientFillHelper.cpp
    render-gdi.cpp)

if (NOT ENABLE_SOUI_COM_LIB)
    set (render-gdi_src  ${render-gdi_src} render-gdi.rc)
    add_library(render-gdi SHARED ${render-gdi_src})
    target_link_libraries(render-gdi utilities)
else()
    add_library(render-gdi STATIC ${render-gdi_src})
endif()

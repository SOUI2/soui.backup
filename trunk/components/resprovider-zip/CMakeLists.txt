

include_directories(${PROJECT_SOURCE_DIR}/third-part/zlib)
include_directories(${PROJECT_SOURCE_DIR}/config)
include_directories(${PROJECT_SOURCE_DIR}/utilities/include)
include_directories(${PROJECT_SOURCE_DIR}/SOUI/include)

set(resprovider-zip_src 
    cursoricon.cpp
    SResProviderZip.cpp
    zipArchive.cpp)

if (NOT ENABLE_SOUI_COM_LIB)
    set (resprovider-zip_src  ${resprovider-zip_src} resprovider-zip.rc)
    add_library(resprovider-zip SHARED ${resprovider-zip_src})
    target_link_libraries(resprovider-zip utilities zlib)
else()
    add_library(resprovider-zip STATIC ${resprovider-zip_src})
endif()

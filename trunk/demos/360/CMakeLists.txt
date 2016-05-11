include_directories(${PROJECT_SOURCE_DIR}/config)
include_directories(${PROJECT_SOURCE_DIR}/components)
include_directories(${PROJECT_SOURCE_DIR}/utilities/include)
include_directories(${PROJECT_SOURCE_DIR}/SOUI/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR} srcs)

add_executable(360 WIN32 ${srcs} 360.rc)
set_target_properties(360 PROPERTIES DEBUG_POSTFIX d)
target_link_libraries(360 soui utilities)
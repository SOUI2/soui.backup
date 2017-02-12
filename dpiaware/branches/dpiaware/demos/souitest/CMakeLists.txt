include_directories(${PROJECT_SOURCE_DIR}/third-part/gtest/include)
include_directories(${PROJECT_SOURCE_DIR}/components)
include_directories(${PROJECT_SOURCE_DIR}/utilities/include)
include_directories(${PROJECT_SOURCE_DIR}/SOUI/include)

add_executable(souitest souitest.cpp)
set_target_properties(souitest PROPERTIES DEBUG_POSTFIX d)
target_link_libraries(souitest gtest)
add_dependencies(souitest gtest)
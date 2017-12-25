include_directories(${PROJECT_SOURCE_DIR}/third-part/gtest/include)
include_directories(${PROJECT_SOURCE_DIR}/components)
include_directories(${PROJECT_SOURCE_DIR}/utilities/include)
include_directories(${PROJECT_SOURCE_DIR}/SOUI/include)
include_directories(${PROJECT_SOURCE_DIR}/config)

file(GLOB_RECURSE CURRENT_HEADERS  *.h *.hpp)
file(GLOB_RECURSE CURRENT_SRCS  *.cpp)

source_group("Header Files" FILES ${CURRENT_HEADERS})
source_group("Source Files" FILES ${CURRENT_SRCS})

add_executable(souitest ${CURRENT_HEADERS} ${CURRENT_SRCS})

set_target_properties(souitest PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})
target_link_libraries(souitest gtest soui utilities ${COM_LIBS})
add_dependencies(souitest gtest)
set_target_properties (souitest PROPERTIES
    FOLDER demos
)
cotire(souitest)

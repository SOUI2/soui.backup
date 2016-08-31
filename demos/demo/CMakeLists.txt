include_directories(${PROJECT_SOURCE_DIR}/config)
include_directories(${PROJECT_SOURCE_DIR}/components)
include_directories(${PROJECT_SOURCE_DIR}/utilities/include)
include_directories(${PROJECT_SOURCE_DIR}/SOUI/include)
include_directories(${PROJECT_SOURCE_DIR}/third-part/wke/include)
include_directories(${PROJECT_SOURCE_DIR}/third-part/mhook/mhook-lib)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB_RECURSE EXTENDCOTROLS ${CMAKE_SOURCE_DIR}/controls.extend/*.cpp ${CMAKE_SOURCE_DIR}/controls.extend/*.h)
file(GLOB_RECURSE CURRENT_HEADERS *.h *.hpp)
file(GLOB_RECURSE CURRENT_SRCS  *.cpp)
file(GLOB_RECURSE CURRENT_SOUIS  *.xml *.idx *.ico)
file(GLOB_RECURSE CURRENT_RC *.rc *.rc2)

source_group("Header Files" FILES ${CURRENT_HEADERS})
source_group("Source Files" FILES ${CURRENT_SRCS})
source_group("SoUI Resouece" FILES ${CURRENT_SOUIS})
source_group("Resource Files" FILES ${CURRENT_RC})
source_group("Extend Cotrols" FILES ${EXTENDCOTROLS})

add_executable(demo WIN32 ${CURRENT_HEADERS} ${CURRENT_SRCS} ${CURRENT_SOUIS} ${CURRENT_RC} ${EXTENDCOTROLS})

set_target_properties(demo PROPERTIES DEBUG_POSTFIX d)
target_link_libraries(demo soui utilities mhook smiley)
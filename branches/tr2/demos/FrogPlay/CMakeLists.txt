add_definitions(-D_CRT_SECURE_NO_WARNINGS)

include_directories(${PROJECT_SOURCE_DIR}/config)
include_directories(${PROJECT_SOURCE_DIR}/components)
include_directories(${PROJECT_SOURCE_DIR}/utilities/include)
include_directories(${PROJECT_SOURCE_DIR}/SOUI/include)
include_directories(${PROJECT_SOURCE_DIR}/third-part)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
link_directories(${PROJECT_SOURCE_DIR}/third-part/vlc/lib)

file(GLOB_RECURSE CURRENT_HEADERS  *.h *.hpp)
file(GLOB_RECURSE CURRENT_SRCS  *.cpp)
file(GLOB_RECURSE CURRENT_SOUIS  *.xml *.idx *.ico)
file(GLOB_RECURSE CURRENT_RC *.rc *.rc2)

set(EXTENDCOTROLS ${EXTENDCOTROLS}
	${PROJECT_SOURCE_DIR}/controls.extend/SRadioBox2.h
	${PROJECT_SOURCE_DIR}/controls.extend/SRadioBox2.cpp
)
source_group("Header Files" FILES ${CURRENT_HEADERS})
source_group("Source Files" FILES ${CURRENT_SRCS})
source_group("SoUI Resouece" FILES ${CURRENT_SOUIS})
source_group("Resource Files" FILES ${CURRENT_RC})
source_group("Extend Cotrols" FILES ${EXTENDCOTROLS})

add_executable(FrogPlay WIN32 ${CURRENT_HEADERS} ${CURRENT_SRCS} ${CURRENT_SOUIS} ${CURRENT_RC} ${EXTENDCOTROLS})

set_target_properties(360 PROPERTIES DEBUG_POSTFIX d)
SET (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SAFESEH:NO")

add_dependencies(FrogPlay soui-sys-resource ${COM_LIBS})
target_link_libraries(FrogPlay soui utilities libvlc libvlccore)

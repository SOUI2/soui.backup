#
# lijinggang@021.com
#
add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_CRT_NON_CONFORMING_SWPRINTFS)

include_directories(${PROJECT_SOURCE_DIR}/config)
include_directories(${PROJECT_SOURCE_DIR}/components)
include_directories(${PROJECT_SOURCE_DIR}/utilities/include)
include_directories(${PROJECT_SOURCE_DIR}/SOUI/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB_RECURSE CURRENT_HEADERS  *.h *.hpp)
file(GLOB_RECURSE CURRENT_SRCS  *.cpp)
file(GLOB_RECURSE CURRENT_SOUIS  *.xml *.idx *.ico)
file(GLOB_RECURSE CURRENT_RC *.rc *.rc2)

set(CURRENT_HEADERS ${CURRENT_HEADERS}
	${CMAKE_SOURCE_DIR}/controls.extend/STurn3DView.h
	${CMAKE_SOURCE_DIR}/controls.extend/image3d/3dlib.h
	${CMAKE_SOURCE_DIR}/controls.extend/image3d/3dmatrix.h
	${CMAKE_SOURCE_DIR}/controls.extend/image3d/3dTransform.h
)
set(CURRENT_SRCS ${CURRENT_SRCS}
	${CMAKE_SOURCE_DIR}/controls.extend/STurn3DView.cpp
	${CMAKE_SOURCE_DIR}/controls.extend/image3d/3dlib.cpp
	${CMAKE_SOURCE_DIR}/controls.extend/image3d/3dmatrix.cpp
	${CMAKE_SOURCE_DIR}/controls.extend/image3d/3dTransform.cpp
)

source_group("Header Files" FILES ${CURRENT_HEADERS})
source_group("Source Files" FILES ${CURRENT_SRCS})
source_group("SoUI Resouece" FILES ${CURRENT_SOUIS})
source_group("Resource Files" FILES ${CURRENT_RC})

add_executable(QQLogin WIN32 ${CURRENT_HEADERS} ${CURRENT_SRCS} ${CURRENT_SOUIS} ${CURRENT_RC})

add_dependencies(QQLogin soui-sys-resource ${COM_LIBS})
target_link_libraries(QQLogin utilities soui)

set_target_properties(QQLogin PROPERTIES DEBUG_POSTFIX d)
set_target_properties(QQLogin PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")
#cotire(QQLogin)

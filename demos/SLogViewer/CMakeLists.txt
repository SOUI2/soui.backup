#
# 1394020320@qq.com
#

include_directories(${PROJECT_SOURCE_DIR}/config)
include_directories(${PROJECT_SOURCE_DIR}/components)
include_directories(${PROJECT_SOURCE_DIR}/utilities/include)
include_directories(${PROJECT_SOURCE_DIR}/SOUI/include)
include_directories(${PROJECT_SOURCE_DIR}/third-part/scintilla/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/controls.extend)

set(EXTENDCOTROLS ${EXTENDCOTROLS}
	${CMAKE_SOURCE_DIR}/controls.extend/SRealWndHandler_Scintilla.h
	${CMAKE_SOURCE_DIR}/controls.extend/ScintillaWnd.h
)

set(EXTENDCOTROLS ${EXTENDCOTROLS}
	${CMAKE_SOURCE_DIR}/controls.extend/SRealWndHandler_Scintilla.cpp
	${CMAKE_SOURCE_DIR}/controls.extend/ScintillaWnd.cpp
)

file(GLOB_RECURSE CURRENT_HEADERS  *.h *.hpp)
file(GLOB_RECURSE CURRENT_SRCS  *.cpp)
file(GLOB_RECURSE CURRENT_SOUIS  *.xml *.idx *.ico)
file(GLOB_RECURSE CURRENT_RC *.rc *.rc2)

source_group("Header Files" FILES ${CURRENT_HEADERS})
source_group("Source Files" FILES ${CURRENT_SRCS})
source_group("SoUI Resouece" FILES ${CURRENT_SOUIS})
source_group("Resource Files" FILES ${CURRENT_RC})

add_executable(SLogViewer WIN32 ${CURRENT_HEADERS} ${CURRENT_SRCS} ${CURRENT_SOUIS} ${CURRENT_RC} ${EXTENDCOTROLS})

add_dependencies(SLogViewer soui-sys-resource)
target_link_libraries(SLogViewer utilities soui ${COM_LIBS} Scintilla imm32)

set_target_properties(SLogViewer PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})
set_target_properties (SLogViewer PROPERTIES
    FOLDER demos
)
cotire(SLogViewer)

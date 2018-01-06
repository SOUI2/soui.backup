#
# 1394020320@qq.com
#

include_directories(${PROJECT_SOURCE_DIR}/config)
include_directories(${PROJECT_SOURCE_DIR}/components)
include_directories(${PROJECT_SOURCE_DIR}/utilities/include)
include_directories(${PROJECT_SOURCE_DIR}/SOUI/include)
include_directories(${PROJECT_SOURCE_DIR}/third-part/Scintilla/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/controls.extend)


set(EXTENDCOTROLS ${EXTENDCOTROLS}
	${CMAKE_SOURCE_DIR}/controls.extend/image3d/3dlib.h
	${CMAKE_SOURCE_DIR}/controls.extend/image3d/3dmatrix.h
	${CMAKE_SOURCE_DIR}/controls.extend/image3d/3dTransform.h
	${CMAKE_SOURCE_DIR}/controls.extend/ExtendSkins.h
	${CMAKE_SOURCE_DIR}/controls.extend/image3d/PerspectiveTransform.h
	${CMAKE_SOURCE_DIR}/controls.extend/gif/SAniImgFrame.h
	${CMAKE_SOURCE_DIR}/controls.extend/SButtonEx.h
	${CMAKE_SOURCE_DIR}/controls.extend/SChromeTabCtrl.h
	${CMAKE_SOURCE_DIR}/controls.extend/SFreeMoveWindow.h
	${CMAKE_SOURCE_DIR}/controls.extend/gif/SGifPlayer.h
	#${CMAKE_SOURCE_DIR}/controls.extend/SImageEx.h
	${CMAKE_SOURCE_DIR}/controls.extend/SImageMaskWnd.h
	${CMAKE_SOURCE_DIR}/controls.extend/gif/SSkinAPNG.h
	${CMAKE_SOURCE_DIR}/controls.extend/gif/SSkinGif.h
	${CMAKE_SOURCE_DIR}/controls.extend/SSkinMutiFrameImg.h
	${CMAKE_SOURCE_DIR}/controls.extend/STabCtrl2.h
	${CMAKE_SOURCE_DIR}/controls.extend/SText.h
	${CMAKE_SOURCE_DIR}/controls.extend/STurn3DView.h
	${CMAKE_SOURCE_DIR}/controls.extend/SScrollText.h
	${CMAKE_SOURCE_DIR}/controls.extend/SIPAddressCtrl.h
	${CMAKE_SOURCE_DIR}/controls.extend/SListCtrlEx.h
	${CMAKE_SOURCE_DIR}/controls.extend/SRatingBar.h
	${CMAKE_SOURCE_DIR}/controls.extend/SRadioBox2.h
	${CMAKE_SOURCE_DIR}/controls.extend/SMcListViewEx/SHeaderCtrlEx.h
)

set(EXTENDCOTROLS ${EXTENDCOTROLS}
	${CMAKE_SOURCE_DIR}/controls.extend/image3d/3dlib.cpp
	${CMAKE_SOURCE_DIR}/controls.extend/image3d/3dmatrix.cpp
	${CMAKE_SOURCE_DIR}/controls.extend/image3d/3dtransform.cpp
	${CMAKE_SOURCE_DIR}/controls.extend/ExtendSkins.cpp
	${CMAKE_SOURCE_DIR}/controls.extend/SButtonEx.cpp
	${CMAKE_SOURCE_DIR}/controls.extend/SChromeTabCtrl.cpp
	${CMAKE_SOURCE_DIR}/controls.extend/SFreeMoveWindow.cpp
	${CMAKE_SOURCE_DIR}/controls.extend/gif/SGifPlayer.cpp
	#${CMAKE_SOURCE_DIR}/controls.extend/SImageEx.cpp
	${CMAKE_SOURCE_DIR}/controls.extend/SImageMaskWnd.cpp
	${CMAKE_SOURCE_DIR}/controls.extend/gif/SSkinAPNG.cpp
	${CMAKE_SOURCE_DIR}/controls.extend/gif/SSkinGif.cpp
	${CMAKE_SOURCE_DIR}/controls.extend/SSkinMutiFrameImg.cpp
	${CMAKE_SOURCE_DIR}/controls.extend/STabCtrl2.cpp
	${CMAKE_SOURCE_DIR}/controls.extend/SText.cpp
	${CMAKE_SOURCE_DIR}/controls.extend/STurn3DView.cpp
	${CMAKE_SOURCE_DIR}/controls.extend/SScrollText.cpp
	${CMAKE_SOURCE_DIR}/controls.extend/SIPAddressCtrl.cpp
	${CMAKE_SOURCE_DIR}/controls.extend/SListCtrlEx.cpp
	${CMAKE_SOURCE_DIR}/controls.extend/SRatingBar.cpp
	${CMAKE_SOURCE_DIR}/controls.extend/SRadioBox2.cpp
	${CMAKE_SOURCE_DIR}/controls.extend/SMcListViewEx/SHeaderCtrlEx.cpp
)

file(GLOB_RECURSE CURRENT_HEADERS  *.h *.hpp)
file(GLOB_RECURSE CURRENT_SRCS  *.cpp)
file(GLOB_RECURSE CURRENT_SOUIS  *.xml *.idx *.ico)
file(GLOB_RECURSE CURRENT_RC *.rc *.rc2)

source_group("Header Files" FILES ${CURRENT_HEADERS})
source_group("Source Files" FILES ${CURRENT_SRCS})
source_group("SoUI Resouece" FILES ${CURRENT_SOUIS})
source_group("Resource Files" FILES ${CURRENT_RC})
source_group("Extend Cotrols" FILES ${EXTENDCOTROLS})

add_executable(SouiEditor WIN32 ${CURRENT_HEADERS} ${CURRENT_SRCS} ${CURRENT_SOUIS} ${CURRENT_RC} ${EXTENDCOTROLS})

add_dependencies(SouiEditor soui-sys-resource)
target_link_libraries(SouiEditor utilities soui ${COM_LIBS} Scintilla imm32)

set_target_properties(SouiEditor PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})
set_target_properties (SouiEditor PROPERTIES
    FOLDER demos
)
cotire(SouiEditor)

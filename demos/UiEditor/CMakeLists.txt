#
# lijinggang@021.com
#
include_directories(${PROJECT_SOURCE_DIR}/config)
include_directories(${PROJECT_SOURCE_DIR}/components)
include_directories(${PROJECT_SOURCE_DIR}/utilities/include)
include_directories(${PROJECT_SOURCE_DIR}/SOUI/include)
include_directories(${PROJECT_SOURCE_DIR}/controls.extend)
include_directories(${PROJECT_SOURCE_DIR}/third-part/SciLexer/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/soui-sys-resource)

file(GLOB_RECURSE CURRENT_HEADERS  *.h *.hpp)
file(GLOB_RECURSE CURRENT_SRCS  *.cpp)
file(GLOB_RECURSE CURRENT_SOUIS  *.xml *.idx *.ico)
file(GLOB_RECURSE CURRENT_RC *.rc *.rc2)

set(EXTENDCOTROLS ${EXTENDCOTROLS}
	${CMAKE_SOURCE_DIR}/controls.extend/gif/SSkinGif.h
	${CMAKE_SOURCE_DIR}/controls.extend/gif/SSkinAPNG.h
	${CMAKE_SOURCE_DIR}/controls.extend/STabCtrl2.h
	${CMAKE_SOURCE_DIR}/controls.extend/SChromeTabCtrl.h
	${CMAKE_SOURCE_DIR}/controls.extend/SImageMaskWnd.h
	${CMAKE_SOURCE_DIR}/controls.extend/SFreeMoveWindow.h
	${CMAKE_SOURCE_DIR}/controls.extend/SButtonEx.h
	${CMAKE_SOURCE_DIR}/controls.extend/SSkinMutiFrameImg.h
	${CMAKE_SOURCE_DIR}/controls.extend/imre/SImRichEdit.h
	${CMAKE_SOURCE_DIR}/controls.extend/imre/RichEditObj.h
	${CMAKE_SOURCE_DIR}/controls.extend/imre/RichEditOleCallback.h
	${CMAKE_SOURCE_DIR}/controls.extend/imre/RichEditObjFactory.h
	${CMAKE_SOURCE_DIR}/controls.extend/imre/RichEditOleCtrls.h
	${CMAKE_SOURCE_DIR}/controls.extend/imre/RichEditOleBase.h
	${CMAKE_SOURCE_DIR}/controls.extend/imre/ImgProvider.h
	${CMAKE_SOURCE_DIR}/controls.extend/image3d/3dTransform.h
	${CMAKE_SOURCE_DIR}/controls.extend/image3d/3dlib.h
	${CMAKE_SOURCE_DIR}/controls.extend/image3d/3dmatrix.h
	${CMAKE_SOURCE_DIR}/controls.extend/ExtendSkins.h
	${CMAKE_SOURCE_DIR}/controls.extend/SImagePlayer.h
	${CMAKE_SOURCE_DIR}/controls.extend/SText.h
	${CMAKE_SOURCE_DIR}/controls.extend/SImageEx.h
	${CMAKE_SOURCE_DIR}/controls.extend/SSplitBar.h
	${CMAKE_SOURCE_DIR}/controls.extend/STurn3DView.h
	${CMAKE_SOURCE_DIR}/controls.extend/SAnimImg.h
	${CMAKE_SOURCE_DIR}/controls.extend/SSkinImgFrame2.h
	${CMAKE_SOURCE_DIR}/controls.extend/SScrollText.h	
)
set(EXTENDCOTROLS ${EXTENDCOTROLS}
	${CMAKE_SOURCE_DIR}/controls.extend/gif/SSkinGif.cpp
	${CMAKE_SOURCE_DIR}/controls.extend/gif/SSkinAPNG.cpp
	${CMAKE_SOURCE_DIR}/controls.extend/STabCtrl2.cpp
	${CMAKE_SOURCE_DIR}/controls.extend/SChromeTabCtrl.cpp
	${CMAKE_SOURCE_DIR}/controls.extend/SImageMaskWnd.cpp
	${CMAKE_SOURCE_DIR}/controls.extend/SFreeMoveWindow.cpp
	${CMAKE_SOURCE_DIR}/controls.extend/SButtonEx.cpp
	${CMAKE_SOURCE_DIR}/controls.extend/SSkinMutiFrameImg.cpp
	${CMAKE_SOURCE_DIR}/controls.extend/imre/SImRichEdit.cpp
	${CMAKE_SOURCE_DIR}/controls.extend/imre/RichEditObj.cpp
	${CMAKE_SOURCE_DIR}/controls.extend/imre/RichEditOleCallback.cpp
	${CMAKE_SOURCE_DIR}/controls.extend/imre/RichEditObjFactory.cpp
	${CMAKE_SOURCE_DIR}/controls.extend/imre/RichEditOleCtrls.cpp
	${CMAKE_SOURCE_DIR}/controls.extend/imre/RichEditOleBase.cpp
	${CMAKE_SOURCE_DIR}/controls.extend/imre/ImgProvider.cpp
	${CMAKE_SOURCE_DIR}/controls.extend/image3d/3dtransform.cpp
	${CMAKE_SOURCE_DIR}/controls.extend/image3d/3dlib.cpp
	${CMAKE_SOURCE_DIR}/controls.extend/image3d/3dmatrix.cpp
	${CMAKE_SOURCE_DIR}/controls.extend/ExtendSkins.cpp
	${CMAKE_SOURCE_DIR}/controls.extend/SImagePlayer.cpp
	${CMAKE_SOURCE_DIR}/controls.extend/SText.cpp
	${CMAKE_SOURCE_DIR}/controls.extend/SImageEx.cpp
	${CMAKE_SOURCE_DIR}/controls.extend/SSplitBar.cpp
	${CMAKE_SOURCE_DIR}/controls.extend/STurn3DView.cpp
	${CMAKE_SOURCE_DIR}/controls.extend/SAnimImg.cpp
	${CMAKE_SOURCE_DIR}/controls.extend/SSkinImgFrame2.cpp
	${CMAKE_SOURCE_DIR}/controls.extend/SScrollText.cpp		
)

source_group("Extend Cotrols" FILES ${EXTENDCOTROLS})
source_group("Header Files" FILES ${CURRENT_HEADERS})
source_group("Source Files" FILES ${CURRENT_SRCS})
source_group("SoUI Resouece" FILES ${CURRENT_SOUIS})
source_group("Resource Files" FILES ${CURRENT_RC})

add_executable(UiEditor WIN32 ${CURRENT_HEADERS} ${CURRENT_SRCS} ${CURRENT_SOUIS} ${CURRENT_RC} ${EXTENDCOTROLS})

add_dependencies(UiEditor soui-sys-resource ${COM_LIBS})
target_link_libraries(UiEditor utilities soui)

set_target_properties(UiEditor PROPERTIES DEBUG_POSTFIX d)
set_target_properties(UiEditor PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")
set_target_properties(UiEditor PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})
#cotire(UiEditor)

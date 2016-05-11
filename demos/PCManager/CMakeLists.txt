#
# lijinggang@021.com
#
include_directories(${PROJECT_SOURCE_DIR}/config)
include_directories(${PROJECT_SOURCE_DIR}/components)
include_directories(${PROJECT_SOURCE_DIR}/utilities/include)
include_directories(${PROJECT_SOURCE_DIR}/SOUI/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set (srcs
    MainDlg.cpp
    PCManager.cpp
    PCManager.rc

    Switch/SSwitch.cpp
    Switch/SSwitch.h

    Toolbox/SToolbox.cpp
    Toolbox/SToolbox.h
)

add_executable(PCManager WIN32 ${srcs})
target_link_libraries(PCManager utilities soui)

set_target_properties(PCManager PROPERTIES DEBUG_POSTFIX d)
set_target_properties(PCManager PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")
set_target_properties(PCManager PROPERTIES LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS\",5.01\"")
set_target_properties(PCManager PROPERTIES DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX})
cotire(PCManager)
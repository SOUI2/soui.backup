# 
# SOUI cmake配置文件
#

add_definitions(-DUTILITIES_EXPORTS -D_CRT_SECURE_NO_WARNINGS)

include_directories(${PROJECT_SOURCE_DIR}/config)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(include)

set(headers
	stdafx.h
	include/gdialpha.h
	include/souicoll.h
	include/trace.h
	include/snew.h
	include/utilities-def.h
	include/utilities.h
	include/soui_mem_wrapper.h
	include/atl.mini/atldef.h
	include/atl.mini/SComCli.h
	include/atl.mini/SComHelper.h
	include/pugixml/pugiconfig.hpp
	include/pugixml/pugixml.hpp
	include/string/strcpcvt.h
	include/string/tstring.h
	include/unknown/obj-ref-i.h
	include/unknown/obj-ref-impl.hpp
	include/com-loader.hpp
	include/wtl.mini/msgcrack.h
	include/wtl.mini/souimisc.h
)

aux_source_directory(src         utilities_src)
aux_source_directory(src/pugixml utilities_pugixml)
aux_source_directory(src/string  utilities_string)

set(source ${utilities_src} ${utilities_pugixml} ${utilities_string})

source_group("Header Files" FILES ${headers})
source_group("Source Files" FILES ${source})

if (NOT ENABLE_SOUI_CORE_LIB)
    set (source  ${source} utilities.rc)
    add_library(utilities SHARED ${source} ${headers})
    target_link_libraries(utilities png zlib)
else()
    add_library(utilities STATIC ${source} ${headers})
endif()

# add_library(utilities SHARED ${source} ${headers})

set_target_properties(utilities PROPERTIES COTIRE_CXX_PREFIX_HEADER_INIT "stdafx.h")
cotire(utilities)
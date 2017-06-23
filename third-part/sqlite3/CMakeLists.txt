#
# 1394020320@qq.com
#
add_definitions(-D_CRT_SECURE_NO_WARNINGS -DSQLITE_ENABLE_COLUMN_METADATA)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

file(GLOB_RECURSE CURRENT_HEADERS  *.h *.hpp)
file(GLOB_RECURSE CURRENT_SRCS *.c)

source_group("Header Files" FILES ${CURRENT_HEADERS})
source_group("Source Files" FILES ${CURRENT_SRCS})

add_library(sqlite3 STATIC ${CURRENT_HEADERS} ${CURRENT_SRCS})

#set(COM_LIBS ${COM_LIBS} sqlite3 CACHE INTERNAL "com_lib")
set_target_properties (sqlite3 PROPERTIES
    FOLDER third-part
)

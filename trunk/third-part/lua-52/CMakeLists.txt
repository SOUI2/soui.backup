#
#
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}src)
aux_source_directory(src/ sources )

add_library(lua STATIC ${sources})
set_target_properties(lua PROPERTIES OUTPUT_NAME lua-52)
# set_target_properties(lua PROPERTIES PREFIX lib)

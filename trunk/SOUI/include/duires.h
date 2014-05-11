#pragma once

#   define DEFINE_DUIRES(id, type, file_name)  \
    id type file_name

//////////////////////////////////////////////////////////////////////////

#   define DEFINE_XML(id, file_name)     \
    DEFINE_DUIRES(id, XML, file_name)

#   define DEFINE_BMP(id, file_name)     \
    DEFINE_DUIRES(id, BITMAP, file_name)

#   define DEFINE_ICO(id, file_name)     \
    DEFINE_DUIRES(id, ICON, file_name)

#   define DEFINE_IMGX(id, file_name)     \
    DEFINE_DUIRES(id, IMGX, file_name)

//////////////////////////////////////////////////////////////////////////

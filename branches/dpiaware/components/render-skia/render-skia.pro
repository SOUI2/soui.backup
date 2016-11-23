######################################################################
# Automatically generated by qmake (2.01a) ?? ?? 23 19:27:59 2014
######################################################################
TEMPLATE = lib
TARGET = render-skia

!LIB_ALL:!COM_LIB{
	RC_FILE += render-skia.rc
	CONFIG += dll
}
else{
	CONFIG += staticlib
}


DEPENDPATH += .
INCLUDEPATH += . \
			   ../../soui/include \
			   ../../utilities/include \
			   ../../third-part/skia \
			   ../../third-part/skia/include \
			   ../../third-part/skia/include/config \
			   ../../third-part/skia/include/core \

dir = ../..
include($$dir/common.pri)

CONFIG(debug,debug|release){
	LIBS += utilitiesd.lib skiad.lib shlwapi.lib
}
else{
	LIBS += utilities.lib skia.lib shlwapi.lib
}
LIBS += Usp10.lib opengl32.lib

PRECOMPILED_HEADER = stdafx.h

# Input
HEADERS += drawtext-skia.h render-skia.h render-skia2-i.h render-skia2.h skia2rop2.h
SOURCES += drawtext-skia.cpp render-skia.cpp render-skia2.cpp skia2rop2.cpp

TEMPLATE = subdirs
TARGET = third-part
DEPENDPATH += .
INCLUDEPATH += .

include(../cpy-cfg.pri)

SUBDIRS += gtest
SUBDIRS += png
SUBDIRS += skia
SUBDIRS += zlib
SUBDIRS += lua-52
SUBDIRS += smiley
SUBDIRS += mhook
SUBDIRS += 7z

CONFIG(c++11){
#7z需要c11支持
	
}

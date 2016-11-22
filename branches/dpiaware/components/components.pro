TEMPLATE = subdirs
TARGET = components
DEPENDPATH += .
INCLUDEPATH += .

include(../cpy-cfg.pri)

SUBDIRS += imgdecoder-wic
SUBDIRS += render-gdi
SUBDIRS += render-skia
SUBDIRS += translator
SUBDIRS += resprovider-zip
SUBDIRS += imgdecoder-stb
SUBDIRS += imgdecoder-png
SUBDIRS += imgdecoder-gdip
SUBDIRS += ScriptModule-LUA
SUBDIRS += log4z
CONFIG(c++11){
#7z需要c11支持
	SUBDIRS += resprovider-7zip
}

imgdecoder-png.depends += zlib png
render-skia.depends += skia
resprovider-zip.depends += zlib utilities
translator.depends += utilities
resprovider-zip.depends += zlib utilities
ScriptModule-LUA.depends += soui lua-52
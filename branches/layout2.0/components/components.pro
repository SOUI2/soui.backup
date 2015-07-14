TEMPLATE = subdirs
TARGET = components
DEPENDPATH += .
INCLUDEPATH += .

SUBDIRS += imgdecoder-wic
SUBDIRS += render-gdi
SUBDIRS += skia
SUBDIRS += render-skia
SUBDIRS += translator
SUBDIRS += zlib
SUBDIRS += png
SUBDIRS += resprovider-zip
SUBDIRS += imgdecoder-stb
SUBDIRS += imgdecoder-png
SUBDIRS += imgdecoder-gdip
SUBDIRS += ScriptModule-LUA

imgdecoder-png.depends += zlib png
render-skia.depends += skia
resprovider-zip.depends += zlib utilities
translator.depends += utilities
resprovider-zip.depends += zlib utilities

TEMPLATE = subdirs
TARGET = third-part
DEPENDPATH += .
INCLUDEPATH += .

include(../cpy-cfg.pri)

SUBDIRS += png
SUBDIRS += skia
SUBDIRS += zlib
SUBDIRS += lua-52
SUBDIRS += smiley
SUBDIRS += mhook


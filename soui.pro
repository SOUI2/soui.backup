TEMPLATE = subdirs
TARGET = soui
DEPENDPATH += .
INCLUDEPATH += .

include(cpy-cfg.pri)

SUBDIRS += soui-mem
SUBDIRS += utilities
SUBDIRS += soui
SUBDIRS += components

SUBDIRS += soui-sys-resource
SUBDIRS += demo
SUBDIRS += qqlogin
SUBDIRS += 360
SUBDIRS += souispy

utilities.depends += soui-mem
soui.depends += utilities

360.depends += utilities soui
demo.depends += utilities soui
qqlogin.depends += utilities soui
souispy.depends += utilities soui

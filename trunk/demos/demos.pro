TEMPLATE = subdirs
TARGET = demos
DEPENDPATH += .
INCLUDEPATH += .

include(../cpy-cfg.pri)

SUBDIRS += demo
SUBDIRS += qqlogin
SUBDIRS += 360
SUBDIRS += 360Preview
SUBDIRS += souispy
SUBDIRS += PcManager
SUBDIRS += SoTool
SUBDIRS += mclistview_demo

360.depends += soui
360Preview.depends += soui
demo.depends += soui mhook smiley
qqlogin.depends += soui
souispy.depends += soui
PcManager.depends += soui
SoTool.depends += soui
mclistview_demo.depends += soui
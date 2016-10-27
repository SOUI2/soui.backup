TEMPLATE = subdirs
TARGET = demos
DEPENDPATH += .
INCLUDEPATH += .

include(../cpy-cfg.pri)

SUBDIRS += UiEditor
SUBDIRS += demo
SUBDIRS += qqlogin
SUBDIRS += 360
SUBDIRS += 360Preview
SUBDIRS += souispy
SUBDIRS += PcManager
SUBDIRS += SoTool
SUBDIRS += mclistview_demo

360.depends += soui skia
360Preview.depends += soui skia
demo.depends += soui mhook smiley skia
qqlogin.depends += soui skia
souispy.depends += soui skia
PcManager.depends += soui skia
SoTool.depends += soui skia
mclistview_demo.depends += soui skia
UiEditor.depends += soui skia
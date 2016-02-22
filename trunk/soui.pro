TEMPLATE = subdirs
TARGET = soui
DEPENDPATH += .
INCLUDEPATH += .

include(cpy-cfg.pri)

SUBDIRS += utilities
SUBDIRS += soui
SUBDIRS += components

SUBDIRS += soui-sys-resource
SUBDIRS += SoSmiley
SUBDIRS += demo
SUBDIRS += qqlogin
SUBDIRS += 360
SUBDIRS += 360Preview
SUBDIRS += souispy
SUBDIRS += MusicPlayer
SUBDIRS += PcManager
SUBDIRS += SoTool

soui.depends += utilities soui-sys-resource

360.depends += soui
360Preview.depends += soui
demo.depends += soui SoSmiley
qqlogin.depends += soui
souispy.depends += soui
MusicPlayer.depends += soui
PcManager.depends += soui
SoTool.depends += soui
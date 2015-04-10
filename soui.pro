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

soui.depends += utilities

360.depends += utilities soui
360Preview.depends += utilities soui
demo.depends += utilities soui SoSmiley
qqlogin.depends += utilities soui
souispy.depends += utilities soui
MusicPlayer.depends += utilities soui
PcManager.depends += utilities soui
SoTool.depends += utilities soui
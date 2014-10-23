TEMPLATE = subdirs
TARGET = ScriptModule-LUA
DEPENDPATH += .
INCLUDEPATH += .

SUBDIRS += lua-51
SUBDIRS += ScriptModule

ScriptModule.depends += lua-51 soui

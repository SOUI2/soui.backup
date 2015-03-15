TEMPLATE = subdirs
TARGET = ScriptModule-LUA
DEPENDPATH += .
INCLUDEPATH += .

SUBDIRS += lua-52
SUBDIRS += ScriptModule

ScriptModule.depends += lua-52 soui

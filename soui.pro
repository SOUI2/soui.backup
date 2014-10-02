TEMPLATE = subdirs
TARGET = soui
DEPENDPATH += .
INCLUDEPATH += .

SUBDIRS += utilities
SUBDIRS += soui
SUBDIRS += soui-sys-resource
SUBDIRS += components/imgdecoder-wic
SUBDIRS += components/render-gdi
SUBDIRS += components/myskia
SUBDIRS += components/render-skia
SUBDIRS += components/translator
SUBDIRS += components/zlib
SUBDIRS += components/png
SUBDIRS += components/resprovider-zip
SUBDIRS += components/imgdecoder-stb
SUBDIRS += components/imgdecoder-png

DLL_SOUI{
	SUBDIRS += components/ScriptModule-LUA/lua-51
	SUBDIRS += components/ScriptModule-LUA/ScriptModule
}
SUBDIRS += demo
!LIB_SOUI_COM{
	SUBDIRS += qqlogin
	SUBDIRS += 360
	SUBDIRS += souispy
}

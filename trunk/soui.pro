TEMPLATE = subdirs
TARGET = soui
DEPENDPATH += .
INCLUDEPATH += .

DLL_SOUI{
	system(copy soui\def\dll.h soui\include\def.h)
}
else{
	system(copy soui\def\lib.h soui\include\def.h)
}

CONFIG(LIB_SOUI_COM){
	DEFINES += LIB_SOUI_COM
}

SUBDIRS += utilities
SUBDIRS += soui
SUBDIRS += soui-sys-resource
SUBDIRS += components

SUBDIRS += demo
!LIB_SOUI_COM{
	SUBDIRS += qqlogin
	SUBDIRS += 360
	SUBDIRS += souispy
}


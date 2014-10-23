TEMPLATE = subdirs
TARGET = soui
DEPENDPATH += .
INCLUDEPATH += .

SUBDIRS += utilities
SUBDIRS += soui
SUBDIRS += soui-sys-resource
SUBDIRS += components

SUBDIRS += demo
!LIB_SOUI_COM{
	SUBDIRS += qqlogin
	SUBDIRS += 360
	SUBDIRS += souispy
	
	qqlogin.depends = soui
	360.depends = soui
	souispy.depends = soui
}

soui.depends += utilities
demo.depends += soui
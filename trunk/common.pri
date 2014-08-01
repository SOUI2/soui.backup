CONFIG -= qt

CharacterSet = 1
#DEFINES -= UNICODE


CONFIG(debug, debug|release) {
	OBJECTS_DIR =   $$dir/obj/debug/$$TARGET
	DESTDIR = $$dir/bin
	QMAKE_LIBDIR += $$DESTDIR
}
else {
	OBJECTS_DIR =   $$dir/obj/release/$$TARGET
	DESTDIR = $$dir/bin
	QMAKE_LIBDIR += $$DESTDIR
}

#<--下面这段代码为debug和release生成不同的文件名
SAVE_TEMPLATE = $$TEMPLATE
TEMPLATE = fakelib
TARGET = $$qtLibraryTarget($$TARGET)
TEMPLATE = $$SAVE_TEMPLATE
#-->

DEFINES += _CRT_SECURE_NO_WARNINGS

QMAKE_LFLAGS += /MACHINE:X86
QMAKE_CXXFLAGS += -EHsc
QMAKE_CXXFLAGS += -Fd$(IntDir)

QMAKE_CXXFLAGS_RTTI_ON += /GR-
QMAKE_CXXFLAGS_RELEASE += /O1
QMAKE_CXXFLAGS_RELEASE += /Zi

CONFIG(CAN_DEBUG){
	#Release版本允许生产调试符号
	QMAKE_LFLAGS_RELEASE += /DEBUG
	QMAKE_LFLAGS_RELEASE += /OPT:REF /OPT:ICF
}

CONFIG(USING_MT){
	#使用MT链接CRT
	QMAKE_CXXFLAGS_RELEASE += /MT
	QMAKE_CXXFLAGS_DEBUG += /MTd
}

win32-msvc*{
    QMAKE_CXXFLAGS += /wd4100 /wd4101 /wd4102 /wd4189 /wd4996
}
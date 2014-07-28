CONFIG -= qt

CharacterSet = 1

CONFIG(debug, debug|release) {
	OBJECTS_DIR =   $$dir/obj/debug/$$TARGET
	DESTDIR = $$dir/bin/debug
	QMAKE_LIBDIR += $$DESTDIR
}
else {
	OBJECTS_DIR =   $$dir/obj/release/$$TARGET
	DESTDIR = $$dir/bin/release
	QMAKE_LIBDIR += $$DESTDIR
}

DEFINES += _CRT_SECURE_NO_WARNINGS

QMAKE_LFLAGS += /MACHINE:X86
#QMAKE_LFLAGS_DEBUG += /debugtype:cv,fixup
QMAKE_CXXFLAGS += -EHsc
QMAKE_CXXFLAGS += -Fd$(IntDir)\

win32-msvc*{
    QMAKE_CXXFLAGS += /wd4100 /wd4101 /wd4102 /wd4189 /wd4996
}
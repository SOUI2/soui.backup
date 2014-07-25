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
CONFIG -= qt

CharacterSet = 1

CONFIG(debug, debug|release) {
OBJECTS_DIR =   $$dir/obj/debug/$$TARGET
}
else {
OBJECTS_DIR =   $$dir/obj/release/$$TARGET
}

TARGET = $$qtLibraryTarget($$TARGET)

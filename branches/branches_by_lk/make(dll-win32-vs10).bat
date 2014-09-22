call "%VS100COMNTOOLS%..\..\VC\vcvarsall.bat" x86
tools\qmake -tp vc -r -spec .\tools\mkspecs\win32-msvc2010 "CONFIG += DLL_SOUI USING_MT CAN_DEBUG"

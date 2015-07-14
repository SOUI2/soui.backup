rem LIB_ALL 所有模块均生成LIB。
rem USING_MT 指示整个项目使用MT方式连接CRT
rem CAN_DEBUG 指示为生成的Release版本生产调试符号
call "%VS90COMNTOOLS%..\..\VC\vcvarsall.bat" x86
tools\qmake -tp vc -r -spec .\tools\mkspecs\win32-msvc2008 "CONFIG += USING_MT CAN_DEBUG LIB_ALL"
rem soui.sln
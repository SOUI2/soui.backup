rem LIB_ALL 所有模块均生成LIB。
rem USING_MT 指示整个项目使用MT方式连接CRT
rem CAN_DEBUG 指示为生成的Release版本生产调试符号

SET mt=1
SET unicode=1
SET wchar=1

if exist .\config\build.cfg del .\config\build.cfg
set configStr=[BuiltConfig]
echo %configStr% >> .\config\build.cfg
set configStr=UNICODE=%unicode%
echo %configStr% >> .\config\build.cfg
set configStr=WCHAR=%wchar%
echo %configStr% >> .\config\build.cfg
set configStr=MT=%mt%
echo %configStr% >> .\config\build.cfg

call "%VS90COMNTOOLS%..\..\VC\vcvarsall.bat" x86
tools\qmake -tp vc -r -spec .\tools\mkspecs\win32-msvc2008 "CONFIG += USING_MT CAN_DEBUG LIB_ALL"
rem soui.sln
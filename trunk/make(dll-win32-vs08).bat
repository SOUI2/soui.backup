rem 保存项目默认配置
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


rem DLL_SOUI 指示为SOUI产生DLL版本。
rem USING_MT 指示整个项目使用MT方式连接CRT
rem CAN_DEBUG 指示为生成的Release版本生产调试符号
call "%VS90COMNTOOLS%..\..\VC\vcvarsall.bat" x86


tools\qmake -tp vc -r -spec .\tools\mkspecs\win32-msvc2008 "CONFIG += USING_MT CAN_DEBUG"
rem soui.sln
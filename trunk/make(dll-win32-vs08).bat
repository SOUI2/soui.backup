rem DLL_SOUI 指示为SOUI产生DLL版本。
rem USING_MT 指示整个项目使用MT方式连接CRT
rem CAN_DEBUG 指示为生成的Release版本生产调试符号
tools\qmake -tp vc -r -spec .\tools\mkspecs\win32-msvc2008 "CONFIG += DLL_SOUI USING_MT CAN_DEBUG"
rem soui.sln
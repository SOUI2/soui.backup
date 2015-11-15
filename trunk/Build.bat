@ECHO off
SETLOCAL enabledelayedexpansion
cls
COLOR 1f

ECHO.
ECHO.
ECHO   ##############################################################
ECHO   #               欢迎使用 SOUI 工程配置向导                   #
ECHO   #                                启程软件 2014.10.31         #
ECHO   ##############################################################
ECHO.
ECHO.

SET cfg=
SET specs=
SET target=x86
SET selected=
rem 选择编译版本
SET /p selected=1.选择编译版本[1=x86;2=x64]:
if %selected%==1 (
	SET target=x86
) else if %selected%==2 (
	SET target=x64
	SET cfg=!cfg! x64
) else (
	goto error
)

rem 选择开发环境
SET /p selected=2.选择开发环境[1=vs2008;2=vs2010;3=vs2012;4=vs2013;5=2005]:
if %selected%==1 (
	SET specs=win32-msvc2008
	call "%VS90COMNTOOLS%..\..\VC\vcvarsall.bat" %target%
) else if %selected%==2 (
	SET specs=win32-msvc2010
	call "%VS100COMNTOOLS%..\..\VC\vcvarsall.bat" %target%
) else if %selected%==3 (
	SET specs=win32-msvc2010
	call "%VS110COMNTOOLS%..\..\VC\vcvarsall.bat" %target%
) else if %selected%==4 (
	SET specs=win32-msvc2010
	call "%VS120COMNTOOLS%..\..\VC\vcvarsall.bat" %target%
) else if %selected%==5 (
	SET specs=win32-msvc2005
	call "%VS80COMNTOOLS%..\..\VC\vcvarsall.bat" %target%
) else (
	goto error
)

rem 选择编译类型
SET /p selected=3.选择SOUI编译模式[1=全模块DLL;2=全模块LIB;3=内核LIB,组件DLL(不能使用LUA脚本模块)]:
if %selected%==1 (
	rem do nothing
) else if %selected%==2 (
	SET cfg=!cfg! LIB_ALL
) else if %selected%==3 (
	SET cfg=!cfg! CORE_LIB
) else (
	goto error
)

rem 选择字符集
SET /p selected=4.选择字符集[1=UNICODE;2=MBCS]:
if %selected%==1 (
	rem do nothing
) else if %selected%==2 (
	SET cfg=!cfg! MBCS
) else (
	goto error
)

rem 选择WCHAR支持
SET /p selected=5.将WCHAR作为内建类型[1=是;2=否]:
if %selected%==1 (
	rem do nothing
) else if %selected%==2 (
	SET cfg=!cfg! DISABLE_WCHAR
) else (
	goto error
)

rem CRT
SET /p selected=6.选择CRT链接模式[1=静态链接(MT);2=动态链接(MD)]:
if %selected%==1 (
	SET cfg=!cfg! USING_MT
) else if %selected%==2 (
	rem do nothing
) else (
	goto error
)

rem 为release版本生成调试信息
SET /p selected=7.是否为release版本生成调试信息[1=生成;2=不生成]:
if %selected%==1 (
	SET cfg=!cfg! CAN_DEBUG
) else if %selected%==2 (
	rem do nothing
) else (
	goto error
)

rem 参数配置完成

tools\qmake -tp vc -r -spec .\tools\mkspecs\%specs% "CONFIG += %cfg%"

SET /p selected=open[o], compile[c] "soui.sln" or quit(q) [o,c or q]?
if "%selected%" == "o" (
	soui.sln
) else if "%selected%" == "c" (
	call devenv soui.sln /Clean Debug
	call devenv soui.sln /build Debug
	call devenv soui.sln /Clean Release
	call devenv soui.sln /build Release
) else (
	goto final
)

goto final

:error
	ECHO 选择错误，请重新选择

:final


















rem pause
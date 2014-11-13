@ECHO off
SETLOCAL enabledelayedexpansion
cls
COLOR 1f

ECHO.
ECHO.
ECHO   ##############################################################
ECHO   #                   SOUI 工程配置向导                        #
ECHO   #  注意:生成vs2010以上版本的工程时,所有LIB库工程的Debug配置  #
ECHO   #       均需要手动将输出文件改成项目文件+d.lib的格式。       #
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
) else (
	goto error
)

rem 选择开发环境
SET /p selected=2.选择开发环境[1=vs2008;2=vs2010;3=vs2012;4=vs2013]:
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

rem CLR
SET /p selected=5.选择CLR开关[1=不支持;2=支持]:
if %selected%==1 (
	rem do nothing
) else if %selected%==2 (
	SET cfg=!cfg! USING_CLR
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
	SET buildParam="devenv" soui.sln /build	
	call !buildParam! "Debug"
	call !buildParam! "Release"
) else (
	goto final
)

goto final

:error
	ECHO 选择错误，请重新选择

:final


















rem pause
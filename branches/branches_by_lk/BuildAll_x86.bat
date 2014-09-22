@ECHO off

cls
COLOR 1f

ECHO.
ECHO.
ECHO   ##############################################################
ECHO   #        Please choose the compile IDE version               #
ECHO   # 1 - Microsoft Visual Studio 2008                           #
ECHO   # 2 - Microsoft Visual Studio 2010                           #
ECHO   # 3 - Microsoft Visual Studio 2008 Static                    #
ECHO   # 4 - Microsoft Visual Studio 2010 Static                    #
ECHO   ##############################################################
ECHO.
ECHO.


:: Delete variable %A%
SET "A="
SET /P A=Set Your Choice And Press Enter: 
ECHO Loading .........

IF "%A%"=="1" (
	SET "COMPILEDIR=%VS90COMNTOOLS%"
	SET MSVCVER=win32-msvc2008
	SET EXT=.vcproj
	SET QMAKECFG="CONFIG += DLL_SOUI USING_MT CAN_DEBUG"
	SET SKIPLUA="FALSE"
	goto start
)

IF "%A%"=="2" (
	SET "COMPILEDIR=%VS100COMNTOOLS%"
	SET MSVCVER=win32-msvc2010
	SET EXT=.vcxproj
	SET QMAKECFG="CONFIG += DLL_SOUI"
	SET SKIPLUA="FALSE"
	goto start
)

IF "%A%"=="3" (
	SET "COMPILEDIR=%VS90COMNTOOLS%"
	SET MSVCVER=win32-msvc2008
	SET EXT=.vcproj
	SET SKIPLUA="TRUE"
	goto start
)

IF "%A%"=="4" (
	SET "COMPILEDIR=%VS100COMNTOOLS%"
	SET MSVCVER=win32-msvc2010
	SET EXT=.vcxproj
	SET SKIPLUA="TRUE"
	goto start
)

ECHO.
:start
call "%COMPILEDIR%..\..\VC\vcvarsall.bat" x86
tools\qmake -tp vc -r -spec .\tools\mkspecs\%MSVCVER% %QMAKECFG%
ECHO Start compile debug version...
call "%COMPILEDIR%..\IDE\devenv" soui.sln /build "Debug" /project "soui-sys-resource\soui-sys-resource%EXT%" /projectconfig "Debug"
call "%COMPILEDIR%..\IDE\devenv" soui.sln /build "Debug" /project "utilities\utilities%EXT%" /projectconfig "Debug"
call "%COMPILEDIR%..\IDE\devenv" soui.sln /build "Debug" /project "soui\soui%EXT%" /projectconfig "Debug"
call "%COMPILEDIR%..\IDE\devenv" soui.sln /build "Debug" /project "components\translator\translator%EXT%" /projectconfig "Debug"
call "%COMPILEDIR%..\IDE\devenv" soui.sln /build "Debug" /project "components\imgdecoder-wic\imgdecoder-wic%EXT%" /projectconfig "Debug"
call "%COMPILEDIR%..\IDE\devenv" soui.sln /build "Debug" /project "components\render-gdi\render-gdi%EXT%" /projectconfig "Debug"
call "%COMPILEDIR%..\IDE\devenv" soui.sln /build "Debug" /project "components\zlib\zlib%EXT%" /projectconfig "Debug"
call "%COMPILEDIR%..\IDE\devenv" soui.sln /build "Debug" /project "components\resprovider-zip\resprovider-zip%EXT%" /projectconfig "Debug"
call "%COMPILEDIR%..\IDE\devenv" soui.sln /build "Debug" /project "components\freetype\freetype%EXT%" /projectconfig "Debug"
call "%COMPILEDIR%..\IDE\devenv" soui.sln /build "Debug" /project "components\myskia\myskia%EXT%" /projectconfig "Debug"
call "%COMPILEDIR%..\IDE\devenv" soui.sln /build "Debug" /project "components\render-skia\render-skia%EXT%" /projectconfig "Debug"
IF %SKIPLUA% NEQ "TRUE" (
	call "%COMPILEDIR%..\IDE\devenv" soui.sln /build "Debug" /project "components\ScriptModule-LUA\lua-51\lua-51%EXT%" /projectconfig "Debug"
	call "%COMPILEDIR%..\IDE\devenv" soui.sln /build "Debug" /project "components\ScriptModule-LUA\ScriptModule\scriptmodule-lua%EXT%" /projectconfig "Debug"
)
call "%COMPILEDIR%..\IDE\devenv" soui.sln /build "Debug" /project "demo\demo%EXT%" /projectconfig "Debug"
call "%COMPILEDIR%..\IDE\devenv" soui.sln /build "Debug" /project "qqlogin\qqlogin%EXT%" /projectconfig "Debug"

ECHO Compile debug version finished.
ECHO Press any key to compile release version.
pause
ECHO Start compile release version...
call "%COMPILEDIR%..\IDE\devenv" soui.sln /build "Release" /project "soui-sys-resource\soui-sys-resource%EXT%" /projectconfig "Release"
call "%COMPILEDIR%..\IDE\devenv" soui.sln /build "Release" /project "utilities\utilities%EXT%" /projectconfig "Release"
call "%COMPILEDIR%..\IDE\devenv" soui.sln /build "Release" /project "soui\soui%EXT%" /projectconfig "Release"
call "%COMPILEDIR%..\IDE\devenv" soui.sln /build "Release" /project "components\translator\translator%EXT%" /projectconfig "Release"
call "%COMPILEDIR%..\IDE\devenv" soui.sln /build "Release" /project "components\imgdecoder-wic\imgdecoder-wic%EXT%" /projectconfig "Release"
call "%COMPILEDIR%..\IDE\devenv" soui.sln /build "Release" /project "components\render-gdi\render-gdi%EXT%" /projectconfig "Release"
call "%COMPILEDIR%..\IDE\devenv" soui.sln /build "Release" /project "components\zlib\zlib%EXT%" /projectconfig "Release"
call "%COMPILEDIR%..\IDE\devenv" soui.sln /build "Release" /project "components\resprovider-zip\resprovider-zip%EXT%" /projectconfig "Release"
call "%COMPILEDIR%..\IDE\devenv" soui.sln /build "Release" /project "components\freetype\freetype%EXT%" /projectconfig "Release"
call "%COMPILEDIR%..\IDE\devenv" soui.sln /build "Release" /project "components\myskia\myskia%EXT%" /projectconfig "Release"
call "%COMPILEDIR%..\IDE\devenv" soui.sln /build "Release" /project "components\render-skia\render-skia%EXT%" /projectconfig "Release"
IF %SKIPLUA% NEQ "TRUE" (
	call "%COMPILEDIR%..\IDE\devenv" soui.sln /build "Release" /project "components\ScriptModule-LUA\lua-51\lua-51%EXT%" /projectconfig "Release"
	call "%COMPILEDIR%..\IDE\devenv" soui.sln /build "Release" /project "components\ScriptModule-LUA\ScriptModule\scriptmodule-lua%EXT%" /projectconfig "Release"
)
call "%COMPILEDIR%..\IDE\devenv" soui.sln /build "Release" /project "demo\demo%EXT%" /projectconfig "Release"
call "%COMPILEDIR%..\IDE\devenv" soui.sln /build "Release" /project "qqlogin\qqlogin%EXT%" /projectconfig "Release"
ECHO Compile debug version finished.

pause
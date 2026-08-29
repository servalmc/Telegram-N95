@echo off
rem Usage: build.bat [gcce|winscw] [urel|udeb]
rem Defaults to a GCCE release build for the device.

setlocal EnableDelayedExpansion
call "%~dp0symbian-env.bat" || exit /b 1

set "PLATFORM=%~1"
if "%PLATFORM%"=="" set "PLATFORM=gcce"
set "VARIANT=%~2"
if "%VARIANT%"=="" set "VARIANT=urel"

rem Icons are built outside abld. The svgtbinencode.exe in this SDK is a WINSCW
rem binary and cannot run on Windows, so the vector route is unavailable: the
rem icon is rasterised to a bitmap pair and packed by mifconv, which puts bitmap
rem icons into an .mbm rather than the .mif.
if not exist "%~dp0..\inc\SymgramApiLocal.h" (
    copy /y "%~dp0..\inc\SymgramApiLocal.h.example" "%~dp0..\inc\SymgramApiLocal.h" >nul
    echo Created inc\SymgramApiLocal.h from the example. Paste api_id and api_hash from my.telegram.org.
)

cd /d "%~dp0..\gfx" || (echo ERROR: no gfx directory & exit /b 1)

if not exist Symgram.bmp (
    powershell -ExecutionPolicy Bypass -File "%~dp0make-icon.ps1" || exit /b 1
)

call mifconv Symgram_aif.mif /HSymgram_aif.mbg /E /c24,8 Symgram.bmp || exit /b 1
copy /y Symgram_aif.mbm "%SYMBIAN_SDK%\epoc32\data\z\resource\apps\" >nul || exit /b 1

if not exist emoji\e00.bmp (
    powershell -ExecutionPolicy Bypass -File "%~dp0make-emoji.ps1" || exit /b 1
)
set "EMOJI_ARGS="
for /L %%i in (0,1,9) do set "EMOJI_ARGS=!EMOJI_ARGS! /c24,8 emoji\e0%%i.bmp"
for /L %%i in (10,1,39) do set "EMOJI_ARGS=!EMOJI_ARGS! /c24,8 emoji\e%%i.bmp"
call mifconv Symgram_emoji.mif /HSymgram_emoji.mbg !EMOJI_ARGS! || exit /b 1
copy /y Symgram_emoji.mbm "%SYMBIAN_SDK%\epoc32\data\z\resource\apps\" >nul || exit /b 1

cd /d "%~dp0..\group" || (echo ERROR: no group directory & exit /b 1)

call bldmake bldfiles || exit /b 1
call abld build %PLATFORM% %VARIANT%

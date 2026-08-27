@echo off
rem Usage: build.bat [gcce|winscw] [urel|udeb]
rem Defaults to a GCCE release build for the device.

setlocal
call "%~dp0symbian-env.bat" || exit /b 1

set "PLATFORM=%~1"
if "%PLATFORM%"=="" set "PLATFORM=gcce"
set "VARIANT=%~2"
if "%VARIANT%"=="" set "VARIANT=urel"

cd /d "%~dp0..\group" || (echo ERROR: no group directory & exit /b 1)

call bldmake bldfiles || exit /b 1
call abld build %PLATFORM% %VARIANT%

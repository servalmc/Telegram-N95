@echo off
rem Sets up the S60 3rd Edition FP1 (Symbian 9.2) command-line build environment.
rem Must be run from cmd.exe, not PowerShell: the SDK tool launchers are resolved
rem from PATH verbatim, and PowerShell expands them back to their long paths,
rem which breaks on the spaces in "Program Files (x86)".

set "SYMBIAN_SDK=C:\Symbian\9.2\S60_3rd_FP1"

rem EPOCROOT carries no drive letter; the tools resolve it against the current
rem drive. D:\Symbian is a junction to C:\Symbian so that builds work from either.
set "EPOCROOT=\Symbian\9.2\S60_3rd_FP1\"

set "PATH=%PATH%;%SYMBIAN_SDK%\Epoc32\tools"
set "PATH=%PATH%;C:\PROGRA~2\COMMON~1\Symbian\tools"
set "PATH=%PATH%;C:\PROGRA~2\CSLARM~1\bin"
set "PATH=%PATH%;D:\Perl\bin"

if not exist "%SYMBIAN_SDK%\Epoc32\tools\bldmake.bat" (
    echo ERROR: SDK not found at %SYMBIAN_SDK%
    exit /b 1
)

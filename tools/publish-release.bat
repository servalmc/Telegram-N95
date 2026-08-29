@echo off
rem Builds (if needed) and publishes sis\Symgram.sisx as a GitHub Release.
rem Tag, title and update/latest.txt are taken from inc/SymgramVersion.h.

setlocal EnableDelayedExpansion
cd /d "%~dp0.." || exit /b 1

if not exist "sis\Symgram.sisx" (
    echo sis\Symgram.sisx is missing, building...
    call "%~dp0release.bat" || exit /b 1
)

set "VER="
for /f "tokens=3" %%i in ('findstr /C:"SYMGRAM_VERSION_STRING" inc\SymgramVersion.h') do (
    set "VER=%%~i"
)
set "VER=!VER:"=!"
if "!VER!"=="" (
    echo ERROR: could not read SYMGRAM_VERSION_STRING
    exit /b 1
)

if not exist update mkdir update
> update\latest.txt echo v!VER!
>> update\latest.txt echo https://github.com/servalmc/Telegram-N95/releases/latest

where gh >nul 2>&1
if errorlevel 1 (
    echo ERROR: GitHub CLI (gh) is not on PATH.
    echo Install it and run: gh auth login
    exit /b 1
)

echo Publishing GitHub release v!VER! ...
gh release view "v!VER!" >nul 2>&1
if not errorlevel 1 (
    echo Release v!VER! already exists, replacing the .sisx.
    gh release upload "v!VER!" "sis\Symgram.sisx" --clobber || exit /b 1
) else (
    gh release create "v!VER!" "sis\Symgram.sisx" --title "Symgram !VER!" --notes-file CHANGELOG.md --latest || exit /b 1
)

echo.
echo Done: https://github.com/servalmc/Telegram-N95/releases/tag/v!VER!
echo Commit and push update\latest.txt if it changed.

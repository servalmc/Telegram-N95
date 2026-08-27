@echo off
rem Builds a self-signed Symgram.sisx from an existing GCCE release build.
rem Generates a self-signing certificate on first run; both are gitignored.

setlocal
call "%~dp0symbian-env.bat" || exit /b 1

set "SIGN_PASS=symgram"

cd /d "%~dp0..\sis" || exit /b 1

if not exist selfsign.cer (
    echo Generating self-signing certificate...
    call makekeys -cert -password %SIGN_PASS% -len 1024 ^
        -dname "CN=Symgram OU=Development OR=Self-signed CO=FI" ^
        selfsign.key selfsign.cer || exit /b 1
)

call makesis Symgram.pkg Symgram.sis || exit /b 1
call signsis Symgram.sis Symgram.sisx selfsign.cer selfsign.key %SIGN_PASS% || exit /b 1

echo.
echo Built:
dir /b Symgram.sisx

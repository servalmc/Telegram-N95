@echo off
rem Builds a self-signed Symgram.sisx from an existing GCCE release build.
rem
rem sis\selfsign.cer and sis\selfsign.key are the upgrade identity. They live
rem in git so every GitHub build signs with the same certificate. Generating a
rem new pair would make the phone refuse an in-place upgrade (or wipe the
rem private folder, including session.bin).

setlocal
call "%~dp0symbian-env.bat" || exit /b 1

set "SIGN_PASS=symgram"

cd /d "%~dp0..\sis" || exit /b 1

if not exist selfsign.cer (
    echo ERROR: sis\selfsign.cer is missing.
    echo Restore it from git. Do not run makekeys — a new certificate cannot
    echo upgrade an already-installed Symgram and will force a re-login.
    exit /b 1
)

call makesis Symgram.pkg Symgram.sis || exit /b 1
call signsis Symgram.sis Symgram.sisx selfsign.cer selfsign.key %SIGN_PASS% || exit /b 1

echo.
echo Built:
dir /b Symgram.sisx

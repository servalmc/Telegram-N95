@echo off
rem Compiles Symgram.rss using the Symbian cpp/rcomp pair.
setlocal
call "%~dp0symbian-env.bat" || exit /b 1
D:

set "CPP=C:\PROGRA~2\COMMON~1\Symbian\tools\cpp.exe"
set "RCOMP=C:\PROGRA~2\COMMON~1\Symbian\tools\rcomp.exe"
set "INCL=-I D:\Projects\N95\Telegram-N95\data -I D:\Projects\N95\Telegram-N95\inc -I- -I D:\Symbian\9.2\S60_3rd_FP1\EPOC32\include -I D:\Symbian\9.2\S60_3rd_FP1\epoc32\include\variant"
set "TMP=\Symbian\9.2\S60_3rd_FP1\EPOC32\BUILD\Projects\N95\Telegram-N95\group\SYMGRAM\GCCE"
set "OUT=\Symbian\9.2\S60_3rd_FP1\EPOC32\DATA\Z\resource\apps"
set "SRC=\Projects\N95\Telegram-N95\data\Symgram.rss"
set "RSG=%TMP%\Symgram.rsg"
set "RPP=%TMP%\Symgram.rpp"
set "HDR=D:\Symbian\9.2\S60_3rd_FP1\EPOC32\INCLUDE\Symgram.RSG"

call :lang RSC LANGUAGE_SC
if errorlevel 1 exit /b 1
call :lang R01 LANGUAGE_01
if errorlevel 1 exit /b 1
call :lang R16 LANGUAGE_16
if errorlevel 1 exit /b 1

call :writehdr
if errorlevel 1 exit /b 1
echo Resources compiled.
exit /b 0

:lang
echo Compiling %~1 ...
"%CPP%" -undef -C -D_UNICODE -D%~2 %INCL% D:\Projects\N95\Telegram-N95\data\Symgram.rss -o D:\Symbian\9.2\S60_3rd_FP1\EPOC32\BUILD\Projects\N95\Telegram-N95\group\SYMGRAM\GCCE\Symgram.rpp
if errorlevel 1 (
    echo CPP failed for %~1
    exit /b 1
)
"%RCOMP%" -u -m045,046,047 -o%OUT%\Symgram.%~1 -h%RSG% -s%RPP% -i%SRC%
if errorlevel 1 (
    echo RCOMP failed for %~1
    exit /b 1
)
exit /b 0

:writehdr
rem Common Files rcomp does not emit -h, so the named-resource IDs are kept here
rem in the same order as data\Symgram.rss.
(
echo #define R_DEFAULT_DOCUMENT_NAME                   0x5fd70002
echo #define R_SYMGRAM_MENUBAR                         0x5fd70004
echo #define R_SYMGRAM_MENU                            0x5fd70005
echo #define R_SYMGRAM_ABOUT_TEXT                      0x5fd70006
echo #define R_SYMGRAM_STATUS_OFFLINE                  0x5fd70007
echo #define R_SYMGRAM_STATUS_UNSIGNED                 0x5fd70008
echo #define R_SYMGRAM_SIGNIN_TITLE                    0x5fd70009
echo #define R_SYMGRAM_SIGNIN_HINT                     0x5fd7000a
echo #define R_SYMGRAM_PQ_OK                           0x5fd7000b
echo #define R_SYMGRAM_CODE_TITLE                      0x5fd7000c
echo #define R_SYMGRAM_CODE_HINT                       0x5fd7000d
echo #define R_SYMGRAM_PASSWORD_PROMPT                 0x5fd7000e
echo #define R_SYMGRAM_FIELD_COUNTRY                   0x5fd7000f
echo #define R_SYMGRAM_FIELD_PHONE                     0x5fd70010
echo #define R_SYMGRAM_PASSWORD_QUERY                  0x5fd70011
echo #define R_SYMGRAM_COUNTRIES                       0x5fd70012
echo #define R_SYMGRAM_COUNTRY_QUERY                   0x5fd70013
echo #define R_SYMGRAM_EMPTY_TITLE                     0x5fd70014
echo #define R_SYMGRAM_EMPTY_DETAIL                    0x5fd70015
echo #define R_SYMGRAM_LOCALISABLE_APP_INFO            0x5fd70016
) > "%HDR%"
if errorlevel 1 (
    echo Failed to write %HDR%
    exit /b 1
)
exit /b 0

@echo off
rem Собирает релиз под устройство и упаковывает в самоподписанный .sisx одной командой.

setlocal

call "%~dp0build.bat" gcce urel
if errorlevel 1 (
    echo.
    echo СБОРКА НЕ УДАЛАСЬ
    exit /b 1
)

call "%~dp0package.bat"
if errorlevel 1 (
    echo.
    echo УПАКОВКА НЕ УДАЛАСЬ
    exit /b 1
)

echo.
echo Готово. Пакет для установки: %~dp0..\sis\Symgram.sisx

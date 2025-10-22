@echo off

echo %~dp0 | findstr /I /C:"Program Files" >nul
if %errorlevel% == 0 (
    net session >nul 2>&1
    if %errorlevel% == 0 (
        set TTSRVCFG=%~dp0tt5srv.xml
        set ADMIN_MODE=1
    ) else (
        echo Requesting administrator privileges...
        >nul 2>&1 powershell -Command "Start-Process '%~f0' -Verb RunAs" && exit /b

        set TTSRVCFG=%APPDATA%\BearWare.dk\tt5srv.xml
        set ADMIN_MODE=0
    )
) else (
    set TTSRVCFG=%~dp0tt5srv.xml
    set ADMIN_MODE=0
)

echo   --------------------------------------------------------------------------
echo                       TeamTalk 5 Console Server
echo   --------------------------------------------------------------------------
echo.
if "%ADMIN_MODE%"=="1" (
    echo Running with administrator privileges
) else (
    echo Running without administrator privileges
)
echo Config file: %TTSRVCFG%
echo.
tt5srv.exe -wizard -c %TTSRVCFG%
tt5srv.exe -nd -verbose -c %TTSRVCFG%
pause

@echo off
chcp 65001 >nul
echo   --------------------------------------------------------------------------
echo                       TeamTalk 5 Console Server
echo   --------------------------------------------------------------------------
echo.
tt5srv.exe -wizard
tt5srv.exe -nd -verbose
pause

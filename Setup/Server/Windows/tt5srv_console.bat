@echo off
set TTSRVCFG=%APPDATA%\BearWare.dk\tt5srv.xml
echo   --------------------------------------------------------------------------
echo                       TeamTalk 5 Console Server
echo   --------------------------------------------------------------------------
echo.
echo Config file: %TTSRVCFG%
echo.
tt5srv.exe -wizard -c %TTSRVCFG%
tt5srv.exe -nd -verbose -c %TTSRVCFG%
pause

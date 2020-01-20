@echo off
@set TTSRVCFG=%APPDATA%\BearWare.dk\tt5srv.xml
echo   --------------------------------------------------------------------------
echo                       TeamTalk 5 Console Server
echo   --------------------------------------------------------------------------
echo.
@echo on
tt5srv.exe -wizard -c %TTSRVCFG%
tt5srv.exe -nd -verbose -c %TTSRVCFG%
pause

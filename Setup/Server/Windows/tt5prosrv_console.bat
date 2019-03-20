@echo off
echo   --------------------------------------------------------------------------
echo                  TeamTalk 5 Professional Console Server
echo   --------------------------------------------------------------------------
echo.
echo Ensure the server's configuration file 'tt5prosrv.xml' is in a writable
echo location otherwise no changes made to the file will be saved.
echo.
@echo on
tt5prosrv.exe -wizard
tt5prosrv.exe -nd -verbose
pause

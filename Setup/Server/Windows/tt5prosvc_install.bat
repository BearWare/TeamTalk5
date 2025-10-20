@echo off
echo --------------------------------------------------
echo  Installing TeamTalk 5 Professional NT Service
echo --------------------------------------------------
echo Make sure you're member of the administrator group
echo otherwise the installation will fail.
echo.
echo.
tt5prosvc.exe -wizard
tt5prosvc.exe -i

:ask
set /p choice=Do you want to start the service now? (y/n): 
if /i "%choice%"=="y" (
    tt5prosvc.exe -s
    goto end
) else if /i "%choice%"=="n" (
    goto end
) else goto ask

:end
pause

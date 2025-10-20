@echo off
echo --------------------------------------------------
echo        Installing TeamTalk 5 NT Service
echo --------------------------------------------------
echo Make sure you're member of the administrator group
echo otherwise the installation will fail.
echo.
echo.
tt5svc.exe -wizard
tt5svc.exe -i

:ask
set /p choice=Do you want to start the service now? (y/n): 
if /i "%choice%"=="y" (
    tt5svc.exe -s
    goto end
) else if /i "%choice%"=="n" (
    goto end
) else goto ask

:end
pause

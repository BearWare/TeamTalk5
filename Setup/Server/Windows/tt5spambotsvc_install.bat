@echo off
echo --------------------------------------------------
echo  Installing TeamTalk 5 SpamBot NT Service
echo --------------------------------------------------
echo Make sure you're member of the administrator group
echo otherwise the installation will fail.
echo.
echo.
tt5spambotsvc.exe -wizard
tt5spambotsvc.exe -i
sc.exe config tt5spambotsvc depend= tt5prosvc start= auto

:ask
set /p choice=Do you want to start the service now? (y/n):
if /i "%choice%"=="y" (
    tt5spambotsvc.exe -s
    goto end
) else if /i "%choice%"=="n" (
    goto end
) else goto ask

:end
pause

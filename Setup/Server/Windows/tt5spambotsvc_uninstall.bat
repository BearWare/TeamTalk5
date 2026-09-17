@echo off
echo --------------------------------------------------
echo  Uninstalling TeamTalk 5 SpamBot NT Service
echo --------------------------------------------------
echo Make sure you're member of the administrator group
echo otherwise the installation will fail.
echo.
echo.
tt5spambotsvc.exe -e
tt5spambotsvc.exe -u
pause

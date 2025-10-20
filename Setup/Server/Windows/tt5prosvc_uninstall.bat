@echo off
echo --------------------------------------------------
echo  Uninstalling TeamTalk 5 Professional NT Service
echo --------------------------------------------------
echo Make sure you're member of the administrator group
echo otherwise the installation will fail.
echo.
echo.
tt5prosvc.exe -e
tt5prosvc.exe -u
pause

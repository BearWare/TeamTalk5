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
tt5prosvc.exe -s
pause

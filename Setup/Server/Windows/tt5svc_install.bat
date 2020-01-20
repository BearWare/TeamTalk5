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
tt5svc.exe -s
pause

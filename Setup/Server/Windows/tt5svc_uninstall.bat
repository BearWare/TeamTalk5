@echo off
echo --------------------------------------------------
echo        Uninstalling TeamTalk 5 NT Service
echo --------------------------------------------------
echo Make sure you're member of the administrator group
echo otherwise the installation will fail.
echo.
echo.
tt5svc.exe -e
tt5svc.exe -u
pause

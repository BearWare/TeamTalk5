@echo off
title TeamTalk 5 NT Service Manager
fltmc 1>nul 2>nul || (
cd /d "%~dp0"
cmd /u /c echo Set UAC = CreateObject^("Shell.Application"^) : UAC.ShellExecute "cmd.exe", "/c cd ""%~dp0"" && ""%~dpnx0""", "", "runas", 1 > "%temp%\GetAdmin.vbs"
cscript "%temp%\GetAdmin.vbs"
del /f /q "%temp%\GetAdmin.vbs" 1>nul 2>nul
exit
)
:menu
cls
echo Welcome to the TeamTalk 5 NT Service Manager.
echo Make sure you're member of the administrator group, otherwise the installation may fail.
echo 1) install service
echo 2) uninstall service
echo 3) start service
echo 4) stop service
echo 5) clean unknown files
echo 6) run wizard
echo 7) exit
set /p choice=select an option: 
if %choice%==1 goto install
if %choice%==2 goto uninstall
if %choice%==3 goto start
if %choice%==4 goto stop
if %choice%==5 goto cleanfiles
if %choice%==6 goto wizard
if %choice%==7 goto quit
goto menu
:install
echo installing...
"%~dp0tt5svc.exe" -i
pause
goto menu
:uninstall
echo uninstalling...
"%~dp0tt5svc.exe" -u
pause
goto menu
:start
echo starting...
"%~dp0tt5svc.exe" -s
pause
goto menu
:stop
echo stopping...
"%~dp0tt5svc.exe" -e
pause
goto menu
:cleanfiles
echo cleaning...
"%~dp0tt5svc.exe" -cleanfiles
pause
goto menu
:wizard
echo running...
"%~dp0tt5svc.exe" -wizard
pause
goto menu
:quit

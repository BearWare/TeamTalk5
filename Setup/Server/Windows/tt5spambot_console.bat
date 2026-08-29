@echo off
echo   --------------------------------------------------------------------------
echo                  TeamTalk 5 SpamBot Console
echo   --------------------------------------------------------------------------
echo.
echo Ensure the SpamBot's configuration file 'tt5spambot.xml' is in a writable
echo location otherwise no changes made to the file will be saved.
echo.
tt5spambot.exe -wizard
tt5spambot.exe -nd -verbose
pause

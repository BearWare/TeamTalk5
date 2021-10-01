@ECHO Installing ACE Framework
SET ACE_DIR=%CD%
CD %1
IF NOT "%ERRORLEVEL%" == "0" GOTO fail
SET INSTALL_DIR=%CD%
@ECHO Source Directory %ACE_DIR%
@ECHO Destination Directory %INSTALL_DIR%

IF EXIST %INSTALL_DIR%\include RMDIR /S /Q %INSTALL_DIR%\include
IF EXIST %INSTALL_DIR%\lib RMDIR /S /Q %INSTALL_DIR%\lib
MKDIR %INSTALL_DIR%\include\ace
IF NOT "%ERRORLEVEL%" == "0" GOTO fail
MKDIR %INSTALL_DIR%\lib
IF NOT "%ERRORLEVEL%" == "0" GOTO fail
XCOPY %ACE_DIR%\ACE\ace\*.h /S %INSTALL_DIR%\include\ace
IF NOT "%ERRORLEVEL%" == "0" GOTO fail
XCOPY %ACE_DIR%\ACE\ace\*.inl /S %INSTALL_DIR%\include\ace
IF NOT "%ERRORLEVEL%" == "0" GOTO fail
XCOPY %ACE_DIR%\ACE\ace\*.cpp /S %INSTALL_DIR%\include\ace
IF NOT "%ERRORLEVEL%" == "0" GOTO fail
XCOPY %ACE_DIR%\ACE\protocols\ace\*.h /S %INSTALL_DIR%\include\ace
IF NOT "%ERRORLEVEL%" == "0" GOTO fail
XCOPY %ACE_DIR%\ACE\protocols\ace\*.inl /S %INSTALL_DIR%\include\ace
IF NOT "%ERRORLEVEL%" == "0" GOTO fail
XCOPY %ACE_DIR%\ACE\protocols\ace\*.cpp /S %INSTALL_DIR%\include\ace
IF NOT "%ERRORLEVEL%" == "0" GOTO fail
XCOPY %ACE_DIR%\ACE\lib /S %INSTALL_DIR%\lib
IF NOT "%ERRORLEVEL%" == "0" GOTO fail

@ECHO Finished installing ACE Framework

EXIT /B 0

:fail
EXIT /B 1

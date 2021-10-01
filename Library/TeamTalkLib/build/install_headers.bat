@ECHO Installing header files
SET SOURCE_DIR=%CD%
CD %1
IF NOT "%ERRORLEVEL%" == "0" GOTO fail
SET INSTALL_DIR=%CD%
@ECHO Source Directory %SOURCE_DIR%
@ECHO Destination Directory %INSTALL_DIR%

IF EXIST %INSTALL_DIR%\include RMDIR /S /Q %INSTALL_DIR%\include
MKDIR %INSTALL_DIR%\include
IF NOT "%ERRORLEVEL%" == "0" GOTO fail
CD %SOURCE_DIR%
IF NOT "%ERRORLEVEL%" == "0" GOTO fail
XCOPY *.h /S %INSTALL_DIR%\include
IF NOT "%ERRORLEVEL%" == "0" GOTO fail

@ECHO Finished installing header files

EXIT /B 0

:fail
EXIT /B 1

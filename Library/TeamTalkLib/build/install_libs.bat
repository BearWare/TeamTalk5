@ECHO Installing library files
SET SOURCE_DIR=%CD%
SET LIB_DIR=%SOURCE_DIR%\%1
CD %2
IF NOT "%ERRORLEVEL%" == "0" GOTO fail
SET INSTALL_DIR=%CD%\%1
@ECHO Source Directory %LIB_DIR%
@ECHO Destination Directory %INSTALL_DIR%

IF EXIST %INSTALL_DIR% RMDIR /S /Q %INSTALL_DIR%
MKDIR %INSTALL_DIR%
IF NOT "%ERRORLEVEL%" == "0" GOTO fail
XCOPY %LIB_DIR% /S %INSTALL_DIR%
IF NOT "%ERRORLEVEL%" == "0" GOTO fail

@ECHO Finished installing library files

EXIT /B 0

:fail
EXIT /B 1

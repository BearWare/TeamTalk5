set TEAMTALKJNI_HOME=%CD%
set PATH=%PATH%;%TEAMTALKJNI_HOME%\..\TeamTalk_DLL;%TEAMTALKJNI_HOME%\libs;
set CLASSPATH=%TEAMTALKJNI_HOME%\libs\TeamTalk5.jar;%TEAMTALKJNI_HOME%\libs\junit4.jar;%TEAMTALKJNI_HOME%\libs\hamcrest-core.jar;

cd %TEAMTALKJNI_HOME%\test
javac.exe -source 1.7 -target 1.7 -g -cp %CLASSPATH% -sourcepath %TEAMTALKJNI_HOME%/src dk/bearware/TeamTalkStdTestCase.java dk/bearware/TeamTalkTestCase.java dk/bearware/TeamTalkTestCaseBase.java TeamTalkStdTestSuite.java SingleJUnitTestRunner.java
jar.exe cf ../libs/TeamTalk5Test.jar ./dk/bearware/*.class

@set JAVAPROP=-Djava.library.path=%TEAMTALKJNI_HOME%\libs
@set JAVAPROP=%JAVAPROP% -Ddk.bearware.serverip=
@set JAVAPROP=%JAVAPROP% -Ddk.bearware.sndinputid=
@set JAVAPROP=%JAVAPROP% -Ddk.bearware.sndoutputid=
@set JAVAPROP=%JAVAPROP% -Ddk.bearware.encrypted=0
@set JAVAPROP=%JAVAPROP% -Ddk.bearware.videodevid=
@set JAVAPROP=%JAVAPROP% -Ddk.bearware.serverip=127.0.0.1
@set JAVAPROP=%JAVAPROP% -Ddk.bearware.serverbindip=
@set JAVAPROP=%JAVAPROP% -Ddk.bearware.videofile=
@set JAVAPROP=%JAVAPROP% -Ddk.bearware.audiofile=
@set JAVAPROP=%JAVAPROP% -Ddk.bearware.httpsfile=
@set JAVAPROP=%JAVAPROP% -Ddk.bearware.opustools=
@set JAVAPROP=%JAVAPROP% -Ddk.bearware.verbose=0

java.exe -cp %CLASSPATH% %JAVAPROP% org.junit.runner.JUnitCore TeamTalkStdTestSuite

@REM :again
@REM java.exe -cp %CLASSPATH% %JAVAPROP% SingleJUnitTestRunner dk.bearware.TeamTalkStdTestCase#testVirtualSoundDevice
@REM if %ERRORLEVEL% EQU 0 goto again

cd %TEAMTALKJNI_HOME%

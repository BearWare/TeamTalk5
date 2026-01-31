set TEAMTALKJNI_HOME=%CD%
set PATH=%PATH%;%TEAMTALKJNI_HOME%\..\TeamTalk_DLL;%TEAMTALKJNI_HOME%\libs;
set CLASSPATH=%TEAMTALKJNI_HOME%\libs\junit4.jar;%TEAMTALKJNI_HOME%\libs\hamcrest-core.jar

@set JAVAPROP=-Djava.library.path=%TEAMTALKJNI_HOME%\libs
@set JAVAPROP=%JAVAPROP% -Ddk.bearware.sndinputid=%INPUTDEVICEID%
@set JAVAPROP=%JAVAPROP% -Ddk.bearware.sndoutputid=%OUTPUTDEVICEID%
@set JAVAPROP=%JAVAPROP% -Ddk.bearware.encrypted=%ENCRYPTED%
@set JAVAPROP=%JAVAPROP% -Ddk.bearware.videodevid=%VIDEODEVICEID%
@set JAVAPROP=%JAVAPROP% -Ddk.bearware.serverip=%SERVERIP%
@set JAVAPROP=%JAVAPROP% -Ddk.bearware.serverbindip=%SERVERBINDIP%
@set JAVAPROP=%JAVAPROP% -Ddk.bearware.videofile=%VIDEOFILE%
@set JAVAPROP=%JAVAPROP% -Ddk.bearware.audiofile=%AUDIOFILE%
@set JAVAPROP=%JAVAPROP% -Ddk.bearware.httpsfile=%HTTPSFILE%
@set JAVAPROP=%JAVAPROP% -Ddk.bearware.opustools=%OPUSTOOLS%
@set JAVAPROP=%JAVAPROP% -Ddk.bearware.verbose=%VERBOSE%
@set JAVAPROP=%JAVAPROP% -Ddk.bearware.speexdsp=%SPEEXDSP%
@set JAVAPROP=%JAVAPROP% -Ddk.bearware.webrtc=%WEBRTC%
@set JAVAPROP=%JAVAPROP% -Ddk.bearware.githubskip=%GITHUBSKIP%
@set JAVAPROP=%JAVAPROP% -Ddk.bearware.debug=%GITHUBDEBUG%

@REM :again
@REM java.exe -cp %CLASSPATH% %JAVAPROP% SingleJUnitTestRunner dk.bearware.TeamTalkStdTestCase#testVirtualSoundDevice
@REM if %ERRORLEVEL% EQU 0 goto again

REM %1 can be TeamTalkStdTestSuite TeamTalkProTestSuite TeamTalkServerTestSuite
if %1 EQU TeamTalkStdTestSuite set CLASSPATH=%CLASSPATH%;%TEAMTALKJNI_HOME%\libs\TeamTalk5.jar;%TEAMTALKJNI_HOME%\libs\TeamTalk5Test.jar
if %1 EQU TeamTalkProTestSuite set CLASSPATH=%CLASSPATH%;%TEAMTALKJNI_HOME%\libs\TeamTalk5Pro.jar;%TEAMTALKJNI_HOME%\libs\TeamTalk5ProTest.jar
if %1 EQU TeamTalkServerTestSuite set CLASSPATH=%CLASSPATH%;%TEAMTALKJNI_HOME%\libs\TeamTalk5Pro.jar;%TEAMTALKJNI_HOME%\libs\TeamTalk5SrvTest.jar

cd test
java.exe -cp %CLASSPATH% %JAVAPROP% org.junit.runner.JUnitCore %1

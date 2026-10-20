set TEAMTALKJNI_HOME=%CD%
set PATH=%PATH%;%TEAMTALKJNI_HOME%\..\TeamTalk_DLL;%TEAMTALKJNI_HOME%\libs;
set JUNIT_JAR=%TEAMTALKJNI_HOME%\libs\junit-platform-console-standalone.jar

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

REM %1 can be TeamTalkStdTestSuite TeamTalkProTestSuite TeamTalkServerTestSuite
if %1 EQU TeamTalkStdTestSuite set CLASSPATH=%TEAMTALKJNI_HOME%\libs\TeamTalk5.jar;%TEAMTALKJNI_HOME%\libs\TeamTalk5Test.jar
if %1 EQU TeamTalkProTestSuite set CLASSPATH=%TEAMTALKJNI_HOME%\libs\TeamTalk5Pro.jar;%TEAMTALKJNI_HOME%\libs\TeamTalk5ProTest.jar
if %1 EQU TeamTalkServerTestSuite set CLASSPATH=%TEAMTALKJNI_HOME%\libs\TeamTalk5Pro.jar;%TEAMTALKJNI_HOME%\libs\TeamTalk5SrvTest.jar

cd test
java.exe %JAVAPROP% -jar %JUNIT_JAR% --class-path %CLASSPATH% --select-class %1

@REM java.exe %JAVAPROP% -jar %JUNIT_JAR% --class-path %CLASSPATH% --select-method %1#%2

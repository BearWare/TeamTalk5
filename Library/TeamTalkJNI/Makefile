TEAMTALK_ROOT=$(PWD)/../..

#locations of 'hamcrest-core' and 'junit'
HAMCRESTCORE_JAR ?= $(PWD)/hamcrest-core-1.3.jar
JUNIT_JAR ?= $(PWD)/junit-4.11.jar

ifeq ($(shell uname),Darwin)
SHARED_EXT := dylib
LIBPATHNAME=DYLD_LIBRARY_PATH
else
SHARED_EXT := so
LIBPATHNAME=LD_LIBRARY_PATH
endif

CLASSPATH ?= .:$(HAMCRESTCORE_JAR):$(JUNIT_JAR):$(PWD)/libs/TeamTalk5.jar

# DYLD_LIBRARY_PATH has been removed in Mac OS Sierra, so use
# -Djava.library.path instead
LIBPATH=$(TEAMTALK_ROOT)/Library/TeamTalk_DLL:$(TEAMTALK_ROOT)/Library/TeamTalkJNI/libs

SRCTARGET=-source 1.6 -target 1.6

# Override default settings in dk.bearware.TeamTalkTestCaseBase
JAVAPROP=-Djava.library.path=$(LIBPATH)
JAVAPROP+=-Ddk.bearware.sndinputid=$(INPUTDEVICEID)
JAVAPROP+=-Ddk.bearware.sndoutputid=$(OUTPUTDEVICEID)
JAVAPROP+=-Ddk.bearware.proedition=$(PROEDITION)
JAVAPROP+=-Ddk.bearware.encrypted=$(ENCRYPTED) 
JAVAPROP+=-Ddk.bearware.videodevid=$(VIDEODEVICEID) 
JAVAPROP+=-Ddk.bearware.serverip=$(SERVERIP)
JAVAPROP+=-Ddk.bearware.serverbindip=$(SERVERBINDIP)

help:
	@echo "Targets:"
	@echo "  macnative"
	@echo "    - Setup libTeamTalk5-jni.$(SHARED_EXT) JNI DLL for TeamTalk5 DLL,"
	@echo "      i.e. run install_name_tool to set dependency path (otool -L)"
	@echo "  macnativepro"
	@echo "    - Setup libTeamTalk5Pro-jni.$(SHARED_EXT) JNI DLL for TeamTalk5Pro DLL,"
	@echo "      i.e. run install_name_tool to set dependency path (otool -L)"
	@echo "  java"
	@echo "    - Build TeamTalk5.jar. Java wrapper classes for JNI DLLs"
	@echo "  clean"
	@echo "    - Remove all intermediate files"
	@echo "  cleanjava"
	@echo "    - Remove all Java generated classes and JAR files"
	@echo "  junit"
	@echo "    - Build TeamTalk5Test.jar"
	@echo "  junitsrv"
	@echo "    - Build TeamTalk5ProTest.jar"
	@echo "  runjunit"
	@echo "    - Build TeamTalk5Test.jar and run JUnit test-cases"
	@echo "  runjunitsrv"
	@echo "    - Build TeamTalk5SrvTest.jar and run JUnit test-cases"
	@echo "  testcase"
	@echo "    - Build and run a single test-case in TeamTalkTestCase-class"
	@echo "  E.g make TARGET=test_04_Auth"
	@echo "  testcasesrv"
	@echo "    - Build and run a single test-case in TeamTalkServerTestCase-class"
	@echo "  E.g make TARGET=test_04_Auth"
	@echo ""
	@echo "When running an application which uses the TeamTalk 5 SDK then"
	@echo "ensure $(LIBPATHNAME) is set up with the path to both "
	@echo "TeamTalk 5 DLL dependencies."
	@echo ""
	@echo "For TeamTalk 5 SDK Standard Edition i.e. libTeamTalk5.$(SHARED_EXT)"
	@echo "and libTeamTalk5-jni.$(SHARED_EXT)"
	@echo "For TeamTalk 5 SDK Professional Edition i.e. libTeamTalk5Pro.$(SHARED_EXT)"
	@echo "and libTeamTalk5Pro-jni.$(SHARED_EXT)"
	@echo ""
	@echo "Example:"
	@echo "$$ export $(LIBPATHNAME)=SDK_ROOT/Library/TeamTalk_DLL:SDK_ROOT/Library/TeamTalkJNI/libs"
	@echo ""
	@echo "Useful environment variables:"
	@echo "  INPUTDEVICEID"
	@echo "    1978 = virtual sound device, -1 = default audio device. Default is -1."
	@echo "  OUTPUTDEVICEID"
	@echo "    1978 = virtual sound device, -1 = default audio device. Default is -1."
	@echo "  PROEDITION"
	@echo "    0 = Standard Edition (TeamTalk5-jni), 1 = Professional Edition (TeamTalk5Pro-jni). Default is 0."
	@echo "  ENCRYPTED"
	@echo "    0 = Non-encrypted, 1 = Encrypted. Default is 0."
	@echo "  VIDEODEVICEID"
	@echo "    None = Disable video capure unit tests. Default is None."
	@echo "  SERVERIP"
	@echo "    Client should connect to this hostname/IP-address. Default is 127.0.0.1"
	@echo "  SERVERBINDIP"
	@echo "    Server should bind to this IP-address. Unspecified means same as SERVERIP"

macnative:
	install_name_tool -change @rpath/libTeamTalk5.dylib $(TEAMTALK_ROOT)/Library/TeamTalk_DLL/libTeamTalk5.dylib libs/libTeamTalk5-jni.dylib

macnativepro:
	install_name_tool -change @rpath/libTeamTalk5Pro.dylib $(TEAMTALK_ROOT)/Library/TeamTalk_DLL/libTeamTalk5Pro.dylib libs/libTeamTalk5Pro-jni.dylib

java:
	cd src && javac $(SRCTARGET) -g ./dk/bearware/*.java ./dk/bearware/events/*.java && jar cf ../libs/TeamTalk5.jar ./dk/bearware/*.class ./dk/bearware/events/*.class

clean: cleanjava

cleanjava:
	cd src && find . -name \*.class -exec rm {} \;
	cd test && find . -name \*.class -exec rm {} \;
	rm -f libs/TeamTalk5.jar libs/TeamTalk5Test.jar

junit: java
	cd test && javac $(SRCTARGET) -g -cp "$(CLASSPATH)" -sourcepath ../src dk/bearware/TeamTalkTestCase.java dk/bearware/TeamTalkTestCaseBase.java TeamTalkTestSuite.java SingleJUnitTestRunner.java
	cd test && jar cf ../libs/TeamTalk5Test.jar ./dk/bearware/*.class

junitsrv: java
	cd test && javac $(SRCTARGET) -cp "$(CLASSPATH)" -sourcepath ../src dk/bearware/TeamTalkTestCaseBase.java dk/bearware/TeamTalkServerTestCase.java TeamTalkServerTestSuite.java SingleJUnitTestRunner.java
	cd test && jar cf ../libs/TeamTalk5SrvTest.jar ./dk/bearware/*.class

runjunit:  junit
	cd test && java -cp "$(CLASSPATH)" $(JAVAPROP) org.junit.runner.JUnitCore TeamTalkTestSuite

runjunitsrv:  junitsrv
	cd test && java -cp "$(CLASSPATH)" $(JAVAPROP) org.junit.runner.JUnitCore TeamTalkServerTestSuite

testcase: junit
	cd test && java -cp "$(CLASSPATH)" $(JAVAPROP) SingleJUnitTestRunner dk.bearware.TeamTalkTestCase#$(TARGET)

testcasesrv: junitsrv
	cd test && java -cp "$(CLASSPATH)" $(JAVAPROP) SingleJUnitTestRunner dk.bearware.TeamTalkServerTestCase#$(TARGET)

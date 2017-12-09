TEAMTALK_ROOT=$(PWD)/..
INCLUDE := -I$(TEAMTALK_ROOT)/TeamTalk_DLL
INCLUDE += -I/usr/lib/jvm/java-7-openjdk/include #Mac OS
INCLUDE += -I/usr/lib/jvm/java-7-openjdk-armhf/include #Raspberry
INCLUDE += -I/usr/lib/jvm/java-7-openjdk-i386/include #Debian i386
INCLUDE += -I/usr/lib/jvm/java-7-openjdk-amd64/include #Debian amd64
INCLUDE += -I/usr/lib/jvm/java-8-openjdk-i386/include #Debian 9, i386
INCLUDE += -I/usr/lib/jvm/java-8-openjdk-i386/include/linux # jni_md.h
INCLUDE += -I/usr/lib/jvm/java-8-openjdk-amd64/include #Debian 9, amd64
INCLUDE += -I/System/Library/Frameworks/JavaVM.framework/Versions/A/Headers #Mac OS
INCLUDE += -I/usr/lib/java/include # Slackware-14.1

TT5DLIBS := -L$(TEAMTALK_ROOT)/TeamTalk_DLL -lTeamTalk5
TT5PRODLIBS := -L$(TEAMTALK_ROOT)/TeamTalk_DLL -lTeamTalk5Pro

#locations of 'hamcrest-core' and 'junit'
HAMCRESTCORE_JAR ?= $(PWD)/hamcrest-core-1.3.jar
JUNIT_JAR ?= $(PWD)/junit-4.11.jar

CPPFLAGS := -fPIC

ifeq ($(shell uname),Darwin)
SHARED_EXT := dylib
LIBPATHNAME=DYLD_LIBRARY_PATH
else
SHARED_EXT := so
LIBPATHNAME=LD_LIBRARY_PATH
endif

CLASSPATH ?= .:$(HAMCRESTCORE_JAR):$(JUNIT_JAR):$(PWD)/libs/TeamTalk5.jar

TT5DLL := $(PWD)/libs/libTeamTalk5-jni.$(SHARED_EXT)
TT5PRODLL := $(PWD)/libs/libTeamTalk5Pro-jni.$(SHARED_EXT)

# DYLD_LIBRARY_PATH has been removed in Mac OS Sierra, so use
# -Djava.library.path instead
LIBPATH=$(TEAMTALK_ROOT)/TeamTalk_DLL:$(TEAMTALK_ROOT)/TeamTalkJNI/libs

SRCTARGET=-source 1.5 -target 1.5

# Override default settings in dk.bearware.TeamTalkTestCaseBase
JAVAPROP=-Djava.library.path=$(LIBPATH)
JAVAPROP+=-Ddk.bearware.sndinputid=$(INPUTDEVICEID)
JAVAPROP+=-Ddk.bearware.sndoutputid=$(OUTPUTDEVICEID)
JAVAPROP+=-Ddk.bearware.proedition=$(PROEDITION)
JAVAPROP+=-Ddk.bearware.encrypted=$(ENCRYPTED) 
JAVAPROP+=-Ddk.bearware.videodevid=$(VIDEODEVID) 

help:
	@echo "Targets:"
	@echo "  native"
	@echo "    - Build libTeamTalk5-jni.$(SHARED_EXT) JNI DLL for TeamTalk5 DLL"
	@echo "  macnative"
	@echo "    - Build libTeamTalk5-jni.$(SHARED_EXT) JNI DLL for TeamTalk5 DLL"
	@echo "      and run install_name_tool to set dependency path (otool -L)"
	@echo "  nativepro"
	@echo "    - Build libTeamTalk5Pro-jni.$(SHARED_EXT) JNI DLL for TeamTalk5Pro DLL"
	@echo "  macnativepro"
	@echo "    - Build libTeamTalk5Pro-jni.$(SHARED_EXT) JNI DLL for TeamTalk5Pro DLL"
	@echo "      and run install_name_tool to set depencency path (otool -L)"
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
	@echo "$$ export $(LIBPATHNAME)=SDK_ROOT/TeamTalk_DLL:SDK_ROOT/TeamTalkJNI/libs"
	@echo ""
	@echo "Useful environment variables:"
	@echo "  INPUTDEVICEID"
	@echo "    1978 = virtual sound device, 0 = default audio device."
	@echo "  OUTPUTDEVICEID"
	@echo "    1978 = virtual sound device, 0 = default audio device."
	@echo "  PROEDITION"
	@echo "    0 = Standard Edition (TeamTalk5-jni), 1 = Professional Edition (TeamTalk5Pro-jni)"
	@echo "  ENCRYPTED"
	@echo "    0 = Non-encrypted, 1 = Encrypted"
	@echo "  VIDEODEVID"
	@echo "    None = Disable video capure unit tests"

native:
	cd jni && g++ -O0 -g $(INCLUDE) $(CPPFLAGS) $(TT5DLIBS) -shared teamtalk-jni.cpp ttconvert-jni.cpp -o $(TT5DLL) -Wno-unused-value

nativepro:
	cd jni && g++ -O0 -g $(INCLUDE) $(CPPFLAGS) $(TT5PRODLIBS) -shared teamtalk-jni.cpp teamtalksrv-jni.cpp ttconvert-jni.cpp -o $(TT5PRODLL) -Wno-unused-value

macnative: native
	install_name_tool -change libTeamTalk5.dylib $(TEAMTALK_ROOT)/TeamTalk_DLL/libTeamTalk5.dylib $(TT5DLL)

macnativepro: nativepro
	install_name_tool -change libTeamTalk5Pro.dylib $(TEAMTALK_ROOT)/TeamTalk_DLL/libTeamTalk5Pro.dylib $(TT5PRODLL)

java:
	cd src && javac $(SRCTARGET) -g ./dk/bearware/*.java ./dk/bearware/events/*.java && jar cf ../libs/TeamTalk5.jar ./dk/bearware/*.class ./dk/bearware/events/*.class

clean: cleanjava
	cd jni && rm -f $(TT5DLL) $(TT5PRODLL)

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

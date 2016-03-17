TEAMTALK_ROOT=$(PWD)/..
INCLUDE := -I$(TEAMTALK_ROOT)/TeamTalk_DLL
INCLUDE += -I/usr/lib/jvm/java-7-openjdk/include #Mac OS
INCLUDE += -I/usr/lib/jvm/java-7-openjdk-armhf/include #Raspberry
INCLUDE += -I/usr/lib/jvm/java-7-openjdk-i386/include #Debian i386
INCLUDE += -I/usr/lib/jvm/java-7-openjdk-amd64/include #Debian amd64
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

help:
	@echo "Targets:"
	@echo "  native"
	@echo "    - Build JNI DLL for TeamTalk5 DLL"
	@echo "  nativepro"
	@echo "    - Build JNI DLL for TeamTalk5Pro DLL"
	@echo "  java"
	@echo "    - Build TeamTalk5.jar. Java wrapper classes for JNI DLLs"
	@echo "  clean"
	@echo "    - Remove all intermediate files"
	@echo "  cleanjava"
	@echo "    - Remove all Java generated classes and JAR files"
	@echo "  junit"
	@echo "    - Build and run JUnit test-cases"
	@echo "  testcase"
	@echo "    - Build and run a single test-case in TeamTalkTestCase-class"
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


native:
	cd jni && g++ -O0 -g $(INCLUDE) $(CPPFLAGS) $(TT5DLIBS) -shared teamtalk-jni.cpp ttconvert-jni.cpp -o $(TT5DLL) -Wno-unused-value

nativepro:
	cd jni && g++ -O0 -g $(INCLUDE) $(CPPFLAGS) $(TT5PRODLIBS) -shared teamtalk-jni.cpp teamtalksrv-jni.cpp ttconvert-jni.cpp -o $(TT5PRODLL) -Wno-unused-value

java:
	cd src && mkdir -p build && javac -g:vars -d ./build ./dk/bearware/*.java ./dk/bearware/events/*.java && cd ./build && jar cf ../../libs/TeamTalk5.jar ./dk/bearware/*.class ./dk/bearware/events/*.class

clean: cleanjava
	cd jni && rm -f $(TT5DLL) $(TT5PRODLL)

cleanjava:
	cd src && find . -name \*.class -exec rm {} \;
	cd test && find . -name \*.class -exec rm {} \;
	cd src && rm -rf build

junitcompile: java
	cd test && javac -cp "$(CLASSPATH)" -sourcepath ../src dk/bearware/TeamTalkTestCase.java dk/bearware/TeamTalkTestCaseBase.java TeamTalkTestSuite.java SingleJUnitTestRunner.java

junitsrvcompile: java
	cd test && javac -cp "$(CLASSPATH)" -sourcepath ../src dk/bearware/TeamTalkTestCaseBase.java dk/bearware/TeamTalkServerTestCase.java TeamTalkServerTestSuite.java SingleJUnitTestRunner.java

junit:  junitcompile
	cd test && java -cp "$(CLASSPATH)" org.junit.runner.JUnitCore TeamTalkTestSuite

junitsrv:  junitsrvcompile
	cd test && java -cp "$(CLASSPATH)" org.junit.runner.JUnitCore TeamTalkServerTestSuite

testcase: junitcompile
	cd test && java -cp "$(CLASSPATH)" SingleJUnitTestRunner dk.bearware.TeamTalkTestCase#$(TARGET)

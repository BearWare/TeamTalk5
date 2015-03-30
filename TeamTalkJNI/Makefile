TEAMTALK_ROOT=$(PWD)/..
INCLUDE := -I$(TEAMTALK_ROOT)/TeamTalk_DLL
INCLUDE += -I/usr/lib/jvm/java-7-openjdk/include #Mac OS
INCLUDE += -I/usr/lib/jvm/java-7-openjdk-armhf/include #Raspberry
INCLUDE += -I/System/Library/Frameworks/JavaVM.framework/Versions/A/Headers #Mac OS
INCLUDE += -I/usr/lib/java/include # Slackware-14.1


TT5DLIBS := -L$(TEAMTALK_ROOT)/TeamTalk_DLL -lTeamTalk5
TT5PRODLIBS := -L$(TEAMTALK_ROOT)/TeamTalk_DLL -lTeamTalk5Pro

JARLIB := #location of 'hamcrest-core' and 'junit'

CPPFLAGS := -fPIC
ARCH=Linux

ifdef DYLD_LIBRARY_PATH
TT5DLL := libTeamTalk5-jni.dylib
TT5PRODLL := libTeamTalk5Pro-jni.dylib
else
TT5DLL := libTeamTalk5-jni.so
TT5PRODLL := libTeamTalk5Pro-jni.so
endif

all: native nativepro java

native:
	cd jni && g++ $(INCLUDE) $(CPPFLAGS) $(TT5DLIBS) -shared teamtalk-jni.cpp ttconvert-jni.cpp -o $(TT5DLL) -Wno-unused-value

nativepro:
	cd jni && g++ $(INCLUDE) $(CPPFLAGS) $(TT5PRODLIBS) -shared teamtalk-jni.cpp ttconvert-jni.cpp -o $(TT5PRODLL) -Wno-unused-value

java:
	cd src && mkdir ./build && javac -d ./build ./dk/bearware/*.java ./dk/bearware/events/*.java && cd ./build && jar cf ./TeamTalk5.jar ./dk/bearware/*.class ./dk/bearware/events/*.class

clean: cleanjava
	cd jni && rm $(TT5DLL) $(TT5PRODLL)

cleanjava:
	cd src && find . -name \*.class -exec rm {} \;
	cd test && find . -name \*.class -exec rm {} \;
	cd src && rm -r build

junitcompile: native
	cd test && javac -cp .:$(JARLIB)/hamcrest-core-1.3.jar:$(JARLIB)/junit-4.11.jar -sourcepath ../src dk/bearware/TeamTalkTestCase.java dk/bearware/TeamTalkTestCaseBase.java TeamTalkTestSuite.java SingleJUnitTestRunner.java

junit:  junitcompile
	cd test && java -cp .:../src:$(JARLIB)/hamcrest-core-1.3.jar:$(JARLIB)/junit-4.11.jar org.junit.runner.JUnitCore TeamTalkTestSuite

testcase: junitcompile
	cd test && java -cp .:../src:$(JARLIB)/hamcrest-core-1.3.jar:$(JARLIB)/junit-4.11.jar SingleJUnitTestRunner dk.bearware.TeamTalkTestCase#$(TARGET)

android:
	cd jni && $(NDK)/ndk-build


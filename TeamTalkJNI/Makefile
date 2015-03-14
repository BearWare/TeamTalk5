INCLUDE := -I../TeamTalk_DLL
INCLUDE += -I/usr/lib/jvm/java-7-openjdk/include #Mac OS
INCLUDE += -I/usr/lib/jvm/java-7-openjdk-armhf/include #Raspberry
INCLUDE += -I/System/Library/Frameworks/JavaVM.framework/Versions/A/Headers #Mac OS

TT5DLIBS := -L../TeamTalk_DLL -lTeamTalk5
TT5PRODLIBS := -L../TeamTalk_DLL -lTeamTalk5Pro

JARLIB := #location of 'hamcrest-core' and 'junit'

CPPFLAGS := -fPIC
ARCH=Linux

ifdef LD_LIBRARY_PATH
TT5DLL := libTeamTalk5-jni.so
TT5PRODLL := libTeamTalk5Pro-jni.so
else
TT5DLL := libTeamTalk5-jni.dylib
TT5PRODLL := libTeamTalk5Pro-jni.dylib
endif

all: native java

native:
	cd jni && g++ $(INCLUDE) $(CPPFLAGS) $(TT5DLIBS) -shared teamtalk-jni.cpp ttconvert-jni.cpp -o $(TT5DLL) -Wno-unused-value

nativepro:
	cd jni && g++ $(INCLUDE) $(CPPFLAGS) $(TT5DLIBS) -shared teamtalk-jni.cpp ttconvert-jni.cpp -o $(TT5PRODLL) -Wno-unused-value

clean: cleanjava
	cd jni && rm $(TT5DLL) $(TT5PRODLL)

cleanjava:
	cd src && find . -name \*.class -exec rm {} \;
	cd test && find . -name \*.class -exec rm {} \;

junit:  native
	cd test && javac -cp .:$(JARLIB)/hamcrest-core-1.3.jar:$(JARLIB)/junit-4.11.jar -sourcepath ../src dk/bearware/TeamTalkTestCase.java dk/bearware/TeamTalkTestCaseBase.java TeamTalkTestSuite.java
	cd test && java -cp .:../src:$(JARLIB)/hamcrest-core-1.3.jar:$(JARLIB)/junit-4.11.jar org.junit.runner.JUnitCore TeamTalkTestSuite

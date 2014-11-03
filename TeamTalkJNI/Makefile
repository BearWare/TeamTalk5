INCLUDE := -I../TeamTalk_DLL
INCLUDE += -I/usr/lib/jvm/java-7-openjdk/include #Mac OS
INCLUDE += -I/usr/lib/jvm/java-7-openjdk-armhf/include #Raspberry
INCLUDE += -I/System/Library/Frameworks/JavaVM.framework/Versions/A/Headers #Mac OS

DLIBS := -L../TeamTalk_DLL -lTeamTalk5

JARLIB := #location of 'hamcrest-core' and 'junit'

CPPFLAGS := -fPIC
ARCH=Linux

ifdef LD_LIBRARY_PATH
OUTPUTFILE := libTeamTalk5-jni.so
else
OUTPUTFILE := libTeamTalk5-jni.dylib
endif

all: native java

native:
	cd jni && g++ $(INCLUDE) $(CPPFLAGS) $(DLIBS) -shared teamtalk-jni.cpp ttconvert-jni.cpp -o $(OUTPUTFILE) -Wno-unused-value

clean: cleanjava
	cd jni && rm $(OUTPUTFILE)

cleanjava:
	cd src && find . -name \*.class -exec rm {} \;
	cd test && find . -name \*.class -exec rm {} \;

junit:  native
	cd test && javac -cp .:$(JARLIB)/hamcrest-core-1.3.jar:$(JARLIB)/junit-4.11.jar -sourcepath ../src dk/bearware/TeamTalkTestCase.java dk/bearware/TeamTalkTestCaseBase.java TeamTalkTestSuite.java
	cd test && java -cp .:../src:$(JARLIB)/hamcrest-core-1.3.jar:$(JARLIB)/junit-4.11.jar org.junit.runner.JUnitCore TeamTalkTestSuite

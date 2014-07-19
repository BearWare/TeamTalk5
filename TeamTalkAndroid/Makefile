INCLUDE := -I$(TEAMTALK_ROOT)/TeamTalk_DLL
INCLUDE += -I/usr/lib/jvm/java-7-openjdk/include #Mac OS
INCLUDE += -I/usr/lib/jvm/java-7-openjdk-armhf/include #Raspberry
INCLUDE += -I/System/Library/Frameworks/JavaVM.framework/Versions/A/Headers
DLIBS := -L$(TEAMTALK_ROOT)/TeamTalk_DLL -lTeamTalk5d

CPPFLAGS := -fPIC
ARCH=Linux

ifdef LD_LIBRARY_PATH
OUTPUTFILE := libteamtalk5-jni.so
else
OUTPUTFILE := libteamtalk5-jni.dylib
endif

all: native java

native:
	cd jni && g++ $(INCLUDE) $(CPPFLAGS) $(DLIBS) -shared teamtalk5-jni.cpp ttconvert-jni.cpp -o $(OUTPUTFILE)

clean: cleanjava
	cd jni && rm $(OUTPUTFILE)

cleanjava:
	cd src && find . -name \*.class -exec rm {} \;

java:	cleanjava
	cd src && javac dk/bearware/TeamTalk5Test.java

run:
	cd src && java dk.bearware.TeamTalk5Test

android:
	cd jni && ~/android/android-ndk-r8d/ndk-build

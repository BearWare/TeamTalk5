# TeamTalk 5 JNI shared libraries

This folder is the location for the TeamTalk 5 shared library
containing the Java Native Interface (JNI).

On Android the JNI shared library is placed in an architecture
specific folder, i.e. armeabi-v7a, arm64-v8a, x86 and x86_64.

The Java classes in the TeamTalk5.jar or TeamTalk5Pro.jar file calls
the functions in the TeamTalk 5 shared library.

The Android application (`Client/TeamTalkAndroid`) references JAR
files and JNI shared libraries directly from this folder.

# Java Native Interface for TeamTalk DLL

C++ source code for JNI calls in *jni* sub-directory. Java wrapper
classes in *src* sub-directory.

Use CMake to generate project files and/or build the TeamTalk JNI DLL
(shared library) and TeamTalk JAR files.

CMake build option for TeamTalk JNI DLL:

* `BUILD_TEAMTALK_JNI`
  * JNI library for TeamTalk Standard Edition
* `BUILD_TEAMTALK_PROJNI`
  * JNI library for TeamTalk Professional Edition
* `BUILD_TEAMTALK_JAR`
  * TeamTalk5.jar Java classes for libTeamTalk5-jni.so/TeamTalk5-jni.dll  
* `BUILD_TEAMTALK_PROJAR`
  * TeamTalk5Pro.jar Java classes for libTeamTalk5Pro-jni.so/TeamTalk5Pro-jni.dll  

## Makefile for running JUnit tests

Type `make` and you'll see the different test options.

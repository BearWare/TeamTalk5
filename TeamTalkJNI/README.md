# Java Native Interface for TeamTalk DLL

C++ source code for JNI calls in jni sub-directory.
Java wrapper classes in src sub-directory.

## Makefile build procedure

Just type ```make``` and you'll see the different build options.

## Visual Studio build procedure

Edit TeamTalk5JNI.vcxproj or TeamTalk5ProJNI.vcxproj in a text editor and replace
```$(JDK_ROOT)``` with the installation folder of the Java SDK (JDK). For instance:
```c:\Program Files (x86)\Java\jdk1.8.0_45```.

The project files contains both win32 and x64 configuration. Remember
to have the TeamTalk 5 import library in ../TeamTalk_DLL

# TeamTalk 5 Java Client

Build Java client application by downloading either TeamTalk 5
Standard Edition or TeamTalk 5 Professional Edition from
[www.bearware.dk](http://www.bearware.dk)

Library dependencies:

* **TeamTalk_DLL**
  * libTeamTalk5.so or libTeamTalk5Pro.so
    * Linux
    * SDK path `Library/C-API/TeamTalk_DLL`
    * Put in environment variable `LD_LIBRARY_PATH`
  * libTeamTalk5.dylib or libTeamTalk5Pro.dylib
    * Mac OS X
    * SDK path `Library/C-API/TeamTalk_DLL`
    * Put in environment variable `DYLD_LIBRARY_PATH`
  * TeamTalk5.dll or TeamTalk5Pro.dll
    * Windows
    * SDK path `Library/C-API/TeamTalk_DLL`
    * Put in environment variable `PATH`
* **TeamTalkJNI**
  * libTeamTalk5-jni.so or libTeamTalk5Pro-jni.so
    * Linux
    * SDK path `Library/Java/TeamTalkJNI`
    * Put in environment variable `LD_LIBRARY_PATH`
  * libTeamTalk5-jni.dylib or libTeamTalk5Pro-jni.dylib
    * Mac OS X
    * SDK path `Library/Java/TeamTalkJNI`
    * Put in environment variable `DYLD_LIBRARY_PATH`
  * TeamTalk5-jni.dll or TeamTalk5Pro-jni.dll
    * Windows
    * SDK path `Library/Java/TeamTalkJNI`
    * Put in environment variable `PATH`
